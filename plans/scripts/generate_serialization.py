#!/usr/bin/env python3
"""
Serialization Method Generator for Qt-Native Migration

This script generates ToQDataStream, FromQDataStream, ToJsonObject,
FromJsonObject, ToQDomDocument, and FromQDomElement methods for C++ classes.

Usage:
    python generate_serialization.py <header_file> <cpp_file> <class_name>

Example:
    python generate_serialization.py C_OscNode.hpp C_OscNode.cpp C_OscNode
"""

import sys
import re
import os
from pathlib import Path


def parse_class_fields(header_content, class_name):
    """Extract public member variables from class definition."""
    fields = []

    # Find the class definition
    class_pattern = rf"class\s+{class_name}\s*{{([^}}]+)}}"
    match = re.search(class_pattern, header_content, re.DOTALL)

    if not match:
        return fields

    class_body = match.group(1)

    # Split into sections by public/private/protected
    sections = re.split(r"(public:|private:|protected:)", class_body)

    current_access = "private"
    for i, section in enumerate(sections):
        if section in ["public:", "private:", "protected:"]:
            current_access = section[:-1]
            continue

        if current_access != "public":
            continue

        # Parse each line for member variables
        lines = section.split("\n")
        current_type = None

        for line in lines:
            line = line.strip()
            if not line or line.startswith("//") or line.startswith("/*"):
                continue

            # Skip function declarations
            if "(" in line and ")" in line:
                current_type = None
                continue

            # Check for type declaration
            type_match = re.match(
                r"^(QList<QList<\s*(\w+)\s*>>|QList<\s*(\w+)\s*>|QHash<\s*\w+\s*,\s*\w+\s*>|QSet<\w+>|QString|uint\d+|int\d+|uint8_t|uint16_t|uint32_t|uint64_t|int8_t|int16_t|int32_t|int64_t|float32_t|float64_t|bool|E_\w+)",
                line,
            )

            if type_match:
                # Extract type and variable name
                type_part = line.split(";")[0]
                var_match = re.search(r"\b(\w+)\s*[;,=]", type_part)
                if var_match:
                    var_name = var_match.group(1)
                    if not var_name.startswith("m_") and not var_name.startswith("h_"):
                        fields.append(
                            {
                                "type": type_part.strip(),
                                "name": var_name,
                                "comment": extract_comment(lines, i),
                            }
                        )

    return fields


def extract_comment(lines, index):
    """Extract comment from nearby lines."""
    for i in range(max(0, index - 3), min(len(lines), index + 1)):
        if "///<" in lines[i]:
            return lines[i].split("///<")[-1].strip()
    return ""


def generate_serialization_methods(class_name, fields):
    """Generate all serialization methods for a class."""

    # Binary serialization (QDataStream)
    to_datastream = generate_to_datastream(class_name, fields)
    from_datastream = generate_from_datastream(class_name, fields)

    # JSON serialization (QJsonObject)
    to_json = generate_to_json(class_name, fields)
    from_json = generate_from_json(class_name, fields)

    # XML serialization (QDomDocument)
    to_xml = generate_to_xml(class_name, fields)
    from_xml = generate_from_xml(class_name, fields)

    return {
        "to_datastream": to_datastream,
        "from_datastream": from_datastream,
        "to_json": to_json,
        "from_json": from_json,
        "to_xml": to_xml,
        "from_xml": from_xml,
    }


