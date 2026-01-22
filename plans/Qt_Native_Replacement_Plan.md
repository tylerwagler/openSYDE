# Phased Plan: Complete Qt Native Migration

**Status**: Phase 1 Complete (TGL Layer Elimination) ✅
**Date Created**: 2026-01-16
**Project**: openSYDE Codebase Simplification

---

## Application Status Matrix

This matrix tracks the completion status of each phase across all openSYDE applications/tools.

### Applications Covered

| Application | Directory | Core Library | Notes |
|-------------|-----------|--------------|-------|
| **openSYDE** | `opensyde_tool/pjt/openSYDE/` | `opensyde_tool/libs/opensyde_core/` | Main GUI tool |
| **SYDEflash** | `opensyde_tool/pjt/SYDEflash/` | `opensyde_tool/libs/opensyde_core/` | Flash tool (shares core) |
| **openSYDE CAN Monitor** | `opensyde_tool/pjt/openSYDE_CAN_Monitor/` | `opensyde_tool/libs/opensyde_core/` | CAN monitor (shares core) |
| **opensyde_cmd_line_flash_tool** | `opensyde_cmd_line_flash_tool/` | `opensyde_cmd_line_flash_tool/libs/osy_core/` | CLI flash (own core copy) |
| **opensyde_syde_coder_c** | `opensyde_syde_coder_c/` | `opensyde_syde_coder_c/libs/osy_core/` | C code generator (own core copy) |
| **opensyde_syde_sup** | `opensyde_syde_sup/` | `opensyde_syde_sup/libs/opensyde_core/` | Service update package (own core copy) |
| **opensyde_syde_x_gen** | `opensyde_syde_x_gen/` | `opensyde_syde_x_gen/libs/opensyde_core/` | X generator (own core copy) |

### Phase Completion Matrix

| Phase | Description | openSYDE | SYDEflash | CAN Monitor | cmd_line_flash | syde_coder_c | syde_sup | syde_x_gen |
|-------|-------------|----------|-----------|-------------|----------------|--------------|----------|------------|
| **Phase 1** | TGL Layer Elimination | ✅ | ✅ | ✅ | N/A* | N/A* | N/A* | N/A* |
| **Phase 2** | C_SclString → QString | ⏳ | ⏳ | ⏳ | N/A* | N/A* | N/A* | N/A* |
| **Phase 3** | C_SclStringList → QStringList | ❌ | ❌ | ❌ | N/A* | N/A* | N/A* | N/A* |
| **Phase 4** | C_SclDynamicArray → QList | ✅ | ✅ | ✅ | N/A* | N/A* | N/A* | N/A* |
| **Phase 5** | C_SclIniFile → QSettings | ✅ | ✅ | ✅ | N/A* | N/A* | N/A* | N/A* |
| **Phase 6** | C_SclChecksums (Optional) | ❌ | ❌ | ❌ | N/A* | N/A* | N/A* | N/A* |

*\* CLI tools use C++98 without Qt dependency - Qt-based migrations not applicable*

**Legend:**
- ✅ Complete
- ⏳ In Progress / Planned
- ❌ Not Started
- N/A Not Applicable

### Notes on Shared vs. Separate Core Libraries

The openSYDE ecosystem has **two patterns** for core library usage:

1. **Shared Core** (opensyde_tool applications):
   - `openSYDE`, `SYDEflash`, and `openSYDE CAN Monitor` all share `opensyde_tool/libs/opensyde_core/`
   - Changes to the core library automatically apply to all three applications
   - These applications are marked together in the matrix

2. **Separate Core Copies** (standalone CLI tools):
   - `opensyde_cmd_line_flash_tool`, `opensyde_syde_coder_c`, `opensyde_syde_sup`, `opensyde_syde_x_gen`
   - Each has its own copy of the core library in `libs/osy_core/` or `libs/opensyde_core/`
   - **IMPORTANT**: These CLI tools use **C++98** and do **NOT** have Qt as a dependency
   - Qt-specific migrations (Phases 4, 5) are **N/A** for CLI tools unless Qt is added as a dependency
   - Only pure C++ migrations (like `std::vector` replacements) would apply without adding Qt

---

## Executive Summary

This document outlines a phased approach to complete the migration from custom wrapper classes to Qt native implementations. Phase 1 (TGL layer elimination) has been successfully completed. The remaining phases focus on eliminating the SCL (STW Component Library) wrapper classes that add unnecessary abstraction over standard C++ and Qt functionality.

---

## Phase 1: TGL (Target Glue Layer) Elimination ✅ COMPLETE

**Files Modified**: 410+
**Files Deleted**: 22
**Impact**: Core infrastructure modernization

### Application Status

| Application | Status | Date | Notes |
|-------------|--------|------|-------|
| openSYDE | ✅ Complete | 2026-01-16 | TGL layer fully removed |
| SYDEflash | ✅ Complete | 2026-01-16 | Shares opensyde_core with openSYDE |
| openSYDE CAN Monitor | ✅ Complete | 2026-01-16 | Shares opensyde_core with openSYDE |
| opensyde_cmd_line_flash_tool | ❌ Not Started | - | Has own copy in libs/osy_core/ |
| opensyde_syde_coder_c | ❌ Not Started | - | Has own copy in libs/osy_core/ |
| opensyde_syde_sup | ❌ Not Started | - | Has own copy in libs/opensyde_core/ |
| opensyde_syde_x_gen | ❌ Not Started | - | Has own copy in libs/opensyde_core/ |

