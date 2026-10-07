# Scope: UDS (ISO 14229) as a first-class protocol

Status: **scoping, not started.** Written 2026-10-07 from a survey of the tree at
`ad07f7841`. Nothing below is implemented; the "what exists" section is verified
against the code, the estimates are not.

## The question this document answers

"First class next to CANopen and J1939" has a precise meaning in this tree: a value
of `C_OscCanProtocol::E_Type`, a tab in the bus editor, a COMM datapool per node,
messages and signals with protocol-specific fields, XML persistence, DBC/catalog
import, code export, and CAN Monitor interpretation. CANopen and J1939 are both
**cyclic, signal-in-frame** protocols and the whole model is shaped around that:
a message is a CAN ID with a DLC and a transmission method, signals sit at bit
positions inside it.

UDS is not that. It is a **request/response** protocol over a transport (ISO 15765-2
on CAN, DoIP on Ethernet). There are no cyclic messages and no CAN-ID-per-message;
there are *services*, and the two that carry user data, ReadDataByIdentifier (0x22)
and WriteDataByIdentifier (0x2E), carry it as a **DID payload whose layout is a
signal layout**. That is the bridge: a UDS "message" in openSYDE's model is a DID,
not a CAN frame.

So the scope splits into three layers that can ship independently, and the first
decision is which of them "first class" means:

| Layer | What the user gets | Depends on |
|---|---|---|
| **A. Data model + bus editor** | A UDS tab where a node's DIDs are described as messages with signals; persisted in the project and in `device.syd`; CAN Monitor decodes UDS responses into named signals | nothing |
| **B. Runtime client** | openSYDE talks UDS to a third-party ECU: read/write DIDs, sessions, security access, routines, DTCs, TesterPresent | ISO-TP conformance fixes in the CAN transport |
| **C. Consumers** | Dashboards read DIDs live; SYDEflash and the CLI flash tool program a UDS-only ECU; CAN Monitor's generator sends UDS requests built from the DID table | A and B |

Recommended order: **B before A.** The data model is the larger diff but the
smaller risk; the transport layer is where the unknowns are, and a DID table nobody
can exercise against hardware is a schema, not a feature. Layer 0 below is
independent of the decision and worth doing regardless.

## What already exists (verified 2026-10-07)

The openSYDE protocol **is** UDS with supplier-specific extensions, so a great deal
of the runtime is already here. The surprise is how much of the CAN Monitor side
exists too, and in what state.

### Core protocol driver — openSYDE-flavoured UDS

`protocol_drivers/C_OscProtocolDriverOsy` (6,500 lines) implements, with standard
framing and NRC handling:

- Standard services: 0x10, 0x11, 0x22, 0x23, 0x27, 0x2E, 0x31, 0x34, 0x36, 0x37,
  0x38, 0x3D, 0x3E, 0x84.
- Standard DIDs F180, F184, F186, F18C, F192, F193.
- Sessions 0x01/0x02/0x03; reset 0x02; NRC 0x78 extends the timeout; other NRCs
  surface through an `opu8_NrCode` out-parameter and `h_GetOpenSydeServiceErrorDetails`.
- Missing standard services: 0x14 ClearDTC, 0x19 ReadDTCInformation, 0x28
  CommunicationControl, 0x2F IOControl, 0x35 RequestUpload, 0x85 ControlDTCSetting,
  0x87 LinkControl. NRC 0x21 BusyRepeatRequest is not handled.

What is openSYDE-specific and would have to be bypassed or parameterised for a
generic client: SIDs 0xBA–0xBE, DIDs 0xA800–0xA825, routine IDs 0x0202–0x0225,
session 0x60, reset 0x60, the fixed 42/23 seed-key in non-secure mode, the 0x84
encryption sublayer and its whitelist, the signature-address TransferExit, and the
fact that EcuReset does not wait for a response.

### CAN transport — ISO 15765-2, but only the openSYDE dialect

`C_OscProtocolDriverOsyTpCan` (2,100 lines) does SF/FF/CF/FC plus two openSYDE
frame types (OSF 0xF0, OMF 0xE0). Against a third-party UDS server it will fail,
for four verified reasons:

