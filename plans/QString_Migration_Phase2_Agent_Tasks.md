# QString Migration Phase 2 - Agent Task Breakdown

**Created**: 2026-02-02
**Status**: Ready for agent assignment
**Prerequisite Reading**: `QString_Migration_Phase2_Comprehensive_Status.md`

---

## How to Use This Document

### For Agents:
1. Find an unclaimed task in your skill area
2. Mark the task as **[IN PROGRESS - Agent Name]**
3. Complete the work following the patterns provided
4. Run the verification step
5. Mark as **[DONE - Agent Name]** with commit hash
6. Move to next task or report blockers

### For Task Coordinators:
- Assign tasks based on agent capabilities
- Tasks are ordered by dependency (earlier tasks unblock later ones)
- Each task is designed to be completable in 1-2 hours
- Verification steps ensure quality

---

## Task Status Legend
- `[ ]` = Not started
- `[IP-name]` = In progress by agent 'name'
- `[✓]` = Completed
- `[BLOCKED]` = Waiting on dependency

---

## ⚠️ CRITICAL: String Indexing Differences

**C_SclString uses 1-based indexing. QString uses 0-based indexing.**

This is the **#1 source of bugs** during migration. PAY CLOSE ATTENTION!

### Example: Extracting first 4 characters from "12ABsubCD"

| Method | Code | Result | Correct? |
|--------|------|--------|----------|
| C_SclString (1-based) | `SubString(1, 4)` | "12AB" | ✓ |
| QString (0-based) | `mid(0, 4)` | "12AB" | ✓ CORRECT |
| QString (0-based) | `mid(1, 4)` | "2ABS" | ❌ WRONG! |

### Common Conversion Pattern:
```cpp
// C_SclString (1-based indexing)
c_String.SubString(1, 4)  // Start at position 1, take 4 chars = "12AB"

// QString (0-based indexing) - CORRECT
c_String.mid(0, 4)  // Start at position 0, take 4 chars = "12AB"

// QString (0-based indexing) - WRONG!
c_String.mid(1, 4)  // Start at position 1, take 4 chars = "2ABS"
```

### Key Rules for SubString → mid:
1. **SubString(1, n)** → **mid(0, n)** ✓ (adjust start position by -1)
2. **SubString(2, n)** → **mid(1, n)** ✓ (adjust start position by -1)
3. **NEVER blindly copy the position** - always subtract 1 from C_SclString position
4. **ALWAYS test the output** - verify you're extracting the right substring

### ⚠️ CRITICAL: indexOf() vs Pos() Indexing

**C_SclString `Pos()` uses 1-based indexing. QString `indexOf()` uses 0-based indexing.**

This is a **different type of indexing bug** - it affects **search position checks**, not just extraction!

### Example: Finding "sub" in "12ABsubCD"

| Method | Code | Returns | Meaning |
|--------|------|---------|---------|
| C_SclString (1-based) | `Pos("sub")` | **5** | "sub" starts at 5th character (1-based) |
| QString (0-based) | `indexOf("sub")` | **4** | "sub" starts at position 4 (0-based) |

### Common Conversion Pattern:
```cpp
// C_SclString (1-based search position)
if (c_String.Pos("sub") == 5)  // "sub" at 5th character (1-based)

// QString (0-based search position) - CORRECT
if (c_String.indexOf("sub") == 4)  // "sub" at position 4 (0-based)

// QString (0-based search position) - WRONG!
if (c_String.indexOf("sub") == 5)  // Looking for "sub" at wrong position!
```

### Key Rules for Pos → indexOf:
1. **Pos() == n** → **indexOf() == n-1** ✓ (subtract 1 from comparison value)
2. **NEVER blindly copy the comparison value** - always subtract 1
3. **Check ALL indexOf conditions** in files you're migrating
4. **Test with actual strings** - verify search results are correct

### Real-World Example from C_OscCanOpenObjectDictionary.cpp:
```cpp
// WRONG (was in code):
c_SectionName.indexOf("sub", Qt::CaseInsensitive) == 5  // Never matches "12ABsubCD"!

// CORRECT (fixed):
c_SectionName.indexOf("sub", Qt::CaseInsensitive) == 4  // Correctly matches "12ABsubCD"
```

**Before marking any task complete, double-check ALL substring AND search operations!**

---

