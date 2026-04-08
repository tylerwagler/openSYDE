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
**Next:** **Phase B step 8** — HALC. This is the big one. `C_OscHalcDef*` (definition) + `C_OscHalcConfig*` (configuration). It's the load-bearing cluster of `_Serialization.cpp` files (~17 of them) that block the `C_OscNodeDataPoolContent` strip and most of the other strips. Once step 8 lands, the entire HALC zombie graph dies in one move and most of Phase B's "leave the on-class methods in place" debt resolves itself. The `C_OscHalcDefContent` inheritance chain through `C_OscNodeDataPoolContent` makes this nontrivial — read the canonical plan section 6 ("HALC vendor XML → JSON converter") and section 9 ("Resolved decisions") before starting, since the vendor XML import path goes to a separate offline tool and the main filer becomes JSON-only.

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

## Open tasks (post-Option A, post-Phase-B6)

1. **DO NOT strip on-class serialization from `C_OscNodeDataPoolContent` yet** (corrected 2026-04-08 after attempting it). The data class is the leaf of a deeply connected zombie graph: `C_OscHalcDefContent` inherits from it and that inheritance chains through ~17 `_Serialization.cpp` files in `halc/definition/`, `halc/configuration/`, `halc/configuration/standalone/` plus `C_OscFilerUtil.hpp` template helpers (CRTP-shaped) and several other top-level types (`C_OscView*`, `C_OscSystemBus`, `C_OscNode*`, `C_OscCanProtocol`, `C_OscTargetSupportPackage*`). They all call each other through on-class methods. Stripping the leaf alone breaks the whole graph. The correct order is to land Phase B steps 6→14 bottom-up; the on-class zombies on each type get deleted when its specific filer's last on-class caller dies. The `C_OscNodeDataPoolContent` strip becomes trivial after Phase B step 8 (HALC) — at which point the entire HALC `_Serialization.cpp` graph dies in one move.
2. **Delete `C_PuiSvHandlerFilerV1.{cpp,hpp}`** along with the V1 branches in `C_PuiSvHandler.cpp` (`oq_UseDeprecatedV1Format` and `s32_FileVersion == 1`) and the CMake entries. No backward compat per the locked rules. Best done as part of, or just before, the Phase D rewrite of `C_PuiSvHandlerFiler`.
3. **Triage `C_SdClipBoardHelper.cpp`** — pre-existing `C_OscNodeDataPoolFilerV2::*` references (see above). Likely the entire file's clipboard XML round-trips need to be migrated or stubbed in one pass.
4. **Begin Phase B step 6** (CAN message / container / protocol filers) once the next session is back.

Earlier completed tasks (#1–9) covered Phase A foundation and Phase B steps 1–5. Re-derive those from the file list above; don't trust stale task IDs after compaction.

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
