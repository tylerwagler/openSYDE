# Additional STL Types Migration Investigation Report

## Summary
In addition to vectors and maps, `std::set` and `std::pair` are significant STL types used in the openSYDE codebase that have Qt-native equivalents.

## Findings

### 1. Codebase Statistics (as of 2026-02-04)
The `opensyde_tool/` directory shows the following additional STL usage:

| Container Type | Occurrences | Qt Equivalent | Notes |
| :--- | :--- | :--- | :--- |
| `std::set` | 255 | `QSet` | Moderate usage. Significant for uniqueness/membership checks. |
| `std::pair` | 131 | `QPair` | Common in various configurations and return types. |
| `std::deque` | 3 | -- | Very rare. Recommend keeping `std::deque` for performance. |
| `std::unique_ptr` | 2 | -- | Prefer keeping `std::unique_ptr` over `QScopedPointer` (Qt 6 best practice). |

### 2. Analysis

#### QSet vs std::set
- `std::set` is sorted (balanced tree); `QSet` is unordered (hash table).
- `QSet` is generally faster for lookup and insertion ($O(1)$ amortized vs $O(\log n)$).
- **Migration Caution**: If code relies on the sorted order of `std::set`, use `QMap<T, void>` as a workaround or keep `std::set`.

#### QPair vs std::pair
- Structurally identical in most use cases.
- `QPair` integrates better with `QList`, `QMap`, and other Qt APIs.

## Recommendation
Include `std::set` and `std::pair` in the **opportunistic migration** plan. Convert them in files that are already being modified for the `QString` or `std::vector` migrations.

## Decision
**MIGRATE OPPORTUNISTICALLY**