1. **Flow control acceptance** (`m_HandleIncomingFlowControl`, `.cpp:356`): only
   DLC 3 with BS = 0 and STmin = 0 is accepted. Nearly every production ECU pads FC
   to DLC 8, and many send a nonzero BS or STmin. FC WAIT (0x31) and OVFLW (0x32)
   are not handled.
2. **Addressing** (`m_GetTxIdentifier`, `.cpp:1117`): CAN IDs are derived from the
   bus/node identifier pair as `0x18DA<target><source>`. There is no way to give
   the transport an arbitrary request/response ID pair, and no 11-bit support.
3. **Padding**: single frames are sent at DLC = length + 1, not padded to 8. Some
   ECUs reject unpadded frames.
4. **No CAN FD**, no FF length escape above 4095, no P2/P2\* timing distinct from
   the openSYDE timeouts.

`C_OscProtocolDriverOsyTpIp` is DoIP-shaped (0x02/0xFD header, 0x8001 diagnostic
message, port 13400) but has no routing activation (0x0005), no diagnostic ACK/NACK
(0x8002/0x8003) and no alive check, so it is not interoperable with a standard DoIP
gateway either.

### Test harness — ready to reuse

`tests/osy_virtual_ecu.hpp` is an in-process UDS server sitting behind both a
`C_CanDispatcher` double (`test_can_transport_virtual_ecu`) and a
`C_OscIpDispatcher` double (`test_su_sequences_virtual_ecu`). The CAN double already
does FF/FC/CF segmentation. Every transport change in layer B lands with a test
here; extending the server with configurable FC behaviour and arbitrary IDs is the
first task.

### CAN Monitor — more than expected, partly broken, partly never built

- `cmon_protocols/C_CanMonProtocolUds` (843 lines): a generic per-frame UDS decoder
  with SIDs, NRCs and F18x DID names, selectable in the trace as "UDS" alongside
  "CAN-TP". **Bug:** `MessageToString` (`.cpp:655–667`) stores the 12-bit first-frame
  length in a `uint8_t`, so every first frame renders as "SF(DLC too short)" or
  "SF(empty)".
- `communication/C_CamCanTpDecoder` / `C_CamCanTpTransmitter` (GUI tool, not core):
  receive-side reassembly keyed by CAN ID, 11- or 29-bit, up to 32 sessions; a
  transmitter that waits for FC. This is a second ISO-TP implementation, separate
  from the core transport, and in some respects (11-bit, arbitrary IDs) more
  general.
- Message generator rows carry `E_TxProtocol { eTX_CAN, eTX_CAN_TP, eTX_UDS }` and
  the project file persists `uds-service-id`, `uds-sub-function`, `uds-did`,
  `uds-data`.
- `C_CamUdsRequestBuilderDialog.{cpp,hpp}` exists in the source tree but is in **no
  CMakeLists and referenced by nothing**. It has never been compiled on this branch.
- Roadmap open item 1: the NRC→text table is duplicated between
  `C_CanMonProtocolUds.cpp:412` and `C_CanMonProtocolOpenSyde.cpp:1043` and has
  already drifted. A third copy lives in the driver's
  `h_GetOpenSydeServiceErrorDetails`.

### Node model — a two-value switch

`C_OscNodeProperties` has `E_DiagnosticServerProtocol { eDS_NONE, eDS_OPEN_SYDE }`
and `E_FlashLoaderProtocol { eFL_NONE, eFL_OPEN_SYDE }`. KEFEX was never a driver
here and the STW Flashloader was gutted in `69d44ccae`, so there is **no
protocol-neutral diagnostic or flash interface** left above the driver: every call
is `SendOsy*` typed on `C_OscProtocolDriverOsy*`. The one abstraction that survives
is `C_OscDiagProtocolBase`, the datapool-index read/write interface the data dealer
and dashboards consume. That matters for layer C.

Device definitions (`C_OscSubDeviceDefinition`) carry only openSYDE flags
(`q_DiagnosticProtocolOpenSydeCan`, `q_FlashloaderOpenSydeCan`, ...) and the filer
recognises only an `<opensyde>` sub-element under `<protocols-diagnostics>` and
`<protocols-flashloader>`.

## Layer 0: finish what is already there

