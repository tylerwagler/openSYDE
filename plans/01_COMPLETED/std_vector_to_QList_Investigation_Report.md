# std::vector to QList Migration Investigation Report

## Summary
The investigation into migrating from `std::vector` to `QList` (and `QStringList`) confirms that this is a beneficial move for the openSYDE project, aligning it with modern Qt 6 practices while maintaining performance and improving consistency.

## Findings

### 1. Codebase Statistics (as of 2026-02-04)
The `opensyde_tool/` directory shows the following container usage:

| Container Type | Occurrences | Notes |
| :--- | :--- | :--- |
| `std::vector` | 6,219 | Ubiquitous throughout the source code. |
| `std::vector<QString>` | 615 | Primary candidates for conversion to `QStringList`. |
| `QList` | 905 | Already in use in many areas. |

### 2. Performance Analysis (Qt 6)
- **Memory Layout**: In Qt 6, `QList` is an alias for `QVector` and uses **contiguous memory**, making it performance-equivalent to `std::vector` for random access and iteration.
- **Append Performance**: Amortized O(1), similar to `std::vector`.
- **Implicit Sharing**: `QList` uses copy-on-write (COW). This provides a significant advantage when containers are passed through signals/slots or returned from functions by value, as it avoids deep copies unless the data is modified.

### 3. API Integration
- **Qt Compatibility**: Most Qt functions (e.g., in `QtWidgets`, `QtGui`) return or accept `QList`. Migrating reduces the need for frequent conversions.
- **External Libraries**: `blf_driver_library` and `dbc_driver_library` use `std::vector` in their public headers. These should remain as-is to avoid breaking external dependencies or requiring massive refactors of third-party-style code.

## Recommendation: MIGRATE (Phased Approach)

A phased migration to `QList` is recommended, prioritizing areas currently being refactored for the `QString` migration.

### Phase 1: High-Impact Conversions (Immediate)
- **`std::vector<QString>` → `QStringList`**: Over 600 occurrences. This provides access to `QStringList`'s powerful string manipulation methods (e.g., `join`, `filter`).
- **New Code**: All new code should use `QList` by default unless `std::vector` is strictly required for interfacing with a library.

### Phase 2: General Refactoring (Ongoing)
- **`std::vector<T>` → `QList<T>`**: Convert internal tool code as files are touched for other reasons.
- **`std::map<QString, T>` → `QHash<QString, T>`**: (Related) Recommended for better lookup performance with string keys.

### Exceptions (Keep `std::vector`)
- **Third-party Library Drivers**: `blf_driver_library`, `dbc_driver_library`.
- **Low-level Bit/Byte Buffers**: Where `std::vector<uint8_t>` is used for raw data processing and interoperability with non-Qt code is primary.

## Decision
**MIGRATE TO QLIST/QSTRINGLIST**

**Status Updates**:
1. Incorporate `std::vector<QString>` → `QStringList` into the existing `QString` migration tasks.
2. Update coding standards to prefer `QList` over `std::vector`.
3. Begin conversion starting with the `Device Configuration` and `CAN Monitor` modules.
