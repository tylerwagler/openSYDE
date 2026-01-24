# QString Migration Master Plan

**Created**: 2026-01-23
**Last Updated**: 2026-01-23
**Goal**: Complete migration from C_SclString to QString in opensyde_core

---

## Current Status Overview

| Build Target | Status | Notes |
|--------------|--------|-------|
| SYDEflash | ❌ FAILING | Many QString/C_SclString API mismatches discovered |
| CAN Monitor | ❓ UNKNOWN | Needs rebuild after fixes |
| Main openSYDE | ❌ FAILING | Type mismatches in multiple directories |

**Last Updated**: 2026-01-24

**Key Discovery**: The migration is more extensive than initially assessed. The original builds were passing because some components weren't being compiled. A full rebuild reveals significant work remaining.

---

## Phase Structure

### Phase 1: TGL Layer Elimination ✅ COMPLETE
### Phase 4: C_SclDynamicArray → QList ✅ COMPLETE
### Phase 5: C_SclIniFile → QSettings ✅ COMPLETE
### Phase 2: C_SclString → QString 🔄 IN PROGRESS
### Phase 3: C_SclStringList → QStringList ❌ NOT STARTED

---

## Phase 2 Breakdown

### Phase 2-PREP: Utility Classes ✅ COMPLETE
- [x] C_OscUtils dual-API migration
- [x] Path variable constants migrated to QString

### Phase 2A: Data Model Layer 🔄 IN PROGRESS

**Completed:**
- [x] C_OscSystemBus::c_Name → QString
- [x] C_OscSystemBus::c_Comment → QString
- [x] C_OscSuSequences::C_DoFlash file path members → QString
- [x] Multiple filer fixes for CAN Monitor/SYDEflash

**Current Blockers (Main openSYDE build):**

#### Issue Group 1: system_update_package Type Mismatches

| File | Line | Issue | Fix Required |
|------|------|-------|--------------|
| C_OscSupNodeDefinitionFiler.cpp | 146, 150 | `mh_LoadFilesSection()` expects `vector<C_SclString>&`, gets `vector<QString>` | Update function signature |
| C_OscSupNodeDefinitionFiler.cpp | 286 | Cannot assign `C_SclString` to `QString` | Add `.ToQString()` |
| C_OscSupServiceUpdatePackageV1.cpp | 219 | `h_CreateTemporaryFolder()` expects `QString`, gets `C_SclString` | Add `.ToQString()` |
| C_OscSupServiceUpdatePackageV1.cpp | 241, 243 | `h_AppendFilesRelative()` expects `vector<C_SclString>&`, gets `vector<QString>` | Update C_OscZipFile API |
| C_OscSupServiceUpdatePackageV1.cpp | 598, 602 | Same as line 146/150 issue | Update function signature |

#### Issue Group 2: C_OscZipFile API Mismatch
- `C_OscZipFile::h_AppendFilesRelative()` takes `std::vector<C_SclString>&`
- `C_DoFlash::c_FilesToFlash` is now `std::vector<QString>`
- **Fix**: Update C_OscZipFile to use QString vectors

---

## Immediate Action Items

### Step 1: Fix system_update_package Build Errors

1. **C_OscSupNodeDefinitionFiler.hpp/.cpp**
   - Change `mh_LoadFilesSection()` to take `std::vector<QString>&` instead of `std::vector<C_SclString>&`
   - Fix internal string assignments

2. **C_OscSupServiceUpdatePackageV1.hpp/.cpp**
   - Change `mh_LoadFilesSection()` to take `std::vector<QString>&`
   - Add conversion where needed for `h_CreateTemporaryFolder()` call

3. **C_OscZipFile.hpp/.cpp**
   - Update `h_AppendFilesRelative()` to accept `std::vector<QString>&`
   - Update internal set type from `std::set<C_SclString>` to `std::set<QString>`

### Step 2: Rebuild and Fix Cascading Errors

After Step 1, rebuild and address any new errors that appear.

### Step 3: Continue Phase 2A Data Model Migration

After build is green, continue migrating remaining data classes:
- [ ] C_OscNodeProperties::c_Name
- [ ] C_OscProject members
- [ ] C_OscNode members
- [ ] C_OscSystemDefinition members

---

## File-by-File Migration Checklist

### opensyde_core/system_update_package/

| File | Status | Changes Needed |
|------|--------|----------------|
| C_OscSupNodeDefinition.hpp | ⏳ | Check string members |
| C_OscSupNodeDefinitionFiler.hpp | ❌ | Update mh_LoadFilesSection signature |
| C_OscSupNodeDefinitionFiler.cpp | ❌ | Update implementation |
| C_OscSupServiceUpdatePackageV1.hpp | ❌ | Update mh_LoadFilesSection signature |
| C_OscSupServiceUpdatePackageV1.cpp | ❌ | Update implementation + conversions |
| C_OscSupSignatureFiler.hpp | ⏳ | Check for C_SclString usage |
| C_OscSupSignatureFiler.cpp | ⏳ | Check for C_SclString usage |
| C_OscSupDefinitionFiler.cpp | ⏳ | Check for C_SclString usage |

### opensyde_core/ (root)

| File | Status | Changes Needed |
|------|--------|----------------|
| C_OscZipFile.hpp | ❌ | Update h_AppendFilesRelative to QString vectors |
| C_OscZipFile.cpp | ❌ | Update implementation |
| C_OscAesFile.hpp | ⏳ | Check for C_SclString usage |
| C_OscAesFile.cpp | ⏳ | Check for C_SclString usage |
| C_OscUtils.hpp | ✅ | Dual API complete |
| C_OscUtils.cpp | ✅ | Dual API complete |

### opensyde_core/protocol_drivers/system_update/