### What Was Accomplished

#### 1.1 TGL Platform Abstraction Removed
- **TglFile** → `QFile`, `QFileInfo`, `QDir`
- **TglUtils** → Qt file/path utilities
- **TglTime** → `QDateTime`, `QElapsedTimer`
- **TglTasks (C_TglCriticalSection)** → `QRecursiveMutex`, `QMutexLocker`

#### 1.2 Additional Custom Classes Eliminated
- **C_SclDateTime** → `QDateTime`
- **tinyxml2** → `QDomDocument`/`QDomElement`
- **C_SclIniFile** → Reimplemented using `QSettings` internally

#### 1.3 Platform Support Changes
- Removed Linux-specific implementations (now Windows-only with Qt)
- Deleted `can_dispatcher/target_linux_socket_can/`
- Deleted `ip_dispatcher/target_linux_sock/`

### Key Achievements
✅ Complete TGL layer removal (291 function calls replaced)
✅ Thread synchronization migrated to Qt (`QRecursiveMutex`)
✅ XML parsing completely on Qt DOM
✅ Date/time operations standardized on `QDateTime`
✅ Build system updated for Qt6 Core and XML modules

---

## Phase 2: C_SclString Migration to QString

### Priority: HIGH
**Actual Impact**: ~25,675 occurrences across 1,912 files (updated 2026-01-18)
**Complexity**: High (pervasive throughout entire codebase)
**Risk**: Medium (well-understood migration pattern)

### Application Status

| Application | Status | Date | Notes |
|-------------|--------|------|-------|
| openSYDE | ⏳ Planned | - | Detailed planning complete, ready for Phase 2-PREP |
| SYDEflash | ⏳ Planned | - | Shares opensyde_core with openSYDE |
| openSYDE CAN Monitor | ⏳ Planned | - | Shares opensyde_core with openSYDE |
| opensyde_cmd_line_flash_tool | ❌ Not Started | - | Has own copy in libs/osy_core/ |
| opensyde_syde_coder_c | ❌ Not Started | - | Has own copy in libs/osy_core/ |
| opensyde_syde_sup | ❌ Not Started | - | Has own copy in libs/opensyde_core/ |
| opensyde_syde_x_gen | ❌ Not Started | - | Has own copy in libs/opensyde_core/ |

### Current State Analysis

#### C_SclString Overview
- **Purpose**: Borland AnsiString compatibility wrapper around `std::string`
- **Current Usage**: 25,675 occurrences in 1,912 files
- **Location**: `opensyde_core/scl/C_SclString.{cpp,hpp}`
- **Key Insight**: Already has `ToQString()` and `FromQString()` methods - Qt migration was anticipated!

#### Key Functionality to Replace
| C_SclString Method | QString Equivalent |
|-------------------|-------------------|
| `.c_str()` | `.toStdString().c_str()` or `.toLatin1().constData()` |
| `.AsStdString()` | `.toStdString()` |
| `.Length()` | `.length()` or `.size()` |
| `.SubString(pos, len)` | `.mid(pos-1, len)` (note: 0-based vs 1-based) |
| `.Pos(search)` | `.indexOf(search) + 1` (note: 0-based vs 1-based) |
| `.UpperCase()` | `.toUpper()` |
| `.LowerCase()` | `.toLower()` |
| `.Trim()` | `.trimmed()` |
| `.StringPrintFormatted()` | `QString::asprintf()` or `QString::arg()` |
| `.IntToStr()` | `QString::number()` |
| `.StringToInt()` | `.toInt()` |
| `.Delete(pos, len)` | `.remove(pos-1, len)` |
| `.Insert(str, pos)` | `.insert(pos-1, str)` |

### Detailed Scope Analysis (Updated 2026-01-18)

#### Usage Breakdown by Module

| Module/Directory | Files | Occurrences | Priority |
|-----------------|-------|-------------|----------|
| opensyde_core/project/ | 35+ | HIGH | CRITICAL (core data model) |
| opensyde_core/kefex_diaglib/ | 20+ | HIGH | HIGH (legacy compatibility) |
| opensyde_core/halc/ | 20+ | HIGH | HIGH (hardware abstraction) |
| opensyde_core/exports/ | 20+ | HIGH | HIGH (code generation) |
| opensyde_core/imports/ | 6+ | MEDIUM | MEDIUM |
| opensyde_core/can_dispatcher/ | Multiple | MEDIUM | MEDIUM (protocol layer) |
| opensyde_core/data_dealer/ | Multiple | MEDIUM | MEDIUM |
| opensyde_tool/src/ | 158+ | 967 | LOW (GUI - after core) |

#### Critical API Dependencies (Must Migrate Together)

**Tier 1 - Foundation Classes:**
1. **C_OscProject** - Member variables: `c_Author`, `c_Editor`, `c_OpenSydeVersion`, `c_Template`, `c_Version`
2. **C_OscSystemDefinition** - Methods: `AddNode()`, `SetNodeName()`, `CheckMessageNameBus()`
3. **C_OscNode** - Member: `c_DeviceType`
4. **C_OscCanMessage** - Members: `c_Name`, `c_Comment` (affects hundreds per project)
5. **C_OscSystemBus** - Name/comment fields

**Tier 2 - Utility/Infrastructure:**
6. **C_OscUtils** - 30+ public static methods (HIGH frequency, used everywhere)
7. **C_OscLoggingHandler** - Logging is pervasive
8. **C_Md5Checksum** - Specialized checksum operations
9. **C_OscConfFileHandler** - Configuration file loading