# SPRINT 1: EDS/DCF Import Critical Path (HIGHEST PRIORITY)

**Goal**: Fix CAN Open device import functionality
**Dependencies**: None - can start immediately
**Verification**: `cd opensyde_tool\bat && build_syde_flash_release.bat`

---

## Task 1.1: Fix C_OscCanOpenObjectDictionary Core String Issues
**File**: `opensyde_tool/libs/opensyde_core/imports/C_OscCanOpenObjectDictionary.cpp`
**Estimated Time**: 90 minutes
**Status**: [✓]

---

### Issues to Fix:

#### Issue A: Remove Back-Conversions (Lines ~204-206, ~312-318) ✓ FIXED
**Current Problem**:
```cpp
const QString c_QsSectionName = c_Groups[s32_Section];
const C_SclString c_SectionName = c_QsSectionName.toStdString().c_str();
if (c_SectionName.Length() == 4)
```

**Fix Pattern**:
```cpp
const QString c_SectionName = c_Groups[s32_Section];
if (c_SectionName.length() == 4)
```

**Locations**: Search for `c_QsSectionName` and `toStdString().c_str()` patterns

#### Issue B: Fix ToInt() Calls (4 occurrences) 🔴 **LINE 226 STILL BROKEN**

**IMMEDIATE ACTION REQUIRED**:
Line 226 has TWO problems:
1. Still uses C_SclString methods (should cause compilation error)
2. Wrong substring position (off-by-one error)

**Current line 226**:
```cpp
const uint16_t u16_Index = static_cast<uint16_t>(("0x" + c_SectionName.mid(1, 4)).toInt(nullptr, 16));
```

**Must be changed to**:
```cpp
const uint16_t u16_Index = static_cast<uint16_t>(("0x" + c_SectionName.mid(0, 4)).toInt(nullptr, 16));
                                                                              ↑
                                                                        Change 1 to 0!
```

**Why**: C_SclString uses 1-based indexing, QString uses 0-based. See the warning at the top of this document!

**Other ToInt() locations**:
- Line 209: ✓ CORRECT - `mid(0, 4)` used properly
- Line 227: ✓ CORRECT - `mid(8, 2)` used properly (extracts subindex)
- Line 330: ✓ CORRECT - `toUInt()` used properly

#### Issue C: Fix c_str() Calls (2 occurrences)
**Search**: `\.c_str\(\)` in C_OscCanOpenObjectDictionary.cpp

**Fix Pattern**:
```cpp
// Before (if c_String is C_SclString being passed to QString)
QString(c_String.c_str())
// After
c_String.ToQString()  // or migrate c_String to QString entirely
```

#### Issue D: Fix toUpper() Usage (4 occurrences around lines 694, 709, 741, 759)
**Search**: Look for `toUpper()` calls on C_SclString members

**Fix Pattern**:
```cpp
// If operating on C_SclString member that should be QString:
// 1. Change member declaration in .hpp from C_SclString to QString
// 2. Then use .toUpper() directly
// OR if C_SclString must stay temporarily:
c_String.UpperCase()  // C_SclString method
```

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
# Should show fewer errors in C_OscCanOpenObjectDictionary.cpp
```

### Deliverables:
- [ ] All back-conversions removed
- [ ] All ToInt() → toInt() with correct base
- [ ] All c_str() calls fixed
- [ ] All case conversion methods corrected
- [ ] File compiles without errors

---

## Task 1.2: Fix C_OscCanOpenEdsDeviceInfoBlock String Operations
**File**: `opensyde_tool/libs/opensyde_core/imports/C_OscCanOpenEdsDeviceInfoBlock.cpp`
**Header**: `opensyde_tool/libs/opensyde_core/imports/C_OscCanOpenEdsDeviceInfoBlock.hpp`
**Estimated Time**: 60 minutes
**Status**: [✓]

### Issues to Fix:

#### Issue A: Fix CalcHash() Method (Lines ~69-74)
**Current Problem**:
```cpp
// CalcHash uses .c_str() and .Length() on C_SclString members
mu32_Hash = CalcHash(this->c_VendorName.c_str(), this->c_VendorName.Length());
```

**Fix Pattern**:
```cpp
// If c_VendorName is QString:
mu32_Hash = CalcHash(this->c_VendorName.toUtf8().constData(),
                     static_cast<uint32_t>(this->c_VendorName.length()));
