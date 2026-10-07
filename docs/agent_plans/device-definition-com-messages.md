# Plan: Embed CAN messages in the device definition

**Status:** active — 2026-10-05 — model, filer, seeding, tests and generator are in; devices are being regenerated one by one (see the dated status sections at the end)

## Context

This fork's device library stores one `device.syd` per device — the `opensyde-device-definition` hardware card (name, bus counts, bitrates, protocols, EEPROM). A device's CAN messages live separately in a node config (`C_OscNode` + `C_OscCanProtocol` + `C_OscNodeDataPool`), which today is only reachable through a `.syde_tsp` wrapper the user must import when placing the device on the topology.

The messages for a given device are static — they do not change per node. We forked the tool and have the source, so we should simplify: **embed a device's CAN messages directly in its `device.syd`**, and have placing the device seed the node's COM datapool + CAN protocol automatically. This eliminates the TSP wrapper and the separate `comm_*.xml` / `dp_*.xml` files for the message payload. (Devices that need extra things — template project, code-export settings — can still use the TSP mechanism; the sensors do not.)

Intended outcome: `device.syd` is the single source of truth for a device's hardware **and** its messages; dragging the device onto the topology produces a node pre-populated with those messages.

## Approach

### 1. Data model — `C_OscDeviceDefinition`

File: `libraries/opensyde_core/project/system/device_definition/C_OscDeviceDefinition.hpp`

