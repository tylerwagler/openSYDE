# std::map to QMap/QHash Migration Investigation

**Created**: 2026-02-04
**Status**: COMPLETED - 2026-02-04
**Report**: [std_map_to_QMap_Investigation_Report.md](std_map_to_QMap_Investigation_Report.md)
**Priority**: TBD
**Related Work**: QString Migration, std::vector to QList Migration

---

## Overview

Investigate whether openSYDE should migrate from `std::map` to Qt container classes (`QMap` or `QHash`) as part of the broader Qt native replacement strategy.

---

## Questions to Investigate

### 1. Technical Considerations

#### Performance & Characteristics
- [ ] Compare `std::map` (ordered) vs `QMap` (ordered) performance.
- [ ] Compare `std::unordered_map` vs `QHash` (unordered) performance.
- [ ] Evaluate the impact of "Implicit Sharing" in Qt maps/hashes.
- [ ] Analyze the overhead of `QMap` (skip-list/red-black tree) vs `QHash` (hash table).

#### Qt Integration
- [ ] How well do these integrate with Qt APIs?
- [ ] Search/lookup convenience with `QString` keys.

### 2. Investigation Tasks

- [x] Task 1: Data Collection - Count `std::map` and `std::unordered_map` occurrences. (Completed)
- [x] Task 2: API Analysis - Identify public APIs and external library constraints. (Completed)
- [x] Task 3: Decision - Recommend between `QMap` and `QHash` for specific use cases. (Completed)

---

## Decision Framework

### Prefer QMap if:
- Order matters (sorted by key).
- Minimal memory overhead for small maps is preferred.

### Prefer QHash if:
- Speed of lookup is critical.
- Using `QString` as a key (highly optimized in Qt).
- Order does not matter.

---

## Document Status
**Last Updated**: 2026-02-04
**Action**: Investigation started.
