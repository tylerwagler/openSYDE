# Scope: Electrical connectors with pinouts on the node definition

**Status:** active — 2026-10-07 — scoping; nothing implemented, decisions 1–5 below are open

Written 2026-10-07 from a survey of the tree at `293ec7ae6` and of the device library
installed at `~/.local/opt/openSYDE/devices`. The "what exists" section is verified
against the code; the estimates are not.

## The question this document answers

"Connectors with pinouts as part of the node definition" has to be split along one
line before anything else: **what is fixed by the hardware** and **what is chosen per
installed unit**.

- A device has a fixed set of connectors (X1, X2, …), each with a fixed set of pins,
  and each pin has a fixed function: CAN 1 high, supply, ground, digital output 3.
  That is a property of the *device*, identical for every node of that type.
- Which harness wire lands on a pin, what the net is called in the vehicle, which
  mating plug and seal are used: that is a property of the *node* in a particular
  system.

The tree already draws exactly this line for messages: a device's CAN messages live
in `device.syd` and are copied onto the node when it is placed. Connectors should
follow it. The pinout belongs to the device definition; the node carries only wiring
annotations on top, if it carries anything at all.

So the scope has three layers that ship independently:

| Layer | What the user gets | Depends on |
|---|---|---|
| **1. Device pinout** | `device.syd` describes connectors and pins; the node editor shows them read-only; the interface table says which pins carry each CAN/Ethernet port | nothing |
| **2. Node wiring** | Per node, each pin can carry a net name, wire label and comment, persisted in the project | 1 |
| **3. Wiring list export** | A CSV of every node, connector and pin in the system with the bus each CAN pin is on, derived from the topology | 1 (2 enriches it) |

Recommended order is the numbering. Layer 1 is the data model and has to be right
first; layer 3 is the actual payoff for a harness or integration engineer and is
cheap once layer 1 exists; layer 2 is the one whose value depends on how the library
is used and can wait.

## What already exists (verified 2026-10-07)

**Nothing models a pin or a connector.** Every `connector` in the sources is the
topology's bus-connector line (`C_GiLiBusConnector*`) or the tool-plugin folder
`connectors/` next to the executable (crypto agent, generators). Every `pin` is test
prose.

**The device library is one `device.syd` per device, discovered by directory scan.**
`C_OscDeviceManager::LoadFromPaths` walks the root paths recursively for files named
`device.syd`; `devices.ini` is legacy and unused by the loader. The installed library
has 61 devices in category folders, almost all third-party (New Eagle, Enovation
Controls, Saft, Delta Cosworth, Brightloop, Trombetta, …), with datasheets, manuals,
DBC files and a product image alongside each manifest. Those PDFs are where the
pinouts currently are. 21 devices are already on file version 0x0004 with embedded
messages; the rest are 0x0002.

**The device definition** (`C_OscDeviceDefinition`, filer at
`project/system/device_definition/`) carries device-level name, description,
image path (expanded relative to the manifest), bus counts, bitrates, CAN features,
manufacturer links, a vector of sub-devices, and since 0x0003 the embedded COM
protocols and datapools. The sub-device carries the protocol flags, UDS config,
J1939 source address, timeouts and a `c_ConnectedInterfaces` map keyed by interface
name. Current file version is 0x0004; the filer reads 0x0002 to 0x0004 and writes
the latest.

**The node** (`C_OscNode`) references its device by `c_DeviceType` and a resolved
`pc_DeviceDefinition` pointer. Its COM interfaces (`C_OscNodeComInterfaceSettings`)
are identified by `e_InterfaceType` plus 0-based `u8_InterfaceNumber`, and know the
bus they are connected to through `u32_BusIndex`. That pair is the natural key for
"this pin is CAN 2 high".

**HALC** channels (`C_OscHalcDefChannelDef`) have a name and nothing else, no stable
id. Domains have ids. A pin-to-channel link therefore has to be by domain id plus
channel name. None of the 61 library devices uses HALC, so this link is a hook for
STW controllers, not something the current library would populate.

**The node editor** (`C_SdNdeNodeEditWidget`) is a `QTabWidget` with six pages:
Properties, Datapools, COMM Messages, CANopen Manager, HALC, Data Logger. Tabs are
created lazily, numbered by `hs32_TAB_INDEX_*`, and hidden per node through
`m_HandleVisibleTabs`. Adding a page is a known six-step edit.

**The only document export is the RTF export**, and it shells out to an external
`osy_docu_creator` under `connectors/DocuCreator/` that is not shipped in this fork
and does not exist on Linux. It is dead here. A pinout or wiring report needs its own
exporter; CSV is the pragmatic format and the one harness engineers can open.