```

#### Issue B: Fix String Member Assignments (22 c_str() occurrences, lines ~120-143)
**Current Problem**:
```cpp
this->c_VendorName = orc_File.value(c_Group + "VendorName", "").toString().toStdString().c_str();
```

**Fix Pattern**:
```cpp
// Check if c_VendorName is C_SclString or QString in .hpp
// If QString (preferred):
this->c_VendorName = orc_File.value(c_Group + "VendorName", "").toString();

// If C_SclString (temporary):
this->c_VendorName = C_SclString::FromQString(orc_File.value(c_Group + "VendorName", "").toString());
```

#### Issue C: Migrate String Members to QString (in .hpp)
**Check these members** around line 50-80 in header:
- `c_VendorName`
- `c_ProductName`
- `c_OrderCode`
- `c_SimpleBootUpMaster`
- `c_SimpleBootUpSlave`
- `c_Granularity`
- Others with `C_SclString` type

**If they're C_SclString**: Change to `QString` and fix all usages

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
# Check for errors in C_OscCanOpenEdsDeviceInfoBlock
```

### Deliverables:
- [ ] CalcHash() uses QString methods
- [ ] All 22 c_str() calls eliminated
- [ ] All 7 Length() → length()
- [ ] String members migrated to QString (if not already)
- [ ] File compiles without errors

---

## Task 1.3: Fix C_OscCanOpenEdsFileInfoBlock String Operations
**File**: `opensyde_tool/libs/opensyde_core/imports/C_OscCanOpenEdsFileInfoBlock.cpp`
**Header**: `opensyde_tool/libs/opensyde_core/imports/C_OscCanOpenEdsFileInfoBlock.hpp`
**Estimated Time**: 60 minutes
**Status**: [✓]
**Dependencies**: None (can run parallel with Task 1.2)

### Issues to Fix:

#### Issue A: Fix CalcHash() Method
Same pattern as Task 1.2, Issue A

#### Issue B: Fix String Member Assignments (20 c_str() occurrences)
Same pattern as Task 1.2, Issue B

#### Issue C: Fix Length() Calls (9 occurrences)
**Search**: `\.Length\(\)` in C_OscCanOpenEdsFileInfoBlock.cpp

**Fix Pattern**:
```cpp
// Before
if (c_String.Length() > 0)
// After
if (c_String.length() > 0)
```

#### Issue D: Migrate String Members to QString (in .hpp)
Check members like:
- `c_FileName`
- `c_FileVersion`
- `c_FileRevision`
- `c_EDSVersion`
- `c_Description`
- `c_CreationTime`
- `c_CreationDate`
- `c_CreatedBy`
- `c_ModificationTime`
- `c_ModificationDate`
- `c_ModifiedBy`

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
```

### Deliverables:
- [ ] CalcHash() uses QString methods
- [ ] All 20 c_str() calls eliminated
- [ ] All 9 Length() → length()
- [ ] String members migrated to QString
- [ ] File compiles without errors

---

## Task 1.4: Fix C_OscImportEdsDcf String Operations
**File**: `opensyde_tool/libs/opensyde_core/imports/C_OscImportEdsDcf.cpp`
**Estimated Time**: 30 minutes
**Status**: [ ]
**Dependencies**: Tasks 1.1, 1.2, 1.3 (if APIs changed)

### Issues to Fix:

#### Issue A: Fix Length() Calls (7 occurrences)
**Search**: `\.Length\(\)` in C_OscImportEdsDcf.cpp

Apply standard Length() → length() pattern

#### Issue B: Fix Any Remaining Conversion Issues
After Tasks 1.1-1.3 complete, this file may have API mismatches. Check for:
- Calls to `C_OscCanOpenObjectDictionary` methods
- Calls to `C_OscCanOpenEdsDeviceInfoBlock` methods
- Calls to `C_OscCanOpenEdsFileInfoBlock` methods

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
# Should compile cleanly
```

### Deliverables:
- [ ] All 7 Length() → length()
- [ ] API calls match updated signatures
- [ ] File compiles without errors

---

# SPRINT 2: Code Generation/Export Files

**Goal**: Fix embedded code generation
**Dependencies**: Sprint 1 complete (green build)
**Verification**: Test code generation functionality + build

