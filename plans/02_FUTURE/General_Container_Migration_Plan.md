# General Container Migration Plan

**Created**: 2026-02-22
**Last Updated**: 2026-02-22
**Goal**: Migrate remaining std::vector<T> and std::map<K,V> containers to Qt-native equivalents

---

## Overview

Following the successful completion of the QString migration project, the next major opportunity for codebase improvement is the migration of remaining STL containers to Qt-native equivalents. This work will:

- Reduce codebase size by eliminating redundant STL container usage
- Improve consistency with Qt-native coding standards
- Leverage Qt's optimized container implementations (QList, QHash, QSet)
- Improve integration with Qt's signal/slot system and property system

## Migration Targets

### Priority 1: std::vector<T> → QList<T>
- **Current state**: 6,138 occurrences across 861 files
- **Target**: Migrate to QList<T> for internal data structures
- **Exceptions**: Keep std::vector<uint8_t> for binary data with external APIs

### Priority 2: std::map<K,V> → QHash<K,V>
- **Current state**: Approximately 640 occurrences (based on previous audits)
- **Target**: Migrate to QHash<K,V> for string-keyed maps
- **Target**: Migrate to QMap<K,V> for sorted collections where order matters

### Priority 3: std::set<T> → QSet<T>
- **Current state**: Approximately 255 occurrences (based on previous audits)
- **Target**: Migrate to QSet<T> for unique element collections

## Migration Strategy

### Phase 1: Audit and Analysis (1 week)
1. Generate detailed usage statistics:
   ```bash
   # Find all std::vector<T> usage
   grep -r "std::vector<" opensyde_tool/libs/opensyde_core/ --include="*.cpp" --include="*.hpp" > vector_usage_audit.txt
   
   # Find all std::map<K,V> usage
   grep -r "std::map<" opensyde_tool/libs/opensyde_core/ --include="*.cpp" --include="*.hpp" > map_usage_audit.txt
   
   # Find all std::set<T> usage
   grep -r "std::set<" opensyde_tool/libs/opensyde_core/ --include="*.cpp" --include="*.hpp" > set_usage_audit.txt
   ```
2. Categorize containers by type:
   - Binary data (std::vector<uint8_t>) - keep as is
   - Internal object collections (std::vector<CustomType>) - migrate to QList
   - Configuration pairs (std::vector<std::pair<QString, T>>) - migrate to QHash
   - String-keyed maps (std::map<QString, T>) - migrate to QHash
   - Unique element sets (std::set<T>) - migrate to QSet

### Phase 2: Migration Implementation (3-4 weeks)

#### Task 1: Migrate Internal Object Collections
- Target: `std::vector<CustomObject>` → `QList<CustomObject>`
- Focus: Files with no external API dependencies
- Conversion:
  ```cpp
  // BEFORE
  std::vector<C_OscNode> mc_Nodes;
  
  // AFTER
  QList<C_OscNode> mc_Nodes;
  
  // API changes:
  mc_Nodes.push_back(node);  →  mc_Nodes.append(node);
  mc_Nodes.size();           →  mc_Nodes.count();
  mc_Nodes[0];               →  mc_Nodes[0];  // Same syntax
  ```

#### Task 2: Migrate Configuration Pairs
- Target: `std::vector<std::pair<QString, T>>` → `QHash<QString, T>`
- Conversion:
  ```cpp
  // BEFORE
  std::vector<std::pair<QString, uint32_t>> mc_Configs;
  
  // AFTER
  QHash<QString, uint32_t> mc_Configs;
  
  // API changes:
  mc_Configs.push_back(std::make_pair(key, value));  →  mc_Configs.insert(key, value);
  for (const auto& pair : mc_Configs) { ... }        →  for (const auto& key : mc_Configs.keys()) { auto value = mc_Configs[key]; }
  ```

#### Task 3: Migrate String-Indexed Maps
- Target: `std::map<QString, T>` → `QHash<QString, T>`
- Conversion:
  ```cpp
  // BEFORE
  std::map<QString, C_OscSignal> mc_Signals;
  
  // AFTER
  QHash<QString, C_OscSignal> mc_Signals;
  
  // API changes:
  auto it = mc_Signals.find(key);  →  if (mc_Signals.contains(key)) { auto value = mc_Signals[key]; }
  mc_Signals.insert(std::make_pair(key, value));  →  mc_Signals.insert(key, value);
  ```

