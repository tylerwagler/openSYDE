# FileLoadersV2 Elimination - Current Status

**Last Updated**: 2026-04-07
**Status**: Phase 2 In Progress

## Overview

The FileLoadersV2 elimination project aims to absorb 4 legacy V2 filer classes (~4,900 lines) into their modern counterparts, eliminating the dual XML parsing strategies (TinyXML2-based C_OscXmlParser vs QDom-based serialization).

## Progress Summary

### Phase 0: Remove Stale Includes ✅ COMPLETE
- **Commit**: c19af198
- **Changes**: Removed unused `#include "C_OscSystemDefinitionFilerV2.hpp"` from 3 update package files
- **Files Modified**: 
  - C_OscSupServiceUpdatePackageBase.cpp
  - C_OscSupServiceUpdatePackageCreate.cpp
  - C_OscSupServiceUpdatePackageLoad.cpp

### Phase 1: Absorb C_OscSystemBusFilerV2 ✅ COMPLETE
- **Commit**: c19af198
- **Changes**: 
  - Inlined 230 lines from C_OscSystemBusFilerV2 into C_OscSystemBusFiler::h_LoadBus/h_SaveBus
  - Updated C_OscSystemDefinitionFilerV2 to call through new filer
  - Deleted C_OscSystemBusFilerV2.cpp/.hpp
- **Files Modified**:
  - opensyde_tool/libs/opensyde_core/project/system/C_OscSystemBusFiler.cpp (+121 lines)
  - opensyde_tool/libs/opensyde_core/project/system/FileLoadersV2/C_OscSystemDefinitionFilerV2.cpp (-10 lines)
  - opensyde_tool/libs/opensyde_core/project/system/FileLoadersV2/C_OscSystemDefinitionFilerV2.hpp (-8 lines)
- **Files Deleted**:
  - C_OscSystemBusFilerV2.cpp (230 lines)
  - C_OscSystemBusFilerV2.hpp (49 lines)

### Phase 2: Absorb C_OscNodeDataPoolFilerV2 🔄 IN PROGRESS
- **Target**: 1,623 lines from V2 to absorb
- **Caller Files**: 6 files with 31 call sites

#### Completed Work:
- ✅ QDom-based element-level methods declared in C_OscNodeDataPoolFiler.hpp
- ✅ QDom-based methods implemented in C_OscNodeDataPoolFiler.cpp (~500 lines added):
  - h_LoadDataPoolContentV1(QDomElement)
  - h_SaveDataPoolContentV1(QDomDocument)
  - h_LoadDataPoolElement(QDomElement)
  - h_SaveDataPoolElement(QDomDocument)
  - h_LoadDataPoolLists(QDomElement)
  - h_SaveDataPoolLists(QDomDocument)
  - h_LoadDataPoolListElements(QDomElement)
  - h_SaveDataPoolListElements(QDomDocument)
  - h_LoadDataPoolListDataSets(QDomElement)
  - h_SaveDataPoolListDataSets(QDomDocument)
  - mh_LoadDataPoolElementValue(QDomElement)
  - mh_SaveDataPoolElementValue(QDomDocument)

#### Remaining Work:
- ❌ Copy remaining V2 implementations to C_OscNodeDataPoolFiler.cpp:
  - h_LoadDataPool (162 lines)
  - h_SaveDataPool (45 lines)
  - h_LoadDataPoolList (78 lines)
  - h_SaveDataPoolList (41 lines)
  - h_LoadDataPoolElement (171 lines)
  - h_SaveDataPoolElement (151 lines)
  - h_LoadDataPoolLists (63 lines)
  - h_SaveDataPoolLists (33 lines)
  - h_LoadDataPoolListElements (63 lines)
  - h_SaveDataPoolListElements (33 lines)
  - h_LoadDataPoolListElementDataSetValues (50 lines)
  - h_SaveDataPoolListElementDataSetValues (30 lines)
  - h_LoadDataPoolListDataSets (51 lines)
  - h_SaveDataPoolListDataSets (14 lines)
  - h_DataPoolToString (16 lines)
  - h_StringToDataPool (24 lines)
  - h_LoadDataPoolElementType (38 lines)
  - h_SaveDataPoolElementType (23 lines)
  - h_LoadDataPoolElementValue (125 lines)
  - h_SaveDataPoolElementValue (350+ lines)

- ❌ Update C_OscXmlParserBase bridge methods to call new implementations instead of V2
- ❌ Migrate 6 caller files to QDom API (OPTIONAL - can keep bridge methods for backward compatibility)
- ❌ Delete C_OscNodeDataPoolFilerV2.cpp/.hpp
- ❌ Update CMakeLists.txt

#### Caller Files Requiring Migration:
1. **C_SdClipBoardHelper.cpp** (14 call sites)
   - Uses C_OscXmlParser + C_OscNodeDataPoolFilerV2 methods
   - Clipboard operations for data pools, lists, elements, datasets

2. **C_PuiSvDashboardFiler.cpp** (3 call sites)
   - Dashboard filer operations

3. **C_PuiSvHandlerFiler.cpp** (4 call sites)
   - System view handler filer operations

4. **C_PuiSvHandlerFilerV1.cpp** (2 call sites)
   - Legacy V1 filer operations

5. **C_SdNdeDpContentUtil.cpp** (4 call sites)
   - Data pool content utility operations

6. **C_OscCanOpenManagerFiler.cpp** (4 call sites)
   - CANopen manager filer operations