Independent of any decision, small, and each item is a defect today.

| Item | Size |
|---|---|
| Fix the first-frame length truncation in `C_CanMonProtocolUds::MessageToString` | hours |
| Consolidate the three NRC tables into one core table (roadmap item 1; needs the wording decision) | half day |
| Either wire `C_CamUdsRequestBuilderDialog` into the CAN Monitor build and the generator, or delete it. Deleting is the honest default until layer C decides what the generator should do with a DID table | hours either way |
| Add an exhaustive round-trip test over `hc_ALL_PROTOCOLS` for the string converters (`h_CommunicationProtocolToString` and its inverse) so the eventual `eUDS` cannot be forgotten in a filer | hours |

## Layer B: a generic UDS client in core

### B1. Make the CAN transport ISO 15765-2 conformant

Change `C_OscProtocolDriverOsyTpCan` rather than adopt the CAN Monitor's
`C_CamCanTp*` pair: the core transport has the Tx/Rx service queues, the
`Cycle()` contract, the broadcast handling and the test double, and the Monitor's
pair lives in a GUI tool where core cannot reach it. Work:

- Accept padded FC (DLC up to 8), honour BS and STmin on transmit, handle FC WAIT
  and OVFLW, add the FF length escape.
- Add a configurable addressing mode next to the derived one: explicit request and
  response CAN IDs, 11-bit or 29-bit, optional functional (broadcast) ID, optional
  Tx padding to DLC 8 with a configurable pad byte.
- Keep the openSYDE derived addressing as the default so the existing nine tools
  and every existing test are untouched. Verify with the full nine-tool build and
  the virtual CAN bus suite.
- CAN FD and DoIP routing activation: **out of scope for the first pass**, listed
  under decisions.

### B2. A generic UDS service layer

Do not grow `C_OscProtocolDriverOsy` further; it is 6,500 lines and every method
assumes an openSYDE server. Do not try to extract a shared base from it either;
the request/response/poll core (`m_SendRequest`, `m_ReadResponse`,
`m_PollForSpecificServiceResponse`) is about 500 lines and copying it into a new
class is cheaper and safer than refactoring a class that the whole update path
sits on.

New `protocol_drivers/C_OscProtocolDriverUds` on top of `C_OscProtocolDriverOsyTpBase`:

- Services: 0x10, 0x11, 0x14, 0x19 (sub-functions 0x01, 0x02, 0x0A), 0x22, 0x27,
  0x28, 0x2E, 0x31, 0x34, 0x36, 0x37, 0x3E, 0x85. Each returns `std::error_code`
  with the NRC through the same `opu8_NrCode` convention the openSYDE driver uses.
- P2 / P2\* from the session, NRC 0x78 extends to P2\*, 0x21 retries.
- Seed-key: an interface with one virtual method, `CalculateKey(seed, level) ->
  key`. Ship one built-in implementation (the constant one the openSYDE driver
  uses) and leave the algorithm per device as a decision below. The security
  sublayer (0x84) and the crypto agent do not apply here.
- Extend `osy_virtual_ecu.hpp` to answer standard DIDs, hold a DTC list and a
  seed-key, and run the new driver's suite against it over the CAN double.

Rough size: driver 1,500–2,000 lines, transport changes 400–600, tests 800–1,000.

## Layer A: data model and bus editor

### A1. Representing UDS in `E_Type`

Append `eUDS` **at the end** of `E_Type`. The integer value is persisted in two
user-settings paths and packed into a navigation bitmask, and the CANopen usage
array is indexed by it, so inserting anywhere else corrupts existing user settings.

Mapping of the existing message model onto a DID, so that filers, tables, the
layout grid and the sync manager keep working with minimal special-casing:

| `C_OscCanMessage` field | Meaning for UDS |
|---|---|
| `u32_CanId` | the 16-bit DID; `q_IsExtended` unused |
| `u16_Dlc` | DID payload length in bytes; `q_IsMultipacket` true when > 8 so the paginated layout grid (already built for J1939 TP) renders it |
| `e_TxMethod` | one new value, `eTX_METHOD_UDS_ON_REQUEST`; cycle, delay and timeout hidden |
| Tx / Rx container | Tx = the node answers this DID to 0x22 (readable); Rx = the node accepts it via 0x2E (writable). A DID in both is read/write |
| `c_Signals` | unchanged; CANopen OD and J1939 SPN fields unused |