#### Leaf Modules (Safe to Migrate First)
These modules use C_SclString internally but don't export it in public API:
1. `security/` module - Already uses `std::string` in API
2. `xml_parser/` module - Internal usage only
3. `miniz/` module - External compression library
4. `md5/` module - Small API surface
5. `aes/` module - Minimal exposure

### Migration Strategy

#### Option A: Gradual Module-by-Module Migration (RECOMMENDED)
1. **Start with leaf modules** (minimal dependencies)
2. **Update module signatures** to accept/return `QString`
3. **Convert internal usage** within module
4. **Add temporary conversion** at module boundaries
5. **Propagate upward** through dependency tree

**Advantages**:
- Lower risk (isolated changes)
- Incremental testing possible
- Can pause/resume work easily
- Easier code review

**Disadvantages**:
- Temporary conversion overhead at boundaries
- Longer overall timeline

#### Option B: Big-Bang Automated Replacement
1. **Create comprehensive regex replacement** script
2. **Run across entire codebase** in single operation
3. **Fix compilation errors** in batch
4. **Extensive testing** required

**Advantages**:
- Faster completion
- No temporary boundary conversions

**Disadvantages**:
- Higher risk
- Difficult to test incrementally
- Large changeset for code review
- Risk of subtle bugs from automated conversion

### Recommended Approach: Option A (Module-by-Module)

#### 2.0 Phase 2-PREP: Utility Classes First (START HERE)
**Target**: C_OscUtils - the most frequently called utility class
**Rationale**: High-frequency utility class with no dependencies. Establishes patterns for the rest of the migration.

**Files to migrate:**
- `C_OscUtils.cpp/hpp` - 30+ static methods

**Strategy**:
1. Add QString overloads for all public methods (dual API)
2. Mark C_SclString versions as deprecated
3. Convert internal implementations to use QString
4. Update callers incrementally over time

**Example pattern:**
```cpp
// Before: Only C_SclString
static bool h_CheckValidFileName(const C_SclString & orc_Name);

// After: Dual API during transition
static bool h_CheckValidFileName(const QString & orc_Name);
[[deprecated]] static bool h_CheckValidFileName(const C_SclString & orc_Name) {
   return h_CheckValidFileName(orc_Name.ToQString());
}
```

#### 2.1 Phase 2A: Data Model Layer
**Target**: Core data structures with minimal external dependencies

**Files to migrate (priority order):**
1. `project/C_OscProject.cpp/hpp` - Root configuration object
2. `project/system/C_OscSystemBus.cpp/hpp` - Bus definitions
3. `project/system/node/C_OscNode.cpp/hpp` - Node definitions
4. `project/system/C_OscSystemDefinition.cpp/hpp` - System model

**Strategy**:
- Update public API signatures to `QString`
- Convert internal `C_SclString` member variables to `QString`
- Update all string operations to Qt equivalents
- Must migrate together due to tight coupling

#### 2.2 Phase 2B: Message Infrastructure
**Target**: CAN message and signal definitions (high multiplicative impact)

**Files to migrate:**
- `project/system/node/can/C_OscCanMessage.cpp/hpp`
- `project/system/node/can/C_OscCanSignal.cpp/hpp`
- `project/system/node/can/C_OscCanMessageContainer.cpp/hpp`

**Rationale**: These classes are instantiated hundreds of times per project. Migration has multiplicative benefit.

#### 2.3 Phase 2C: Protocol Drivers
**Target**: Communication protocol implementations

Files to migrate (example):
- `protocol_drivers/C_OscProtocolDriverOsy.cpp/hpp`
- `protocol_drivers/C_OscDiagProtocolOsy.cpp/hpp`
- `kefex_diaglib/` subdirectories (legacy compatibility)

#### 2.4 Phase 2D: File I/O and Parsing
**Target**: Import/export and file handling modules

Files to migrate (example):
- `imports/C_OscImport*.cpp/hpp`
- `exports/C_OscExport*.cpp/hpp`
- `xml_parser/C_OscXmlParser.cpp/hpp`

#### 2.5 Phase 2E: Logging and Configuration
**Target**: Infrastructure support modules

Files to migrate:
- `logging/C_OscLoggingHandler.cpp/hpp`
- `conf_file_handler/C_OscConfFileHandler.cpp/hpp`
- `scl/C_SclChecksums.cpp/hpp`

#### 2.6 Phase 2F: GUI Layer
**Target**: opensyde_tool GUI components (after core library complete)

**Note**: GUI layer already uses 61+ instances of `ToQString()`/`FromQString()` conversions. Many areas are ready to complete the transition.

Files to migrate:
- `opensyde_tool/src/` entire tree
- Update all UI-related string handling
- Remove conversion calls at GUI boundaries

### Critical Considerations

#### Index Differences (CRITICAL)
- **C_SclString**: 1-based indexing (Borland compatibility)
- **QString**: 0-based indexing (standard C++)
- **Action Required**: Carefully audit all `.SubString()`, `.Pos()`, `.Insert()`, `.Delete()` calls

#### Character Encoding
- **C_SclString**: ASCII/Latin-1 (wraps `std::string`)
- **QString**: UTF-16 Unicode
- **Impact**: Generally transparent, but verify special characters

#### Performance
- **QString** is more efficient for UI operations (native Qt type)
- **QString** has better Unicode support
- **QString** integrates seamlessly with Qt APIs

