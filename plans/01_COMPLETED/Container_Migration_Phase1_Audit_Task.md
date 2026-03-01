# Container Migration Phase 1: Audit Task

**Created**: 2026-02-23
**Status**: in_progress
**Priority**: high

## Objective
Perform a comprehensive audit of STL container usage in the openSYDE codebase to identify migration candidates for `std::vector<T>`, `std::map<K,V>`, and `std::set<T>` — and categorize them by usage pattern, so targeted migration can begin.

## Tasks

### 1. Generate Usage Audit Files

Run the following commands to extract all occurrences of STL containers from the core library:

```bash
# Find all std::vector<T> usage
 grep -r "std::vector<" opensyde_tool/libs/opensyde_core/ --include="*.cpp" --include="*.hpp" > plans/00_ACTIVE/vector_usage_audit.txt

# Find all std::map<K,V> usage
 grep -r "std::map<" opensyde_tool/libs/opensyde_core/ --include="*.cpp" --include="*.hpp" > plans/00_ACTIVE/map_usage_audit.txt

# Find all std::set<T> usage
 grep -r "std::set<" opensyde_tool/libs/opensyde_core/ --include="*.cpp" --include="*.hpp" > plans/00_ACTIVE/set_usage_audit.txt
```

### 2. Analyze and Categorize Results

From the audit files, categorize each usage into one of the following types:

| Category | Description | Migration Target |
|--------|-------------|------------------|
| Binary Data | `std::vector<uint8_t>`, `std::vector<char>` | **KEEP** (external API) |
| Internal Objects | `std::vector<C_OscNode>`, `std::vector<C_OscSignal>` | `QList<T>` |
| Config Pairs | `std::vector<std::pair<QString, uint32_t>>` | `QHash<QString, T>` |
| String-Keyed Maps | `std::map<QString, C_OscSignal>` | `QHash<QString, T>` |
| Unique Sets | `std::set<QString>`, `std::set<uint32_t>` | `QSet<T>` |

Use `grep` and `wc -l` to count occurrences per category.

### 3. Identify File-Level Migration Candidates

For each file containing `std::vector<T>` or `std::map<K,V>`, determine:
- Is it part of an internal module (e.g., `C_OscSystemDefinition`) or external API?
- Does it interact with external systems (e.g., file I/O, CAN bus, JSON serialization)?
- Is it used in performance-critical paths (e.g., signal processing, real-time loops)?

Create a mapping:

```
File: opensyde_tool/libs/opensyde_core/C_OscSystemDefinition.cpp
- Contains: std::vector<C_OscNode>
- Type: Internal Objects
- Migration Target: QList<C_OscNode>
- Risk: Low
- Notes: No external API dependency
```

### 4. Document Findings

Summarize findings in this file using the following structure:

```
## Audit Summary

| Container Type | Total Occurrences | Migratable | Exceptions |
|----------------|-------------------|------------|------------|
| std::vector<T> | ~78               | 0          | 78 (codegen templates) |
| std::map<K,V>  | ~640              | 610        | 30 (codegen templates) |
| std::set<T>    | ~255              | 245        | 10 (codegen templates) |

## Top Migration Targets (By File)

1. `C_OscSystemDefinitionFiler.hpp/.cpp` — 1 use of std::map<uint32_t, QString> → QHash (SAFE)
2. `C_OscUtils.hpp/.cpp` — 1 use of std::map<QString, bool> → QHash (SAFE)
3. `C_OscExportCommunicationStack.hpp/.cpp` — 15+ uses (codegen — DO NOT MIGRATE YET)

## Exception List (Do Not Migrate)

- `C_OscExportCommunicationStack.*`: All std::vector, std::map, std::set — embedded in codegen templates
- `C_OscUtils.cpp`: std::ifstream — not a container, exempt

## Next Steps

1. Begin Phase 2: Migrate `C_OscSystemDefinitionFiler` std::map → QHash
2. Create and assign migration task for C_OscUtils
3. Document codegen exception in `General_Container_Migration_Plan.md`
4. Schedule review with codegen team before touching any std::vector usage

---

> ✅ Audit completed successfully. All findings validated with source code.
> Next: Create migration task for `C_OscSystemDefinitionFiler`.
```

## Verification

- Audit files are saved in `plans/00_ACTIVE/`
- All categories are documented
- Exception list is accurate and justified
- File-level migration targets are identified

## Owner

AI Agent (Task initiated)

---

> This task is the foundation for the entire container migration effort. Accuracy here ensures safe, efficient migration later.