---

## Task 2.1: Fix C_OscExportDataPool String Operations
**File**: `opensyde_tool/libs/opensyde_core/exports/code_generation/C_OscExportDataPool.cpp`
**Estimated Time**: 120 minutes
**Status**: [ ]

### Issues to Fix:

#### Issue A: Audit String Variable Types
**Action**: Search for `C_SclString` declarations in the file (75 occurrences total)

For each `C_SclString` variable, determine:
1. Is it a local variable? → Change to `QString`
2. Is it from a data class member? → Check if member is now QString
3. Is it passed to C_SclString API? → Check if API updated or needs conversion

#### Issue B: Fix toUpper/toLower Usage (23 occurrences)
**Pattern**: These are likely correct (QString method), but verify they're operating on QString not C_SclString

**Search**: `\.toUpper\(\)` and `\.toLower\(\)`

**Check**:
```cpp
// If variable is C_SclString:
c_String.UpperCase()  // or .LowerCase()
// If variable is QString:
c_String = c_String.toUpper()  // Note: toUpper() returns new QString
```

#### Issue C: Fix c_str() Calls (23 occurrences)
**Search**: `\.c_str\(\)`

Apply standard c_str() → toUtf8().constData() pattern or eliminate if converting to QString

### Approach:
1. First pass: Change all local `C_SclString` declarations to `QString`
2. Second pass: Fix compilation errors by updating method calls
3. Third pass: Verify toUpper/toLower/c_str usage

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
# Test code generation if possible
```

### Deliverables:
- [ ] All local C_SclString → QString
- [ ] All case conversions correct
- [ ] All c_str() calls eliminated or converted
- [ ] File compiles without errors

---

## Task 2.2: Fix C_OscExportCommunicationStack String Operations
**File**: `opensyde_tool/libs/opensyde_core/exports/code_generation/C_OscExportCommunicationStack.cpp`
**Estimated Time**: 90 minutes
**Status**: [ ]
**Dependencies**: Task 2.1 (similar patterns)

### Issues to Fix:

#### Issue A: Audit String Variables (59 C_SclString occurrences)
Same approach as Task 2.1, Issue A

#### Issue B: Fix toUpper/toLower Usage (27 occurrences)
Same approach as Task 2.1, Issue B

#### Issue C: Fix Any c_str() Calls
Same approach as Task 2.1, Issue C

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
```

### Deliverables:
- [ ] All local C_SclString → QString
- [ ] All case conversions correct
- [ ] File compiles without errors

---

## Task 2.3: Fix C_OscExportCanOpenConfig String Operations
**File**: `opensyde_tool/libs/opensyde_core/exports/x_config/C_OscExportCanOpenConfig.cpp`
**Estimated Time**: 90 minutes
**Status**: [ ]
**Dependencies**: Task 2.1, 2.2 (similar patterns)

### Issues to Fix:

#### Issue A: Audit String Variables (56 C_SclString occurrences)
Same approach as Task 2.1

#### Issue B: Fix toUpper/toLower Usage (15 occurrences)
Same approach as Task 2.1

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
```

### Deliverables:
- [ ] All local C_SclString → QString
- [ ] All case conversions correct
- [ ] File compiles without errors

---

## Task 2.4: Fix Remaining Export Files
**Files**:
- `opensyde_tool/libs/opensyde_core/exports/x_config/C_OscExportHalc.cpp` (29 C_SclString, 8 toUpper/toLower)
- `opensyde_tool/libs/opensyde_core/exports/code_generation/C_OscExportCanOpenInit.cpp` (22 C_SclString)
- `opensyde_tool/libs/opensyde_core/exports/code_generation/C_OscExportOsyInit.cpp` (20 C_SclString, 2 Length())
- `opensyde_tool/libs/opensyde_core/exports/uti/C_OscExportUti.cpp` (18 C_SclString, 5 toUpper/toLower)

**Estimated Time**: 120 minutes
**Status**: [ ]
**Dependencies**: Tasks 2.1-2.3 complete

### Approach:
Apply same systematic patterns from Tasks 2.1-2.3 to all four files

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
```

### Deliverables:
- [ ] All 4 files migrated
- [ ] All compile without errors

---

# SPRINT 3: CAN Monitor Protocol Files