### Testing Strategy
1. **Unit tests**: Create for each migrated module
2. **Integration tests**: Verify module interactions
3. **String index tests**: Specific tests for 1-based → 0-based conversion
4. **Unicode tests**: Verify special characters handled correctly
5. **Performance tests**: Ensure no regression in critical paths

---

## Phase 3: C_SclStringList Migration to QStringList

### Priority: MEDIUM
**Estimated Impact**: ~214 occurrences
**Complexity**: Medium
**Risk**: Low (straightforward mapping)
**Dependency**: Should follow Phase 2 (C_SclString migration)

### Application Status

| Application | Status | Date | Notes |
|-------------|--------|------|-------|
| openSYDE | ❌ Not Started | - | Depends on Phase 2 completion |
| SYDEflash | ❌ Not Started | - | Shares opensyde_core with openSYDE |
| openSYDE CAN Monitor | ❌ Not Started | - | Shares opensyde_core with openSYDE |
| opensyde_cmd_line_flash_tool | ❌ Not Started | - | Has own copy in libs/osy_core/ |
| opensyde_syde_coder_c | ❌ Not Started | - | Has own copy in libs/osy_core/ |
| opensyde_syde_sup | ❌ Not Started | - | Has own copy in libs/opensyde_core/ |
| opensyde_syde_x_gen | ❌ Not Started | - | Has own copy in libs/opensyde_core/ |

### Current State Analysis

#### C_SclStringList Overview
- **Purpose**: Borland TStringList compatibility wrapper
- **Current Usage**: 214 occurrences
- **Location**: `opensyde_core/scl/C_SclStringList.{cpp,hpp}`
- **Internal Storage**: `C_SclDynamicArray<C_SclString>`

### Migration Mapping

| C_SclStringList Method | QStringList Equivalent |
|------------------------|------------------------|
| `.Add(str)` | `.append(str)` |
| `.Append(str)` | `.append(str)` |
| `.Clear()` | `.clear()` |
| `.Delete(idx)` | `.removeAt(idx)` |
| `.Exchange(idx1, idx2)` | `.swapItemsAt(idx1, idx2)` (Qt 5.13+) |
| `.Insert(idx, str)` | `.insert(idx, str)` |
| `.IndexOf(str)` | `.indexOf(str)` |
| `.GetText(sep)` | `.join(sep)` |
| `.GetCount()` | `.count()` or `.size()` |
| `.LoadFromFile(path)` | Manual: `QFile` read + `.split('\n')` |
| `.SaveToFile(path)` | Manual: `QFile` write + `.join('\n')` |
| `.IndexOfName(key)` | Custom helper or iterate |
| `.ValueFromIndex(idx)` | Custom helper |
| `.Values(key)` | Custom helper |
| `.AddStrings(list)` | `.append(*list)` |
| `.Sort()` | `.sort()` |
| `.Strings[idx]` | `[idx]` (direct indexing) |

### Migration Strategy

#### 3.1 Replace Direct Usage
Simple replacements where `C_SclStringList` is used locally:
```cpp
// Before:
C_SclStringList c_List;
c_List.Add("item");
c_List.SaveToFile("output.txt");

// After:
QStringList c_List;
c_List.append("item");
QFile file("output.txt");
file.open(QIODevice::WriteOnly);
file.write(c_List.join("\n").toUtf8());
file.close();
```

#### 3.2 Key=Value Functionality
C_SclStringList has special key=value pair functionality (like INI files).
**Options**:
- **Option A**: Create helper functions for key=value operations
- **Option B**: Replace with `QMap<QString, QString>` where key=value semantics are needed
- **Option C**: Use `QSettings` for configuration-style usage

**Recommendation**: Option B (QMap) for clarity and type safety

#### 3.3 File I/O Patterns
For `.LoadFromFile()` and `.SaveToFile()`:
```cpp
// Helper function to maintain compatibility
QStringList LoadStringListFromFile(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QStringList();
    }
    QTextStream in(&file);
    QStringList result;
    while (!in.atEnd()) {
        result.append(in.readLine());
    }
    return result;
}

void SaveStringListToFile(const QStringList& list, const QString& path) {
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << list.join("\n");
    }
}
```

### Execution Plan

1. **Create helper utilities** for LoadFromFile/SaveToFile patterns
2. **Migrate low-usage files first** (1-2 occurrences)
3. **Update core modules** with heavy usage
4. **Remove C_SclStringList class** after all migrations complete

---

## Phase 4: C_SclDynamicArray Migration to QList

### Priority: LOW-MEDIUM
**Actual Impact**: 689 occurrences across 213 files (updated 2026-01-18)
**Complexity**: Low-Medium
**Risk**: Low (simple 1:1 API mapping, both use 0-based indexing)
**Dependencies**: None (independent of Phase 2/3)

### Application Status

| Application | Status | Date | Notes |
|-------------|--------|------|-------|
| openSYDE | ✅ Complete | 2026-01-18 | C_SclDynamicArray.hpp removed from shared core |
| SYDEflash | ✅ Complete | 2026-01-18 | Shares opensyde_core with openSYDE |
| openSYDE CAN Monitor | ✅ Complete | 2026-01-18 | Shares opensyde_core with openSYDE |
| opensyde_cmd_line_flash_tool | ❌ Not Started | - | Has own copy in libs/osy_core/ |
| opensyde_syde_coder_c | ❌ Not Started | - | Has own copy in libs/osy_core/ |
| opensyde_syde_sup | ❌ Not Started | - | Has own copy in libs/opensyde_core/ |
| opensyde_syde_x_gen | ❌ Not Started | - | Has own copy in libs/opensyde_core/ |

