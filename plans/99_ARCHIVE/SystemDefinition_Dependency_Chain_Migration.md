# C_OscSystemDefinition Dependency Chain Migration Plan

**Date**: 2026-03-01  
**Status**: 🚧 In Progress  
**Goal**: Migrate the complete dependency chain to enable C_OscSystemDefinition serialization

---

## Problem Statement

C_OscSystemDefinition has serialization methods implemented but fails to compile because its dependent classes (C_OscSystemBus, C_OscNode, C_OscNodeSquad) lack the required serialization methods.

**Error**: Missing `ToQDataStream()`, `FromQDataStream()`, `ToJsonObject()`, `FromJsonObject()`, `ToQDomDocument()`, `FromQDomElement()` methods in dependencies.

---

## Dependency Analysis

### Direct Dependencies

| Class | Complexity | Status | Notes |
|-------|------------|--------|-------|
| **C_OscSystemBus** | Low | ❌ Not migrated | Simple structure, 10 fields |
| **C_OscNode** | Very High | ❌ Not migrated | Complex, 290+ lines, many nested types |
| **C_OscNodeSquad** | Low | ❌ Not migrated | Simple structure, 3 fields |

### C_OscNode Nested Dependencies

C_OscNode contains these types that need serialization:

| Class | Complexity | Status | Already Migrated? |
|-------|------------|--------|-------------------|
| C_OscNodeProperties | Medium | ❌ No | - |
| C_OscNodeDataPool | High | ❌ No | - |
| C_OscNodeApplication | Medium | ❌ No | - |
| C_OscCanProtocol | High | ❌ No | - |
| C_OscHalcConfig | High | ❌ No | - |
| C_OscCanOpenManagerInfo | Medium | ❌ No | - |
| **C_OscDataLoggerJob** | Medium-High | ✅ **Yes** | Migrated 2026-03-01 |
| **C_OscXappProperties** | Simple | ✅ **Yes** | Migrated 2026-03-01 |

---

## Migration Strategy

### Phase 1: Simple Classes (Independent)

**Priority**: High - These are simple, self-contained classes

1. **C_OscSystemBus** (~70 lines)
   - Simple struct-like class
   - 10 fields: enums, QString, primitives
   - No nested complex types
   - **Estimated**: 1-2 hours

2. **C_OscNodeSquad** (~65 lines)
   - Simple struct-like class
   - 3 fields: QString, QList<uint32_t>
   - No complex nested types
   - **Estimated**: 1 hour

### Phase 2: Medium Complexity Classes

**Priority**: Medium - These have some nested structures

3. **C_OscNodeProperties** (~150 lines estimated)
   - Contains various node properties
   - May have nested structures
   - **Estimated**: 2-3 hours

4. **C_OscCanOpenManagerInfo** (~100 lines estimated)
   - CANopen manager information
   - Moderate complexity
   - **Estimated**: 2 hours

5. **C_OscNodeApplication** (~200 lines estimated)
   - Application data blocks
   - Contains hex application data
   - **Estimated**: 2-3 hours

### Phase 3: High Complexity Classes

**Priority**: High - These are critical dependencies

6. **C_OscNodeDataPool** (~400+ lines estimated)
   - Very complex data pool structure
   - Contains lists, elements, signals
   - Multiple nested types
   - **Estimated**: 4-6 hours

7. **C_OscCanProtocol** (~500+ lines estimated)
   - Communication protocol definitions
   - Messages, signals, complex structures
   - **Estimated**: 4-6 hours

8. **C_OscHalcConfig** (~300+ lines estimated)
   - HALC configuration
   - Domains, channels, parameters
   - **Estimated**: 3-4 hours

### Phase 4: C_OscSystemDefinition

**Priority**: Final - Once all dependencies are migrated

9. **C_OscSystemDefinition** (~2000 lines)
   - Already has serialization methods
   - Just needs dependencies to compile
   - **Estimated**: 0 hours (just verification)

---

## Recommended Order