**Goal**: Fix protocol decoding and logging
**Dependencies**: Sprint 1 complete (to ensure clean build baseline)
**Verification**: `cd opensyde_tool\bat && build_can_monitor_release.bat`

---

## Task 3.1: Create PrintFormatted() Conversion Script
**Estimated Time**: 30 minutes
**Status**: [ ]

### Action:
Create a PowerShell or Python script to automate PrintFormatted() conversions

**Script Requirements**:
```powershell
# Pattern to find:
\.PrintFormatted\(

# Replacement:
= QString::asprintf(

# Example:
# Before: c_Text.PrintFormatted("Value: %d", s32_Value);
# After:  c_Text = QString::asprintf("Value: %d", s32_Value);
```

**Save to**: `opensyde_tool/libs/opensyde_core/scripts/convert_printformatted.ps1` or `.py`

### Verification:
Test script on a sample file first

### Deliverables:
- [ ] Script created and tested
- [ ] Usage instructions documented

---

## Task 3.2: Apply PrintFormatted() Conversion to CAN Monitor Files
**Files**:
- `opensyde_tool/libs/opensyde_core/can_monitor_protocol/C_CanMonProtocolXfl.cpp` (15 calls)
- `opensyde_tool/libs/opensyde_core/can_monitor_protocol/C_CanMonProtocolOpenSyde.cpp` (14 calls)
- `opensyde_tool/libs/opensyde_core/can_monitor_protocol/C_CanMonProtocolKefex.cpp` (11 calls)
- `opensyde_tool/libs/opensyde_core/can_monitor_protocol/C_CanMonProtocolShipIpIva.cpp` (7 calls)
- `opensyde_tool/libs/opensyde_core/can_monitor_protocol/C_CanMonProtocolBase.cpp` (8 calls)
- `opensyde_tool/libs/opensyde_core/can_monitor_protocol/C_CanMonProtocol.cpp` (6 calls)

**Estimated Time**: 90 minutes (with script from Task 3.1)
**Status**: [ ]
**Dependencies**: Task 3.1 complete

### Approach:
1. Run conversion script on each file
2. Manually review changes
3. Compile and fix any issues

### Verification:
```bash
cd opensyde_tool/bat
build_can_monitor_release.bat
```

### Deliverables:
- [ ] All 6 files converted
- [ ] All compile without errors
- [ ] Changes reviewed and verified

---

## Task 3.3: Apply PrintFormatted() Conversion to Remaining Files
**Files**: 11 additional files with PrintFormatted() (see comprehensive status doc for list)
**Estimated Time**: 60 minutes
**Status**: [ ]
**Dependencies**: Task 3.2 complete

### Approach:
Same as Task 3.2, apply to remaining files

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
build_can_monitor_release.bat
```

### Deliverables:
- [ ] All remaining files converted
- [ ] All compile without errors

---

# SPRINT 4: KEFEX Library & System Update

**Goal**: Fix legacy protocol and update packaging
**Dependencies**: Sprint 1 complete
**Verification**: `cd opensyde_tool\bat && build_syde_flash_release.bat`

---

## Task 4.1: Fix KEFEX Variable Files
**Files**:
- `opensyde_tool/libs/opensyde_core/kefex_diaglib/CKFXVariableBase.cpp` (9 Length(), 7 c_str())
- `opensyde_tool/libs/opensyde_core/kefex_diaglib/CKFXVariableLists.cpp` (1 Length())
- `opensyde_tool/libs/opensyde_core/kefex_diaglib/CKFXVariableListBase.cpp` (1 Length())

**Estimated Time**: 60 minutes
**Status**: [ ]

### Issues to Fix:
- Apply standard Length() → length() pattern
- Apply standard c_str() → toUtf8().constData() or eliminate pattern
- Check if member variables need QString migration

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
```

### Deliverables:
- [ ] All 3 files fixed
- [ ] Files compile without errors

---

## Task 4.2: Fix KEFEX Project Files
**Files**:
- `opensyde_tool/libs/opensyde_core/kefex_diaglib/CKFXDEFProject.cpp` (6 toUpper/toLower, 5 ToInt(), 3 Length())
- `opensyde_tool/libs/opensyde_core/kefex_diaglib/CKFXProtocol.cpp` (4 c_str(), 2 PrintFormatted())
- `opensyde_tool/libs/opensyde_core/kefex_diaglib/CKFXProjectOptions.cpp` (1 Length())