### Current State Analysis (Updated 2026-01-18)

#### C_SclDynamicArray Overview
- **Purpose**: Borland DynamicArray compatibility wrapper
- **Current Implementation**: Wraps `std::vector<T>` internally (line 42 of C_SclDynamicArray.hpp)
- **Current Usage**: 689 occurrences in 213 files
- **Location**: `opensyde_core/scl/C_SclDynamicArray.hpp` (header-only template)
- **Indexing**: 0-based (same as QList - no index conversion needed!)

#### Key Characteristics
- Header-only template class
- Wraps `std::vector<T>` (not QList as previously noted)
- 0-based indexing throughout
- Simple API with direct QList equivalents

### Migration Mapping

| C_SclDynamicArray Method | QList Equivalent | Notes |
|--------------------------|------------------|-------|
| `[idx]` | `[idx]` or `.at(idx)` | Identical |
| `.Delete(idx)` | `.removeAt(idx)` | Direct replacement |
| `.Insert(idx, item)` | `.insert(idx, item)` | Identical signature |
| `.GetLength()` | `.size()` | Direct replacement |
| `.GetHigh()` | `.size() - 1` | Returns last valid index |
| `.SetLength(len)` | `.resize(len)` | Direct replacement |
| `.IncLength(by)` | `.resize(.size() + by)` | Need to reference container |

### Migration Strategy

#### 4.1 Use QList Exclusively
In Qt 6, `QVector` is now an alias for `QList`. Always use `QList<T>` for:
- Consistency across codebase
- Qt 6 recommended practice
- No functional difference from QVector in Qt 6

#### 4.2 Replacement Patterns

**Type declarations:**
```cpp
// Before
C_SclDynamicArray<int32_t> c_Array;
C_SclDynamicArray<C_SomeClass> c_Objects;

// After
QList<int32_t> c_Array;
QList<C_SomeClass> c_Objects;
```

**Method replacements:**
```cpp
// Before                          // After
c_Array.GetLength()                c_Array.size()
c_Array.GetHigh()                  (c_Array.size() - 1)  // or c_Array.size() - 1 in expressions
c_Array.SetLength(10)              c_Array.resize(10)
c_Array.IncLength()                c_Array.resize(c_Array.size() + 1)
c_Array.IncLength(5)               c_Array.resize(c_Array.size() + 5)
c_Array.Delete(idx)                c_Array.removeAt(idx)
c_Array.Insert(idx, item)          c_Array.insert(idx, item)
```

#### 4.3 Special Cases Requiring Attention

1. **GetHigh() in loop conditions:**
   ```cpp
   // Before - careful: GetHigh() returns 0 for empty arrays
   for (int32_t i = 0; i <= c_Array.GetHigh(); i++)

   // After - use size() instead for cleaner logic
   for (int32_t i = 0; i < c_Array.size(); i++)
   // Or with range-based for:
   for (const auto& item : c_Array)
   ```

2. **IncLength() with default parameter:**
   ```cpp
   // Before
   c_Array.IncLength();  // default by=1

   // After - must be explicit
   c_Array.resize(c_Array.size() + 1);
   // Or use append for adding single element:
   c_Array.append(T());
   ```

3. **Nested templates:**
   ```cpp
   // Before
   C_SclDynamicArray<C_SclDynamicArray<int32_t>> c_2DArray;

   // After
   QList<QList<int32_t>> c_2DArray;
   ```

4. **Include statement:**
   ```cpp
   // Remove
   #include "C_SclDynamicArray.hpp"

   // Add (if not already present)
   #include <QList>
   ```

### Execution Plan

#### Step 1: Identify all files
- 213 files contain C_SclDynamicArray usage
- Primary locations: can_dispatcher/, data_dealer/, halc/, imports/, kefex_diaglib/

#### Step 2: Migration order (by module)
1. **Leaf modules first** (minimal dependencies)
2. **Core modules** (higher impact)
3. **Update CMakeLists** if needed for Qt dependencies

#### Step 3: For each file
1. Replace `#include "C_SclDynamicArray.hpp"` with `#include <QList>`
2. Replace type declarations `C_SclDynamicArray<T>` → `QList<T>`
3. Replace method calls per mapping table
4. Handle GetHigh() loops carefully
5. Handle IncLength() default parameter

#### Step 4: Verification
1. Compile after each module
2. Run existing tests
3. Manual review of GetHigh() loop conversions

#### Step 5: Cleanup
1. Remove `C_SclDynamicArray.hpp` from scl/
2. Update any documentation referencing the class
3. Remove from CMakeLists source lists

---

## Phase 5: C_SclIniFile Migration to QSettings

### Priority: MEDIUM
**Actual Impact**: 8 files use C_SclIniFile (6 in opensyde_tool/src, 2 in opensyde_core)
**Complexity**: Medium (custom INI parser with comment support)
**Risk**: Medium (need to verify comment handling and special features)
**Selected Approach**: Option B - Migrate to direct QSettings usage

### Application Status