def generate_to_datastream(class_name, fields):
    """Generate ToQDataStream method."""
    code = f"""//----------------------------------------------------------------------------------------------------------------------
/*!
   \\brief   Serialize to QDataStream (binary format)
   
   \\param[in,out]  orc_Stream    Data stream to write to
   
   \\return Error code
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t {class_name}::ToQDataStream(QDataStream& orc_Stream) const {{
   int32_t s32_Retval = stw::errors::C_NO_ERR;
"""

    for field in fields:
        name = field["name"]
        field_type = field["type"]

        if (
            "QList" in field_type
            or "QHash" in field_type
            or "QStringList" in field_type
        ):
            code += f"""   uint32_t u32_{name}_Count = static_cast<uint32_t>(this->{name}.size());
   orc_Stream << u32_{name}_Count;
   for (const auto& rc_Item : this->{name}) {{
      // Serialize item (implement recursively for complex types)
      orc_Stream << rc_Item;
   }}
"""
        elif "QByteArray" in field_type:
            code += f"""   orc_Stream << this->{name};
"""
        elif "QString" in field_type:
            code += f"""   orc_Stream << this->{name};
"""
        elif "bool" in field_type:
            code += f"""   orc_Stream << this->{name};
"""
        elif "uint" in field_type or "int" in field_type or "float" in field_type:
            code += f"""   orc_Stream << this->{name};
"""
        else:
            # Assume it's a custom class with ToQDataStream method
            code += f"""   s32_Retval = this->{name}.ToQDataStream(orc_Stream);
   if (s32_Retval != stw::errors::C_NO_ERR) return s32_Retval;
"""

    code += """   
   return s32_Retval;
}

"""
    return code


def generate_from_datastream(class_name, fields):
    """Generate FromQDataStream method."""
    code = f"""//----------------------------------------------------------------------------------------------------------------------
/*!
   \\brief   Deserialize from QDataStream (binary format)
   
   \\param[in,out]  orc_Stream    Data stream to read from
   
   \\return Error code
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t {class_name}::FromQDataStream(QDataStream& orc_Stream) {{
   int32_t s32_Retval = stw::errors::C_NO_ERR;
"""

    for field in fields:
        name = field["name"]
        field_type = field["type"]

        if (
            "QList" in field_type
            or "QHash" in field_type
            or "QStringList" in field_type
        ):
            code += f"""   uint32_t u32_{name}_Count = 0;
   orc_Stream >> u32_{name}_Count;
   this->{name}.clear();
   this->{name}.reserve(u32_{name}_Count);
   for (uint32_t u32_I = 0; u32_I < u32_{name}_Count; ++u32_I) {{
      // Deserialize item (implement recursively for complex types)
      auto c_Item = /* default value */;
      orc_Stream >> c_Item;
      this->{name}.append(c_Item);
   }}
"""
        elif "QByteArray" in field_type:
            code += f"""   orc_Stream >> this->{name};
"""
        elif "QString" in field_type:
            code += f"""   orc_Stream >> this->{name};
"""
        elif "bool" in field_type:
            code += f"""   orc_Stream >> this->{name};
"""
        elif "uint" in field_type or "int" in field_type or "float" in field_type:
            code += f"""   orc_Stream >> this->{name};
"""
        else:
            code += f"""   s32_Retval = this->{name}.FromQDataStream(orc_Stream);
   if (s32_Retval != stw::errors::C_NO_ERR) return s32_Retval;
"""

    code += """   
   return s32_Retval;
}

"""
    return code


def generate_to_json(class_name, fields):
    """Generate ToJsonObject method."""
    code = f"""//----------------------------------------------------------------------------------------------------------------------
/*!
   \\brief   Serialize to QJsonObject (JSON format)
   
   \\return JSON object containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject {class_name}::ToJsonObject() const {{
   QJsonObject c_Object;
"""

    for field in fields:
        name = field["name"]
        field_type = field["type"]

        if "QString" in field_type:
            code += f'''   c_Object["{name}"] = this->{name};
'''
        elif "bool" in field_type:
            code += f'''   c_Object["{name}"] = this->{name};
'''
        elif "uint" in field_type or "int" in field_type:
            code += f'''   c_Object["{name}"] = static_cast<qint64>(this->{name});
'''
        elif "float" in field_type:
            code += f'''   c_Object["{name}"] = this->{name};
'''
        elif "QList" in field_type or "QStringList" in field_type:
            code += f'''   QJsonArray c_{name}_Array;
   for (const auto& rc_Item : this->{name}) {{
      c_{name}_Array.append(rc_Item.ToJsonObject());  // Adapt for actual type
   }}
   c_Object["{name}"] = c_{name}_Array;
'''
        elif "QByteArray" in field_type:
            code += f'''   c_Object["{name}"] = QString::fromUtf8(this->{name}.toBase64());
'''
        else:
            code += f'''   c_Object["{name}"] = this->{name}.ToJsonObject();  // Adapt for actual type
'''

    code += """   
   return c_Object;
}

"""
    return code


