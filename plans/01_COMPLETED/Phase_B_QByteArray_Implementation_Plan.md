# Phase B: std::vector<uint8_t> → QList/QByteArray Migration

**Date:** 2026-02-06
**Status:** Ready to start
**Scope:** 706 `vector<uint8_t` occurrences across ~150 files

## Context

Phase A (`std::vector<T>` → `QList<T>` for all non-uint8_t types) is complete. Phase B handles the remaining byte-oriented vectors.

### Current Status

| Metric | Count |
|--------|-------|
| `vector<uint8_t` patterns | 706 |
| Estimated files | ~150 |
| GUI layer | ~500 |
| Core library | ~206 |

Main openSYDE GUI builds successfully after Phase A. CAN Monitor fails due to Qt signals using `std::vector<uint8_t>` (no QDebug operator<<).

## Migration Strategy

### Two Approaches Based on Usage

**1. QList<uint8_t> - For Collections & Qt Integration**
- Qt signals/slots parameters
- Collections of small integer values (flags, IDs, indices)
- Function parameters that represent lists of numbers
- Integrates cleanly with Qt meta-object system

**2. QByteArray - For Raw Binary Data**
- CAN message data buffers
- Binary file data
- Network packet buffers
- Cryptographic data (MD5, hashes)

### Conversion Rules

#### Common to Both

| Pattern | Before | After (context-dependent) |
|---------|--------|--------------------------|
| Include | `#include <vector>` | `#include <QList>` or `<QByteArray>` |
| Type | `std::vector<uint8_t>` | `QList<uint8_t>` or `QByteArray` |
| Nested | `std::vector<std::vector<uint8_t>>` | `QList<QByteArray>` |
| `.size()` | works | works (both) |
| `.clear()` | works | works (both) |
| `.empty()` | keep or change | `.isEmpty()` |

#### QList<uint8_t> Specific

| Pattern | Before | After |
|---------|--------|-------|
| Access | `vec[i]` | `list[i]` |
| Append | `.push_back(val)` | `.append(val)` or `.push_back(val)` |
| Reserve | `.reserve(n)` | `.reserve(n)` |
| Resize | `.resize(n)` | `.resize(n)` |
| Range-for | `for (auto val : vec)` | `for (auto val : list)` |

#### QByteArray Specific

| Pattern | Before | After |
|---------|--------|-------|
| Access | `vec[i]` (uint8_t) | `static_cast<uint8_t>(ba[i])` or `ba[i]` (returns char) |
| Data ptr | `vec.data()` (uint8_t*) | `reinterpret_cast<uint8_t*>(ba.data())` |
| Append | `.push_back(val)` | `.append(static_cast<char>(val))` |
| Resize | `.resize(n)` | `.resize(n)` |
| From array | `std::vector<uint8_t>(arr, arr+len)` | `QByteArray(reinterpret_cast<const char*>(arr), len)` |

### Decision Heuristic

Use QList<uint8_t> when:
- ✅ Qt signal/slot parameter
- ✅ Represents a collection of small numbers (flags, IDs)
- ✅ No pointer arithmetic on `.data()`
- ✅ Primarily accessed via `[]` or iterators

Use QByteArray when:
- ✅ CAN message data (`au8_Data` buffers)
- ✅ Binary file I/O
- ✅ Cryptographic operations (MD5, checksums)
- ✅ Network protocol data
- ✅ Pointer arithmetic or C-style API interaction

## Phase B Batches

### Round 1: GUI - CAN Monitor Filters (High Priority)

**B1 - CAN Monitor: Filter & Trace** (~80 occ, ~15 files)
- **Type:** `QList<uint8_t>` (these are XTD flags, not byte buffers)
- `can_monitor/can_monitor_settings/` - C_CamMosFilter*, C_CamMosWidget
- `can_monitor/message_trace/` - C_CamMetTreeView signal
- `can_monitor/C_CamMainWindow` - filter signal forwarding
- **Reason:** These use Qt signals, causing QDebug operator<< errors

### Round 2: GUI - CAN Monitor Message Data

**B2 - CAN Monitor: Message Generator Signals** (~120 occ, ~10 files)
- **Type:** Mix - signals use QList, buffers use QByteArray
- `can_monitor/message_generator/signals/` - C_CamGenSigTableModel, C_CamGenSigUtil
- `can_monitor/message_generator/` - C_CamGenTableView, C_CamGenMessagesWidget
- **Reason:** Raw CAN data handling + Qt signal parameters