| Application | Status | Date | Notes |
|-------------|--------|------|-------|
| openSYDE | ✅ Complete | 2026-01-18 | C_SclIniFile now uses QSettings internally |
| SYDEflash | ✅ Complete | 2026-01-18 | Shares opensyde_core with openSYDE |
| openSYDE CAN Monitor | ✅ Complete | 2026-01-18 | Shares opensyde_core with openSYDE |
| opensyde_cmd_line_flash_tool | N/A | - | Has own copy in libs/osy_core/ - Qt not applicable |
| opensyde_syde_coder_c | N/A | - | Has own copy in libs/osy_core/ - Qt not applicable |
| opensyde_syde_sup | N/A | - | Has own copy in libs/opensyde_core/ - Qt not applicable |
| opensyde_syde_x_gen | N/A | - | Has own copy in libs/opensyde_core/ - Qt not applicable |

### Notes on Shared vs. Separate Core Libraries

The openSYDE ecosystem has **two patterns** for core library usage:

1. **Shared Core** (opensyde_tool applications):
   - `openSYDE`, `SYDEflash`, and `openSYDE CAN Monitor` all share `opensyde_tool/libs/opensyde_core/`
   - Changes to the core library automatically apply to all three applications
   - These applications are marked together in the matrix

2. **Separate Core Copies** (standalone CLI tools):
   - `opensyde_cmd_line_flash_tool`, `opensyde_syde_coder_c`, `opensyde_syde_sup`, `opensyde_syde_x_gen`
   - Each has its own copy of the core library in `libs/osy_core/` or `libs/opensyde_core/`
   - **IMPORTANT**: These CLI tools use **C++98** and do **NOT** have Qt as a dependency
   - Qt-specific migrations (Phases 4, 5) are **N/A** for CLI tools unless Qt is added as a dependency
   - Only pure C++ migrations (like `std::vector` replacements) would apply without adding Qt

### Current State Analysis (Updated 2026-01-18)

#### C_SclIniFile Overview
- **Purpose**: Custom INI file parser (Borland TIniFile compatibility)
- **Implementation**: NOW uses QSettings internally (previously custom parsing with C_SclStringList)
- **Location**: `opensyde_core/scl/C_SclIniFile.{cpp,hpp}`
- **Inheritance**: `C_OscChecksummedIniFile` extends `C_SclIniFile` with checksum functionality

#### Files Using C_SclIniFile
**opensyde_tool/src/ (6 files):**
- `project_operations/C_PopCreateServiceProjDialogWidget.cpp`
- `syde_flash/user_settings/C_UsFiler.cpp`
- `syde_flash/user_settings/C_UsFiler.hpp`
- `system_definition/C_SdTopologyToolbox.cpp`
- `system_definition/node_edit/canopen_manager/C_SdNdeCoAddDeviceDialog.cpp`
- `user_settings/C_UsFiler.cpp`

**opensyde_core/ (2 files):**
- `C_OscChecksummedIniFile.cpp` (extends C_SclIniFile)
- `C_OscChecksummedIniFile.hpp`

#### Current C_SclIniFile Public API
| Method | QSettings Equivalent |
|--------|---------------------|
| `ReadString(section, key, default)` | `value(section/key, default).toString()` |
| `ReadInteger(section, key, default)` | `value(section/key, default).toInt()` |
| `ReadBool(section, key, default)` | `value(section/key, default).toBool()` |
| `ReadFloat(section, key, default)` | `value(section/key, default).toDouble()` |
| `ReadUint8/ReadUint16(...)` | `value(...).toUInt()` with cast |
| `WriteString(section, key, value)` | `setValue(section/key, value)` |
| `WriteInteger(section, key, value)` | `setValue(section/key, value)` |
| `WriteBool(section, key, value)` | `setValue(section/key, value)` |
| `WriteFloat(section, key, value)` | `setValue(section/key, value)` |
| `EraseSection(section)` | `remove(section)` |
| `DeleteKey(section, key)` | `remove(section/key)` |
| `UpdateFile()` | `sync()` |
| `SectionExists(section)` | `childGroups().contains(section)` |
| `ValueExists(section, key)` | `contains(section/key)` |
| `ReadSection(section, strings)` | `beginGroup(section); childKeys(); endGroup()` |
| `ReadSectionValues(section, strings)` | Iterate keys with values |
| `ReadSections(strings)` | `childGroups()` |
| `GetFileAsStringList(strings)` | Custom serialization needed |

#### Special Considerations
1. **Comment Support**: C_SclIniFile preserves comments (`;` prefix). QSettings does NOT support comments - they will be lost on save.
2. **C_OscChecksummedIniFile**: Inherits from C_SclIniFile and adds checksum functionality. Will need to be updated.
3. **ForceAppend Parameter**: Some Write methods have `oq_ForceAppend` for performance. QSettings doesn't need this.
4. **In-Memory Mode**: Constructor with empty filename creates memory-only INI structure.
5. **Destructor Behavior**: Current destructor auto-saves if dirty. QSettings sync() is explicit.

### Migration Strategy (Option B)

#### Step 1: Create QSettings-based Replacement
Reimplement `C_SclIniFile` to use `QSettings` internally while maintaining the same public API.

**New Implementation Pattern:**
```cpp
class C_SclIniFile
{
private:
   std::unique_ptr<QSettings> mpc_Settings;
   QString mc_FileName;

public:
   C_SclIniFile(const C_SclString & orc_FileName) {
      mc_FileName = orc_FileName.ToQString();
      if (!mc_FileName.isEmpty()) {
         mpc_Settings = std::make_unique<QSettings>(mc_FileName, QSettings::IniFormat);
      }
   }

   C_SclString ReadString(const C_SclString & orc_Section, const C_SclString & orc_Key,
                          const C_SclString & orc_Default) {
      QString c_Key = orc_Section.ToQString() + "/" + orc_Key.ToQString();
      return C_SclString::FromQString(mpc_Settings->value(c_Key, orc_Default.ToQString()).toString());
   }
   // ... etc
};
```

