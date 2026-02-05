# std::map to QMap/QHash Migration Investigation Report

## Summary
Migrating from `std::map` to Qt-native map containers (`QMap` and `QHash`) is recommended to improve consistency with the Qt-native strategy and leverage Qt 6's performance optimizations, especially for `QString` keys.

## Findings

### 1. Codebase Statistics (as of 2026-02-04)
The `opensyde_tool/` directory shows the following map usage:

| Container Type | Occurrences | Notes |
| :--- | :--- | :--- |
| `std::map` | 640 | Most common map container. |
| `std::unordered_map` | 0 | Not used in the tool codebase. |
| `QMap` | 447 | Already widely used in many components. |
| `QHash` | 6 | Rare usage. |

### 2. Comparison: QMap vs QHash (Qt 6)

| Feature | `QMap<K, T>` | `QHash<K, T>` |
| :--- | :--- | :--- |
| **Ordering** | Sorted by key. | Unordered. |
| **Data Structure** | Red-Black Tree. | Hash Table. |
| **Lookup Speed** | O(log N). | Amortized O(1). |
| **Memory** | Lower per-item overhead. | higher per-item overhead. |
| **Best For** | When order matters. | Fast lookups with `QString` keys. |

### 3. Recommendation

I recommend a **phased migration** with a strong preference for `QHash` when using `QString` keys, unless sorting is required.

#### Phase 1: High-Impact Conversions
- **`std::map<QString, T>` → `QHash<QString, T>`**: (70 occurrences). `QHash` is significantly faster for string lookups and better integrates with Qt's string handling.
- **`std::map<K, T>` → `QMap<K, T>`**: General replacement for internal logic where order is important or assumed.

#### Phase 2: Incremental Replacement
- Like the `std::vector` migration, maps can be migrated file-by-file during other refactoring tasks (like Phase 1 "Quick Wins" or `QString` migration).

### Exceptions
- **External Headers**: Keep `std::map` in public headers of libraries (`blf_driver_library`, `dbc_driver_library`) to maintain binary compatibility and avoid forcing Qt dependencies on users of those libraries.

## Decision
**MIGRATE TO QMAP/QHASH**

**Status Updates**:
1. Update coding standards to prefer `QHash` for string-keyed maps.
2. Update existing migration plans to include map conversions in targeted files.