**Round-trip tests** exist for the device definition (`FilerRoundTrip.DeviceDefinition`,
`DeviceDefinitionV2LoadsWithoutCom`) and for the whole system definition
(`FilerRoundTrip.SystemDefinitionFile`), with builders in `osy_test_models.hpp`.
Every new model class needs `CalcHash`, because the system definition hash is what
decides whether a project is dirty.

## Data model

### Layer 1: on the device definition

Device level, not sub-device level: a multi-CPU device shares one housing, and the
interface numbering a pin links to is device-wide.

```cpp
class C_OscDeviceConnectorPin
{
public:
   enum E_Function
   {
      eSUPPLY,            ///< battery or ignition supply in
      eGROUND,
      eSENSOR_SUPPLY,     ///< regulated supply out
      eCAN_HIGH, eCAN_LOW, eCAN_SHIELD,
      eETHERNET,          ///< one line of a pair; name says which
      eDIGITAL_INPUT, eDIGITAL_OUTPUT,
      eANALOG_INPUT, eANALOG_OUTPUT,
      eFREQUENCY_INPUT, ePWM_OUTPUT,
      eSERIAL,            ///< RS-232/485, LIN, K-line
      eNOT_CONNECTED,
      eOTHER
   };
   enum E_Link { eLINK_NONE, eLINK_COM_INTERFACE, eLINK_HALC_CHANNEL };

   std::string c_Designation;      ///< pin label on the housing: "1", "A3", "K12"
   std::string c_Name;             ///< manufacturer signal name: "CAN1_H", "VBAT", "DO_03"
   E_Function e_Function;
   E_Link e_Link;
   C_OscSystemBus::E_Type e_InterfaceType; ///< valid when e_Link == eLINK_COM_INTERFACE
   uint8_t u8_InterfaceNumber;             ///< 0-based, same numbering as the COM interfaces
   std::string c_HalcDomainId;             ///< valid when e_Link == eLINK_HALC_CHANNEL
   std::string c_HalcChannelName;
   std::string c_Electrical;       ///< free text: "9–32 V, 4 A", "0–5 V, 10-bit"
   std::string c_Comment;
   void CalcHash(uint32_t & oru32_HashValue) const;
};

class C_OscDeviceConnector
{
public:
   std::string c_Name;             ///< "X1"
   std::string c_Description;      ///< "Main I/O, 48-way"
   std::string c_HousingPartNumber;///< device side
   std::string c_MatingPartNumber; ///< harness side (plug, terminals, seal)
   std::string c_ImagePath;        ///< optional pin-face drawing, relative to device.syd like c_ImagePath
   std::vector<C_OscDeviceConnectorPin> c_Pins;
   void CalcHash(uint32_t & oru32_HashValue) const;
};

// on C_OscDeviceDefinition
std::vector<C_OscDeviceConnector> c_Connectors;
```

The function enum is deliberately short. It exists so that derived views can be
built (every `eCAN_*` pin with a COM-interface link is a bus pin; every `eSUPPLY`
pin goes in the power section of the wiring list). The manufacturer's own wording
goes in `c_Name` and `c_Electrical`, which are free text, so nothing is lost by the
enum being coarse. `eOTHER` is the escape hatch.

**Persistence.** A `<connectors>` section after `<sub-devices>` in `device.syd`:

```xml
<connectors>
  <connector name="X1" description="Main I/O" housing="DT04-12PA" mating="DT06-12SA" image="X1_pinface.png">
    <pin designation="1" name="VBAT" function="supply" electrical="9-32 V, 4 A"/>
    <pin designation="5" name="CAN1_H" function="can-high" interface="can1"/>
    <pin designation="6" name="CAN1_L" function="can-low" interface="can1"/>
    <pin designation="9" name="DO_03" function="digital-output" halc-domain="DO" halc-channel="DO_03"/>
  </connector>
</connectors>
```

Interface references reuse the `can1` / `ethernet1` spelling the sub-device's
`connected-interfaces` section already uses (1-based in the file, 0-based in memory,
same as today). File version goes to 0x0005 with `mhu16_FILE_VERSION_NO_CONNECTORS = 0x0004`
still read; an absent section loads as no connectors. The filer validates on load:
connector names unique, pin designations unique within a connector, interface links
within the device's bus counts, link fields present only for the matching `E_Link`.
A failing validation is a load error with the connector and pin named, not a
silent drop.

### Layer 2: on the node

```cpp
class C_OscNodeConnectorPinWiring
{
public:
   std::string c_ConnectorName;    ///< key into the device definition
   std::string c_PinDesignation;   ///< key into the connector
   std::string c_NetName;          ///< "CAN_VEH_H"
   std::string c_WireLabel;        ///< harness wire id or colour
   std::string c_Comment;
   void CalcHash(uint32_t & oru32_HashValue) const;
};

// on C_OscNode
std::vector<C_OscNodeConnectorPinWiring> c_ConnectorWiring;
```