#### Step 2: Handle C_OscChecksummedIniFile
Update `C_OscChecksummedIniFile` to work with the new QSettings-based implementation:
- Checksum calculation will need to iterate through QSettings structure
- May need to expose internal QSettings for checksum access

#### Step 3: Update Callers
Review and update all 6 calling files in opensyde_tool/src/:
- Verify they don't depend on comment preservation
- Update any direct member access patterns

#### Step 4: Testing Requirements
1. **INI file read/write round-trip**: Verify all data types
2. **Section operations**: Create, delete, enumerate sections
3. **Key operations**: Create, delete, check existence
4. **C_OscChecksummedIniFile**: Verify checksum still works
5. **Edge cases**: Empty sections, special characters, long values

#### Step 5: Cleanup
1. Remove custom parsing code from C_SclIniFile.cpp
2. Update includes to use `<QSettings>`
3. Remove dependency on C_SclStringList for file I/O

### Risk Mitigation

#### Comment Handling
**Issue**: QSettings doesn't preserve comments. If existing INI files have important comments, they will be lost.
**Mitigation**:
- Audit existing INI files for critical comments
- If comments are important, keep custom implementation for those specific files
- Document that comments will not be preserved

#### Checksum Compatibility
**Issue**: C_OscChecksummedIniFile calculates checksums over INI content.
**Mitigation**:
- Ensure checksum calculation produces same results after migration
- Test with existing checksummed INI files

### Files to Modify
1. `scl/C_SclIniFile.hpp` - Rewrite class to use QSettings
2. `scl/C_SclIniFile.cpp` - Replace custom parsing with QSettings calls
3. `C_OscChecksummedIniFile.cpp` - Update checksum calculation
4. `CMakeLists.txt` - May need to verify Qt Core is linked (should already be)

---

## Phase 6: C_SclChecksums and C_SclResourceStrings (Optional)

### Priority: VERY LOW
**Impact**: Minimal
**Complexity**: Low

### Application Status

| Application | Status | Date | Notes |
|-------------|--------|------|-------|
| openSYDE | ❌ Not Started | - | Optional - evaluate need |
| SYDEflash | ❌ Not Started | - | Shares opensyde_core with openSYDE |
| openSYDE CAN Monitor | ❌ Not Started | - | Shares opensyde_core with openSYDE |
| opensyde_cmd_line_flash_tool | ❌ Not Started | - | Has own copy in libs/osy_core/ |
| opensyde_syde_coder_c | ❌ Not Started | - | Has own copy in libs/osy_core/ |
| opensyde_syde_sup | ❌ Not Started | - | Has own copy in libs/opensyde_core/ |
| opensyde_syde_x_gen | ❌ Not Started | - | Has own copy in libs/opensyde_core/ |

### C_SclChecksums
**Current Purpose**: Checksum calculation utilities
**Qt Alternative**: QCryptographicHash
**Recommendation**: Migrate only if needed for consistency

### C_SclResourceStrings
**Current Purpose**: String resource management
**Qt Alternative**: Qt Linguist / translation system
**Recommendation**: Evaluate if used; may be legacy

---

## Execution Order and Timeline

### Recommended Sequence

1. **Phase 2: C_SclString → QString** (HIGHEST PRIORITY)
   - Start with Phase 2A (Data Model Layer)
   - Continue through 2B-2E incrementally
   - Estimate: 4-8 weeks depending on thoroughness

2. **Phase 3: C_SclStringList → QStringList** (AFTER Phase 2)
   - Depends on Phase 2 completion
   - Estimate: 1-2 weeks

3. **Phase 4: C_SclDynamicArray → QList** (CAN RUN PARALLEL to Phase 3)
   - Independent of string migration
   - Good candidate for automation
   - Estimate: 1 week

4. **Phases 5-6: Optional cleanup** (AS NEEDED)
   - Only if pursuing absolute minimal wrapper count
   - Estimate: 1 week

### Total Estimated Timeline
- **Conservative**: 6-11 weeks
- **Aggressive**: 4-6 weeks (with automation and parallel work)

---

## Risk Mitigation

### Key Risks

1. **Index Off-by-One Errors** (C_SclString migration)
   - **Mitigation**: Comprehensive unit tests, manual audit of SubString/Pos calls
   - **Severity**: HIGH

2. **Unicode/Encoding Issues** (C_SclString migration)
   - **Mitigation**: Test with special characters, international text
   - **Severity**: MEDIUM

3. **Performance Regression**
   - **Mitigation**: Benchmark critical paths before/after
   - **Severity**: LOW (QString generally faster)

4. **API Breaking Changes**
   - **Mitigation**: Update all consumers in same changeset
   - **Severity**: MEDIUM (internal codebase only)

### Testing Requirements

For each phase:
- ✅ Unit tests for migrated modules
- ✅ Integration tests across module boundaries
- ✅ Manual testing of critical features
- ✅ Performance benchmarking of hot paths
- ✅ Unicode/special character testing (Phase 2)
- ✅ Index boundary testing (Phase 2)

---

## Success Metrics

### Phase Completion Criteria

