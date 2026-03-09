# HALC & CANopen Filer Migration - Completion Guide

## Overview

This document provides the roadmap for completing the remaining 4 Filer migrations in the Qt Native Serialization Framework.

## Current Status

✅ **Completed**: 22/26 Filers (85%)  
❌ **Remaining**: 4/26 Filers (15%)

## Remaining Filers

### 1. C_OscHalcConfigFiler (High Priority)

**Data Classes Requiring Qt Serialization:**
- `C_OscHalcConfig` (133 lines) - Main config class
- `C_OscHalcConfigDomain` (89 lines) - Domain configuration  
- `C_OscHalcConfigChannel` (222 lines) - Channel configuration
- `C_OscHalcConfigParameterStruct` (~150 lines) - Parameter structures
- `C_OscHalcConfigParameter` (~180 lines) - Individual parameters

**Estimated Effort**: 1-2 days
**Complexity**: Very High

**Implementation Steps:**
1. Add `ToQDataStream/FromQDataStream` to all 5 classes
2. Add `ToJsonObject/FromJsonObject` to all 5 classes  
3. Add `ToQDomDocument/FromQDomDocument` to all 5 classes
4. Create `C_OscHalcConfigFiler_New` with multi-format support
5. Test with sample HALC configuration files

**Template Example (C_OscHalcConfig):**
```cpp
// In C_OscHalcConfig.hpp
void ToQDataStream(QDataStream &ro_DataStream) const;
void FromQDataStream(QDataStream &ro_DataStream);
QJsonObject ToJsonObject() const;
void FromJsonObject(const QJsonObject &orc_Object);
QDomElement ToQDomDocument(QDomDocument &orc_Doc, const QString &orc_ElementName) const;
void FromQDomDocument(const QDomElement &orc_Element);

// In C_OscHalcConfig.cpp
void C_OscHalcConfig::ToQDataStream(QDataStream &ro_DataStream) const {
    ro_DataStream << static_cast<int32_t>(e_SafetyMode);
    ro_DataStream << c_Name;
    // Serialize domains
    ro_DataStream << static_cast<qint32>(mc_Domains.size());
    for (const auto &c_Domain : mc_Domains) {
        c_Domain.ToQDataStream(ro_DataStream);
    }
}

void C_OscHalcConfig::FromQDataStream(QDataStream &ro_DataStream) {
    int32_t s_SafetyMode;
    ro_DataStream >> s_SafetyMode;
    e_SafetyMode = static_cast<E_SafetyMode>(s_SafetyMode);
    ro_DataStream >> c_Name;
    
    qint32 s_DomainsSize;
    ro_DataStream >> s_DomainsSize;
    mc_Domains.clear();
    for (qint32 s_I = 0; s_I < s_DomainsSize; ++s_I) {
        C_OscHalcConfigDomain c_Domain;
        c_Domain.FromQDataStream(ro_DataStream);
        mc_Domains.append(c_Domain);
    }
}
```

### 2. C_OscHalcDefFiler (Very High Complexity)

**Data Classes Requiring Qt Serialization:**
- `C_OscHalcDef` (~300 lines) - Main definition
- `C_OscHalcDefDomain` (~250 lines)
- `C_OscHalcDefChannelUseCase` (~200 lines)
- `C_OscHalcDefStruct` (~400 lines)
- `C_OscHalcDefElement` (~350 lines)
- `C_OscHalcDefContent` (~200 lines)
- Plus 5+ more supporting classes

**Estimated Effort**: 2-3 days  
**Complexity**: Extremely High

**Key Challenges:**
- Deep inheritance hierarchy
- Complex struct/element relationships
- Multiple content types (plain, enum, bitmask)
- Extensive validation logic

### 3. C_OscHalcDefStructFiler (High Complexity)

**Data Classes Requiring Qt Serialization:**
- `C_OscHalcDefStruct` (~450 lines)
- `C_OscHalcDefStructElement` (~200 lines)
- Supporting type definitions

**Estimated Effort**: 1-2 days  
**Complexity**: High

### 4. C_OscCanOpenManagerFiler (Medium-High Complexity)

**Data Classes Requiring Qt Serialization:**
- `C_OscCanOpenManager` (~600 lines)
- `C_OscCanOpenObjectDictionary` (~400 lines)
- `C_OscCanOpenNode` (~350 lines)
- `C_OscCanOpenSignal` (~200 lines)

**Estimated Effort**: 1-2 days  
**Complexity**: Medium-High

## Migration Pattern

All HALC/CANopen migrations follow the established pattern:

### Step 1: Add Qt Serialization to Data Classes
```cpp
class C_Example {
public:
    // Binary
    void ToQDataStream(QDataStream&) const;
    void FromQDataStream(QDataStream&);
    
    // JSON
    QJsonObject ToJsonObject() const;
    void FromJsonObject(const QJsonObject&);
    
    // XML
    QDomElement ToQDomDocument(QDomDocument&, const QString&) const;
    void FromQDomDocument(const QDomElement&);
};
```

### Step 2: Create Filer Class
```cpp
class C_ExampleFiler_New {
public:
    static int32_t h_LoadFile(C_Example&, const QString&);
    static int32_t h_SaveFile(const C_Example&, const QString&);
    
    static int32_t h_LoadBinary(...);
    static int32_t h_SaveBinary(...);
    static int32_t h_LoadJson(...);
    static32_t h_SaveJson(...);
    static int32_t h_LoadXml(...);
    static int32_t h_SaveXml(...);
    
private:
    static int32_t mh_DetectAndLoad(C_Example&, const QString&);
};
```

### Step 3: Implementation
Follow the exact pattern used in completed migrations (see `C_OscNodeDataPoolFiler_New.cpp` for reference)

## Estimated Total Effort

| Filer | Complexity | Estimated Time |
|-------|------------|----------------|
| C_OscHalcConfigFiler | Very High | 1-2 days |
| C_OscHalcDefFiler | Extremely High | 2-3 days |
| C_OscHalcDefStructFiler | High | 1-2 days |
| C_OscCanOpenManagerFiler | Medium-High | 1-2 days |
| **Total** | | **5-9 days** |

## Recommendations

### Option A: Complete All (5-9 days)
- Pros: 100% migration complete, consistent codebase
- Cons: Significant time investment, diminishing returns

### Option B: Prioritize by Usage (2-3 days)
- Identify which HALC/CANopen features are actually used in production
- Complete only the most critical Filer(s)
- Defer others to future sprints

### Option C: Framework Ready, Implement as Needed (1 day)
- Create skeleton implementations for all 4 Filers
- Document the pattern thoroughly
- Implement full serialization only when specific features are needed

## Files Already Created

- ✅ `C_OscHalcConfigDomain_Serialization.cpp` - Partial implementation
- ✅ `C_OscHalcConfigFiler_New.hpp` - Header only

## Next Steps

1. **Review existing HALC code** to understand complete class hierarchy
2. **Choose migration approach** (Option A, B, or C)
3. **Add Qt serialization** to all required data classes
4. **Create Filer implementations** following established pattern
5. **Test** with sample configuration files
6. **Update documentation** and commit changes

## Reference Materials

- Completed migrations: `C_OscNodeDataPoolFiler_New`, `C_OscParamSetInterpretedNodeFiler_New`
- Qt Serialization Framework: `plans/00_ACTIVE/Qt_Serialization_Framework_Migration.md`
- Coding Standards: `plans/02_FUTURE/Qt_Native_Coding_Standards.md`

---

**Status**: Framework Ready - Implementation Required  
**Estimated Completion**: 5-9 days for full migration  
**Production Impact**: None - Existing code fully functional
