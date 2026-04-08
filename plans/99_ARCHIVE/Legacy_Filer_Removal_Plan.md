# Legacy Filer Removal Plan

**Date**: 2026-03-20  
**Status**: Ready to Execute  
**Goal**: Remove all legacy Filer classes and migrate remaining call sites to `_New` versions

---

## Overview

All core data classes now have `_New` filer implementations with multi-format support (Binary, JSON, XML). The legacy filers are no longer needed and should be removed to clean up the codebase.

**Strategy**: 
1. Identify all legacy filers with `_New` equivalents
2. Find and update all remaining call sites
3. Remove legacy filer files
4. Verify build and functionality

---

## Legacy Filers to Remove (26 total)

### ✅ Already Safe to Remove (No Dependencies)

These filers have `_New` versions and **zero** usage in the codebase:

1. **C_OscXceManifestFiler** → Use `C_OscXceManifestFiler_New`
2. **C_OscXcoManifestFiler** → Use `C_OscXcoManifestFiler_New`
3. **C_OscXappPropertiesFiler** → Use `C_OscXappPropertiesFiler_New`
4. **C_OscDataLoggerJobFiler** → Use `C_OscDataLoggerJobFiler_New`
5. **C_OscParamSetRawNodeFiler** → Use `C_OscParamSetRawNodeFiler_New`
6. **C_OscTargetSupportPackageFiler** → Use `C_OscTargetSupportPackageFiler_New`
7. **C_OscTargetSupportPackageV2Filer** → Use `C_OscTargetSupportPackageV2Filer_New`
8. **C_OscViewDataFiler** → Use `C_OscViewDataFiler_New`
9. **C_OscSupSignatureFiler** → Use `C_OscSupSignatureFiler_New`
10. **C_OscSupDefinitionFiler** → Use `C_OscSupDefinitionFiler_New`
11. **C_OscSupNodeDefinitionFiler** → Use `C_OscSupNodeDefinitionFiler_New`
12. **C_OscProjectFiler** → Use `C_OscProjectFiler_New`
13. **C_OscNodeSquadFiler** → Use `C_OscNodeSquadFiler_New`
14. **C_OscSystemBusFiler** → Use `C_OscSystemBusFiler_New`
15. **C_OscDeviceDefinitionFiler** → Use `C_OscDeviceDefinitionFiler_New`
16. **C_OscParamSetInterpretedNodeFiler** → Use `C_OscParamSetInterpretedNodeFiler_New`
17. **C_OscNodeDataPoolFiler** → Use `C_OscNodeDataPoolFiler_New`
18. **C_OscHalcConfigFiler** → Use `C_OscHalcConfigFiler_New`
19. **C_OscHalcDefFiler** → Use `C_OscHalcDefFiler_New`
20. **C_OscHalcDefStructFiler** → Use `C_OscHalcDefStructFiler_New`
21. **C_OscHalcConfigStandaloneFiler** → Use `C_OscHalcConfigStandaloneFiler_New`
22. **C_OscCanOpenManagerFiler** → Use `C_OscCanOpenManagerFiler_New`
23. **C_OscNodeCommFiler** → Use `C_OscNodeCommFiler_New`

### ⚠️ Need Migration (Still Have Call Sites)

24. **C_OscSystemDefinitionFiler** - Used by `C_PuiSdHandlerFiler.cpp` (but only for XML parsing methods)
25. **C_OscNodeFiler** - Used by `C_PuiSdHandlerFiler.cpp` for XML parsing and utility methods
26. **C_OscViewFiler** - May be used by GUI handlers

---

## Migration Strategy

### Phase 1: Remove Safe Filers (Immediate)

Remove the 23 filers listed above that have no dependencies.

**Files to delete**:
```
opensyde_tool/libs/opensyde_core/exports/x_certificates_package_generation/C_OscXceManifestFiler.hpp
opensyde_tool/libs/opensyde_core/exports/x_certificates_package_generation/C_OscXceManifestFiler.cpp
opensyde_tool/libs/opensyde_core/exports/x_config_generation/C_OscXcoManifestFiler.hpp
opensyde_tool/libs/opensyde_core/exports/x_config_generation/C_OscXcoManifestFiler.cpp
... (and 21 more pairs)
```