Each phase is complete when:
1. ✅ All target files migrated
2. ✅ All tests passing
3. ✅ No compiler warnings introduced
4. ✅ Code review approved
5. ✅ Legacy wrapper classes removed from build
6. ✅ Documentation updated

### Overall Project Success

Project is successful when:
1. ✅ All SCL wrapper classes eliminated (or justified exceptions documented)
2. ✅ Codebase uses Qt native types throughout
3. ✅ No performance regressions
4. ✅ All functionality maintained
5. ✅ Build system simplified (fewer custom dependencies)

---

## Appendix: Migration Tools

### Automated Migration Scripts

#### A.1 C_SclString → QString Helper Script
```python
#!/usr/bin/env python3
"""
Script to assist with C_SclString to QString migration
"""

import re
import sys

def convert_file(filepath):
    replacements = [
        # Member access patterns
        (r'\.Length\(\)', r'.length()'),
        (r'\.UpperCase\(\)', r'.toUpper()'),
        (r'\.LowerCase\(\)', r'.toLower()'),
        (r'\.Trim\(\)', r'.trimmed()'),

        # Note: SubString and Pos require manual review due to indexing!
        # (r'\.SubString\((\d+), (\d+)\)', r'.mid(\1-1, \2)'),

        # Static methods
        (r'C_SclString::IntToStr\(', r'QString::number('),

        # Type declarations
        (r'C_SclString\s+', r'QString '),
        (r'<C_SclString>', r'<QString>'),
    ]

    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    for pattern, replacement in replacements:
        content = re.sub(pattern, replacement, content)

    # Flag manual review items
    if '.SubString(' in content or '.Pos(' in content:
        print(f"⚠️  {filepath} - MANUAL REVIEW REQUIRED (SubString/Pos indexing)")

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)

if __name__ == '__main__':
    convert_file(sys.argv[1])
```

#### A.2 C_SclDynamicArray → QList Script
```python
#!/usr/bin/env python3
"""
Script to convert C_SclDynamicArray to QList
"""

import re
import sys

def convert_file(filepath):
    replacements = [
        (r'C_SclDynamicArray<(.+?)>', r'QList<\1>'),
        (r'\.GetLength\(\)', r'.size()'),
        (r'\.GetHigh\(\)', r'(.size() - 1)'),
        (r'\.SetLength\(', r'.resize('),
        (r'\.Delete\(', r'.removeAt('),
        # IncLength needs special handling
    ]

    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    for pattern, replacement in replacements:
        content = re.sub(pattern, replacement, content)

    # Special handling for IncLength
    # .IncLength() → .resize(size() + 1)
    # .IncLength(n) → .resize(size() + n)
    content = re.sub(r'\.IncLength\(\)', r'.resize(size() + 1)', content)
    content = re.sub(r'\.IncLength\((.+?)\)', r'.resize(size() + \1)', content)

    with open(filepath, 'w', encoding='utf-8') as f:
        f.write(content)

if __name__ == '__main__':
    convert_file(sys.argv[1])
```

---

## Document Revision History

| Date | Version | Changes |
|------|---------|---------|
| 2026-01-16 | 1.0 | Initial plan created after Phase 1 completion |
| 2026-01-18 | 1.1 | Detailed scope analysis completed. Updated impact estimate from 4,940 to 25,675 occurrences. Added module dependency analysis, critical API dependencies, and recommended migration sequence starting with C_OscUtils. |
| 2026-01-18 | 1.2 | Updated Phase 4 with accurate metrics (689 occurrences in 213 files). Corrected internal implementation (wraps std::vector, not QList). Added detailed migration patterns and special case handling. Noted QVector is alias for QList in Qt 6. |
| 2026-01-18 | 1.3 | **Phase 4 Complete for opensyde_tool**: Removed C_SclDynamicArray.hpp, updated C_SclIniFile to use QList. Other tool projects retain their own copies. |
| 2026-01-18 | 1.4 | **Phase 5 Updated**: Corrected current state (does NOT use QSettings). Selected Option B - migrate to QSettings. Added detailed API mapping, special considerations (comments, checksum), and step-by-step migration plan. |
| 2026-01-18 | 1.5 | **Phase 5 Complete for opensyde_tool**: Rewrote C_SclIniFile to use QSettings internally (~810 lines vs ~1100 lines). Removed C_SclIniSection/C_SclIniKey classes. C_OscChecksummedIniFile unchanged (uses public API). Breaking: comments no longer preserved, ForceAppend ignored. |
| 2026-01-18 | 1.6 | **Added Application Status Matrix**: Added per-application tracking for all 7 openSYDE tools. Each phase now includes an Application Status table showing completion per tool. Documented shared vs. separate core library patterns. |

---

## Notes

- This plan focuses on **opensyde_tool** and **opensyde_core** libraries
- Other tool projects (opensyde_cmd_line_flash_tool, opensyde_syde_coder_c, etc.) may need similar migrations
- Plan assumes continued Windows-only support (Linux support removed in Phase 1)
- All Qt references assume Qt 6.x (currently using Qt 6.10.1)
- **Key Discovery**: C_SclString already has `ToQString()` and `FromQString()` methods - migration was anticipated in original design
- **GUI Layer Status**: Already has 61+ conversion calls at boundaries - partial migration already started

---

**Last Updated**: 2026-01-18
**Status**: Phase 1 Complete, Phase 4 Complete (opensyde_tool), Phase 5 Complete (opensyde_tool), Phase 2 Detailed Planning Complete, Ready to Begin Phase 2-PREP (C_OscUtils)