| File | Status | Changes Needed |
|------|--------|----------------|
| C_OscSuSequences.hpp | ⏳ | Verify C_DoFlash members are QString |
| C_OscSuSequences.cpp | ⏳ | Check implementation |

---

## Migration Patterns Reference

### Pattern 1: Simple String Member
```cpp
// Before
C_SclString c_Name;
// After
QString c_Name;
```

### Pattern 2: Vector of Strings
```cpp
// Before
std::vector<C_SclString> c_Files;
// After
std::vector<QString> c_Files;
```

### Pattern 3: Function Parameter
```cpp
// Before
void LoadFiles(std::vector<C_SclString> & orc_Files);
// After
void LoadFiles(std::vector<QString> & orc_Files);
```

### Pattern 4: C_SclString to QString Assignment
```cpp
// Before (error)
QString c_Target = c_SclStringSource;
// After
QString c_Target = c_SclStringSource.ToQString();
```

### Pattern 5: QString to C_SclString (when needed)
```cpp
// Before (error)
C_SclString c_Target = c_QStringSource;
// After
C_SclString c_Target = C_SclString::FromQString(c_QStringSource);
```

### Pattern 6: Method Replacements
| C_SclString | QString |
|-------------|---------|
| `.c_str()` | `.toUtf8().constData()` |
| `.Length()` | `.length()` |
| `.LowerCase()` | `.toLower()` |
| `.UpperCase()` | `.toUpper()` |
| `.Trim()` | `.trimmed()` |
| `C_SclString::IntToStr(n)` | `QString::number(n)` |

---

## Risk Notes

1. **Index Differences**: C_SclString uses 1-based indexing, QString uses 0-based
   - Audit all `.SubString()`, `.Pos()`, `.Insert()`, `.Delete()` calls

2. **Encoding**: C_SclString is Latin-1, QString is UTF-16
   - Generally transparent but verify file I/O

3. **Cascading Changes**: Changing a data class member affects all users
   - Build frequently to catch errors early

---

## Progress Log

### 2026-01-24 (Session)
**Files Fixed:**
- C_OscDeviceDefinitionFiler.cpp:
  - Removed `C_SclString::FromQString()` call at line 889 (logging already takes QString)
  - Fixed missing `c_Xml.CreateNodeChild("display-value", ...)` statement at line 1419
- C_OscNode.cpp - Removed `.ToQString()` from `c_Name` comparison (c_Name is already QString)
- C_OscViewFiler.cpp:
  - Changed `const std::vector<C_SclString> &` to `const std::vector<QString> &` for path variables
  - Changed `.AnsiCompare()` to `.compare()` (2 occurrences)
- C_OscCanMessageContainer.cpp - Changed `.LowerCase()` to `.toLower()` (4 occurrences)
- C_OscBuSequences.cpp - Added `#include "C_SclStringList.hpp"`
- C_OscProtocolDriverOsy.cpp - Replaced 30 `c_ErrorText.PrintFormatted(` with `c_ErrorText = QString::asprintf(`
- CXFLProtocol.cpp - Fixed `.Length()` → `.length()` and `.c_str()` → `.toUtf8().constData()`

**Key Findings - Scope of Migration:**
The QString migration has revealed ~166 `PrintFormatted()` calls across 24 files that need conversion to `QString::asprintf()`.

**Common Patterns Requiring Fixes:**
| C_SclString Method | QString Equivalent |
|--------------------|--------------------|
| `.PrintFormatted("fmt", ...)` | `= QString::asprintf("fmt", ...)` |
| `.Length()` | `.length()` |
| `.LowerCase()` | `.toLower()` |
| `.UpperCase()` | `.toUpper()` |
| `.c_str()` | `.toUtf8().constData()` |
| `.AnsiCompare()` | `.compare()` |
| `C_SclStringList c_List` | Need explicit `#include "C_SclStringList.hpp"` or use `QStringList` |

**Build Status:** SYDEflash still has errors - many files need systematic updates

**Remaining Work - Files with PrintFormatted calls to fix:**
- C_OscSuSequences.cpp (19 calls)
- C_CanMonProtocolXfl.cpp (15 calls)
- C_CanMonProtocolOpenSyde.cpp (15 calls)
- C_CanMonProtocolKefex.cpp (11 calls)
- C_CanMonProtocolBase.cpp (10 calls)
- C_OscBuSequences.cpp (8 calls)
- And 18 more files...

### 2026-01-23 (Evening Session)
**Files Fixed:**
- C_OscConfFileHandler.cpp - Added missing `using namespace stw::scl;` for C_SclStringList
- C_OscSuSequences.cpp - Multiple fixes:
  - Changed function signatures from `std::vector<C_SclString>&` to `std::vector<QString>&`
  - Fixed `.Trim()` → `.trimmed()` calls
  - Removed `.ToQString()` on already-QString members
  - Fixed `h_CreateTemporaryFolder` signature
- C_OscSupServiceUpdatePackageLoad.cpp - Fixed `.IsEmpty()` → `.isEmpty()`
- C_OscDeviceDefinitionFiler.cpp - Removed 9 `orc_Path.ToQString()` calls (orc_Path is already QString)
- C_OscSystemDefinition.cpp - Removed 2 `rc_Message.c_Name.ToQString()` calls (c_Name is already QString)

**Build Status:** Needs verification - build was interrupted before completion

**Next Steps:**
1. Run SYDEflash build to verify fixes
2. Run CAN Monitor build
3. Run main openSYDE build
4. Fix any remaining errors

### 2026-01-23 (Initial)
- Analyzed current build state
- SYDEflash and CAN Monitor passing
- Main openSYDE has type mismatches in system_update_package
- Created this master plan
- Next: Fix C_OscSupNodeDefinitionFiler and C_OscZipFile APIs