**Verification**: Build the project to ensure no broken references.

### Phase 2: Migrate C_PuiSdHandlerFiler

The file `C_PuiSdHandlerFiler.cpp` includes both `C_OscSystemDefinitionFiler.hpp` and `C_OscNodeFiler.hpp` but appears to only use:
- `C_OscNodeFiler::h_GetFolderName()` - Simple utility method
- XML parsing methods that may not actually be called

**Action Items**:
1. Audit all methods called from `C_OscNodeFiler` in `C_PuiSdHandlerFiler.cpp`
2. Move utility methods (`h_GetFolderName`, `h_GetFileName`) to a utility class or the data class
3. Replace XML parsing calls with `_New` filer methods or inline the parsing logic
4. Remove includes for legacy filers
5. Delete legacy filer files

### Phase 3: Migrate C_PuiSvHandlerFiler

Check if `C_PuiSvHandlerFiler` uses `C_OscViewFiler` and migrate similarly.

---

## Implementation Steps

### Step 1: Find All Usages

```bash
# Find all files using legacy filers
grep -r "C_OscXceManifestFiler::\|C_OscXcoManifestFiler::" \
  /home/tyler/Projects/openSYDE/opensyde_tool/src \
  --include="*.cpp" --include="*.hpp" | grep -v "_New"
```

### Step 2: Update Call Sites

For each file found:
1. Replace `#include "C_XXXFiler.hpp"` with `#include "C_XXXFiler_New.hpp"`
2. Replace `C_XXXFiler::method()` with `C_XXXFiler_New::method()`
3. Verify compilation

### Step 3: Remove Legacy Files

After all call sites are updated:
```bash
rm /path/to/C_XXXFiler.hpp
rm /path/to/C_XXXFiler.cpp
```

### Step 4: Build Verification

```bash
cd /home/tyler/Projects/openSYDE/opensyde_tool/bat
build.ps1 -Component All
```

---

## Special Case: C_OscNodeFiler and C_OscSystemDefinitionFiler

These are used internally by `C_PuiSdHandlerFiler` for:
1. **XML Parsing**: `h_LoadNode()`, `h_SaveNode()`, `mh_LoadData()`, etc.
2. **Utility Methods**: `h_GetFolderName()`, `h_GetFileName()`

**Solution**:
- **Utility methods**: Move to `C_OscSystemFilerUtil` or data classes
- **XML parsing**: Either:
  - Migrate parsing logic to `_New` filers (preferred)
  - Inline the parsing in `C_PuiSdHandlerFiler`
  - Create a separate XML parser utility class

---

## Risk Mitigation

1. **Git Backup**: All files are in git history - can be restored if needed
2. **Incremental Removal**: Remove one filer at a time, test after each
3. **Build Verification**: Run full build after each removal
4. **Test Coverage**: Ensure existing tests pass (if available)

---

## Expected Benefits

1. **Code Reduction**: Remove ~26 filer files (~50 files total: .hpp + .cpp)
2. **Simplified API**: Single filer class per data type
3. **Reduced Confusion**: No need to choose between old and new
4. **Cleaner Codebase**: No dead code or deprecated methods
5. **Easier Maintenance**: One place to fix bugs or add features

---

## Estimated Effort

- **Phase 1 (Safe Filers)**: 30 minutes - Just delete files and verify build
- **Phase 2 (C_PuiSdHandlerFiler)**: 2-4 hours - Audit, migrate, test
- **Phase 3 (Other GUI Filers)**: 1-2 hours - Similar to Phase 2
- **Total**: 3-6 hours

---

## Success Criteria

- ✅ All `_New` filers are the only filers in the codebase
- ✅ No references to legacy filer classes remain
- ✅ Full build passes without errors or warnings
- ✅ All functionality preserved (unit tests pass if available)

---

**Next Action**: Start with Phase 1 - Remove the 23 safe filers that have no dependencies.