def generate_from_json(class_name, fields):
    """Generate FromJsonObject method."""
    code = f"""//----------------------------------------------------------------------------------------------------------------------
/*!
   \\brief   Deserialize from QJsonObject (JSON format)
   
   \\param[in]  orc_Object    JSON object containing serialized data
   
   \\return Error code
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t {class_name}::FromJsonObject(const QJsonObject& orc_Object) {{
   int32_t s32_Retval = stw::errors::C_NO_ERR;
"""

    for field in fields:
        name = field["name"]
        field_type = field["type"]

        if "QString" in field_type:
            code += f'''   this->{name} = orc_Object["{name}"].toString();
'''
        elif "bool" in field_type:
            code += f'''   this->{name} = orc_Object["{name}"].toBool();
'''
        elif "uint" in field_type or "int" in field_type:
            code += f'''   this->{name} = static_cast<{field_type}>(orc_Object["{name}"].toVariant().toULongLong());
'''
        elif "float" in field_type:
            code += f'''   this->{name} = static_cast<{field_type}>(orc_Object["{name}"].toDouble());
'''
        elif "QList" in field_type or "QStringList" in field_type:
            code += f'''   QJsonArray c_{name}_Array = orc_Object["{name}"].toArray();
   this->{name}.clear();
   this->{name}.reserve(c_{name}_Array.size());
   for (const QJsonValue& rc_Value : c_{name}_Array) {{
      auto c_Item = /* default value */;
      c_Item.FromJsonObject(rc_Value.toObject());  // Adapt for actual type
      this->{name}.append(c_Item);
   }}
'''
        elif "QByteArray" in field_type:
            code += f'''   this->{name} = QByteArray::fromBase64(orc_Object["{name}"].toString().toUtf8());
'''
        else:
            code += f'''   s32_Retval = this->{name}.FromJsonObject(orc_Object["{name}"].toObject());
   if (s32_Retval != stw::errors::C_NO_ERR) return s32_Retval;
'''

    code += """   
   return s32_Retval;
}

"""
    return code


def generate_to_xml(class_name, fields):
    """Generate ToQDomDocument method."""
    code = f"""//----------------------------------------------------------------------------------------------------------------------
/*!
   \\brief   Serialize to QDomElement (XML format)
   
   \\param[in,out]  orc_Doc              XML document
   \\param[in]      orc_RootElementName  Name for the root element
   
   \\return XML element containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement {class_name}::ToQDomDocument(QDomDocument& orc_Doc, 
                                         const QString& orc_RootElementName) const {{
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
"""

    for field in fields:
        name = field["name"]
        field_type = field["type"]

        if "QString" in field_type:
            code += f'''   c_Element.setAttribute("{name}", this->{name});
'''
        elif "bool" in field_type:
            code += f'''   c_Element.setAttribute("{name}", this->{name});
'''
        elif "uint" in field_type or "int" in field_type:
            code += f'''   c_Element.setAttribute("{name}", QString::number(this->{name}));
'''
        elif "float" in field_type:
            code += f'''   c_Element.setAttribute("{name}", QString::number(this->{name}));
'''
        elif "QList" in field_type or "QStringList" in field_type:
            code += f'''   QDomElement c_{name}_Element = orc_Doc.createElement("{name}");
   for (const auto& rc_Item : this->{name}) {{
      QDomElement c_ItemElement = rc_Item.ToQDomDocument(orc_Doc, "{name[:-1]}");  // Adapt for actual type
      c_{name}_Element.appendChild(c_ItemElement);
   }}
   c_Element.appendChild(c_{name}_Element);
'''
        elif "QByteArray" in field_type:
            code += f'''   c_Element.setAttribute("{name}", QString::fromUtf8(this->{name}.toBase64()));
'''
        else:
            code += f'''   QDomElement c_{name}_Element = this->{name}.ToQDomDocument(orc_Doc, "{name}");
   c_Element.appendChild(c_{name}_Element);
'''

    code += """   
   return c_Element;
}

"""
    return code