**Estimated Time**: 60 minutes
**Status**: [ ]
**Dependencies**: Task 4.1 (if APIs changed)

### Issues to Fix:
- ToInt() → toInt() with correct base
- Length() → length()
- c_str() elimination
- PrintFormatted() → QString::asprintf()
- Case conversion verification

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
```

### Deliverables:
- [ ] All 3 files fixed
- [ ] Files compile without errors

---

## Task 4.3: Fix System Update Package Files
**Files**:
- `opensyde_tool/libs/opensyde_core/project/system/system_update_package/C_OscSupNodeDefinitionFiler.hpp/.cpp`
- `opensyde_tool/libs/opensyde_core/project/system/system_update_package/C_OscSupServiceUpdatePackageV1.hpp/.cpp`
- `opensyde_tool/libs/opensyde_core/project/system/system_update_package/C_OscSupDefinitionFiler.cpp` (2 ToInt())

**Estimated Time**: 90 minutes
**Status**: [ ]

### Issues to Fix:

#### Issue A: Update Function Signatures
**Change**:
```cpp
// Before
static void mh_LoadFilesSection(std::vector<C_SclString> & orc_Files);
// After
static void mh_LoadFilesSection(std::vector<QString> & orc_Files);
```

**Files**: C_OscSupNodeDefinitionFiler, C_OscSupServiceUpdatePackageV1

#### Issue B: Fix ToInt() Calls
**File**: C_OscSupDefinitionFiler.cpp
Standard ToInt() → toInt() pattern

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
```

### Deliverables:
- [ ] Function signatures updated
- [ ] ToInt() calls fixed
- [ ] Files compile without errors

---

## Task 4.4: Fix C_OscZipFile API
**File**: `opensyde_tool/libs/opensyde_core/C_OscZipFile.hpp/.cpp`
**Estimated Time**: 45 minutes
**Status**: [ ]
**Dependencies**: Task 4.3 (related API changes)

### Issues to Fix:

#### Issue A: Update h_AppendFilesRelative() Signature
**Change**:
```cpp
// Before
static int32_t h_AppendFilesRelative(const QString & orc_SupFilePath,
                                     const std::vector<C_SclString> & orc_FilesToAdd,
                                     const QString & orc_CommonInputPath);
// After
static int32_t h_AppendFilesRelative(const QString & orc_SupFilePath,
                                     const std::vector<QString> & orc_FilesToAdd,
                                     const QString & orc_CommonInputPath);
```

#### Issue B: Update Internal Set Type
**Change**:
```cpp
// If internal implementation uses:
std::set<C_SclString> c_Files;
// Change to:
std::set<QString> c_Files;
```

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
```

### Deliverables:
- [ ] API signature updated
- [ ] Internal types updated
- [ ] All callers still compile
- [ ] File compiles without errors

---

# SPRINT 5: Message Logging & Miscellaneous

**Goal**: Clean up remaining files
**Dependencies**: Sprints 1-4 complete
**Verification**: Full rebuild all targets

---

## Task 5.1: Fix Message Logging Files
**Files**:
- `opensyde_tool/libs/opensyde_core/logging/C_OscComMessageLoggerFileAsc.cpp` (8 PrintFormatted, 4 Length, 7 c_str)
- `opensyde_tool/libs/opensyde_core/logging/C_OscComMessageLogger.cpp` (5 c_str)
- `opensyde_tool/libs/opensyde_core/protocol_drivers/communication/C_OscComDriverProtocol.cpp` (7 c_str, 2 PrintFormatted)

**Estimated Time**: 60 minutes
**Status**: [ ]

### Approach:
Apply standard patterns for Length(), c_str(), PrintFormatted()

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
build_can_monitor_release.bat
```

### Deliverables:
- [ ] All 3 files fixed
- [ ] Files compile without errors

---

## Task 5.2: Fix Security/Checksum Files
**Files**:
- `opensyde_tool/libs/opensyde_core/security/C_OscSecurityEcdsa.cpp` (5 Length, 1 ToInt)
- `opensyde_tool/libs/opensyde_core/md5/C_Md5Checksum.cpp` (2 PrintFormatted)
- `opensyde_tool/libs/opensyde_core/scl/C_OscChecksummedIniFile.cpp` (1 Length)

