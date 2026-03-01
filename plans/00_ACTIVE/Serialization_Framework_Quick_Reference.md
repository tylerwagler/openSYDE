# Qt Serialization Framework - Quick Reference

## Overview
Multi-format serialization framework supporting Binary, JSON, and XML with automatic format detection.

---

## Quick Start

### 1. Add Serialization Methods to Data Class

```cpp
// In header file
class C_YourClass {
public:
   // Binary
   int32_t ToQDataStream(QDataStream&) const;
   int32_t FromQDataStream(QDataStream&);
   
   // JSON
   QJsonObject ToJsonObject() const;
   int32_t FromJsonObject(const QJsonObject&);
   
   // XML
   QDomElement ToQDomDocument(QDomDocument&, const QString&) const;
   int32_t FromQDomElement(const QDomElement&);
};
```

### 2. Implement the Methods

```cpp
// Binary - Stream all fields
int32_t C_YourClass::ToQDataStream(QDataStream& orc_Stream) const {
   using namespace stw::errors;
   orc_Stream << field1 << field2 << field3;
   return (orc_Stream.status() == QDataStream::Ok) ? C_NO_ERR : C_RD_WR;
}

int32_t C_YourClass::FromQDataStream(QDataStream& orc_Stream) {
   using namespace stw::errors;
   orc_Stream >> field1 >> field2 >> field3;
   return (orc_Stream.status() == QDataStream::Ok) ? C_NO_ERR : C_RD_WR;
}

// JSON - Create object with field names
QJsonObject C_YourClass::ToJsonObject() const {
   QJsonObject obj;
   obj["field-name-1"] = field1;
   obj["field-name-2"] = field2;
   return obj;
}

int32_t C_YourClass::FromJsonObject(const QJsonObject& orc_Object) {
   using namespace stw::errors;
   if (!orc_Object.contains("field-name-1") || !orc_Object.contains("field-name-2"))
      return C_CONFIG;
   field1 = orc_Object["field-name-1"].toInt();
   field2 = orc_Object["field-name-2"].toInt();
   return C_NO_ERR;
}

// XML - Create elements
QDomElement C_YourClass::ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_Name) const {
   QDomElement elem = orc_Doc.createElement(orc_Name);
   QDomElement f1 = orc_Doc.createElement("field-name-1");
   f1.appendChild(orc_Doc.createTextNode(QString::number(field1)));
   elem.appendChild(f1);
   return elem;
}

int32_t C_YourClass::FromQDomElement(const QDomElement& orc_Element) {
   using namespace stw::errors;
   QDomNode n1 = orc_Element.namedItem("field-name-1");
   if (n1.isNull()) return C_CONFIG;
   field1 = n1.toElement().text().toUInt();
   return C_NO_ERR;
}
```

### 3. Create Filer Class

