# Plan: Eliminate FileLoadersV2 Directory

## Context

The `FileLoadersV2/` directory contains 4 legacy XML-only filer classes (~4,900 lines) that predate the multi-format filer migration. They use a different XML format (child elements) than the new data class serialization (QDom attributes). All 7 new multi-format filers are complete, but V2 is still called from 45 sites for legacy file format support, element-level parsing, and type conversions.

The goal: converge on the QDom attributes format as the single canonical XML format, absorb all V2 functionality into the proper new filer/data classes, update all callers, and delete `FileLoadersV2/`.

## Key Design Decision: Single XML Format

**QDom (attributes) is the canonical format.** The data class `ToQDomElement`/`FromQDomElement` methods define the XML schema. The V2 child-element format is eliminated — callers that used `C_OscXmlParserBase` with V2 format are migrated to use QDom-based methods.

This means:
- Element-level callers (clipboard, dashboard, CANopen) switch from `C_OscXmlParserBase` to `QDomDocument`/`QDomElement`
- Legacy V1/V2 file **reading** is preserved for backward compatibility (old projects can still be opened)
- Legacy V1/V2 file **writing** is removed — new saves always use the QDom format
- `C_OscXmlParserBase` usage in filers is eliminated (it remains available for other non-filer uses)

## Approach

Bottom-up by dependency order. Each phase produces a compilable build.

### Phase 0: Remove Stale Includes — DONE (commit c19af198)
Removed unused V2 includes from 3 update package files.

### Phase 1: Absorb `C_OscSystemBusFilerV2` — DONE (commit c19af198)
Inlined V2 code into C_OscSystemBusFiler::h_LoadBus/h_SaveBus. Updated C_OscSystemDefinitionFilerV2 to call through new filer. Deleted V2 files.

### Phase 2: Absorb `C_OscNodeDataPoolFilerV2` (1,623 lines)
Highest value — 28 methods called from 6 files (31 call sites).

Three sub-tasks:

**2a: Migrate element-level callers to QDom API**
The callers (clipboard, dashboard, CANopen, content util) currently use methods like `h_LoadDataPoolContentV1(content, xmlParser)`. These need QDom equivalents.

- Add QDom-based element-level methods to `C_OscNodeDataPoolFiler`:
  - `h_LoadDataPoolContentV1(content, QDomElement)` / `h_SaveDataPoolContentV1(content, QDomDocument, elementName)`
  - `h_LoadDataPoolListElements(elements, QDomElement)` / `h_SaveDataPoolListElements(...)`
  - `h_LoadDataPoolLists(lists, QDomElement)` / `h_SaveDataPoolLists(...)`
  - `h_LoadDataPoolListDataSets(datasets, QDomElement)` / `h_SaveDataPoolListDataSets(...)`
  - `h_LoadDataPoolElement(element, QDomElement)` / `h_SaveDataPoolElement(...)`
- These call the data class serialization methods directly (e.g. `C_OscNodeDataPoolContent::FromQDomElement()`)

**2b: Migrate callers**
Update all 6 caller files to use QDom-based methods. The callers currently create a `C_OscXmlParser` (TinyXML2-based) — they need to switch to `QDomDocument`. The clipboard helper already works with XML strings, so the conversion is: parse string → QDomDocument instead of → C_OscXmlParser.

**2c: Migrate type conversion utilities**
Move `h_DataPoolToString()`/`h_StringToDataPool()` to `C_OscNodeDataPoolFiler` (these are pure enum↔string conversions, no XML involvement).

**2d: Fix broken legacy stubs**
Replace the infinite-recursion `h_LoadDataPool`/`h_SaveDataPool` with proper implementations using QDom serialization, or remove them if no callers remain after migration.

Delete V2 files, update CMakeLists.

**Files:** `C_OscNodeDataPoolFiler.hpp/.cpp`, `C_SdClipBoardHelper.cpp`, `C_PuiSvDashboardFiler.cpp`, `C_PuiSvHandlerFiler.cpp`, `C_PuiSvHandlerFilerV1.cpp`, `C_SdNdeDpContentUtil.cpp`, `C_OscCanOpenManagerFiler.cpp`, CMakeLists
**Delete:** `C_OscNodeDataPoolFilerV2.cpp/.hpp`

### Phase 3: Absorb `C_OscNodeFilerV2` (2,007 lines)
Largest file. Contains node properties, applications, datapools, com protocols, messages, signals.

- The node-level `h_LoadNode`/`h_SaveNode` taking `C_OscXmlParserBase` is only called from `C_OscSystemDefinitionFilerV2` (absorbed in Phase 4). After Phase 4, this call will be internal.
- `C_OscCanOpenManagerFiler` calls `h_LoadNodeComSignal`/`h_SaveNodeComSignal` — migrate these to use QDom-based signal serialization via `C_OscCanSignal::FromQDomElement()`/`ToQDomElement()`
- Add missing serialization methods to `C_OscNodeProperties` and `C_OscNodeComInterfaceSettings` (these have NONE currently — needed for complete node QDom serialization)
- Fix `C_OscNode::ToQDomDocument` placeholder that says "properties/datapools/applications/com-protocols not-migrated-yet" — implement the full nested serialization
- Move any remaining V2-only logic into `C_OscNodeFiler` as internal methods for V1/V2 reading compat
- Delete V2 files

**Files:** `C_OscNodeFiler.hpp/.cpp`, `C_OscNodeCommFiler.hpp/.cpp`, `C_OscCanOpenManagerFiler.cpp`, `C_OscNode.cpp`, `C_OscNodeProperties.cpp/.hpp`, `C_OscNodeComInterfaceSettings.cpp/.hpp`, CMakeLists
**Delete:** `C_OscNodeFilerV2.cpp/.hpp`

### Phase 4: Absorb `C_OscSystemDefinitionFilerV2` (595 lines)
Last V2 class. After Phases 1-3, all V2 dependencies are gone.

- The V2 `h_LoadSystemDefinition` is called for V1/V2 format files. Absorb this as a private method in `C_OscSystemDefinitionFiler` for backward-compatible reading of old project files.
- `h_SaveSystemDefinitionFile` (called by update package V1) and `h_SaveSystemDefinition` (called by PuiSdHandlerData) — these write V2 format. Migrate them to write QDom format instead, since we're converging on one format.
- Update callers, delete V2 files

**Files:** `C_OscSystemDefinitionFiler.hpp/.cpp`, `C_OscSupServiceUpdatePackageV1.cpp`, `C_PuiSdHandlerData.cpp`, CMakeLists
**Delete:** `C_OscSystemDefinitionFilerV2.cpp/.hpp`

### Phase 5: Delete FileLoadersV2 Directory
- Delete directory
- Remove `FileLoadersV2` from CMakeLists include paths
- Full build of all 3 components
- Verify `grep -rn "FilerV2" opensyde_tool/ --include="*.cpp" --include="*.hpp"` returns zero

## Verification

After each phase:
1. All 3 components build clean (GUI, CANMonitor, SYDEflash)
2. `grep -rn "FilerV2" opensyde_tool/ --include="*.cpp" --include="*.hpp"` — count decreases each phase, reaches zero at Phase 5

After Phase 5:
- `FileLoadersV2/` directory is gone
- All XML serialization uses QDom format
- Old V1/V2 project files can still be opened (reading preserved)
- openSYDE GUI launches without crash