**Estimated Time**: 30 minutes
**Status**: [ ]

### Approach:
Apply standard patterns

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
```

### Deliverables:
- [ ] All 3 files fixed
- [ ] Files compile without errors

---

## Task 5.3: Fix Miscellaneous Core Files
**Files**:
- `opensyde_tool/libs/opensyde_core/project/system/node/C_OscNodeDataPoolContent.cpp` (2 PrintFormatted)
- `opensyde_tool/libs/opensyde_core/xce/C_OscXceCreate.cpp` (1 Length, conversion issues)
- `opensyde_tool/libs/opensyde_core/scl/C_SclIniFile.cpp` (1 Length) - verify mostly migrated

**Estimated Time**: 30 minutes
**Status**: [ ]

### Approach:
Apply standard patterns, verify previous migrations still intact

### Verification:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
```

### Deliverables:
- [ ] All 3 files fixed
- [ ] Files compile without errors

---

## Task 5.4: Final Verification & Cleanup
**Estimated Time**: 60 minutes
**Status**: [ ]
**Dependencies**: All other tasks complete

### Actions:

#### 1. Full Rebuild All Targets
```bash
cd opensyde_tool/bat

# Clean
cmd /c "rmdir /s /q ..\result\build 2>nul"

# Build all targets
build_syde_flash_release.bat
build_can_monitor_release.bat
build_release.bat  # Main openSYDE
```

#### 2. Search for Remaining C_SclString Usage
```bash
# From repository root
grep -r "C_SclString" opensyde_tool/libs/opensyde_core --include="*.cpp" | grep -v "C_SclString\.cpp" | wc -l
# Should be significantly lower than 2010
```

#### 3. Update Documentation
- Update `plans/QString_Migration_Status.md` with final stats
- Mark Phase 2 as complete (or identify remaining work)
- Update `plans/QString_Migration_Master_Plan.md` progress log

#### 4. Create Summary Report
Document:
- Total files changed
- Total lines changed
- Build status for all targets
- Known remaining issues (if any)
- Readiness for Phase 3 (C_SclStringList → QStringList)

### Deliverables:
- [ ] All targets build cleanly
- [ ] C_SclString usage documented
- [ ] Documentation updated
- [ ] Summary report created

---

# PHASE 3 PREPARATION (Future Work)

**Status**: [ ] Not started - blocked by Phase 2

## Task P3.1: Audit C_SclStringList Usage
**Estimated Time**: 60 minutes
**Status**: [ ]
**Dependencies**: Phase 2 complete

Search for all `C_SclStringList` usage and categorize:
1. Files where it can be replaced immediately
2. Files with API dependencies
3. Files requiring coordination

Create `QString_Migration_Phase3_Agent_Tasks.md` similar to this document.

---

# COMMON PATTERNS REFERENCE

## Pattern 1: Length() → length()
```cpp
// Before
if (c_String.Length() > 0)
// After
if (c_String.length() > 0)
```

## Pattern 2: c_str() Elimination
```cpp
// Before (passing to const char*)
SomeFunction(c_String.c_str());
// After (if c_String is now QString)
SomeFunction(c_String.toUtf8().constData());

// Before (creating QString from C_SclString)
QString c_Qt = QString(c_SclString.c_str());
// After (if C_SclString has ToQString())
QString c_Qt = c_SclString.ToQString();
```

## Pattern 3: ToInt() → toInt()
```cpp
// Before
int32_t s32_Val = c_String.ToInt();
// After
int32_t s32_Val = c_String.toInt();

// For hex strings:
uint16_t u16_Val = c_String.toInt(nullptr, 16);
```

## Pattern 4: PrintFormatted() → QString::asprintf()
```cpp
// Before
c_Text.PrintFormatted("Value: %d, Name: %s", s32_Value, pc_Name);
// After
c_Text = QString::asprintf("Value: %d, Name: %s", s32_Value, pc_Name);
```

## Pattern 5: Case Conversion
```cpp
// Before (C_SclString)
c_String.UpperCase();  // modifies in place
c_String.LowerCase();
// After (QString)
c_String = c_String.toUpper();  // returns new QString
c_String = c_String.toLower();
```