#### Task 4: Migrate Unique Element Sets
- Target: `std::set<T>` → `QSet<T>`
- Conversion:
  ```cpp
  // BEFORE
  std::set<QString> mc_UniqueNames;
  
  // AFTER
  QSet<QString> mc_UniqueNames;
  
  // API changes:
  mc_UniqueNames.insert(name);  →  mc_UniqueNames.insert(name);
  if (mc_UniqueNames.find(name) != mc_UniqueNames.end()) { }  →  if (mc_UniqueNames.contains(name)) { }
  ```

### Phase 3: Testing and Validation (2 weeks)

#### Unit Tests
```cpp
// Test QList compatibility
TEST(ContainerMigrationTest, QListBasicOperations) {
   QList<int> list = {1, 2, 3};
   list.append(4);
   EXPECT_EQ(4, list.size());
   EXPECT_EQ(1, list[0]);
}

// Test QHash compatibility
TEST(ContainerMigrationTest, QHashBasicOperations) {
   QHash<QString, int> hash;
   hash.insert("key1", 1);
   hash.insert("key2", 2);
   EXPECT_TRUE(hash.contains("key1"));
   EXPECT_EQ(1, hash["key1"]);
}

// Test QSet compatibility
TEST(ContainerMigrationTest, QSetBasicOperations) {
   QSet<QString> set;
   set.insert("item1");
   set.insert("item2");
   EXPECT_TRUE(set.contains("item1"));
}
```

#### Integration Tests
- Verify all functionality works with migrated containers
- Test serialization/deserialization with XML/JSON
- Test API compatibility with external systems

#### Performance Tests
- Benchmark before/after migration
- Focus on: insertion, lookup, iteration performance
- Target: No performance regression

### Phase 4: Cleanup and Documentation (1 week)

1. Remove migrated STL containers from codebase
2. Update CMakeLists.txt to remove any STL-specific dependencies
3. Update documentation:
   - Add to `plans/02_FUTURE/Qt_Native_Coding_Standards.md`
   - Add to `AGENTS.md`
4. Remove outdated migration files from FUTURE

## Success Criteria

| Metric | Target | Current | Status |
|--------|--------|---------|--------|
| std::vector<T> usage | 0 | 6,138 | 🔴 TODO |
| std::map<K,V> usage | 0 | ~640 | 🔴 TODO |
| std::set<T> usage | 0 | ~255 | 🔴 TODO |
| QList<T> usage | >80% | ~5% | 🔴 TODO |
| QHash<K,V> usage | >80% | ~10% | 🔴 TODO |
| QSet<T> usage | >80% | ~10% | 🔴 TODO |

## Estimated Effort

- **Audit and Analysis**: 1 week
- **Migration Implementation**: 3-4 weeks
- **Testing and Validation**: 2 weeks
- **Cleanup and Documentation**: 1 week
- **Total**: 7-8 weeks

## Dependencies

- Must be done after QString migration completion (already complete)
- Requires familiarity with Qt container APIs
- Requires understanding of codebase architecture

## Risk Mitigation

**Risk**: API breaking changes
- **Mitigation**: Use adapter layers during migration
- **Testing**: Extensive unit and integration tests

**Risk**: Performance regressions
- **Mitigation**: Benchmark before/after migration
- **Testing**: Performance tests for critical paths

**Risk**: Integration failures
- **Mitigation**: Test all external interfaces
- **Testing**: Integration tests with real data

## Next Steps

1. Generate detailed usage statistics (Phase 1, Task 1)
2. Create migration checklist per component
3. Assign migration tasks to developers
4. Begin Phase 1 implementation

---

**Document Status**: Draft
**Owner**: Development Team
**Created**: 2026-02-22

**PRIMARY ENTRY POINT**: This document

**RELEVANT DOCUMENTS**:
- `plans/02_FUTURE/Qt_Native_Coding_Standards.md`
- `plans/01_COMPLETED/QString_Migration_Phase3_Complete_Summary.md`
- `AGENTS.md`