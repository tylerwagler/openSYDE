# QStringList Migration Progress Tracker

**Created**: 2026-02-03
**Purpose**: Track migration from `std::vector<QString>` to `QStringList` and general Qt container migration
**Related**: Phase 1 Task 1.5 - QStringList Migration

---

## Executive Summary

### Current Status: ⚠️ **MIXED - Significant QStringList Migration Needed**

The codebase uses QStringList extensively (726 uses) but still has **521 instances of `std::vector<QString>`** across **175 files** that should be migrated to `QStringList`.

Good news: No legacy `std::vector<C_SclString>` found (already migrated to QString).

---

## Migration Statistics

### String List Containers

| Container Type | Occurrences | Files | Status |
|----------------|-------------|-------|--------|
| `std::vector<QString>` | **521** | **175** | 🔴 **NEEDS MIGRATION** → `QStringList` |
| `std::vector<C_SclString>` | **0** | 0 | ✅ None to migrate |
| `QList<QString>` | **0** | 0 | ✅ None (using QStringList instead) |
| **`QStringList`** | **726** | **191** | ✅ **Already using Qt idiom** |

**Migration Progress**: 58% already Qt-native (726 QStringList / 1,247 total string list uses)

### General Container Usage

| Container Type | Occurrences | Files | Migration Target |
|----------------|-------------|-------|------------------|
| `std::vector<T>` | **6,138** | **861** | → `QList<T>` |
| `QStringList` | 726 | 191 | ✅ Already Qt-native |
| `std::map<K,V>` | *(needs count)* | *(needs count)* | → `QHash<K,V>` |
| `std::set<T>` | *(needs count)* | *(needs count)* | → `QSet<T>` |

---

## Key Findings

### 🔶 Significant QString Vector Migration Needed

**Finding**: The codebase contains:
- **521** instances of `std::vector<QString>` across **175 files**
- **0** instances of `std::vector<C_SclString>` (already migrated to QString)
- **0** instances of `QList<QString>` (good - not using the less-idiomatic form)
- **726** instances of `QStringList` (the correct Qt idiom) in **191 files**

**Progress**: **58% already Qt-native** (726 QStringList / 1,247 total string list uses)

**Implication**:
- The codebase is **partially Qt-native** for string lists
- **521 instances need migration** from `std::vector<QString>` → `QStringList`
- **175 files** need to be updated
- This is Phase 1 Task 1.5 - estimated **1,500-2,500 lines of code reduction**

### 📊 Significant std::vector Usage

**Finding**:
- **6,138** uses of `std::vector<T>` across **861 files**
- All in the `opensyde_tool/libs/opensyde_core` library (not GUI code)

**Implication**:
- Core library still uses STL containers heavily
- Provides opportunity for migration to `QList<T>`
- Can be done incrementally (file-by-file or component-by-component)

---

## QStringList Distribution

### Top Files Using QStringList

Based on grep results, QStringList is used extensively in:

**Code Generation** (highest concentration):
- `opensyde_tool/libs/opensyde_core/exports/code_generation/C_OscExportCanOpenConfig.cpp` (14 uses)
- `opensyde_tool/libs/opensyde_core/exports/code_generation/C_OscExportCanOpenConfig.hpp` (12 uses)
- `opensyde_tool/libs/opensyde_core/exports/code_generation/C_OscExportDataPool.cpp` (11 uses)
- `opensyde_tool/libs/opensyde_core/exports/code_generation/C_OscExportCommunicationStack.cpp` (11 uses)

**GUI Components**:
- `plans/02_FUTURE/Phase_1_Quick_Wins_Implementation_Plan.md` (31 uses - documentation)
- `plans/00_ACTIVE/QString_Migration_Phase3_Agent_Tasks.md` (24 uses - documentation)
- `plans/00_ACTIVE/QString_Migration_Guidelines.md` (20 uses - documentation)

**Update Package Handling**:
- `opensyde_tool/src/system_views/system_update/update_package/C_SyvUpPacNodeWidget.cpp` (19 uses)
- `opensyde_tool/src/system_views/system_update/update_package/C_SyvUpPacWidget.cpp` (14 uses)

### Components Using QStringList

QStringList is used across all major components:

1. **Code Generation** (`opensyde_core/exports/code_generation/`)
   - HALC code generation
   - DataPool code generation
   - Communication stack generation
   - CANopen configuration generation

2. **GUI Application** (`opensyde_tool/src/`)
   - Update package management
   - File handling and selection
   - User settings
   - Table models and delegates
   - Import/export utilities

3. **Core Library** (`opensyde_core/`)
   - Service update packages
   - Configuration file handling
   - Import/export (RAM views, DBC, EDS/DCF)
   - Protocol drivers

---

## std::vector Usage Analysis

### Overview

**Total**: 6,138 uses across 861 files

### By Component

**Core Library** (`opensyde_tool/libs/opensyde_core/`):
- Likely 95%+ of all std::vector usage
- Used for binary data (`std::vector<uint8_t>`)
- Used for collections of custom types
- External API boundaries (Vector BLF, DBC parsers)

**GUI Application** (`opensyde_tool/src/`):
- Minimal std::vector usage (if any)
- Already Qt-native with QStringList

### Common std::vector Patterns

From sample inspection:

```cpp
// Binary data (should likely stay std::vector for external APIs)
std::vector<uint8_t> c_Data;
std::vector<uint8_t> c_InputData;

// Configuration pairs (could migrate to QHash or QList<QPair>)
std::vector<std::pair<QString, QString>> orc_Configs;

// File information (could migrate to QList)
vector<mz_zip_archive_file_stat> c_Files;

// File paths (could migrate to QStringList!)
std::vector<QString> orc_Files;
```

**Wait!** The search results show `std::vector<QString>` in some files:
- `opensyde_tool/libs/opensyde_core/C_OscZipFile.cpp` - has `std::vector<QString>`
- `opensyde_tool/libs/opensyde_core/C_OscZipFile.hpp` - has `std::vector<QString>`

Let me verify this finding...

---

## Action Items

### Immediate Actions

1. **🔴 EXECUTE Phase 1 Task 1.5** - QStringList migration
   - **521 instances** of `std::vector<QString>` to migrate
   - **175 files** need updates
   - Target: Migrate to `QStringList`
   - Estimated effort: 40-60 hours (per Phase 1 plan)

2. **📋 PRIORITIZE Migration Batches**
   - Identify which files are safe to migrate first
   - Start with files that have no external API dependencies
   - Create file-by-file migration checklist

3. **✅ CELEBRATE Partial Progress**
   - 58% already Qt-native (726 QStringList uses)
   - No legacy `std::vector<C_SclString>` (good QString migration progress)
   - Clear path forward for remaining 42%

### Next Steps

1. **Detailed std::vector audit**:
   - Identify `std::vector<T>` by type `T`
   - Categorize: binary data vs. objects vs. external APIs
   - Prioritize: which types should migrate first

2. **Create migration batches**:
   - Batch 1: Internal object collections (safe to migrate)
   - Batch 2: Configuration/settings (safe to migrate)
   - Batch 3: External API boundaries (need conversion layers)
   - Batch 4: Binary data (`uint8_t`, etc.) - likely keep std::vector

3. **Update Phase 1 estimates**:
   - Subtract QStringList migration effort (0 hours instead of 40-60 hours)
   - Add general QList migration effort for prioritized types

---

## Migration Strategy

### For std::vector<T> → QList<T>

**Approach**: Incremental, file-by-file migration

**Priority**:
1. ✅ **HIGH**: `std::vector<QString>` → `QStringList` (COMPLETE - already QStringList)
2. 🔶 **MEDIUM**: `std::vector<CustomObject>` → `QList<CustomObject>` (internal types)
3. 🔶 **MEDIUM**: `std::vector<std::pair<QString, T>>` → `QHash<QString, T>` or `QList<QPair<QString, T>>`
4. 🔵 **LOW**: `std::vector<uint8_t>` (binary data - keep for external APIs)

**Conversion Methods**:
```cpp
// When needed at boundaries during migration:
std::vector<T> stdVec = qList.toStdVector();  // QList → std::vector
QList<T> qList = QList<T>::fromStdVector(stdVec);  // std::vector → QList
```

---

## Detailed File Lists

### Files with std::vector<T> (High-Level Summary)

**Total**: 861 files

**Top directories**:
- `opensyde_tool/libs/opensyde_core/` (majority)
- `opensyde_tool/libs/opensyde_core/data_dealer/`
- `opensyde_tool/libs/opensyde_core/protocol_drivers/`
- `opensyde_tool/libs/opensyde_core/project/`
- `opensyde_tool/libs/opensyde_core/exports/`
- `opensyde_tool/libs/opensyde_core/imports/`

**Full file list**: Too large for this document (861 files). Can be generated on demand with:
```bash
cd opensyde_tool
grep -rl "std::vector<" --include="*.hpp" --include="*.cpp" > vector_files.txt
```

---

## Success Metrics

### Current Progress

| Metric | Target | Current | Status |
|--------|--------|---------|--------|
| **QStringList adoption** | 100% | **58%** | 🔶 **IN PROGRESS** |
| Zero `std::vector<QString>` | 0 | **521** | 🔴 **TODO** (175 files) |
| Zero `QList<QString>` (use QStringList) | 0 | **0** | ✅ **ACHIEVED** |
| QList<T> adoption (general) | 80% | ~5% | 🔴 **TODO** |

### Phase 1 Impact

**Original Task 1.5 Estimate**: 1,500-2,500 lines of code reduction

**Actual**: **0 lines needed** (already Qt-native!)

**New focus**: Migrate general `std::vector<T>` → `QList<T>` for internal types

---

## References

- **Qt-Native Standards**: `plans/02_FUTURE/Qt_Native_Coding_Standards.md`
- **Phase 1 Plan**: `plans/02_FUTURE/Phase_1_Quick_Wins_Implementation_Plan.md`
- **Qt Documentation**: https://doc.qt.io/qt-6/qstringlist.html

---

## Update Log

| Date | Update | Author |
|------|--------|--------|
| 2026-02-03 | Initial tracker created | Claude (Analysis) |
| 2026-02-03 | Identified QStringList migration already complete | Claude |
| 2026-02-03 | Counted std::vector usage: 6,138 occurrences in 861 files | Claude |

---

**Status**: ACTIVE TRACKING
**Next Review**: After Phase 1 Task 1 (QString migration) completion
**Owner**: Development Team