Add two members (mirroring what a node's COM config needs), plus `Clear()` handling:

```cpp
std::vector<C_OscCanProtocol> c_ComProtocols;   ///< CAN protocols (messages/signals) for this device
std::vector<C_OscNodeDataPool> c_ComDataPools;  ///< COM datapools (lists/elements) the protocol signals index into
```

Include `C_OscCanProtocol.hpp` and `C_OscNodeDataPool.hpp`. The existing `C_OscCanProtocol::u32_DataPoolIndex` already links a protocol to its datapool, so carrying both preserves the linkage.

### 2. Filer — `C_OscDeviceDefinitionFiler`

File: `libraries/opensyde_core/project/system/device_definition/C_OscDeviceDefinitionFiler.cpp`

- Bump `mhu16_FILE_VERSION` to `0x0003U`.
- `h_Load` (currently cpp:1230): accept **both** `0x0002` (no messages — existing files) and `0x0003`. For `0x0003`, parse optional top-level `<com-protocols>` and `<data-pools>` sections into `c_ComProtocols` / `c_ComDataPools` using the **existing inline loaders** that `C_OscNodeFiler`'s embedded mode uses (`C_OscNodeCommFiler` / `C_OscNodeDataPoolFiler` load-from-parser functions). If the sections are absent, leave the vectors empty (backward compatible).
- `h_Save` (cpp:1337): write `file-version 0x0003`. If `c_ComProtocols`/`c_ComDataPools` are non-empty, emit `<com-protocols>`/`<data-pools>` inline (reuse the same inline serializers — the embedded-mode path in `C_OscNodeFiler`, or the comm/datapool filers' save-to-parser functions). Keep the rest of the schema unchanged.
- The XML shape to emit (only when messages exist):
  ```xml
  <opensyde-device-definition>
    <file-version>0x0003</file-version>
    <global> ...unchanged... </global>
    <sub-devices> ...unchanged... </sub-devices>
    <com-protocols> <com-protocol> ...inline C_OscCanProtocol... </com-protocol> </com-protocols>
    <data-pools>   <data-pool>   ...inline C_OscNodeDataPool...  </data-pool>   </data-pools>
  </opensyde-device-definition>
  ```

### 3. Seed node creation — GUI

File: `opensyde_tool/src/system_definition/C_SdTopologyScene.cpp`

In `m_InitNodeData` (cpp:3809), after `m_InitNodeComIfSettings` (line 3852, which calls `CreateComInterfaces`), seed the node's messages from the device definition:

- Add a core helper, e.g. `C_OscDeviceDefinition::h_AddComDataToNode(C_OscNode & orc_Node) const` (in `C_OscDeviceDefinition.cpp`), that:
  1. Appends each `c_ComDataPools` entry to `orc_Node.c_DataPools` (recording the appended index).
  2. For each `c_ComProtocols` entry, sets `u32_DataPoolIndex` to that index and pushes to `orc_Node.c_ComProtocols`.
  - No-op if the vectors are empty.
- Call it from `m_InitNodeData` right after the com-interface setup. This makes every placed node come pre-populated with the device's messages (the "template" behavior — operator then customizes node ID / values per instance, as discussed).

### 4. Tests

- **`test_filer_roundtrip.cpp`** `FilerRoundTrip.DeviceDefinition` (lines 435–538): extend the `C_OscDeviceDefinition` construction with an `h_MakeProtocol(eJ1939)`-style protocol + a matching `eCOM` datapool (reusing the `osy_test_models.hpp` builders), and add round-trip asserts on the messages/datapool. Add a second test for a device definition **without** messages to pin backward compatibility (v0x0002 load).
- **`osy_test_models.hpp`**: add a `h_MakeDeviceDefinition()` helper (reusing `h_MakeProtocol` + the `eCOM`-pool block from `h_MakeNode`, lines 293–329) so tests and the generator share one source of truth for a "device with messages."
- **New test** (e.g. `test_device_definition_com.cpp` or an addition to `test_filer_roundtrip`): build a device with messages, seed a `C_OscNode` via the new helper, assert the node's `c_DataPools`/`c_ComProtocols` are populated and `u32_DataPoolIndex` points correctly.
- Watch the `SystemDefinitionFile` test (test_filer_roundtrip.cpp:869) — it asserts `pc_DeviceDefinition == nullptr`; seeding messages into nodes should not change that seam.

### 5. Generator + TSP cleanup

- Update the headless generator (`docs/agent_plans/gen_device_com_messages.cpp`) to emit a `device.syd` **with embedded messages** (via `C_OscDeviceDefinitionFiler::h_Save`) instead of separate `gps.syde_node`/`comm_*.xml`/`dp_*.xml`. Run it for the GPS (and later every device with a DBC).
- The separate node-config files and the `.syde_tsp` are no longer needed for the message payload. The TSP machinery stays in the codebase (still used for programmable devices / template projects) but is not required for the sensors.

## Backward compatibility

- Existing v0x0002 `device.syd` files (the 61 currently in the device library) load unchanged — the new sections are optional, and `h_Load` accepts both versions.
- The DBC-sync convention (a `C_SdNdeDbcSync` helper, a Sync DBC column on the node's interface table, a sibling `<device>_CAN<n>.dbc`) was removed on 2026-10-07 at the user's request; embedded messages are the only path.

## Files to modify

- `libraries/opensyde_core/project/system/device_definition/C_OscDeviceDefinition.hpp` (+ `.cpp` for `Clear`, `h_AddComDataToNode`)
- `libraries/opensyde_core/project/system/device_definition/C_OscDeviceDefinitionFiler.cpp`
- `opensyde_tool/src/system_definition/C_SdTopologyScene.cpp` (`m_InitNodeData`)
- `libraries/opensyde_core/tests/test_filer_roundtrip.cpp`
- `libraries/opensyde_core/tests/osy_test_models.hpp`
- (new) a small test file or addition for node-seeding

## Verification

1. `cmake -S libraries/opensyde_core -B build/core -G Ninja -DCMAKE_BUILD_TYPE=Debug -DOPENSYDE_CORE_BUILD_TESTS=ON` then `cmake --build build/core` and `ctest --test-dir build/core --output-on-failure`.
   - Targeted: `./build/core/tests/test_opensyde_core_filer_rt --gtest_filter='FilerRoundTrip.DeviceDefinition*'`.
2. Run the updated generator on `GPS_r004.dbc` → produces a `device.syd` with the 4 J1939 messages; load it back with `C_OscDeviceDefinitionFiler` (round-trip) and confirm the messages survive.
3. Build the GUI tool (`./build.sh -d opensyde`), place the GPS on the topology, and confirm the node is pre-populated with the J1939 datapool (no TSP import needed).
4. Full core test suite + the existing `test_device_manager` to confirm the scan contract is intact.
5. Confirm all 61 existing `device.syd` still load (no messages → unchanged behavior).

## Status (2026-10-02)

Implemented and verified:
- **Data model**: `C_OscDeviceDefinition` gained `c_ComProtocols` + `c_ComDataPools` (with `Clear()`).
- **Filer**: `C_OscDeviceDefinitionFiler` bumped to `mhu16_FILE_VERSION = 0x0003`; `h_Load` accepts both 0x0002 (backward compat) and 0x0003; `h_Save` writes optional `<data-pools>`/`<com-protocols>` inline, reusing `C_OscNodeDataPoolFiler::h_SaveDataPool`/`h_LoadDataPool` and `C_OscNodeCommFiler::h_SaveNodeComProtocol`/`h_LoadNodeComProtocol`.
- **Node seeding**: `C_OscNode::AddComDataFromDeviceDefinition(const C_OscDeviceDefinition&)` appends the device's COM datapools and rebases each protocol's `u32_DataPoolIndex`; called from `C_SdTopologyScene::m_InitNodeData`.
- **Generator**: `docs/agent_plans/gen_device_com_messages.cpp` loads a device.syd, parses a DBC (Vector::DBC), and re-saves as v0x0003 with embedded messages.
- **Tests**: `FilerRoundTrip.DeviceDefinition` extended with a COM datapool + J1939 protocol round-trip; new `FilerRoundTrip.DeviceDefinitionSeedsNodeCom` and `FilerRoundTrip.DeviceDefinitionV2LoadsWithoutCom` (backward compat). `osy_test_models.hpp` gained `h_MakeDeviceDefinition()`. All pass; `test_device_manager` + `test_can_protocol` pass; full `FilerRoundTrip` suite (23 tests) passes.
- **GPS device.syd** regenerated to v0x0003 with its 4 J1939 messages (12 signals, SPNs); device manager loads all 61 devices `C_NO_ERR`.
- **GyroInclinometer** (Trombetta) regenerated to v0x0003 with its 1 J1939 message (`Slope_Sensor_Information`, 8 signals, SPN unset — proprietary params, no SAE SPNs). Verified round-trip; device manager loads all 61 devices `C_NO_ERR`, `withCom=2` (GPS + Inclinometer).
- **GX16** (Gigavac contactor) regenerated to v0x0003 with its 16 J1939 messages split by DBC transmitter: 9 TX (`CTR1_Status_Report1–8`, `CTR1_Address_Claim_Attempt`) + 7 RX (`CTR1_Contactor_Command`, `CTR1_Trip_Time/Current_Setting`, `CTR1_Misc_Settings`, `CTR1_Commanded_Address_Packet1/2`, `CTR1_BAM_for_Commanded_Address`), `CAN1_TX`/`CAN1_RX`. Signals have non-standard ranges (`[0|0]`, Intel byte order, signed/unsigned mix) with SPN unset — core accepts them (J1939 validation is permissive; the detailed signal checks only apply to CANopen manager messages). Verified round-trip; device manager loads all 61 devices `C_NO_ERR`, `withCom=3`.

Done / resolved:
- GUI tool builds with the `m_InitNodeData` change; user confirmed node messages stay visible when connected to a bus (the two gates in `C_PuiSdNodeCanMessageSyncManager` and `C_SdBueComIfDescriptionWidget`).
- Redundant `gps.syde_node` / `comm_gps_core.xml` / `dp_gps_core.xml` / `gps.syde_tsp` removed from the GPS device folder.

## Status (2026-10-05)

**Done this session:**
- **9 single-DBC devices installed** (FuelCAN, C2010, Angular Position Sensor, IX3212,
  Hydrapulse, PVCAN20-J, PVCAN35-S, em200, DCAC_ACHV_IMP), each v0x0003 with embedded messages.
- **9 multi-DBC devices installed** (PM250DZ, N32123, DCDC_HVLV_LP, BelFuse 12V, BelFuse 24V,
  CellGuard, Fan, WP32, HBridge) after adding a device→node-name override to the generator
  (`docs/agent_plans/gen_device_com_messages.cpp`, optional 5th arg = the device's node name in the DBC). This fixes
  the TX/RX split that the device-name match alone got wrong (e.g. BelFuse-12V → `BP_12V_1`,
  EMP Fan → `Fan`, H_Bridge → `HBridge`, EMP Pump → `Pump`, DCDC_HVLV_LP → `DCDC`).
- **21 devices now carry embedded messages** (3 earlier + 18 this session). All 61 device.syd
  still load: `OK=61 FAIL=0`.
- **L6T** regenerated from the better DBC the user provided
  (`/home/tyler.wagler.local/Projects/Saft/L6T/extracted/6T-Type1_v6_fixed.dbc`, 42 messages /
  463 signals). The DBC has an empty `BU_` and all transmitters are `Vector__XXX`, so direction
  is not encoded — the **IDD/manual** (`/home/tyler.wagler.local/Projects/Saft/L6T/IDD.txt`) is
  the source of truth. Status/telemetry messages are `Battery Pack Monitor → Any` (**TX**, also
  confirmed by the decoded capture `extracted/decoded_F6_20261005_0921.txt`, all source addr `F6`);
  the command messages in the IDD "Battery Configuration" section (`Any → Battery Pack Monitor`,
  PGN 61184) are **RX**. Split: **36 TX / 6 RX** (RX = `ComandedAddress`, `StopStartBroadcast`,
  `MemoryAccessRequest`, `BootLoadData`, `Request`, `BatteryConfiguration`). Backed up to
  `/tmp/devsyd_backup_l6t_20261005_105135`. **22 devices now carry embedded messages.**
- Generator gained an optional 6th arg: a comma-separated RX-message list (for DBCs that don't
  encode direction). Recompiled against the working tree.
- Backups: `/tmp/devsyd_backup_20261005_092532` (9 single-DBC),
  `/tmp/devsyd_backup_multi_20261005_095353` (9 multi-DBC), and
  `/tmp/devsyd_backup_l6t_20261005_105135` (L6T).

**Outstanding (blocked on user):**
- **3K206-2RC3AG** (HMI_6Button.dbc) and **SenderCAN** (SenderCAN.dbc): DBCs have 0 `BO_`
  messages (attribute-only templates). Left at v0x0002; user is hunting for the real DBCs.

**Best-guess picks that need hardware validation** (user confirmed these are provisional):
- Address variants: BelFuse 12V/24V picked `0xB0` (also 0xB1–0xB3 exist); CellGuard picked
  v3pt3 / 0x30 addresses (0x40 variant exists); DCDC_HVLV_LP picked base (0x702; 0x680 variant exists).
- Hardware variants: Fan picked `8A` (8B exists); WP32 picked `8D` (8E exists).
- PM250DZ node guessed as `INV` (DBC nodes: INV, U2C, VCU, BMS); N32123 node `AlternatorRegulator`.
- HBridge picked `HBridge_0x70_061225.dbc` (Jun 2025, 3 msgs) with node `HBridge`; the base
  `Trombetta_HBridge.dbc` has 15 msgs but no nodes.
- Node names per device are listed in the "Fix TX/RX with mapping" decision; correct any that are wrong.

Note: the generator splits TX/RX by DBC transmitter when the device is a node in the DBC
(`GX16`→TX, other nodes→RX, `Vector__XXX`→RX unless the message name contains `Address_Claim`);
single-ECU DBCs (device not a node) default to all-TX. This keeps sensors like the
GyroInclinometer correct.

