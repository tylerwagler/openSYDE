//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for application related information (implementation)

   Data class for application related information

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "stwerrors.hpp"

#include "C_OscNodeApplication.hpp"

#include "C_OscFilerUtil.hpp"
#include "C_OscHashUtil.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_SclChecksums.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::errors;
using namespace stw::opensyde_core;
using namespace stw::scl;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */

/* -- Module Global Variables
 * ---------------------------------------------------------------------------------------
 */

namespace
{
const C_OscFilerUtil::EnumEntry<C_OscNodeApplication::E_Type> mac_APPLICATION_TYPE_TABLE[] = {
   {C_OscNodeApplication::eBINARY, "binary"},
   {C_OscNodeApplication::ePROGRAMMABLE_APPLICATION, "programming_application"},
   {C_OscNodeApplication::ePARAMETER_SET_HALC, "parameter_set_halc"}
};
}

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscNodeApplication::C_OscNodeApplication(void)
    : e_Type(eBINARY), c_Name(""), c_Comment(""), q_Active(true),
      u8_ProcessId(0), c_ProjectPath(""), c_IdeCall(""),
      c_CodeGeneratorPath(""), c_GeneratePath(""), u16_GenCodeVersion(0) {
  // default: one output file
  c_ResultPaths.resize(1);
  c_ResultPaths[0] = "";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.
   It is not endian-safe, so it should only be used on the same system it is
   created on.

   \param[in,out] oru32_HashValue    Hash value with initial [in] value and
   result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeApplication::CalcHash(uint32_t &oru32_HashValue) const {
   hash_util::CalcHashMembers(oru32_HashValue,
                              this->e_Type, this->c_Name, this->c_Comment,
                              this->q_Active, this->u8_ProcessId,
                              this->c_ProjectPath, this->c_IdeCall,
                              this->c_CodeGeneratorPath, this->c_GeneratePath,
                              this->u16_GenCodeVersion, this->c_ResultPaths);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Transform application type to string

   \param[in] ore_Application Application type

   \return
   Stringified application type
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscNodeApplication::h_ApplicationToString(
    const C_OscNodeApplication::E_Type &ore_Application) {
  return C_OscFilerUtil::h_EnumToString(ore_Application, mac_APPLICATION_TYPE_TABLE);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Transform string to application type

   \param[in]  orc_String String to interpret
   \param[out] ore_Type   Application type
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeApplication::h_StringToApplication(
    const QString &orc_String, C_OscNodeApplication::E_Type &ore_Type) {
  if (C_OscFilerUtil::h_StringToEnum(orc_String, mac_APPLICATION_TYPE_TABLE, ore_Type) != stw::errors::C_NO_ERR) {
    ore_Type = C_OscNodeApplication::eBINARY;
    osc_write_log_warning(
        "Loading node definition",
        "Invalid value for application type. Unknown type: " + orc_String +
            ". "
            "Type is replaced with: " +
            h_ApplicationToString(C_OscNodeApplication::eBINARY));
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream
   \param   ro_DataStream  Output stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeApplication::ToQDataStream(QDataStream &ro_DataStream) const {
  ro_DataStream << static_cast<int32_t>(e_Type) << c_Name << c_Comment
                << q_Active << static_cast<int32_t>(u8_ProcessId)
                << c_ProjectPath << c_IdeCall << c_CodeGeneratorPath
                << c_GeneratePath << static_cast<int32_t>(u16_GenCodeVersion)
                << c_ResultPaths;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream
   \param   ro_DataStream  Input stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeApplication::FromQDataStream(QDataStream &ro_DataStream) {
  int32_t s32_Type;
  ro_DataStream >> s32_Type >> c_Name >> c_Comment >> q_Active
      >> u8_ProcessId >> c_ProjectPath >> c_IdeCall >> c_CodeGeneratorPath
      >> c_GeneratePath >> u16_GenCodeVersion >> c_ResultPaths;
  e_Type = static_cast<E_Type>(s32_Type);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QJsonObject
   \return  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscNodeApplication::ToJsonObject() const {
  QJsonObject c_Obj;
  c_Obj["type"] = h_ApplicationToString(e_Type);
  c_Obj["name"] = c_Name;
  c_Obj["comment"] = c_Comment;
  c_Obj["active"] = q_Active;
  c_Obj["process-id"] = static_cast<int32_t>(u8_ProcessId);
  c_Obj["project-path"] = c_ProjectPath;
  c_Obj["ide-call"] = c_IdeCall;
  c_Obj["code-generator-path"] = c_CodeGeneratorPath;
  c_Obj["generate-path"] = c_GeneratePath;
  c_Obj["generated-code-version"] = static_cast<int32_t>(u16_GenCodeVersion);
  c_Obj["result-paths"] = QJsonValue::fromVariant(c_ResultPaths);
  return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QJsonObject
   \param   orc_Object  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeApplication::FromJsonObject(const QJsonObject &orc_Object) {
  if (orc_Object.contains("type")) {
    h_StringToApplication(orc_Object["type"].toString(), e_Type);
  }
  if (orc_Object.contains("name")) {
    c_Name = orc_Object["name"].toString();
  }
  if (orc_Object.contains("comment")) {
    c_Comment = orc_Object["comment"].toString();
  }
  if (orc_Object.contains("active")) {
    q_Active = orc_Object["active"].toBool();
  }
  if (orc_Object.contains("process-id")) {
    u8_ProcessId = static_cast<uint8_t>(orc_Object["process-id"].toInt());
  }
  if (orc_Object.contains("project-path")) {
    c_ProjectPath = orc_Object["project-path"].toString();
  }
  if (orc_Object.contains("ide-call")) {
    c_IdeCall = orc_Object["ide-call"].toString();
  }
  if (orc_Object.contains("code-generator-path")) {
    c_CodeGeneratorPath = orc_Object["code-generator-path"].toString();
  }
  if (orc_Object.contains("generate-path")) {
    c_GeneratePath = orc_Object["generate-path"].toString();
  }
  if (orc_Object.contains("generated-code-version")) {
    u16_GenCodeVersion =
        static_cast<uint16_t>(orc_Object["generated-code-version"].toInt());
  }
  if (orc_Object.contains("result-paths")) {
    QJsonArray c_Array = orc_Object["result-paths"].toArray();
    c_ResultPaths.clear();
    for (const QJsonValue &c_Value : c_Array) {
      c_ResultPaths.append(c_Value.toString());
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDomElement
   \param   orc_Doc        XML document
   \param   orc_ElementName  Element name
   \return  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscNodeApplication::ToQDomDocument(QDomDocument &orc_Doc,
                                                 const QString &orc_ElementName) const {
  QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
  c_Element.setAttribute("type", h_ApplicationToString(e_Type));
  c_Element.setAttribute("process-id", static_cast<int32_t>(u8_ProcessId));
  c_Element.setAttribute("active", q_Active);
  if (u16_GenCodeVersion > 0) {
    c_Element.setAttribute("generated-code-version",
                           static_cast<int32_t>(u16_GenCodeVersion));
  }

  if (!c_Name.isEmpty()) {
    QDomElement c_NameElement = orc_Doc.createElement("name");
    c_NameElement.appendChild(orc_Doc.createTextNode(c_Name));
    c_Element.appendChild(c_NameElement);
  }
  if (!c_Comment.isEmpty()) {
    QDomElement c_CommentElement = orc_Doc.createElement("comment");
    c_CommentElement.appendChild(orc_Doc.createTextNode(c_Comment));
    c_Element.appendChild(c_CommentElement);
  }
  if (!c_ProjectPath.isEmpty()) {
    QDomElement c_ProjectElement = orc_Doc.createElement("project-path");
    c_ProjectElement.appendChild(orc_Doc.createTextNode(c_ProjectPath));
    c_Element.appendChild(c_ProjectElement);
  }
  if (!c_IdeCall.isEmpty()) {
    QDomElement c_IdeElement = orc_Doc.createElement("ide-call");
    c_IdeElement.appendChild(orc_Doc.createTextNode(c_IdeCall));
    c_Element.appendChild(c_IdeElement);
  }
  if (!c_CodeGeneratorPath.isEmpty()) {
    QDomElement c_GenPathElement = orc_Doc.createElement("code-generator-path");
    c_GenPathElement.appendChild(orc_Doc.createTextNode(c_CodeGeneratorPath));
    c_Element.appendChild(c_GenPathElement);
  }
  if (!c_GeneratePath.isEmpty()) {
    QDomElement c_GenElement = orc_Doc.createElement("generate-path");
    c_GenElement.appendChild(orc_Doc.createTextNode(c_GeneratePath));
    c_Element.appendChild(c_GenElement);
  }
  if (!c_ResultPaths.isEmpty()) {
    QDomElement c_ResultElement = orc_Doc.createElement("result-paths");
    for (const QString &c_Path : c_ResultPaths) {
      QDomElement c_PathElement = orc_Doc.createElement("path");
      c_PathElement.appendChild(orc_Doc.createTextNode(c_Path));
      c_ResultElement.appendChild(c_PathElement);
    }
    c_Element.appendChild(c_ResultElement);
  }

  return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDomElement
   \param   orc_Element  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeApplication::FromQDomDocument(const QDomElement &orc_Element) {
  if (orc_Element.hasAttribute("type")) {
    h_StringToApplication(orc_Element.attribute("type"), e_Type);
  }
  if (orc_Element.hasAttribute("process-id")) {
    u8_ProcessId = static_cast<uint8_t>(orc_Element.attribute("process-id").toInt());
  }
  if (orc_Element.hasAttribute("active")) {
    q_Active = orc_Element.attribute("active").toInt();
  }
  if (orc_Element.hasAttribute("generated-code-version")) {
    u16_GenCodeVersion = static_cast<uint16_t>(
        orc_Element.attribute("generated-code-version").toInt());
  }

  QDomNode c_Node = orc_Element.firstChild();
  while (!c_Node.isNull()) {
    QDomElement c_Elem = c_Node.toElement();
    if (!c_Elem.isNull()) {
      const QString c_TagName = c_Elem.tagName();
      if (c_TagName == "name") {
        c_Name = c_Elem.text();
      } else if (c_TagName == "comment") {
        c_Comment = c_Elem.text();
      } else if (c_TagName == "project-path") {
        c_ProjectPath = c_Elem.text();
      } else if (c_TagName == "ide-call") {
        c_IdeCall = c_Elem.text();
      } else if (c_TagName == "code-generator-path") {
        c_CodeGeneratorPath = c_Elem.text();
      } else if (c_TagName == "generate-path") {
        c_GeneratePath = c_Elem.text();
      } else if (c_TagName == "result-paths") {
        c_ResultPaths.clear();
        QDomNode c_PathNode = c_Elem.firstChild();
        while (!c_PathNode.isNull()) {
          QDomElement c_PathElem = c_PathNode.toElement();
          if (!c_PathElem.isNull() && c_PathElem.tagName() == "path") {
            c_ResultPaths.append(c_PathElem.text());
          }
          c_PathNode = c_PathNode.nextSibling();
        }
      }
    }
    c_Node = c_Node.nextSibling();
  }
}