```cpp
// Header
class C_YourClassFiler_New {
public:
   static int32_t h_LoadFile(C_YourClass& orc_Config, const QString& orc_Path);
   static int32_t h_SaveFile(const C_YourClass& orc_Config, const QString& orc_Path);
   // Optional: format-specific methods
   static int32_t h_LoadBinary(C_YourClass&, const QByteArray&);
   static QByteArray h_SaveBinary(const C_YourClass&);
   // ... and so on
};

// Implementation (streamlined)
int32_t C_YourClassFiler_New::h_LoadFile(C_YourClass& orc_Config, const QString& orc_Path) {
   if (!QFileInfo(orc_Path).exists()) return C_RANGE;
   auto format = stw::opensyde_core::C_OscSerializer::DetectFormat(orc_Path);
   switch (format) {
      case E_SerializationFormat::e_BINARY: {
         QByteArray data;
         if (C_OscSerializer::LoadBinary(data, orc_Path) != C_NO_ERR) return C_RD_WR;
         QDataStream stream(data); stream.setVersion(QDataStream::Qt_6_0);
         return orc_Config.FromQDataStream(stream);
      }
      case E_SerializationFormat::e_JSON: {
         QJsonObject obj;
         if (C_OscSerializer::LoadJson(obj, orc_Path) != C_NO_ERR) return C_RD_WR;
         return orc_Config.FromJsonObject(obj);
      }
      case E_SerializationFormat::e_XML: {
         QDomDocument doc;
         if (C_OscSerializer::LoadXml(doc, orc_Path) != C_NO_ERR) return C_RD_WR;
         return orc_Config.FromQDomElement(doc.documentElement());
      }
      default: return C_CONFIG;
   }
}

int32_t C_YourClassFiler_New::h_SaveFile(const C_YourClass& orc_Config, const QString& orc_Path) {
   auto format = stw::opensyde_core::C_OscSerializer::DetectFormat(orc_Path);
   switch (format) {
      case E_SerializationFormat::e_BINARY:
         return C_OscSerializer::SaveBinary([&]() {
            QByteArray data; QDataStream stream(&data, QIODevice::WriteOnly);
            stream.setVersion(QDataStream::Qt_6_0); orc_Config.ToQDataStream(stream);
            return data;
         }(), orc_Path);
      case E_SerializationFormat::e_JSON:
         return C_OscSerializer::SaveJson(orc_Config.ToJsonObject(), orc_Path);
      case E_SerializationFormat::e_XML: {
         QDomDocument doc;
         QDomElement elem = orc_Config.ToQDomDocument(doc, "root-element");
         QDomElement root = doc.createElement("root");
         root.appendChild(elem); doc.appendChild(root);
         QFile file(orc_Path);
         if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return C_CONFIG;
         QTextStream stream(&file); stream.setEncoding(QStringConverter::Utf8);
         stream << doc.toString(2); file.close();
         return C_NO_ERR;
      }
      default: return C_CONFIG;
   }
}
```

---

## Special Cases

### Enums
```cpp
// Serialize
orc_Stream << static_cast<int32_t>(myEnum);
c_Object["enum-field"] = static_cast<int>(myEnum);

// Deserialize
int32_t temp; orc_Stream >> temp;
myEnum = static_cast<MyEnum>(temp);
myEnum = static_cast<MyEnum>(obj["enum-field"].toInt());
```

### Collections (QList)
```cpp
// Binary
orc_Stream << qint32(list.size());
for (const auto& item : list) item.ToQDataStream(orc_Stream);

// Deserialize
qint32 size; orc_Stream >> size;
list.clear();
for (qint32 i = 0; i < size; i++) {
   ItemType item; item.FromQDataStream(orc_Stream);
   list.append(item);
}

// JSON
QJsonArray arr;
for (const auto& item : list) arr.append(item.ToJsonObject());
obj["list-field"] = arr;

// XML
QDomElement parent = orc_Doc.createElement("list-field");
for (const auto& item : list) {
   QDomElement child = item.ToQDomDocument(orc_Doc, "item");
   parent.appendChild(child);
}
```

---

## Common Mistakes to Avoid

1. **Missing includes**: Always include `<QJsonArray>`, `<QJsonValue>` for JSON
2. **Type conversions**: Use explicit casts for uint32_t → QJsonValue
3. **Qt6 API**: Use `setEncoding(QStringConverter::Utf8)` not `setCodec()`
4. **Namespace**: `using namespace stw::errors;` for error codes
5. **Enum handling**: Always cast to int before serialization

---

## Test Checklist

- [ ] Binary format: Save and load, verify data integrity
- [ ] JSON format: Save and load, verify data integrity
- [ ] XML format: Save and load, verify data integrity
- [ ] Format auto-detection: Test with .bin, .json, .xml extensions
- [ ] Error handling: Test with invalid files
- [ ] Backward compatibility: Verify legacy XML still works
- [ ] Nested objects: Test collections and complex structures

---

## File Structure

```
opensyde_tool/libs/opensyde_core/
├── serialization/
│   ├── C_OscSerializer.hpp/cpp      # Framework base
│   ├── C_OscFilerHelper.hpp         # Template helper
│   └── C_OscFilerHelperExample.hpp  # Usage examples
└── exports/x_config_generation/
    ├── C_OscXcoManifest.hpp/cpp     # Data class with serialization
    └── C_OscXcoManifestFiler_New.hpp/cpp  # New Filer
```

---

**Last Updated**: 2026-03-01  
**Related**: See detailed migration results in `plans/02_FUTURE/` directory
