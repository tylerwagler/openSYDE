# JSON Filer Rewrite — Session Handoff

**Created:** 2026-04-08 mid-session
**Purpose:** Context-window compact point. Read this first when resuming.

The canonical plan is `JSON_Filer_Rewrite_Plan.md` (same directory). This file is the *current state snapshot* — what's done, what's next, and the gotchas the previous session learned the hard way.

---

## Where we are

**Phase A:** Complete.
**Phase B steps 1–5:** Complete (data pool tree filers written + CMake-wired + standalone-compile-clean).
**Option A (caller migration sweep):** Complete (2026-04-08, see section below).
**Phase B step 6:** Complete (2026-04-08). Four new filers written for the CAN message family: `C_OscCanSignalFiler`, `C_OscCanMessageFiler`, `C_OscCanMessageContainerFiler`, `C_OscCanProtocolFiler`. CMake-wired in alphabetical order alongside their data classes. All four standalone-compile-clean with `g++ -std=c++17 -Wall -Wextra` against host Qt 6 (Qt6Core + Qt6Xml on the include path because the data class headers still pull in `<QDomDocument>` etc. for the on-class zombie methods — those headers can be stripped only after Phase B step 8). The on-class `ToJsonObject` / `FromJsonObject` / `ToQDom*` / `FromQDom*` / `ToQDataStream` / `FromQDataStream` methods on `C_OscCanSignal`, `C_OscCanMessage`, `C_OscCanMessageContainer`, `C_OscCanProtocol`, and `C_OscCanInterfaceId` were intentionally **left in place** — same reason as the data pool family: their callers in the wider zombie graph (`C_OscFilerUtil.hpp` template helpers, `C_OscNodeCommFiler.cpp`, the HALC `_Serialization.cpp` chain) are not migrated yet. They become orphaned and trivial to delete once Phase B step 14 (`C_OscNode`) lands.
**Phase B step 7:** Complete (2026-04-08). Three new filers written for the CANopen subtree (`C_OscCanOpenManagerMappableSignalFiler`, `C_OscCanOpenManagerDeviceInfoFiler`, `C_OscCanOpenManagerInfoFiler`) **plus** the existing `C_OscCanOpenManagerFiler` was **fully replaced** in place — the multi-format scaffold (`h_LoadFile`/`h_SaveFile`/`h_LoadBinary`/`h_SaveBinary`/`h_LoadJson`/`h_SaveJson`/`h_LoadXml`/`h_SaveXml`/`h_LoadFromMemory*`/`h_SaveToMemory*`/`h_LoadData`/`h_SaveData` plus all the `mh_LoadManager*` helpers) was deleted entirely and replaced with the two-method shape over `QHash<uint8_t, C_OscCanOpenManagerInfo>` (interface number → manager). Replacement was free because **zero external callers** of the old `C_OscCanOpenManagerFiler::*` API existed (verified by grep) — only the file itself and CMake referenced it. The mappable-signal filer composes the existing `C_OscCanSignalFiler` + `C_OscNodeDataPoolListElementFiler`. The device-info filer intentionally does NOT serialize `mc_EdsFileContent` / `mq_EdsFileContentLoaded` (private mutable; loaded dynamically from the EDS file path on demand). The manager-info filer serializes the inner `QHash<C_OscCanInterfaceId, C_OscCanOpenManagerDeviceInfo>` as a JSON array of `{device_node_index, device_interface_number, device}` entries, which preserves order for stable diffs and dodges the JSON-keys-must-be-strings restriction on the composite key. Same enum-as-string rule for `E_NmtErrorBehaviourType`. CMake-wired alongside their data classes (the existing `_Serialization.cpp` siblings are still listed too — they're zombies pending Phase B step 8 cascade). All four standalone-compile-clean with `-Wall -Wextra` against host Qt 6 (Qt6Core + Qt6Xml + the `imports/` directory on the include path — `imports/C_OscCanOpenObjectDictionary.hpp` is needed because `C_OscCanOpenManagerDeviceInfo.hpp` includes it for the private mutable `mc_EdsFileContent` member). The on-class zombies on `C_OscCanOpenManagerInfo` / `DeviceInfo` / `MappableSignal` were left in place for the same reason as Phase B6.
**Phase B step 8a:** Complete (2026-04-08). Ten new filers written for the HALC Definition tree, bottom-up: `C_OscHalcDefContentBitmaskItemFiler`, `C_OscHalcDefContentFiler` (the cascade-root that composes `C_OscNodeDataPoolContentFiler` for the base portion of `C_OscHalcDefContent` since it inherits from `C_OscNodeDataPoolContent`), `C_OscHalcDefElementFiler`, `C_OscHalcDefStructFiler` (composes Element since `Struct extends Element`), `C_OscHalcDefChannelValuesFiler`, `C_OscHalcDefChannelAvailabilityFiler`, `C_OscHalcDefChannelUseCaseFiler`, `C_OscHalcDefChannelDefFiler`, `C_OscHalcDefDomainFiler`, plus `C_OscHalcDefFiler` and `C_OscHalcDefStructFiler` were **fully replaced** in place — the existing files were paper-thin wrappers around `C_OscFilerUtil::h_LoadList*<T>` template helpers, gutted and replaced with the two-method shape over `C_OscHalcDef`. The `C_OscHalcDefBase` base fields are inlined directly into `C_OscHalcDefFiler::save/load` (no separate Base filer per the "no new abstractions" rule). All sub-object loads use `C_OscJsonUtil::h_GetObject` for missing-key / wrong-type validation; no blind `["key"].toObject()` calls anywhere. CMake-wired in alphabetical order alongside the existing HALC files. All ten standalone-compile-clean with `-Wall -Wextra` against host Qt 6. The on-class `ToJsonObject` / `ToQDom*` / `ToQDataStream` zombies on the HALC data classes and the 16 `_Serialization.cpp` siblings were intentionally **left in place** — they're the load-bearing zombie graph that gets stripped in 8d after 8b and 8c also land.
**Phase B step 8a — review notes:** First-pass review caught: a critical `reinterpret_cast`/`const_cast` UB in `C_OscHalcDefContentFiler::load` (was using a private-field cast hack to set `me_ComplexType`; fixed to call the existing public `SetComplexType` setter); missing `string_value` serialization for `eCT_STRING` complex type (would silently lose data through round-trip; fixed); six `h_GetU64` calls in `C_OscHalcDefFiler::load` against fields that `save` writes as JSON numbers (round-trip broken; fixed by replacing with `h_GetU32` / `h_GetU8` writing directly into the typed fields with no `u64_Temp` intermediate); 4× duplication of type metadata in `C_OscHalcDefElementFiler` (was serializing `type` / `is_array` / `complex_type` / `enum_items` / `bitmask_items` at both the element level AND inside each `c_InitialValue`/`c_MinValue`/`c_MaxValue` content sub-object — the element-level "fields" are actually getters that delegate to the contents; fixed by removing the element-level duplicates and letting the contents be the source of truth); blind `["key"].toObject()` calls in `Element` / `ChannelUseCase` / `Domain` / `Content` (fixed to use `h_GetObject`); inconsistent `isObject()` validation in `ChannelValues` (fixed). All locked rules followed: two-method shape, snake_case keys, both copyright lines on every file, enum-as-string via `C_OscJsonUtil::T_EnumEntry`, composition by direct call only (zero `C_OscFilerUtil` dependency in any new HALC filer), no XML reads anywhere, no `reinterpret_cast`/`const_cast`/`friend`, no plan-doc references in source comments, bottom-up dependency order respected. All ten standalone-compile-clean after final fixes.
**Next:** **Phase B step 8b** — HALC Configuration tree. Five new filers (`C_OscHalcConfigParameterFiler`, `C_OscHalcConfigParameterStructFiler`, `C_OscHalcConfigChannelFiler`, `C_OscHalcConfigDomainFiler`, `C_OscHalcConfigFiler`). Same constraints as 8a: don't delete `_Serialization.cpp`, don't strip on-class methods, don't touch the standalone tree. Depends on 8a's `C_OscHalcDefDomainFiler`, `C_OscHalcDefContentFiler`, `C_OscHalcDefStructFiler`. See the "Phase B step 8 — HALC sub-stepping plan" section below for the full breakdown.

## Files created in this session (do NOT recreate)

All in `opensyde_tool/libs/opensyde_core/`:

| File | Purpose |
|---|---|
| `serialization/C_OscJsonUtil.{hpp,cpp}` | Type-safe JSON getters/setters, enum<->string template, NaN/Inf check. The only abstraction allowed in the new filer subsystem. |
| `project/system/node/C_OscNodeDataPoolContentFiler.{hpp,cpp}` | Polymorphic primitive value (10 types, scalar+array) |
| `project/system/node/C_OscNodeDataPoolDataSetFiler.{hpp,cpp}` | name + comment |
| `project/system/node/C_OscNodeDataPoolListElementFiler.{hpp,cpp}` | element with min/max/value/dataset values, factor/offset, unit, access, NVM |
| `project/system/node/C_OscNodeDataPoolListFiler.{hpp,cpp}` | list with NVM CRC, elements, datasets |
| `project/system/node/C_OscNodeDataPoolFiler.{hpp,cpp}` | **REPLACED** the broken multi-format scaffold. Pool with type, version, NVM, lists. |
| `project/system/node/can/C_OscCanSignalFiler.{hpp,cpp}` | Phase B6: signal positioning, byte order, multiplex, CANopen / J1939 specifics. |
| `project/system/node/can/C_OscCanMessageFiler.{hpp,cpp}` | Phase B6: message identity, tx method, signals array, embedded CANopen owner-interface sub-object. |
| `project/system/node/can/C_OscCanMessageContainerFiler.{hpp,cpp}` | Phase B6: per-interface tx/rx message lists + protocol-used-by-interface flag. |
| `project/system/node/can/C_OscCanProtocolFiler.{hpp,cpp}` | Phase B6: top of CAN family. Protocol type enum, data pool index, per-interface containers. |
| `project/system/node/can/can_open/C_OscCanOpenManagerMappableSignalFiler.{hpp,cpp}` | Phase B7: composes signal + datapool-element filers + auto-min-max flag. |
| `project/system/node/can/can_open/C_OscCanOpenManagerDeviceInfoFiler.{hpp,cpp}` | Phase B7: per-device CANopen settings + EDS file path/name + mappable signal list. |
| `project/system/node/can/can_open/C_OscCanOpenManagerInfoFiler.{hpp,cpp}` | Phase B7: manager-side settings + NMT error behaviour enum + per-device hash as JSON array. |
| `project/system/node/can/can_open/C_OscCanOpenManagerFiler.{hpp,cpp}` | **REPLACED** the multi-format scaffold. Two-method shape over `QHash<uint8_t, C_OscCanOpenManagerInfo>`. |
| `halc/definition/C_OscHalcDefContentBitmaskItemFiler.{hpp,cpp}` | Phase B8a: leaf — display, comment, apply-value-setting, u64 value. |
| `halc/definition/C_OscHalcDefContentFiler.{hpp,cpp}` | Phase B8a: cascade-root. Composes `C_OscNodeDataPoolContentFiler` for the base portion (since `HalcDefContent extends NodeDataPoolContent`). Adds complex-type enum, enum items, bitmask items, and string value for `eCT_STRING`. |
| `halc/definition/C_OscHalcDefElementFiler.{hpp,cpp}` | Phase B8a: id, display, comment, three content sub-objects (initial/min/max), use-case availabilities. Type metadata lives inside the content objects, NOT duplicated at the element level. |
| `halc/definition/C_OscHalcDefStructFiler.{hpp,cpp}` | Phase B8a: **REPLACED**. Composes `C_OscHalcDefElementFiler` for the base (since `Struct extends Element`) plus the nested `c_StructElements` list. |
| `halc/definition/C_OscHalcDefChannelValuesFiler.{hpp,cpp}` | Phase B8a: four parameter/input/output/status struct lists. |
| `halc/definition/C_OscHalcDefChannelAvailabilityFiler.{hpp,cpp}` | Phase B8a: leaf — value index + dependent values list. |
| `halc/definition/C_OscHalcDefChannelUseCaseFiler.{hpp,cpp}` | Phase B8a: id, display, comment, value (NodeDataPoolContent), availability list, default channels. |
| `halc/definition/C_OscHalcDefChannelDefFiler.{hpp,cpp}` | Phase B8a: leaf — just `c_Name`. |
| `halc/definition/C_OscHalcDefDomainFiler.{hpp,cpp}` | Phase B8a: composes ChannelDef + ChannelUseCase + ChannelValues x2 + Category enum. |
| `halc/definition/C_OscHalcDefFiler.{hpp,cpp}` | Phase B8a: **REPLACED**. Top of HALC Definition tree. Inlines `C_OscHalcDefBase` fields directly (safety mode enum, NVM lists, etc.) plus composes `C_OscHalcDefDomainFiler` for the domains list. |

All wired into `OPENSYDE_CORE_SOURCES` and `OPENSYDE_CORE_HEADERS` in `opensyde_tool/libs/opensyde_core/CMakeLists.txt`. All compile clean with `-Wall -Wextra` against host Qt 6.11 (validated standalone with g++).

## The pattern (locked, do not deviate)

```cpp
class C_OscFooFiler
{
public:
   static QJsonObject save(const C_OscFoo & orc_Foo);
   static int32_t     load(const QJsonObject & orc_Json, C_OscFoo & orc_Foo);
};
```

- Two static methods. Nothing else. No `LoadFromFile` / `SaveToString` / `LoadBinary` / `LoadJson` / `LoadXml` family.
- snake_case JSON keys.
- Enums via `C_OscJsonUtil::T_EnumEntry` tables, serialized as strings.
- `uint64_t` / `int64_t` as decimal strings (use `C_OscJsonUtil::h_GetU64` / `h_SetU64`). Smaller integers as JSON numbers.
- Required fields: use `C_OscJsonUtil::h_Get*` (returns `C_NO_ERR` / `C_CONFIG` / `C_RANGE`). Optional: `h_Get*Or`.
- Sub-filers compose by direct call: `c_Json["lists"].append(C_OscNodeDataPoolListFiler::save(rc_List))`.
- Only public Get/Set on data classes — never friend-access them. The on-class serialization methods (`ToJsonObject`, `FromJsonObject`, `ToQDom*`, `FromQDom*`, `ToQDataStream`, `FromQDataStream`) are zombies and get ripped out per type once their callers are migrated.
- File I/O (`QFile` / `QJsonDocument::fromJson` / `toJson(Indented)`) lives ONLY at the top of each filer hierarchy (project, system definition, view, CAN Monitor project). Lower filers never touch `QFile`.

## Hard rules (locked by Tyler)

- **No plan-doc references in source comments.** Plans get archived; references rot. CLAUDE.md is fine; `.cpp/.hpp` are not.
- **New files need both copyright lines:**
  ```
  \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
  ```
- **No backward compat.** No format-version branching. No V1/V2/V3 paths. No multi-format scaffolds.
- **No new abstractions.** No filer base classes. No CRTP. No visitors. No reflection helpers. The only abstraction is `C_OscJsonUtil`.
- **HALC vendor XML never enters the main tool** — it's converted by a separate offline tool (out of scope for this rewrite).
- **The build is currently broken** (was broken before this work started) and will stay broken until caller migration completes. That's expected; don't try to "fix" it by reintroducing the legacy methods.

## Critical context the next session needs to know

### 1. The dev branch `C_OscNodeDataPoolFiler` was already broken before I touched it.

The previous (now-replaced) version had:
- `h_LoadDataPool` and `h_SaveDataPool` as **infinite-recursion stubs** (called themselves)
- A binary/JSON/XML multi-format scaffold (`h_LoadFile`, `h_LoadBinary`, `h_LoadJson`, `h_LoadXml`, etc.) that **had zero external callers in the entire codebase** — pure dead code, cargo-culted from `C_OscSupSignatureFiler`
- **Missing entirely:** `h_LoadDataPoolContentV1`, `h_SaveDataPoolContentV1`, `h_LoadDataPoolElement`, `h_SaveDataPoolElement`, `h_StringToDataPool`, `h_DataPoolToString` — every method that real callers actually use

The header had a misleading comment line saying *"Based on a pre-existing implementation for tinyxml2"* which made me (and a previous agent) believe `C_OscXmlParserBase` was a TinyXML2 wrapper. **It is not.** `C_OscXmlParserBase` is backed by `QDomDocument` / `QDomElement` (see `C_OscXmlParser.hpp` lines 61, 64). **There is no TinyXML2 anywhere in `opensyde_core`.** The `Qt6::Xml` link is the actual dependency target for elimination.

### 2. The "FileLoadersV2 elimination" project is misnamed.

It is **not** about XML libraries. Both `*Filer` and `*FilerV2` use the same QDom backend. "V2" refers to **file format version 2** (older project files), not code version 2. The `*Filer` (without suffix) is sometimes labelled "(V3)" in its docstring — see e.g. `git show master:opensyde_tool/libs/opensyde_core/project/system/node/C_OscNodeDataPoolFiler.hpp` line 4 on master.

The real architectural refactor we're doing — clean-slate JSON-only — supersedes the FileLoadersV2 elimination entirely. None of those prior plans apply. They're archived in `plans/99_ARCHIVE/`.

### 3. The on-class serialization methods on data classes also need stripping.

Several core data classes carry their own `ToJsonObject` / `FromJsonObject` / `ToQDataStream` / `FromQDataStream` / `ToQDomDocument` / `FromQDomDocument` methods, sometimes split into sibling `*_Serialization.cpp` files (look in `halc/configuration/`, `project/system/node/can/can_open/`). These are zombies. They get deleted in the same step where the new filer's callers are migrated. Don't leave them around.

Also: data class headers themselves include `<QDomElement>`, `<QDataStream>`, `<QJsonObject>`. These pull QtXml into the world. **Strip them too** when the on-class methods get deleted. Otherwise the `Qt6::Xml` link can't be removed in Phase E.

### 4. The C_OscSystemNameMaxCharLimitChangeReportItem.hpp include path

Several data class headers `#include "C_OscSystemNameMaxCharLimitChangeReportItem.hpp"`. That file is in `project/system/`, NOT `project/system/node/`. The CMake build handles this naturally; standalone g++ compile checks need both `-I project/system` and `-I project/system/node` on the command line.

## The next task — Option A: caller migration sweep

Find every call site of the now-deleted legacy `C_OscNodeDataPoolFiler` methods and migrate them to the new filer chain.

### Methods that no longer exist (find all callers):

```
C_OscNodeDataPoolFiler::h_LoadDataPool
C_OscNodeDataPoolFiler::h_SaveDataPool
C_OscNodeDataPoolFiler::h_LoadDataPoolContentV1
C_OscNodeDataPoolFiler::h_SaveDataPoolContentV1
C_OscNodeDataPoolFiler::h_LoadDataPoolElement
C_OscNodeDataPoolFiler::h_SaveDataPoolElement
C_OscNodeDataPoolFiler::h_StringToDataPool
C_OscNodeDataPoolFiler::h_DataPoolToString
C_OscNodeDataPoolFiler::h_LoadFile
C_OscNodeDataPoolFiler::h_SaveFile
C_OscNodeDataPoolFiler::h_LoadBinary
C_OscNodeDataPoolFiler::h_SaveBinary
C_OscNodeDataPoolFiler::h_LoadJson
C_OscNodeDataPoolFiler::h_SaveJson
C_OscNodeDataPoolFiler::h_LoadXml
C_OscNodeDataPoolFiler::h_SaveXml
C_OscNodeDataPoolFiler::h_LoadFromMemoryBinary
C_OscNodeDataPoolFiler::h_LoadFromMemoryJson
C_OscNodeDataPoolFiler::h_LoadFromMemoryXml
C_OscNodeDataPoolFiler::h_SaveToMemoryBinary
C_OscNodeDataPoolFiler::h_SaveToMemoryJson
C_OscNodeDataPoolFiler::h_SaveToMemoryXml
```

### Known caller files (from earlier grep):

- `opensyde_tool/src/project_gui/system_views/C_PuiSvHandlerFiler.cpp`
- `opensyde_tool/src/project_gui/system_views/C_PuiSvHandlerFilerV1.cpp`
- `opensyde_tool/src/project_gui/system_views/C_PuiSvDashboardFiler.cpp`
- `opensyde_tool/libs/opensyde_core/project/system/node/can/can_open/C_OscCanOpenManagerFiler.cpp`
- `opensyde_tool/src/system_definition/C_SdClipBoardHelper.cpp`
- `opensyde_tool/src/system_definition/node_edit/datapools/C_SdNdeDpContentUtil.cpp`

Re-grep before starting; the list above was true mid-session and may have moved.

### Migration shape

Old (XML-cursor based):
```cpp
C_OscNodeDataPoolFiler::h_SaveDataPoolContentV1(orc_Content, orc_XmlParser);
```

New (JSON):
```cpp
const QJsonObject c_Json = C_OscNodeDataPoolContentFiler::save(orc_Content);
// then: hand c_Json off to whatever holds the surrounding JSON object,
// or if the caller itself is still XML-based, this is a "caller is also a filer
// that needs rewriting" cascade — see below
```

### Expected cascade

Most (probably all) callers are themselves filer classes that take `C_OscXmlParserBase &`. Migrating them means **rewriting them too** as JSON filers — they become Phase D work pulled forward. When you hit a caller and find it's a filer with its own `h_Save*`/`h_Load*` methods, the right move is:

1. Note the caller's filer in a TODO / task
2. Either rewrite that caller's filer in this same sweep (preferred — keeps the cascade contained), or stub it out with `Q_UNUSED` and move on (less preferred — leaves more dead callers)

Some callers (like `C_SdClipBoardHelper` for clipboard copy/paste of data pools, or `C_SdNdeDpContentUtil` for content widget round-tripping) are **leaf callers** — not filers themselves, just code that needs to convert a `C_OscNodeDataPoolContent` to/from a string/clipboard format. Those are simple to migrate: replace the XML round-trip with `QJsonDocument(C_OscNodeDataPoolContentFiler::save(content)).toJson()` and the inverse.

### Stop conditions for the next session

- All ~20 callers compile against the new filer chain, OR
- The cascade has clearly grown into a multi-day sweep that needs another check-in

If the cascade gets to "we're rewriting half the GUI's filer code," **STOP and report** — that's the moment to pause and discuss whether to prioritize completing the cascade or step back and rewrite more leaf core filers first.

## Option A — caller migration sweep: COMPLETE (2026-04-08)

All in-scope callers of the deleted legacy `C_OscNodeDataPoolFiler::*` methods now compile against the new filer chain (or are stubbed). Sibling library trees (`opensyde_syde_*`, `opensyde_cmd_line_flash_tool/`) carry their own bundled copies of `opensyde_core` with the legacy methods still intact and were intentionally left untouched.

### What was migrated cleanly

| File | Action |
|---|---|
| `opensyde_tool/src/system_definition/node_edit/datapools/C_SdNdeDpContentUtil.cpp` | `h_GetAllContentAsString` / `h_SetAllContentFromString` rewritten to use `C_OscNodeDataPoolContentFiler::save/load` with `QJsonDocument` round-trip. Standalone helper, no cascade. |

### What was stubbed (call sites marked `// TODO: ... when this filer is migrated`)

The 5 files below are themselves XML-cursor-based filers scheduled for full Phase B/D rewrite. Migrating them in this sweep would have ballooned into multi-day work. Each call site of a deleted method was replaced with a comment + skip; field stays default-initialized; surrounding XML cursor logic is untouched.

| File | Sites stubbed | Belongs to |
|---|---|---|
| `opensyde_tool/src/system_definition/C_SdClipBoardHelper.cpp` | 2 (h_SaveDataPool, h_LoadDataPool) | Clipboard helper, surrounded by XML filer chain |
| `opensyde_tool/libs/opensyde_core/.../can/can_open/C_OscCanOpenManagerFiler.cpp` | 2 (h_LoadDataPoolElement, h_SaveDataPoolElement) | Phase B step 7 |
| `opensyde_tool/src/project_gui/system_views/C_PuiSvHandlerFiler.cpp` | 2 (h_*DataPoolContentV1) | Phase D step 4 |
| `opensyde_tool/src/project_gui/system_views/C_PuiSvDashboardFiler.cpp` | 10 (h_*DataPoolContentV1, h_StringToDataPool, h_DataPoolToString) | Phase D step 3 |
| `opensyde_tool/src/project_gui/system_views/C_PuiSvHandlerFilerV1.cpp` | 8 (h_*DataPoolContentV1, h_StringToDataPool, h_DataPoolToString) | V1 zombie — file should be deleted |

Stale `#include "C_OscNodeDataPoolFiler.hpp"` removed from each of those 5 files.

### Pre-existing breakage NOT touched

`opensyde_tool/src/system_definition/C_SdClipBoardHelper.cpp` has **14+ references to a nonexistent `C_OscNodeDataPoolFilerV2::*` class** (the source files under `project/system/FileLoadersV2/C_OscNodeDataPoolFilerV2.{cpp,hpp}` no longer exist — they were deleted in an earlier commit, but only stale `.cpp.o` artifacts remain in `temp_*/`). These references make `C_SdClipBoardHelper.cpp` independently broken in dev. Out of scope for Option A. Next session should grep `C_OscNodeDataPoolFilerV2` and decide whether the whole `C_SdClipBoardHelper` clipboard family gets stubbed/rewritten as part of Phase D or treated as separate cleanup.

Also: `C_PuiSvHandlerFilerV1.{cpp,hpp}` is still on disk and still listed in `opensyde_tool/pjt/openSYDE/CMakeLists.txt` (lines 886, 1703). The "no backward compat" rule says it should be deleted, but doing so cascades into structural changes in `C_PuiSvHandler.cpp` (the `oq_UseDeprecatedV1Format` save path around line 120 and the `if (s32_FileVersion == 1)` load branch around line 3411). Deferred — see open tasks below.

## Open tasks (post-Option A, post-Phase-B6, post-Phase-B7)

1. **DO NOT strip on-class serialization from `C_OscNodeDataPoolContent` yet** (corrected 2026-04-08 after attempting it). The data class is the leaf of a deeply connected zombie graph: `C_OscHalcDefContent` inherits from it and that inheritance chains through ~17 `_Serialization.cpp` files in `halc/definition/`, `halc/configuration/`, `halc/configuration/standalone/` plus `C_OscFilerUtil.hpp` template helpers (CRTP-shaped) and several other top-level types (`C_OscView*`, `C_OscSystemBus`, `C_OscNode*`, `C_OscCanProtocol`, `C_OscTargetSupportPackage*`). They all call each other through on-class methods. Stripping the leaf alone breaks the whole graph. The correct order is to land Phase B steps 6→14 bottom-up; the on-class zombies on each type get deleted when its specific filer's last on-class caller dies. The `C_OscNodeDataPoolContent` strip becomes trivial after Phase B step 8d (HALC big strip) — at which point the entire HALC `_Serialization.cpp` graph dies in one move. **See the "Phase B step 8 — HALC sub-stepping plan" section below for the full breakdown.**
2. **Delete `C_PuiSvHandlerFilerV1.{cpp,hpp}`** along with the V1 branches in `C_PuiSvHandler.cpp` (`oq_UseDeprecatedV1Format` and `s32_FileVersion == 1`) and the CMake entries. No backward compat per the locked rules. Best done as part of, or just before, the Phase D rewrite of `C_PuiSvHandlerFiler`.
3. **Triage `C_SdClipBoardHelper.cpp`** — pre-existing `C_OscNodeDataPoolFilerV2::*` references (see above). Likely the entire file's clipboard XML round-trips need to be migrated or stubbed in one pass.
4. **Phase B step 8 (HALC)** — sub-stepped 8a/8b/8c/8d. See the next section for the detailed plan. This is the next coding step.

Earlier completed tasks (#1–9) covered Phase A foundation and Phase B steps 1–5. Re-derive those from the file list above; don't trust stale task IDs after compaction.

## Phase B step 8 — HALC sub-stepping plan

**Read this entire section before touching any HALC file.** The plan splits the canonical "Phase B step 8" item into four sub-steps so each can finish in one focused session. Sub-steps 8a/8b/8c are independent forward progress (can be done in any order, though the order below is recommended). Sub-step 8d is the coordinated cleanup that closes the on-class zombie debt for HALC *and* `C_OscNodeDataPoolContent` and depends on 8a+8b+8c being done first.

### Why this is sub-stepped

The HALC tree is the load-bearing cluster the rest of the rewrite has been deferring to. Surveying the actual files:
- ~14 HALC data classes across 3 sub-trees (`halc/definition/`, `halc/configuration/`, `halc/configuration/standalone/`)
- 17 `_Serialization.cpp` zombie files that all call each other through on-class `ToJsonObject` / `FromJsonObject` / `ToQDom*` / `FromQDom*` / `ToQDataStream` / `FromQDataStream` methods on the data classes
- 4 existing top-level filer files (`C_OscHalcDefFiler`, `C_OscHalcDefStructFiler`, `C_OscHalcConfigFiler`, `C_OscHalcConfigStandaloneFiler`) that are paper-thin wrappers around `C_OscFilerUtil::h_LoadList*<T>` / `h_SaveList*<T>` template helpers, which in turn call the on-class methods
- 4 external GUI callers that need updating
- The cascade root: `C_OscHalcDefContent : public C_OscNodeDataPoolContent` — the leaf of the data pool family inherits from a HALC type, so the data pool family's on-class strip is gated on this work

Doing this all in one session is honestly 6–12 hours of focused work — too big for a single back-and-forth without running out of context mid-sweep. The sub-stepping below splits it into pieces each roughly the size of Phase B6 or B7.

### Files in scope

Run `find opensyde_tool/libs/opensyde_core/halc -type f \( -name '*.hpp' -o -name '*.cpp' \) | sort` for the authoritative list. As of 2026-04-08:

**Definition tree** — `opensyde_tool/libs/opensyde_core/halc/definition/`:
- `base/C_OscHalcDefBase.{hpp,cpp}` + `base/C_OscHalcDefBase_Serialization.cpp` (abstract base class for both `C_OscHalcDef` and `C_OscHalcConfig`)
- `C_OscHalcDef.{hpp,cpp}` + `C_OscHalcDef_Serialization.cpp` (top of definition tree, extends `C_OscHalcDefBase`)
- `C_OscHalcDefDomain.{hpp,cpp}` + `C_OscHalcDefDomain_Serialization.cpp`
- `C_OscHalcDefChannelDef.{hpp,cpp}` + `C_OscHalcDefChannelDef_Serialization.cpp` (just a name string — leaf)
- `C_OscHalcDefChannelUseCase.{hpp,cpp}` + `C_OscHalcDefChannelUseCase_Serialization.cpp` (uses `C_OscNodeDataPoolContent` value + availability list)
- `C_OscHalcDefChannelAvailability.{hpp,cpp}` + `C_OscHalcDefChannelAvailability_Serialization.cpp` (leaf — just uint32 + list)
- `C_OscHalcDefChannelValues.{hpp,cpp}` + `C_OscHalcDefChannelValues_Serialization.cpp` (4× lists of `C_OscHalcDefStruct`)
- `C_OscHalcDefStruct.{hpp,cpp}` + `C_OscHalcDefStruct_Serialization.cpp` (**extends `C_OscHalcDefElement`**; has list of sub-elements)
- `C_OscHalcDefElement.{hpp,cpp}` + `C_OscHalcDefElement_Serialization.cpp` (uses `C_OscHalcDefContent` for initial/min/max value)
- `C_OscHalcDefContent.{hpp,cpp}` + `C_OscHalcDefContent_Serialization.cpp` (**extends `C_OscNodeDataPoolContent` — the cascade root**; adds enum items + bitmask items)
- `C_OscHalcDefContentBitmaskItem.{hpp,cpp}` + `C_OscHalcDefContentBitmaskItem_Serialization.cpp` (leaf — 4 fields)
- `C_OscHalcDefFiler.{hpp,cpp}` (existing top-level filer — multi-format scaffold, will be replaced)
- `C_OscHalcDefStructFiler.{hpp,cpp}` (**existing standalone filer of unknown role** — read it before deciding what to do with it; may be a struct-list helper used by `C_OscHalcDefFiler` internally, may have external callers, may be obsolete)

**Configuration tree** — `opensyde_tool/libs/opensyde_core/halc/configuration/`:
- `C_OscHalcConfig.{hpp,cpp}` + `C_OscHalcConfig_Serialization.cpp` (top of config tree, **extends `C_OscHalcDefBase`** — same abstract base as `C_OscHalcDef`)
- `C_OscHalcConfigDomain.{hpp,cpp}` + `C_OscHalcConfigDomain_Serialization.cpp` (**extends `C_OscHalcDefDomain`** — a config domain IS-A def domain plus channel configs)
- `C_OscHalcConfigChannel.{hpp,cpp}` + `C_OscHalcConfigChannel_Serialization.cpp`
- `C_OscHalcConfigParameterStruct.{hpp,cpp}` + `C_OscHalcConfigParameterStruct_Serialization.cpp` (**extends `C_OscHalcConfigParameter`**)
- `C_OscHalcConfigParameter.{hpp,cpp}` + `C_OscHalcConfigParameter_Serialization.cpp` (uses `C_OscHalcDefContent`)
- `C_OscHalcConfigFiler.{hpp,cpp}` (existing top-level filer — multi-format scaffold, will be replaced)
- `C_OscHalcConfigUtil.{hpp,cpp}` (utility, **NOT a filer, do not touch**)

**Standalone tree** — `opensyde_tool/libs/opensyde_core/halc/configuration/standalone/`:
- `C_OscHalcConfigStandalone.{hpp,cpp}` (top of standalone tree)
- `C_OscHalcConfigStandaloneDomain.{hpp,cpp}` (**extends `C_OscHalcConfigDomain`**)
- `C_OscHalcConfigStandaloneChannel.{hpp,cpp}` (leaf — just `QStringList c_ParameterIds`)
- `C_OscHalcConfigStandaloneFiler.{hpp,cpp}` (existing top-level filer — multi-format scaffold, will be replaced)

**Other** — `opensyde_tool/libs/opensyde_core/halc/magician/`:
- `C_OscHalcMagicianGenerator.*`, `C_OscHalcMagicianDatapoolListHandler.*`, `C_OscHalcMagicianUtil.*` — code generation, **NOT filers, do not touch in Phase B step 8**

### Critical gotchas before you start

1. **`C_OscHalcDefContent` extends `C_OscNodeDataPoolContent`.** The new `C_OscHalcDefContentFiler::save` should serialize the base-class fields (the data pool content's `type`, `is_array`, `value`) by **calling `C_OscNodeDataPoolContentFiler::save(orc_HalcContent)` and merging the result into a sub-object** — not by reaching into private base fields. Same trick for `load` (call `C_OscNodeDataPoolContentFiler::load` on a sub-object). The existing `C_OscHalcDefContent_Serialization.cpp` wraps the base under a `"base"` key — feel free to use a different key (e.g. `"datapool_content"`) since there's no backward compat. Pick one and stick to it.

2. **Inheritance chains in HALC use the same trick.** `C_OscHalcDefStruct` extends `C_OscHalcDefElement`. `C_OscHalcConfig` extends `C_OscHalcDefBase`. `C_OscHalcConfigDomain` extends `C_OscHalcDefDomain`. `C_OscHalcConfigParameterStruct` extends `C_OscHalcConfigParameter`. `C_OscHalcConfigStandaloneDomain` extends `C_OscHalcConfigDomain`. **Do not write a separate filer for the abstract base `C_OscHalcDefBase`.** Inline the base fields directly into `C_OscHalcDefFiler` and `C_OscHalcConfigFiler` (yes, the same fields appear in two filers — that's fine, "no new abstractions" rule applies). For the concrete `Def → Config` and `Config → Standalone` parent-child cases, the child filer should call the parent filer's `save`/`load` and then add the child-specific fields, same as the `HalcDefContent → NodeDataPoolContent` pattern.

3. **`C_OscFilerUtil.hpp` is poison.** Don't touch it. Don't include it in any new HALC filer. The existing HALC filers delegate to its `h_LoadList*<T>` / `h_SaveList*<T>` template helpers, which are CRTP-shaped and call into the on-class methods. The whole template helper is part of the zombie graph and dies in Phase E. Your new HALC filers compose by direct call only, like every other new filer in Phases B6/B7.

4. **Vendor XML HALC import is out of scope.** Per `JSON_Filer_Rewrite_Plan.md` section 6 ("HALC vendor XML → JSON converter") and section 9 ("Resolved decisions"), the main openSYDE tool reads only JSON HALC files. The vendor XML → JSON conversion is a separate offline tool tracked outside this rewrite. **Do not port any XML reading code from the existing filers into your new ones.** When you replace `C_OscHalcDefFiler`, the new shape is `static int32_t load(const QJsonObject&, C_OscHalcDef&)` — JSON only, no XML, no `QFile`, no extension detection.

5. **`C_OscHalcDefStructFiler` is suspicious.** It exists as its own `.{hpp,cpp}` pair (~150 lines) inside `halc/definition/`. **Read it first** — it might be a helper used by `C_OscHalcDefFiler` internally to serialize struct lists, in which case it's deletable once `C_OscHalcDefStructFiler::save/load` becomes part of `C_OscHalcDefStructFiler` (the new JSON one). Or it might have external callers — grep `C_OscHalcDefStructFiler::` across the tree before deciding. If it's just a struct-list helper, the new `C_OscHalcDefStructFiler` written in 8a replaces it cleanly.

6. **External GUI callers exist.** Grep `C_OscHalcDefFiler::|C_OscHalcConfigFiler::|C_OscHalcConfigStandaloneFiler::|C_OscHalcDefStructFiler::` across `opensyde_tool/` to confirm the full caller list. As of 2026-04-08 the in-scope callers are:
   - `opensyde_tool/src/system_definition/node_edit/halc/C_SdNdeHalcConfigImportDialog.cpp`
   - `opensyde_tool/src/system_definition/node_edit/halc/C_SdNdeHalcWidget.cpp`
   - `opensyde_tool/src/system_definition/node_edit/data_blocks/C_SdNdeDbViewWidget.cpp`
   - `opensyde_tool/src/system_definition/C_SdClipBoardHelper.cpp` (already pre-existing rot — see Option A section above; **don't try to fix the V2 references, just stub the HALC ones if needed**)

   These need their call sites updated to the new two-method API. If a call site is too tangled to migrate cleanly, stub it the same way Option A stubbed `C_PuiSvDashboardFiler` (TODO comment + Q_UNUSED-equivalent + return `C_NO_ERR`). Don't open a Phase D-shaped cascade.

### Sub-step 8a — HALC Definition tree

**Goal:** Write new JSON filers for every type in `halc/definition/` (and `halc/definition/base/`), replace `C_OscHalcDefFiler` and `C_OscHalcDefStructFiler` with the two-method shape, wire CMake, validate. Do NOT delete any `_Serialization.cpp` file. Do NOT strip on-class methods on any data class. Do NOT touch `C_OscHalcConfig*` or `C_OscHalcConfigStandalone*`.

**Filer write order (bottom-up so each new filer only depends on previously-written ones):**
1. `C_OscHalcDefContentBitmaskItemFiler` (leaf — 4 fields: `c_Display`, `c_Comment`, `q_ApplyValueSetting`, `u64_Value`. Note `u64_Value` uses `C_OscJsonUtil::h_SetU64`/`h_GetU64` per the locked convention.)
2. `C_OscHalcDefContentFiler` (cascade root — composes `C_OscNodeDataPoolContentFiler` for the base portion; adds `complex_type` enum [`plain`/`enum`/`bit_mask`/`string`], an `enum_items` array of `{display_name, value}` where `value` is a `C_OscNodeDataPoolContentFiler::save` result, and a `bitmask_items` array of `C_OscHalcDefContentBitmaskItemFiler::save` results)
3. `C_OscHalcDefElementFiler` (uses Content; serializes `c_Id`, `c_Display`, `c_Comment`, `c_InitialValue`, `c_MinValue`, `c_MaxValue`, `c_UseCaseAvailabilities`)
4. `C_OscHalcDefStructFiler` — **REPLACES the existing file**. The new one composes `C_OscHalcDefElementFiler` for the base portion (since `Struct extends Element`) and adds the `c_StructElements` list.
5. `C_OscHalcDefChannelValuesFiler` (4× struct lists: `c_Parameters`, `c_InputValues`, `c_OutputValues`, `c_StatusValues`)
6. `C_OscHalcDefChannelAvailabilityFiler` (leaf — `u32_ValueIndex` + `c_DependentValues` list)
7. `C_OscHalcDefChannelUseCaseFiler` (uses Availability + `C_OscNodeDataPoolContentFiler` for `c_Value`)
8. `C_OscHalcDefChannelDefFiler` (leaf — just `c_Name`)
9. `C_OscHalcDefDomainFiler` (composes ChannelDef + ChannelUseCase + ChannelValues x2 + `E_VariableSelector` and `E_Category` enums via `C_OscJsonUtil::T_EnumEntry` tables)
10. `C_OscHalcDefFiler` — **REPLACES the existing file**. The new one is the two-method shape over `C_OscHalcDef`. Inlines the `C_OscHalcDefBase` fields directly (`u32_ContentVersion`, `c_DeviceName`, `c_FileString`, `c_OriginalFileName`, `e_SafetyMode` enum, `u8_NumConfigCopies`, `q_NvmBasedConfig`, the four `c_NvmSafe*`/`c_NvmNonSafe*` lists, the four `u32_NvmReservedListSize*` fields). Then composes `C_OscHalcDefDomainFiler` for the `mc_Domains` list. **Note: `mc_Domains` is private** — you may need to add a public getter/setter or use a friend declaration; check the existing on-class `ToJsonObject` to see how it accesses the field, but per the pattern rule "Only public Get/Set on data classes — never friend-access them", prefer adding a public accessor.

**Files to NOT touch in 8a:**
- Any `_Serialization.cpp` file (they stay zombies until 8d)
- Any on-class method on any HALC data class (they stay zombies until 8d)
- Any QtXml/QDataStream/QJson include in any data class header (they stay until 8d)
- `C_OscHalcConfig*`, `C_OscHalcConfigStandalone*`, `C_OscFilerUtil.hpp`, `C_OscNodeDataPoolContent.{hpp,cpp}`
- The HALC magician code (`halc/magician/`)
- External GUI callers (those get updated in 8d, after the API surface is stable)

**CMake wiring:** Add the 9 new `*Filer.{cpp,hpp}` pairs to `OPENSYDE_CORE_SOURCES` and `OPENSYDE_CORE_HEADERS` in `opensyde_tool/libs/opensyde_core/CMakeLists.txt`, in alphabetical position alongside the existing HALC files. The two replaced files (`C_OscHalcDefFiler`, `C_OscHalcDefStructFiler`) are already listed — leave their entries alone.

**Validation command** (standalone g++ syntax-check, run from repo root):
```bash
CXXFLAGS="-std=c++17 -fsyntax-only -Wall -Wextra -fPIC $(pkg-config --cflags Qt6Xml Qt6Core)"
INCS="-I opensyde_tool/libs/opensyde_core \
      -I opensyde_tool/libs/opensyde_core/serialization \
      -I opensyde_tool/libs/opensyde_core/stwtypes \
      -I opensyde_tool/libs/opensyde_core/stwerrors \
      -I opensyde_tool/libs/opensyde_core/scl \
      -I opensyde_tool/libs/opensyde_core/project/system \
      -I opensyde_tool/libs/opensyde_core/project/system/node \
      -I opensyde_tool/libs/opensyde_core/project/system/node/can \
      -I opensyde_tool/libs/opensyde_core/imports \
      -I opensyde_tool/libs/opensyde_core/halc/definition \
      -I opensyde_tool/libs/opensyde_core/halc/definition/base \
      -I opensyde_tool/libs/opensyde_core/halc/configuration \
      -I opensyde_tool/libs/opensyde_core/halc/configuration/standalone"

for f in opensyde_tool/libs/opensyde_core/halc/definition/C_OscHalcDef*Filer.cpp; do
  echo "=== $f ==="
  g++ $CXXFLAGS $INCS "$f" 2>&1 | tail -25
done
```

Every file in the loop should produce just the `=== filename ===` header with no error output. If any file fails, fix it before moving on. The Qt6Xml include is needed only because the **data class** headers still pull in `<QDomDocument>` etc. for the on-class zombies — your new filer code should not include any QtXml header itself.

**Stop conditions:**
- All 9 new filers compile clean → write commit → DONE with 8a
- Build broken in a way you don't understand → STOP, write what you found, commit any partial work as WIP
- The cascade unexpectedly grows beyond definition-tree files → STOP and report

**Commit message hint:** `[Filers] JSON filer rewrite: Phase B step 8a (HALC Definition tree)`

### Sub-step 8b — HALC Configuration tree

**Goal:** Write new JSON filers for every type in `halc/configuration/` (excluding the standalone subdirectory and `C_OscHalcConfigUtil`), replace `C_OscHalcConfigFiler` with the two-method shape, wire CMake, validate. Same constraints as 8a.

**Prerequisite:** 8a must be done first (this depends on `C_OscHalcDefDomainFiler`, `C_OscHalcDefContentFiler`, `C_OscHalcDefStructFiler` from 8a).

**Filer write order:**
1. `C_OscHalcConfigParameterFiler` (uses `C_OscHalcDefContentFiler` for `c_Value`; adds `c_Comment`)
2. `C_OscHalcConfigParameterStructFiler` — composes `C_OscHalcConfigParameterFiler` for the base portion (since `ParameterStruct extends Parameter`) and adds the `c_ParameterElements` list
3. `C_OscHalcConfigChannelFiler` (composes ParameterStruct list; adds `c_Name`, `c_Comment`, `q_SafetyRelevant`, `u32_UseCaseIndex`)
4. `C_OscHalcConfigDomainFiler` — composes `C_OscHalcDefDomainFiler` for the base portion (since `ConfigDomain extends DefDomain`) and adds the `c_DomainConfig` channel + `c_ChannelConfigs` list
5. `C_OscHalcConfigFiler` — **REPLACES the existing file**. Two-method shape over `C_OscHalcConfig`. Inlines the `C_OscHalcDefBase` fields just like `C_OscHalcDefFiler` does in 8a (yes, these will be duplicated lines across the two top-level filers — that's correct per the "no new abstractions" rule). Composes `C_OscHalcConfigDomainFiler` for the `mc_Domains` list. Same private-field caveat as `C_OscHalcDef.mc_Domains`.

**Files to NOT touch in 8b:** Same constraints as 8a, plus don't touch anything in `halc/definition/` that you didn't already touch in 8a.

**CMake wiring + validation:** Same shape as 8a. Update the `for` loop to iterate `halc/configuration/C_OscHalcConfig*Filer.cpp`.

**Stop conditions / commit message:** Same shape as 8a. Commit message: `[Filers] JSON filer rewrite: Phase B step 8b (HALC Configuration tree)`

### Sub-step 8c — HALC Standalone tree

**Goal:** Write new JSON filers for the standalone HALC config types, replace `C_OscHalcConfigStandaloneFiler` with the two-method shape, wire CMake, validate.

**Prerequisite:** 8b must be done first (this depends on `C_OscHalcConfigDomainFiler` from 8b).

**Filer write order:**
1. `C_OscHalcConfigStandaloneChannelFiler` (leaf — just `c_ParameterIds` QStringList → JSON array of strings)
2. `C_OscHalcConfigStandaloneDomainFiler` — composes `C_OscHalcConfigDomainFiler` for the base portion (since `StandaloneDomain extends ConfigDomain`) and adds the `c_StandaloneChannels` list
3. `C_OscHalcConfigStandaloneFiler` — **REPLACES the existing file**. Two-method shape over `C_OscHalcConfigStandalone`. Serializes `c_DeviceType`, `u32_DefinitionContentVersion`, and the `c_Domains` list.

**Files to NOT touch in 8c:** Same constraints as 8a/8b.

**CMake wiring + validation:** Same shape as 8a/8b.

**Stop conditions / commit message:** `[Filers] JSON filer rewrite: Phase B step 8c (HALC Standalone tree)`

### Sub-step 8d — The big strip (HALC + `C_OscNodeDataPoolContent`)

**Goal:** Now that all HALC types have new JSON filers, the entire on-class zombie graph for HALC + the leaf zombie on `C_OscNodeDataPoolContent` can die in one coordinated move. This sub-step has the largest blast radius — read the whole sub-step before starting any deletions.

**Prerequisites:** 8a + 8b + 8c all complete. Verify by running the validation command from each sub-step and confirming clean compilation.

**Pre-flight verification (do these before deleting anything):**

1. **Confirm zero new-filer dependency on `C_OscFilerUtil`:**
   ```bash
   grep -rn 'C_OscFilerUtil\|h_LoadList\|h_SaveList' \
     opensyde_tool/libs/opensyde_core/halc/ \
     | grep -v _Serialization.cpp | grep -v 'old.*Filer'
   ```
   Should return zero hits in your new `*Filer.cpp` / `*Filer.hpp` files. If any new filer accidentally pulled in `C_OscFilerUtil`, fix that first.

2. **Confirm no caller of any HALC on-class method outside the zombie graph:**
   ```bash
   grep -rn '\.\(ToJsonObject\|FromJsonObject\|ToQDom\|FromQDom\|ToQDataStream\|FromQDataStream\)(' \
     opensyde_tool/ --include='*.cpp' --include='*.hpp' \
     | grep -i halc | grep -v _Serialization.cpp
   ```
   Should return zero hits. If any non-Serialization file calls a HALC `.ToJsonObject()` etc., that caller needs migration before you can strip.

3. **Confirm no caller of `C_OscNodeDataPoolContent::ToJsonObject` etc. outside the HALC zombie graph:**
   ```bash
   grep -rn 'C_OscNodeDataPoolContent::\(ToJsonObject\|FromJsonObject\|ToQDom\|FromQDom\|ToQDataStream\|FromQDataStream\|h_LoadFromStream\|h_SaveToStream\)' \
     opensyde_tool/
   ```
   Expected: hits only in `halc/definition/C_OscHalcDefContent_Serialization.cpp`. If hits appear elsewhere (e.g. in some non-HALC `_Serialization.cpp`), **STOP** — the cascade is wider than this sub-step is scoped for. Report what you found.

**Deletions (do all of these in one commit so the build state is consistent):**

1. **Delete the 17 `_Serialization.cpp` files** in the HALC tree:
   ```
   opensyde_tool/libs/opensyde_core/halc/definition/base/C_OscHalcDefBase_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/definition/C_OscHalcDef_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/definition/C_OscHalcDefDomain_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/definition/C_OscHalcDefChannelDef_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/definition/C_OscHalcDefChannelUseCase_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/definition/C_OscHalcDefChannelAvailability_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/definition/C_OscHalcDefChannelValues_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/definition/C_OscHalcDefStruct_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/definition/C_OscHalcDefElement_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/definition/C_OscHalcDefContent_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/definition/C_OscHalcDefContentBitmaskItem_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/configuration/C_OscHalcConfig_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/configuration/C_OscHalcConfigDomain_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/configuration/C_OscHalcConfigChannel_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/configuration/C_OscHalcConfigParameterStruct_Serialization.cpp
   opensyde_tool/libs/opensyde_core/halc/configuration/C_OscHalcConfigParameter_Serialization.cpp
   ```
   (Verify the list with `find opensyde_tool/libs/opensyde_core/halc -name '*_Serialization.cpp'` — there should be exactly 16 in `halc/` plus the one under `base/`. Standalone tree currently has none. The exact count may differ if previous sessions added or removed any.)

2. **Strip on-class serialization methods from every HALC data class header AND its `.cpp`.** For each of these data classes, remove the `ToQDataStream` / `FromQDataStream` / `ToJsonObject` / `FromJsonObject` / `ToQDomElement` / `FromQDomElement` (or `ToQDomDocument`/`FromQDomDocument` — the names vary across files) declarations from the `.hpp` and (where they exist) any inline definitions in the `.cpp`. Most of the implementations live in the `_Serialization.cpp` files you just deleted, so the `.cpp` files won't have much to strip — but verify with grep.

   Data classes to strip:
   - `halc/definition/base/C_OscHalcDefBase.hpp` (note: the methods are `virtual` here; child classes override them)
   - `halc/definition/C_OscHalcDef.hpp`
   - `halc/definition/C_OscHalcDefDomain.hpp`
   - `halc/definition/C_OscHalcDefChannelDef.hpp`
   - `halc/definition/C_OscHalcDefChannelUseCase.hpp`
   - `halc/definition/C_OscHalcDefChannelAvailability.hpp`
   - `halc/definition/C_OscHalcDefChannelValues.hpp`
   - `halc/definition/C_OscHalcDefStruct.hpp`
   - `halc/definition/C_OscHalcDefElement.hpp`
   - `halc/definition/C_OscHalcDefContent.hpp`
   - `halc/definition/C_OscHalcDefContentBitmaskItem.hpp`
   - `halc/configuration/C_OscHalcConfig.hpp`
   - `halc/configuration/C_OscHalcConfigDomain.hpp`
   - `halc/configuration/C_OscHalcConfigChannel.hpp`
   - `halc/configuration/C_OscHalcConfigParameterStruct.hpp`
   - `halc/configuration/C_OscHalcConfigParameter.hpp`
   - `halc/configuration/standalone/C_OscHalcConfigStandalone.hpp`
   - `halc/configuration/standalone/C_OscHalcConfigStandaloneDomain.hpp`
   - `halc/configuration/standalone/C_OscHalcConfigStandaloneChannel.hpp`

3. **Strip QtXml/QDataStream/QJson includes from every HALC data class header.** While stripping the methods, also remove `#include <QDataStream>`, `#include <QDomDocument>`, `#include <QDomElement>`, `#include <QJsonObject>` lines that are no longer needed. Be careful: a few headers may legitimately still need `<QJsonObject>` if the data class has some other JSON-using member (unlikely but check). Compile after each strip to catch over-removal.

4. **Strip on-class serialization from `C_OscNodeDataPoolContent`.** This is the pre-existing task #1 from the open tasks list — now unblocked by the HALC strip. Touch:
   - `opensyde_tool/libs/opensyde_core/project/system/node/C_OscNodeDataPoolContent.hpp` — remove the `ToQDataStream` / `FromQDataStream` / `ToJsonObject` / `FromJsonObject` / `ToQDomDocument` / `FromQDomDocument` declarations + the static `h_LoadFromStream` / `h_SaveToStream` declarations + the `<QDataStream>` / `<QDomElement>` / `<QJsonObject>` includes (replace with `<QByteArray>` if needed for the `mc_Data` member type).
   - `opensyde_tool/libs/opensyde_core/project/system/node/C_OscNodeDataPoolContent.cpp` — delete the corresponding implementation block (the previous attempt found the block at lines ~3307–3452 of the file before the HALC cascade blocked the strip; line numbers may have drifted, so search by method name).

5. **Update CMake.** Remove all 17 deleted `_Serialization.cpp` files from `opensyde_tool/libs/opensyde_core/CMakeLists.txt` (`OPENSYDE_CORE_SOURCES` section).

6. **Update external GUI callers** to use the new `C_OscHalcDefFiler` / `C_OscHalcConfigFiler` / `C_OscHalcConfigStandaloneFiler` two-method API. Read each caller, identify the old API call, replace with the new shape. If the caller is doing file I/O with the old multi-format scaffold (extension detection, binary/JSON/XML branching), the new shape needs the caller to handle the file I/O itself: `QFile::readAll` → `QJsonDocument::fromJson` → `C_OscHalcDefFiler::load(doc.object(), data)`. If the caller is too tangled to migrate cleanly in this sub-step, **stub it** with a TODO comment + return `C_NO_ERR` (Option A precedent) — don't open a Phase D-shaped cascade.

7. **Verify `C_OscFilerUtil.hpp` is now unused by HALC code.** It may still be used by other callers (e.g. View, SystemBus) that aren't migrated yet — that's fine. Don't delete `C_OscFilerUtil.hpp` itself in 8d; that's a Phase E task.

**Validation after the deletions:**
- Run the standalone-compile loop from 8a/8b/8c again for every new HALC filer; all should still compile clean
- Additionally compile-check `C_OscNodeDataPoolContent.cpp` standalone (the .cpp may not be self-contained — the previous Phase B5 work managed; re-derive if needed)
- Grep for any orphaned reference to a deleted symbol: `grep -rn 'ToJsonObject\|FromJsonObject' opensyde_tool/libs/opensyde_core/halc/` should return zero hits
- Grep for the freshly-stripped C_OscNodeDataPoolContent methods: `grep -rn 'C_OscNodeDataPoolContent::\(ToJsonObject\|FromJsonObject\|ToQDom\|FromQDom\|ToQDataStream\|FromQDataStream\)' opensyde_tool/` should return zero hits

**Stop conditions:**
- Pre-flight verification (steps 1–3) shows unexpected callers → STOP, report what's still using the methods
- During stripping, an external caller breaks in a way that needs more than a simple migration → STOP, commit what's done so far as WIP, report
- The grep validations after deletion show orphaned references → fix them in this same sub-step, do not commit a half-done strip

**Commit message hint:** `[Filers] JSON filer rewrite: Phase B step 8d (strip HALC + C_OscNodeDataPoolContent zombies)`

### After Phase B step 8 is fully done

The on-class strip cascade is then mostly resolved. Phase B steps 9–13 (paramset, data logger, X-app, device definition, node squad/comm/properties, system bus) should be much easier because their `_Serialization.cpp` graphs no longer share state with HALC. Phase B step 14 (`C_OscNode`) still composes everything and lands last per the canonical plan.

`C_OscFilerUtil.hpp` likely still has callers from the not-yet-migrated trees (`C_OscView*`, `C_OscSystemBus`, `C_OscTargetSupportPackage*`) — leave it alone until those filers are rewritten in their own Phase B / Phase C steps. Final deletion is a Phase E task.

### Honest self-assessment for the agent doing this work

- **8a** is the biggest piece (10 new filers, possibly the most novel patterns because of the inheritance + the cascade-root content type). Allow plenty of context budget for it.
- **8b** is medium (5 new filers, mostly composing 8a's outputs).
- **8c** is small (3 new filers, mostly composing 8b's outputs).
- **8d** has the largest blast radius and the highest risk of unexpected dependencies. The pre-flight verification steps exist specifically to catch surprises before any deletion.
- **If you're partway through 8a and running low on context, STOP, commit what's clean, and write a sub-handoff in this section.** Don't try to push through to a "natural" stopping point — the pattern is bottom-up, so any leaf-most clean filer pair is a valid checkpoint.
- **The bottom-up filer order matters for compile validation.** If you write `C_OscHalcDefDomainFiler` before `C_OscHalcDefChannelDefFiler`, the syntax check will fail because the include doesn't exist yet. Write strictly bottom-up or have a plan for the dependency order.

## Memories already persisted

In `~/.claude/projects/-home-tyler-Projects-openSYDE/memory/`:
- `feedback_no_plan_links_in_source.md` — no plan-doc references in `.cpp/.hpp`
- `project_copyright_block.md` — both STW and Elytron Defense lines on new files

## Tyler's communication style (observed this session)

- Direct. Calls out wrong premises immediately ("Are you sure V2 is the older one?" — and was right).
- Wants proactive commitment: when an agent goes off-rails, the right move is to revert and replan, not patch.
- Comfortable with breaking changes ("no backward compat required or desired"). Don't bikeshed about user impact when he's already accepted it.
- Asks for big-picture design conversations before approving large work. Gave me a clean-slate mandate after I walked him through XML library tradeoffs honestly.
- Wants real plans in `plans/00_ACTIVE/`, archived plans in `plans/99_ARCHIVE/`, not in `01_COMPLETED/` (which is for finished work).
- Notices when feedback should be persisted ('"so those can be ripped out cleanly later" <- make a note somewhere to actually do that').