```
1. C_OscSystemBus          (Simple, independent)
2. C_OscNodeSquad          (Simple, independent)
3. C_OscNodeProperties     (Medium, used by C_OscNode)
4. C_OscCanOpenManagerInfo (Medium, used by C_OscNode)
5. C_OscNodeApplication    (Medium, used by C_OscNode)
6. C_OscNodeDataPool       (High, core of C_OscNode)
7. C_OscCanProtocol        (High, core of C_OscNode)
8. C_OscHalcConfig         (High, used by C_OscNode)
9. C_OscNode               (Very High, depends on all above)
10. C_OscSystemDefinition  (Final, depends on C_OscSystemBus, C_OscNode, C_OscNodeSquad)
```

---

## Implementation Pattern

For each class, implement:

### Header File
```cpp
// Binary Serialization
int32_t ToQDataStream(QDataStream& orc_Stream) const;
int32_t FromQDataStream(QDataStream& orc_Stream);

// JSON Serialization
QJsonObject ToJsonObject() const;
int32_t FromJsonObject(const QJsonObject& orc_Object);

// XML Serialization
QDomElement ToQDomDocument(QDomDocument& orc_Doc, 
                           const QString& orc_RootElementName) const;
int32_t FromQDomElement(const QDomElement& orc_Element);
```

### Implementation Guidelines

**Binary (QDataStream)**:
- Serialize primitives directly
- Serialize enums as uint32_t
- Use count prefixes for QList
- Handle QString with `<<` and `>>`

**JSON (QJsonObject)**:
- Use human-readable strings for enums
- Base64 encode QByteArray if needed
- Nested objects use ToJsonObject()/FromJsonObject()
- Arrays use QJsonArray

**XML (QDomDocument)**:
- Use QDomElement for structure
- Text nodes for values
- Attributes for simple properties
- Nested elements for complex types

---

## Files to Modify

### Phase 1
- `opensyde_tool/libs/opensyde_core/project/system/C_OscSystemBus.hpp`
- `opensyde_tool/libs/opensyde_core/project/system/C_OscSystemBus.cpp`
- `opensyde_tool/libs/opensyde_core/project/system/node/C_OscNodeSquad.hpp`
- `opensyde_tool/libs/opensyde_core/project/system/node/C_OscNodeSquad.cpp`

### Phase 2
- `opensyde_tool/libs/opensyde_core/project/system/node/C_OscNodeProperties.hpp/cpp`
- `opensyde_tool/libs/opensyde_core/project/system/node/C_OscCanOpenManagerInfo.hpp/cpp`
- `opensyde_tool/libs/opensyde_core/project/system/node/C_OscNodeApplication.hpp/cpp`

### Phase 3
- `opensyde_tool/libs/opensyde_core/project/system/node/C_OscNodeDataPool.hpp/cpp`
- `opensyde_tool/libs/opensyde_core/project/system/node/can/C_OscCanProtocol.hpp/cpp`
- `opensyde_tool/libs/opensyde_core/halc/configuration/C_OscHalcConfig.hpp/cpp`

### Phase 4
- `opensyde_tool/libs/opensyde_core/project/system/C_OscSystemDefinition.hpp/cpp` (already has methods, just verify)

---

## Estimated Total Time

- **Phase 1**: 2-3 hours
- **Phase 2**: 6-8 hours
- **Phase 3**: 11-16 hours
- **Phase 4**: 0.5 hours (verification)

**Total**: 19.5-27.5 hours

---

## Success Criteria

1. ✅ All dependency classes have serialization methods
2. ✅ C_OscSystemDefinition compiles without errors
3. ✅ All builds pass successfully
4. ✅ Backward compatibility maintained
5. ✅ Documentation updated

---

## Risk Mitigation

1. **Build After Each**: Build after completing each class to catch issues early
2. **Test Dependencies**: Ensure each class's dependencies are migrated first
3. **Documentation**: Document each migration with results file
4. **Git Commits**: Commit each class separately for easy rollback if needed

---

## Next Steps

1. Start with **C_OscSystemBus** (simplest, least dependencies)
2. Read the full class definition before implementing
3. Follow established patterns from previous migrations
4. Build and verify after each class
5. Document results

---

**Status**: Ready to begin Phase 1  
**First Target**: C_OscSystemBus  
**Estimated Start**: 2026-03-01