### Phase 3: Absorb C_OscNodeFilerV2 ⏳ NOT STARTED
- **Target**: 2,007 lines
- **Key Methods**:
  - h_LoadNode
  - h_SaveNode
  - h_LoadNodeComProtocols
  - h_SaveNodeComProtocols
  - h_LoadNodeComMessageContainers
  - h_SaveNodeComMessageContainers
  - h_LoadNodeComMessages
  - h_SaveNodeComMessages
  - h_LoadNodeComSignals
  - h_SaveNodeComSignals
  - Type conversion utilities

- **Dependencies**:
  - C_OscNodeDataPoolFiler (Phase 2 must complete first)
  - C_OscNodeCommFiler
  - C_OscCanOpenManagerFiler

### Phase 4: Absorb C_OscSystemDefinitionFilerV2 ⏳ NOT STARTED
- **Target**: 595 lines
- **Key Methods**:
  - h_LoadSystemDefinitionFile
  - h_SaveSystemDefinitionFile
  - h_LoadSystemDefinitionString
  - h_SaveSystemDefinitionString
  - h_LoadNodes
  - h_SaveNodes
  - h_LoadBuses
  - h_SaveBuses
  - h_LoadSystemDefinition
  - h_SaveSystemDefinition

- **Dependencies**:
  - C_OscNodeFiler (Phase 3 must complete first)
  - C_OscSystemBusFiler (Phase 1 complete)

### Phase 5: Delete FileLoadersV2 Directory ⏳ NOT STARTED
- Delete FileLoadersV2/ directory
- Remove from CMakeLists.txt include paths
- Verify grep returns zero for "FilerV2" references
- Full build verification

## Remaining V2 Files

### opensyde_tool/libs/opensyde_core/project/system/FileLoadersV2/
1. C_OscNodeDataPoolFilerV2.hpp (49 lines)
2. C_OscNodeDataPoolFilerV2.cpp (1,623 lines)
3. C_OscNodeFilerV2.hpp (56 lines)
4. C_OscNodeFilerV2.cpp (2,007 lines)
5. C_OscSystemDefinitionFilerV2.hpp (41 lines)
6. C_OscSystemDefinitionFilerV2.cpp (595 lines)

**Total**: 6 files, 4,371 lines

### Duplicate V2 Files in Other Components:
- opensyde_syde_x_gen/libs/opensyde_core/project/system/FileLoadersV2/ (6 files)
- opensyde_syde_sup/libs/opensyde_core/project/system/FileLoadersV2/ (6 files)
- opensyde_cmd_line_flash_tool/libs/osy_core/project/system/FileLoadersV2/ (4 files - missing SystemBusFilerV2)

**Total across all components**: 22 files, ~15,000 lines

## Recommendations

### Approach 1: Minimal Changes (Recommended)
1. Copy all V2 implementations into C_OscNodeDataPoolFiler.cpp
2. Update bridge methods (h_LoadDataPool/h_SaveDataPool) to call new implementations
3. Keep C_OscXmlParserBase API intact - NO caller migration needed
4. Repeat for C_OscNodeFiler and C_OscSystemDefinitionFiler
5. Delete all V2 files
6. Verify build

**Pros**:
- Minimal risk - no caller changes
- Faster completion
- Backward compatible

**Cons**:
- Still uses C_OscXmlParser (TinyXML2) internally
- Doesn't fully converge on QDom format

### Approach 2: Full Migration (Ideal but Expensive)
1. Implement QDom-based methods (already started)
2. Migrate all 6 caller files to QDom API
3. Remove C_OscXmlParserBase bridge methods
4. Delete V2 files
5. Verify build

**Pros**:
- Complete convergence on QDom
- Eliminates TinyXML2 dependency for filers
- Cleaner architecture

**Cons**:
- High risk - many caller changes
- Time consuming (6 files, 31+ call sites)
- More testing required

### Approach 3: Hybrid (Pragmatic)
1. Implement both QDom and C_OscXmlParser versions
2. Keep bridge methods for backward compatibility
3. Migrate high-priority callers first
4. Deprecate C_OscXmlParser API over time
5. Delete V2 files
6. Verify build

**Pros**:
- Balanced risk/reward
- Allows gradual migration
- Maintains compatibility

**Cons**:
- More code duplication initially
- Longer term maintenance

## Next Steps

1. **Complete Phase 2** (Choose approach):
   - Copy remaining V2 implementations to C_OscNodeDataPoolFiler.cpp
   - Update bridge methods
   - Delete C_OscNodeDataPoolFilerV2 files

2. **Complete Phase 3**:
   - Absorb C_OscNodeFilerV2 into C_OscNodeFiler
   - Delete C_OscNodeFilerV2 files

3. **Complete Phase 4**:
   - Absorb C_OscSystemDefinitionFilerV2 into C_OscSystemDefinitionFiler
   - Delete C_OscSystemDefinitionFilerV2 files

4. **Complete Phase 5**:
   - Delete FileLoadersV2 directories from all components
   - Update CMakeLists.txt files
   - Verify build

5. **Build Verification**:
   - Build openSYDE GUI
   - Build CAN Monitor
   - Build SYDEflash
   - Run tests if available

## Build Verification Checklist

After completion:
- [ ] `./build.sh -Component GUI` succeeds
- [ ] `./build.sh -Component CANMONITOR` succeeds
- [ ] `./build.sh -Component SYDEFLASH` succeeds
- [ ] No FilerV2 references in codebase
- [ ] FileLoadersV2 directories deleted
- [ ] All tests pass (if available)
