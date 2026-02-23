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
| std::vector<T> | 6138              | 5872       | 266 (binary) |
| std::map<K,V>  | 640               | 590        | 50 (sorted) |
| std::set<T>    | 255               | 245        | 10 (sorted) |

## Top Migration Targets (By File)

1. `C_OscSystemDefinition.cpp` — 87 uses of std::vector<C_OscNode> → QList
2. `C_OscCanInterface.cpp` — 42 uses of std::map<QString, C_OscSignal> → QHash
3. `C_OscDiagnosticSession.cpp` — 31 uses of std::set<uint32_t> → QSet

## Exception List (Do Not Migrate)

- `C_OscBinaryFileReader.cpp`: std::vector<uint8_t> — external file I/O
- `C_OscProtocolParser.cpp`: std::map<uint32_t, uint32_t> — sorted by key for binary search

## Next Steps

1. Create migration task checklist per module
2. Begin Phase 2: Migrate `C_OscSystemDefinition.cpp` first
3. Update `plans/02_FUTURE/General_Container_Migration_Plan.md` with audit results
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