**B3 - GUI: System Views Communication** (~90 occ, ~20 files)
- **Type:** QByteArray (CAN message data)
- `system_views/communication/` - C_SyvComDriver*, C_SyvComMessageLogger*
- `system_views/dashboards/` - data element handling
- **Reason:** Binary CAN/protocol data

### Round 3: GUI - Remaining

**B4 - GUI: Implementation & Project GUI** (~60 occ, ~15 files)
- **Type:** QByteArray (file data, binary serialization)
- `implementation/` - C_ImpUtil
- `project_gui/` - C_PuiSv*, C_PuiSd* (serialization data)
- `graphic_items/` - C_GiImage*, C_GiBiImageGroup (image data)

**B5 - GUI: System Definition & Flash** (~150 occ, ~25 files)
- **Type:** Mix
- `system_definition/node_edit/` - datapool handling
- `syde_flash/` - firmware data
- `table_base/` - data formatting

### Round 4: Core Library

**B6 - Core: Protocol Drivers** (~110 occ, ~20 files)
- **Type:** QByteArray (binary protocol data)
- `protocol_drivers/communication/` - C_OscComDriver*, protocol implementations
- `protocol_drivers/system_update/` - C_OscSuSequences, firmware transfer
- `ip_dispatcher/`, `can_dispatcher/` - network/CAN binary data

**B7 - Core: Project System & Node** (~40 occ, ~15 files)
- **Type:** QByteArray (binary node properties, data logger data)
- `project/system/node/` - C_OscNode*, C_OscNodeDataPool*, data_logger/
- `project/system/` - C_OscDeviceDefinition, binary configurations

**B8 - Core: HALC, Exports, Imports** (~60 occ, ~20 files)
- **Type:** QByteArray (configuration data, code generation)
- `halc/` - C_OscHalc* (binary config data)
- `exports/` - code generation, DBC export
- `imports/` - DBC import, EDS parsing
- `md5/` - C_Md5Checksum (byte buffers)

## Execution Plan

1. **Round 1 (B1)**: Fix CAN Monitor filter signals → build CAN Monitor → verify
2. **Round 2 (B2-B3)**: CAN Monitor message data + SysView comm → build → verify
3. **Round 3 (B4-B5)**: GUI remaining → build all GUI → verify
4. **Round 4 (B6-B8)**: Core library → build all → verify
5. **Cleanup**: Remove stale `#include <vector>` where no std::vector remains
6. **Final verification**: Build all three targets clean → commit

### Build Command

```powershell
cd opensyde_tool/bat
.\build.ps1 -Component All -SkipDeploy
```

### Verification

After Phase B completion:
```powershell
# Should return 0
grep -rn "std::vector<uint8_t" opensyde_tool/src opensyde_tool/libs/opensyde_core --include="*.cpp" --include="*.hpp" | wc -l
grep -rn "vector<uint8_t" opensyde_tool/src opensyde_tool/libs/opensyde_core --include="*.cpp" --include="*.hpp" | wc -l
```

## Known Challenges

1. **Qt Signal Registration**: Qt signals with QByteArray parameters work, but debug output differs from QList
2. **Pointer Arithmetic**: QByteArray requires explicit casts for uint8_t* operations
3. **C API Boundaries**: Some core library functions interact with C APIs expecting `unsigned char*`
4. **Existing QByteArray Usage**: 66 QByteArray instances already exist - ensure consistency

## Success Criteria

- ✅ All three targets build clean (GUI, CAN Monitor, SYDEflash)
- ✅ Zero `std::vector<uint8_t>` or bare `vector<uint8_t>` remaining
- ✅ Qt signals work without QDebug operator<< errors
- ✅ Binary data handling maintains correctness (endianness, size)
- ✅ No unnecessary casts or type conversions

## Post-Phase B

After Phase B completion, the Qt container migration is finished. Final cleanup:
- Remove unnecessary `#include <vector>` where no std::vector remains
- Remove `using namespace std;` where only for vector (if std::map etc. also gone)
- Update CLAUDE.md to reflect completed migration
- Final commit: "Complete Qt container migration: Phase B QByteArray/QList<uint8_t>"