Routines (0x31) and DTCs (0x19) do **not** fit the message model. Routines have a
request record and a response record with independent layouts; DTCs are a list of
24-bit codes with text. Both go in a new per-node `C_OscNodeUdsConfig` rather than
in the message container:

- `C_OscNodeUdsConfig`: addressing (request ID, response ID, functional ID, 11/29
  bit, padding), timing (P2, P2\*, S3), the sessions and security levels the node
  supports, a routine table (RID, name, control sub-functions, request/response
  layouts as signal lists), a DTC table (code, name, severity), and the seed-key
  algorithm selector. Persisted as a `<uds>` sub-node of the node, filer
  alongside `C_OscNodeCommFiler`.

The per-protocol property switches in `C_OscCanProtocol.cpp` (DLC offset, signal
gaps, byte alignment, signals required) each get a UDS row; all four take the
Layer 2 answer.

### A2. Node and device definition

- `E_DiagnosticServerProtocol` gains `eDS_UDS`, `E_FlashLoaderProtocol` gains
  `eFL_UDS`, string forms `"uds"` in `C_OscNodeFiler`. Every `switch` on them
  today has two cases and no default, so the compiler finds the rest.
- `C_OscSubDeviceDefinition` gains `q_DiagnosticProtocolUdsCan` /
  `q_FlashloaderUdsCan` and a default `C_OscNodeUdsConfig`; the device-definition
  filer learns a `<uds>` sub-element under both `<protocols-*>` nodes. This is the
  same shape as the recent `device.syd` v3 work that embedded CAN messages, and a
  sensor's DID table belongs in `device.syd` for the same reason its J1939
  messages do.
- `device.syd` file version bumps to `0x0004`; v3 files load unchanged.

### A3. Bus editor

The tab is hardcoded at five in `C_SdBueComIfDescriptionWidget.ui` and in two
index↔enum switches; it becomes six. The visible surface:

- Message table: columns ID→"DID", DLC→"Length", hide cycle/delay/timeout,
  priority/PGN. Signal table: hide SPN and OD columns.
- Message properties: the protocol-conditional show/hide block (`SetComProtocol`,
  31 enum sites in that one file) gets a UDS branch that hides everything but
  name, comment, DID, length and direction.
- Node properties: a "UDS" panel for `C_OscNodeUdsConfig` (addressing, timing,
  seed-key selector), plus routine and DTC tables. This is the only genuinely new
  UI; everything else is conditional visibility in existing widgets.
- `C_SdUtil::h_AdaptMessageToProtocolType` and its signal twin get a UDS case so
  copy/paste between tabs behaves.
- Import: there is no DBC concept of a DID. The UDS exchange formats are ODX /
  PDX / CDD, all large XML schemas. **Out of scope** for the first pass; a CSV or
  hand-authored `device.syd` is the entry path, which is how the J1939 sensors
  are already being described. Listed under decisions.
- Code export: `mh_GetProtocolNameByType` needs a stem (`"uds"`) so the generator
  does not abort, but generating a UDS server from the DID table is a feature of
  its own and is out of scope. The first pass emits nothing for UDS protocols and
  says so.

### A4. CAN Monitor decoding from the DID table

`C_OscComLoggerProtocolOpenSyde` already shows how a decoder learns names from the
system definition. Give `C_CanMonProtocolUds` the same hook: when the logger has a
system definition, a 0x62 response's payload is decoded through the node's DID
message and its signals, rendered the way J1939 signals are. Depends on A1.

Rough size for layer A, calibrated on J1939: J1939's initial arrival touched about
66 files for roughly 650 lines in core and GUI, before its catalog dialog added
another 2,000. UDS has the same number of enum sites to visit (every one of the 22
`eJ1939` files, plus the 51 `eCAN_OPEN` files to confirm "not CANopen" branches
still hold), plus one new core config class with filer and tests, plus one new
node-properties panel. **80–100 files, 3,000–4,000 lines**, mostly mechanical.

## Layer C: consumers