Keyed by name on purpose: a device library edit that renames a pin must not
silently re-attach a wire to a different pin. The node editor shows an orphaned
entry (key no longer in the device) with a warning and offers to drop it, the same
way the tool handles a device whose definition went missing. Persisted as a
`<connector-wiring>` section in the node's XML through `C_OscNodeFiler`, with the
node file version bumped; hash included.

## GUI

**Node editor: a seventh tab, "Connectors".** The Properties tab is already full,
and the pinout is a table with its own selection state, which argues for a page of
its own. Visible only when the device defines at least one connector, through the
existing `m_HandleVisibleTabs` path, so the 40 devices without pinouts do not grow
an empty tab.

Layout, left to right:

- A connector list (name, pin count, description) with the pin-face image below it,
  loaded the way the Properties tab loads the device image.
- A pin table: Pin, Name, Function, Link (rendered as "CAN 1" or "DO / DO_03"),
  Electrical, Comment, and in layer 2 the editable Net, Wire and Node comment
  columns. The device-side columns are read-only and styled as such; the table base
  is `C_TblViewToolTipBase` so the tooltip fix from this week applies.

**Interface table on the Properties tab: a derived "Connector" column** showing
e.g. `X1.5 / X1.6` for a CAN port, blank when the device has no pinout. Cheap, read
only, and the single most useful line for someone looking at a node: it answers
"where do I plug the bus in". Part of layer 1.

**HALC channel widget: the pin for a channel**, derived from the HALC link. One
label. Deferred until a library device actually carries a HALC link.

**Topology:** no change. A bus-connector tooltip could list the pins at both ends
later; not in scope.

**No connector editor in the tool.** Pinouts are authored in `device.syd` by hand
like the embedded messages are today. Manufacturer pinouts arrive as tables in a
PDF, so the authoring aid that would pay for itself is a **CSV-to-XML importer**
(columns: designation, name, function, interface, electrical, comment), either as a
small tool under `docs/agent_plans/` like `gen_device_com_messages.cpp` or as a menu
action in the device toolbox. Decision 5.

## Layer 3: wiring list export

A core exporter, `C_OscExportWiringList`, producing one CSV for the system
definition:

```
Node, Device, Connector, Pin, Pin name, Function, Net, Wire, Bus, Bus type, Comment
```

`Bus` is derived: a pin linked to a COM interface whose `u32_BusIndex` is set
resolves to that bus's name; power and I/O pins leave it blank. With layer 2, `Net`
and `Wire` come from the node. Hooked into the system definition toolbar next to the
RTF export, writing where the user chooses. A second variant per node (one
connector's pins, for printing next to the bench) is a filter on the same function.
Tests pin the CSV against a golden string from `h_MakeNode` plus a device with two
connectors.

## Decisions that are the user's to make

1. **Device-level pinout only, or node wiring as well?** Recommendation: layer 1 and
   3 now, layer 2 only if the library is used to document installed systems rather
   than to describe parts.
2. **Function vocabulary: the short enum above, or free text?** Recommendation: the
   enum, because derived views (interface column, wiring list bus resolution) need
   it; free text goes in the name and electrical fields.
3. **Keep the HALC link in the model now, or add it when needed?** Recommendation:
   include the two fields and the filer support (an hour), skip the HALC widget
   display until a device uses it.
4. **Export format.** Recommendation: CSV. An HTML report is possible later and the
   same exporter feeds it.
5. **Authoring aid.** Recommendation: a CSV importer as a command-line tool first,
   because sixty devices have to be transcribed from PDFs and that is the dominant
   cost of this feature, not the code.

## Estimates

Reviewable chunks, in order. Each ends with the nine-tool build green and tests
passing.

| Chunk | Content | Size |
|---|---|---|
| 1a | Core model, filer, validation, version bump, round-trip and validation tests | ~1 day |
| 1b | Connectors tab (read-only), interface-table column, visibility handling | 1–2 days |
| 3 | Wiring list exporter, toolbar action, golden test | ~1 day |
| 2 | Node wiring model, node filer, editable columns, orphan handling | ~2 days |
| Authoring | CSV importer tool; then transcribing the library, which is manual work per device | 1 day + per-device effort |

## Open risks

- **Interface numbering drift.** A pin says "can1" and the sub-device says it is
  connected; nothing today checks that the two agree. The validator should.
- **Device library edits after placement.** Nodes resolve their device at project
  load, so a pinout change in the library shows up on existing nodes immediately.
  That is correct for layer 1 and the reason layer 2 keys by name.
- **The RTF export is dead on this fork** and nobody has noticed. Worth either
  removing or replacing with the exporter above; not in this scope but adjacent.
