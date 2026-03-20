# Qt Serialization Framework Pattern Documentation

## Overview
This document describes the consistent serialization pattern used in completed C_Osc* classes for Qt Serialization Framework migration. All classes follow the same 6-method pattern for full serialization support.

## Standard Serialization Methods

All completed classes implement the following 6 methods for comprehensive serialization:

### 1. Binary Serialization (QDataStream)
```cpp
// Serialize to QDataStream (binary format)
int32_t ToQDataStream(QDataStream& orc_Stream) const;

// Deserialize from QDataStream (binary format)
int32_t FromQDataStream(QDataStream& orc_Stream);
```

### 2. JSON Serialization (QJsonObject)
```cpp
// Serialize to QJsonObject
QJsonObject ToJsonObject() const;

// Deserialize from QJsonObject
int32_t FromJsonObject(const QJsonObject& orc_Object);
```

### 3. XML Serialization (QDomDocument)
```cpp
// Serialize to QDomDocument
QDomElement ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName = "data") const;

// Deserialize from QDomElement
int32_t FromQDomElement(const QDomElement& orc_Element);
```

## Implementation Examples

### From C_OscDataLoggerJob.hpp (Completed Class)
```cpp
// Binary Serialization (QDataStream)
int32_t ToQDataStream(QDataStream& orc_Stream) const;
int32_t FromQDataStream(QDataStream& orc_Stream);

// JSON Serialization (QJsonObject)
QJsonObject ToJsonObject() const;
int32_t FromJsonObject(const QJsonObject& orc_Object);

// XML Serialization (QDomDocument)
QDomElement ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName = "job") const;
int32_t FromQDomElement(const QDomElement& orc_Element);
```

### From C_OscViewData.hpp (Completed Class)
```cpp
// Binary Serialization (QDataStream)
int32_t ToQDataStream(QDataStream& orc_Stream) const;
int32_t FromQDataStream(QDataStream& orc_Stream);

// JSON Serialization (QJsonObject)
QJsonObject ToJsonObject() const;
int32_t FromJsonObject(const QJsonObject& orc_Object);

// XML Serialization (QDomDocument)
QDomElement ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName = "view-data") const;
int32_t FromQDomElement(const QDomElement& orc_Element);
```

## Key Implementation Considerations

1. **Error Handling**: All methods return `int32_t` with appropriate error codes (`C_NO_ERR`, `C_UNKNOWN_ERR`, etc.)

2. **Method Signatures**: 
   - `To*` methods are `const`
   - `From*` methods take const references where appropriate
   - Default parameter values provided for root element names

3. **Parameter Consistency**:
   - QDataStream methods take non-const references (for modification during serialization)
   - QJsonObject methods take const references
   - QDomDocument methods take non-const references
   - QDomElement methods take const references

4. **Documentation**: Each method includes Doxygen-style documentation with:
   - Brief description
   - Parameter documentation
   - Return value documentation
   - Error code documentation

## Dependencies and Usage

The serialization methods are designed to be:
- Self-contained (minimal dependencies within the class)
- Efficient (avoid unnecessary copies)
- Safe (handle edge cases gracefully)
- Consistent (follow the same patterns across all classes)

## Testing Requirements

Each implementation should be tested for:
1. Successful serialization and deserialization
2. Round-trip integrity (serialize → deserialize → serialize again)
3. Error handling for malformed data
4. Memory safety and resource management
5. Performance characteristics

## Migration Checklist

When implementing serialization methods for new classes:
1. [ ] Add all 6 method declarations to the header file
2. [ ] Implement binary serialization (QDataStream)
3. [ ] Implement JSON serialization (QJsonObject)
4. [ ] Implement XML serialization (QDomDocument)
5. [ ] Add proper Doxygen documentation
6. [ ] Test all methods thoroughly
7. [ ] Verify error handling works correctly
8. [ ] Confirm consistency with existing patterns