Each of these is a separate feature with its own value; none is required for
"first class" in the bus-editor sense, and all three depend on A and B.

### C1. Dashboards read DIDs live

The dashboard data path consumes `C_OscDiagProtocolBase` through the data dealer,
addressing elements as (datapool, list, element). A `C_OscDiagProtocolUds`
implementing that interface maps the UDS COMM datapool's elements back to their
DID and signal, issues a 0x22, and unpacks the signal. Cyclic reads become a
polling loop at the configured rate; event-driven reads are not available and the
NVM methods return `Errc::no_support`. The dashboard layer needs no change beyond
the `eDS_UDS` case in `C_SyvComDriverDiag`'s protocol switch. Roughly 800 lines
plus tests.

### C2. Flash a UDS-only ECU

`C_OscBuSequences::UpdateNode` is already a 0x10 → 0x27 → RID → 0x34 → 0x36 → 0x37
sequence with one driver and no system definition. A generic sibling,
`C_OscBuSequencesUds`, drops the openSYDE RIDs, the A8xx DIDs, the fingerprint and
the signature-address TransferExit, adds optional 0x31 erase and checksum
routines with configurable RIDs, and takes the seed-key from `C_OscNodeUdsConfig`.
SYDEflash and the CLI flash tool switch on `eFL_UDS`. **`C_OscSuSequences` (the
multi-node system update) stays openSYDE-only**: it is tied to broadcasts,
session 0x60, and routing, and a UDS node in a mixed system would simply be
reported as unsupported there in the first pass. Roughly 1,000 lines plus a
virtual-ECU test that flashes a hex file through the generic path.

### C3. CAN Monitor generator from the DID table

With A in place the generator's `eTX_UDS` rows can offer a DID picker instead of
raw bytes, and the request-builder dialog from layer 0 becomes worth reviving.
Small once A exists.

## Decisions parked on a human

| Item | The question |
|---|---|
| **Which layers are "first class"** | A alone is a schema; B alone is invisible to a user of the GUI; A+B+C1 is the point where a sensor's DIDs appear on a dashboard. Recommend B, then A, then C1, and treat C2 as its own feature |
| **Seed-key algorithms** | Vendor seed-key is usually a secret per ECU. Options: compile-in per device (code in the tree), a loadable shared library per device, a script hook, or "constant key only" for the first pass. The answer decides whether `C_OscNodeUdsConfig` carries a selector string or a path |
| **11-bit addressing** | Needed for most automotive ECUs, irrelevant for the J1939-world sensors in the device library today. Cheap in B1 if done at the start, awkward later |
| **CAN FD** | Neither `C_CanDispatcher` nor the transport knows DLC > 8 on the wire. The J1939 TP work raised `u16_Dlc` in the model but not the dispatcher. Out of the first pass unless a target device needs it |
| **DoIP interoperability** | Routing activation and ACK handling in `TpIp` are needed only if a UDS target is on Ethernet. The R38 bring-in touches the same file; sequence the two |
| **ODX / CDD import** | The industry entry format for DID tables. Big. Decide whether hand-authored `device.syd` is acceptable long-term for the devices this fork serves |
| **Code export for UDS** | Generating a UDS server skeleton from the DID table would make openSYDE useful on the ECU side. Separate feature; needs a target runtime to generate against |
| **NRC wording** | Roadmap item 1, a prerequisite for layer 0 |

## Risks

- **The transport change touches the update path of every shipped tool.** B1
  must keep derived addressing as the default and run the full virtual-ECU suite
  on all three platforms before merge. The CI Windows job is the only place the
  `_WIN32` dispatcher paths are compiled.
- **Enum append order.** `eUDS` goes last in `E_Type`, `eDS_UDS` and `eFL_UDS`
  last in theirs. The user-settings integer cast is the trap.
- **Two ISO-TP implementations will exist** (core transport and the CAN Monitor's
  `C_CamCanTp*`) until the Monitor's receive path is moved onto core. Not required
  for this scope, but it is the same shape of duplication as the NRC tables and
  will drift the same way. Note it on the roadmap when B1 lands.
- **Estimate calibration.** The J1939 numbers come from squashed upstream release
  commits and are approximate. The layer-A estimate is the best-supported one;
  B and C are judgement.