def generate_from_xml(class_name, fields):
    """Generate FromQDomElement method."""
    code = f"""//----------------------------------------------------------------------------------------------------------------------
/*!
   \\brief   Deserialize from QDomElement (XML format)
   
   \\param[in]  orc_Element    XML element containing serialized data
   
   \\return Error code
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t {class_name}::FromQDomElement(const QDomElement& orc_Element) {{
   int32_t s32_Retval = stw::errors::C_NO_ERR;
"""

    for field in fields:
        name = field["name"]
        field_type = field["type"]

        if "QString" in field_type:
            code += f'''   this->{name} = orc_Element.attribute("{name}");
'''
        elif "bool" in field_type:
            code += f'''   this->{name} = orc_Element.attribute("{name}").toBool();
'''
        elif "uint" in field_type or "int" in field_type:
            code += f'''   this->{name} = static_cast<{field_type}>(orc_Element.attribute("{name}").toUInt());
'''
        elif "float" in field_type:
            code += f'''   this->{name} = static_cast<{field_type}>(orc_Element.attribute("{name}").toDouble());
'''
        elif "QList" in field_type or "QStringList" in field_type:
            code += f'''   QDomElement c_{name}_Element = orc_Element.firstChildElement("{name}");
   this->{name}.clear();
   QDomNode c_Node = c_{name}_Element.firstChild();
   while (!c_Node.isNull()) {{
      QDomElement c_ItemElement = c_Node.toElement();
      if (!c_ItemElement.isNull()) {{
         auto c_Item = /* default value */;
         c_Item.FromQDomElement(c_ItemElement);  // Adapt for actual type
         this->{name}.append(c_Item);
      }}
      c_Node = c_Node.nextSibling();
   }}
'''
        elif "QByteArray" in field_type:
            code += f'''   this->{name} = QByteArray::fromBase64(orc_Element.attribute("{name}").toUtf8());
'''
        else:
            code += f'''   QDomElement c_{name}_Element = orc_Element.firstChildElement("{name}");
   s32_Retval = this->{name}.FromQDomElement(c_{name}_Element);
   if (s32_Retval != stw::errors::C_NO_ERR) return s32_Retval;
'''

    code += """   
   return s32_Retval;
}

"""
    return code


def main():
    if len(sys.argv) < 4:
        print(
            "Usage: python generate_serialization.py <header_file> <cpp_file> <class_name>"
        )
        sys.exit(1)

    header_file = sys.argv[1]
    cpp_file = sys.argv[2]
    class_name = sys.argv[3]

    # Read header file
    with open(header_file, "r") as f:
        header_content = f.read()

    # Parse fields
    fields = parse_class_fields(header_content, class_name)

    if not fields:
        print(f"Warning: No public fields found in {class_name}")
        print("This might be a complex class requiring manual implementation.")
        sys.exit(0)

    print(f"Found {len(fields)} public fields in {class_name}")

    # Generate methods
    methods = generate_serialization_methods(class_name, fields)

    # Output header additions
    print("\n=== ADD TO HEADER FILE ===")
    print("#include <QDataStream>")
    print("#include <QJsonObject>")
    print("#include <QDomDocument>")
    print("#include <QDomElement>")
    print("\n// Add these method declarations in the public section:")
    print("int32_t ToQDataStream(QDataStream& orc_Stream) const;")
    print("int32_t FromQDataStream(QDataStream& orc_Stream);")
    print("QJsonObject ToJsonObject() const;")
    print("int32_t FromJsonObject(const QJsonObject& orc_Object);")
    print(
        'QDomElement ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName = "root") const;'
    )
    print("int32_t FromQDomElement(const QDomElement& orc_Element);")

    # Output cpp additions
    print("\n=== ADD TO CPP FILE ===")
    print(methods["to_datastream"])
    print(methods["from_datastream"])
    print(methods["to_json"])
    print(methods["from_json"])
    print(methods["to_xml"])
    print(methods["from_xml"])


if __name__ == "__main__":
    main()