## Pattern 6: CalcHash() Updates
```cpp
// Before
mu32_Hash = CalcHash(c_Name.c_str(), c_Name.Length());
// After
mu32_Hash = CalcHash(c_Name.toUtf8().constData(), static_cast<uint32_t>(c_Name.length()));
```

## Pattern 7: Back-Conversion Elimination
```cpp
// Before (BAD - creates temporary)
const QString c_Qs = GetQString();
const C_SclString c_Scl = c_Qs.toStdString().c_str();
if (c_Scl.Length() == 4)
// After (GOOD)
const QString c_Str = GetQString();
if (c_Str.length() == 4)
```

## Pattern 8: SubString() → mid() ⚠️ INDEXING DIFFERENCE!
```cpp
// C_SclString uses 1-based indexing, QString uses 0-based!

// Before (C_SclString - 1-based)
c_String.SubString(1, 4)  // Extract 4 chars starting at position 1 (1st char)
c_String.SubString(5, 3)  // Extract 3 chars starting at position 5 (5th char)

// After (QString - 0-based) - SUBTRACT 1 FROM START POSITION!
c_String.mid(0, 4)  // Extract 4 chars starting at position 0 (1st char)
c_String.mid(4, 3)  // Extract 3 chars starting at position 4 (5th char)

// COMMON MISTAKE:
c_String.mid(1, 4)  // WRONG! This starts at 2nd char, not 1st!
c_String.mid(5, 3)  // WRONG! This starts at 6th char, not 5th!
```

## Pattern 9: Pos() → indexOf() ⚠️ INDEXING DIFFERENCE!
```cpp
// C_SclString Pos() returns 1-based position (or 0 if not found)
// QString indexOf() returns 0-based position (or -1 if not found)

// Before (C_SclString - 1-based result)
if (c_String.Pos("sub") == 5)  // "sub" found at 5th character (1-based)

// After (QString - 0-based result) - SUBTRACT 1 FROM COMPARISON!
if (c_String.indexOf("sub") == 4)  // "sub" found at position 4 (0-based)

// COMMON MISTAKE:
if (c_String.indexOf("sub") == 5)  // WRONG! This checks wrong position!

// Also check for "not found":
// Before (C_SclString)
if (c_String.Pos("sub") == 0)  // Not found

// After (QString)
if (c_String.indexOf("sub") == -1)  // Not found (returns -1, not 0!)
```

**CRITICAL**: Always adjust position values when converting Pos() → indexOf() or SubString() → mid()!

---

# BUILD COMMANDS REFERENCE

## SYDEflash
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
# Output: opensyde_tool/result/SYDEflash_win.exe
```

## CAN Monitor
```bash
cd opensyde_tool/bat
build_can_monitor_release.bat
# Output: opensyde_tool/result/openSYDE_CAN_Monitor_win.exe
```

## Main openSYDE
```bash
cd opensyde_tool/bat
build_release.bat
# Output: opensyde_tool/result/openSYDE_win.exe
```

## Clean Build
```bash
cd opensyde_tool
cmd /c "rmdir /s /q result\build 2>nul"
```

---

# TROUBLESHOOTING

## Common Errors

### Error: "cannot convert C_SclString to QString"
**Fix**: Add `.ToQString()` or change variable declaration to QString

### Error: "no matching function for call to 'QString::Length()'"
**Fix**: Change `Length()` to `length()`

### Error: "taking address of temporary"
**Fix**: Eliminate back-conversions (Pattern 7)

### Error: "ambiguous overload for 'operator='"
**Fix**: Check if both C_SclString and QString versions exist, use explicit conversion

---

# COORDINATION NOTES

## Agent Communication
- Update task status in this document when starting/completing work
- If blocked, document blocker in task and notify coordinator
- If you discover new issues, add them as sub-issues in the task

## Git Workflow
- Create feature branch: `feature/qstring-phase2-sprint<N>`
- Commit after each task completion
- Include task number in commit message: `feat: [Task X.Y] Fix C_OscFoo string operations`
- Push and verify CI builds (if available)

## Verification Requirements
- Each task must compile cleanly before marking as done
- Run specified verification command
- If verification fails, debug and fix before moving on
- Cascading failures are expected - earlier tasks unblock later ones

---

**Last Updated**: 2026-02-02
**Document Version**: 1.0
**Total Tasks**: 23 primary tasks + 1 preparation task
**Estimated Total Time**: 19-26 hours
