//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE target support package V2 data handling class
   (implementation)

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscTargetSupportPackageV2.hpp"

#include <QDataStream>
#include <QDomDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::opensyde_core;

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

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Constructor

   Initialize all class elements with default values
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscTspApplication::C_OscTspApplication(void)
    : q_IsProgrammable(true), u8_ProcessId(0), c_Name(""), c_Comment(""),
      c_IdeCall(""), q_IsStandardSydeCoderCe(true), c_CodeGeneratorPath(""),
      c_ProjectFolder(""), c_GeneratePath(""), u16_GenCodeVersion(0),
      q_GeneratesPsiFiles(false) {
  c_ResultPaths.clear();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Constructor

   Initialize all class elements with default values
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscTargetSupportPackageV2::C_OscTargetSupportPackageV2(void)
    : c_DeviceName(""), c_Comment(""), u8_ApplicationIndex(0U),
      u8_MaxParallelTransmissions(64U), u16_MaxMessageBufferTx(585U),
      u16_MaxRoutingMessageBufferRx(585U), c_TemplatePath(""),
      c_HalcDefPath(""), c_HalcComment("") {
  c_Applications.clear();
  c_CodeExportSettings.Initialize();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Clear elements

   Sets all strings to "" and other values to default.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscTargetSupportPackageV2::Clear(void) {
  c_DeviceName = "";
  c_Comment = "";
  c_TemplatePath = "";

  u8_ApplicationIndex = 0U;
  u8_MaxParallelTransmissions = 64U;
  u16_MaxMessageBufferTx = 585U;
  u16_MaxRoutingMessageBufferRx = 585U;

  c_Applications.clear();
  c_CodeExportSettings.Initialize();
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize C_OscTspApplication to QDataStream
   \param   ro_DataStream  Output stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscTspApplication::ToQDataStream(QDataStream &ro_DataStream) const {
  ro_DataStream << q_IsProgrammable << u8_ProcessId << c_Name << c_Comment
                << c_IdeCall << q_IsStandardSydeCoderCe << c_CodeGeneratorPath
                << c_ProjectFolder << c_GeneratePath << u16_GenCodeVersion
                << c_ResultPaths << q_GeneratesPsiFiles;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize C_OscTspApplication from QDataStream
   \param   ro_DataStream  Input stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscTspApplication::FromQDataStream(QDataStream &ro_DataStream) {
  ro_DataStream >> q_IsProgrammable >> u8_ProcessId >> c_Name >> c_Comment
      >> c_IdeCall >> q_IsStandardSydeCoderCe >> c_CodeGeneratorPath
      >> c_ProjectFolder >> c_GeneratePath >> u16_GenCodeVersion
      >> c_ResultPaths >> q_GeneratesPsiFiles;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize C_OscTspApplication to QJsonObject
   \return  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscTspApplication::ToJsonObject() const {
  QJsonObject c_Obj;
  c_Obj["is-programmable"] = q_IsProgrammable;
  c_Obj["process-id"] = static_cast<int32_t>(u8_ProcessId);
  c_Obj["name"] = c_Name;
  c_Obj["comment"] = c_Comment;
  c_Obj["ide-call"] = c_IdeCall;
  c_Obj["is-standard-syde-coder-ce"] = q_IsStandardSydeCoderCe;
  c_Obj["code-generator-path"] = c_CodeGeneratorPath;
  c_Obj["project-folder"] = c_ProjectFolder;
  c_Obj["generate-path"] = c_GeneratePath;
  c_Obj["generated-code-version"] = static_cast<int32_t>(u16_GenCodeVersion);
  c_Obj["result-paths"] = QJsonValue::fromVariant(c_ResultPaths);
  c_Obj["generates-halc-psi"] = q_GeneratesPsiFiles;
  return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize C_OscTspApplication from QJsonObject
   \param   orc_Object  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscTspApplication::FromJsonObject(const QJsonObject &orc_Object) {
  if (orc_Object.contains("is-programmable")) {
    q_IsProgrammable = orc_Object["is-programmable"].toBool();
  }
  if (orc_Object.contains("process-id")) {
    u8_ProcessId = static_cast<uint8_t>(orc_Object["process-id"].toInt());
  }
  if (orc_Object.contains("name")) {
    c_Name = orc_Object["name"].toString();
  }
  if (orc_Object.contains("comment")) {
    c_Comment = orc_Object["comment"].toString();
  }
  if (orc_Object.contains("ide-call")) {
    c_IdeCall = orc_Object["ide-call"].toString();
  }
  if (orc_Object.contains("is-standard-syde-coder-ce")) {
    q_IsStandardSydeCoderCe =
        orc_Object["is-standard-syde-coder-ce"].toBool();
  }
  if (orc_Object.contains("code-generator-path")) {
    c_CodeGeneratorPath = orc_Object["code-generator-path"].toString();
  }
  if (orc_Object.contains("project-folder")) {
    c_ProjectFolder = orc_Object["project-folder"].toString();
  }
  if (orc_Object.contains("generate-path")) {
    c_GeneratePath = orc_Object["generate-path"].toString();
  }
  if (orc_Object.contains("generated-code-version")) {
    u16_GenCodeVersion =
        static_cast<uint16_t>(orc_Object["generated-code-version"].toInt());
  }
  if (orc_Object.contains("result-paths")) {
    QJsonArray c_ResultPathsArray = orc_Object["result-paths"].toArray();
    c_ResultPaths.clear();
    for (const QJsonValue &c_Value : c_ResultPathsArray) {
      c_ResultPaths.append(c_Value.toString());
    }
  }
  if (orc_Object.contains("generates-halc-psi")) {
    q_GeneratesPsiFiles = orc_Object["generates-halc-psi"].toBool();
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize C_OscTspApplication to QDomElement
   \param   orc_Doc        XML document
   \param   orc_ElementName  Element name
   \return  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscTspApplication::ToQDomDocument(QDomDocument &orc_Doc,
                                                const QString &orc_ElementName) const {
  QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
  c_Element.setAttribute("is-programmable", q_IsProgrammable);
  c_Element.setAttribute("process-id", static_cast<int32_t>(u8_ProcessId));
  c_Element.setAttribute("generates-halc-psi", q_GeneratesPsiFiles);
  if (u16_GenCodeVersion > 0) {
    c_Element.setAttribute("generated-code-version",
                           static_cast<int32_t>(u16_GenCodeVersion));
  }
  if (!c_ResultPaths.isEmpty()) {
    QDomElement c_ResultElement = orc_Doc.createElement("result-paths");
    for (const QString &c_Path : c_ResultPaths) {
      QDomElement c_OutputElement = orc_Doc.createElement("output-file");
      c_OutputElement.appendChild(orc_Doc.createTextNode(c_Path));
      c_ResultElement.appendChild(c_OutputElement);
    }
    c_Element.appendChild(c_ResultElement);
  }
  if (!c_Name.isEmpty()) {
    QDomElement c_NameElement = orc_Doc.createElement("display-name");
    c_NameElement.appendChild(orc_Doc.createTextNode(c_Name));
    c_Element.appendChild(c_NameElement);
  }
  if (!c_Comment.isEmpty()) {
    QDomElement c_CommentElement = orc_Doc.createElement("comment");
    c_CommentElement.appendChild(orc_Doc.createTextNode(c_Comment));
    c_Element.appendChild(c_CommentElement);
  }
  if (!c_IdeCall.isEmpty()) {
    QDomElement c_IdeElement = orc_Doc.createElement("ide-call");
    c_IdeElement.appendChild(orc_Doc.createTextNode(c_IdeCall));
    c_Element.appendChild(c_IdeElement);
  }
  QDomElement c_CodeGenElement = orc_Doc.createElement("code-generation");
  c_CodeGenElement.setAttribute("is-standard-opensyde",
                                q_IsStandardSydeCoderCe);
  if (!q_IsStandardSydeCoderCe && !c_CodeGeneratorPath.isEmpty()) {
    QDomElement c_PathElement = orc_Doc.createElement("path");
    c_PathElement.appendChild(orc_Doc.createTextNode(c_CodeGeneratorPath));
    c_CodeGenElement.appendChild(c_PathElement);
  }
  c_Element.appendChild(c_CodeGenElement);
  if (!c_ProjectFolder.isEmpty()) {
    QDomElement c_ProjFolderElement = orc_Doc.createElement("project-folder");
    c_ProjFolderElement.appendChild(orc_Doc.createTextNode(c_ProjectFolder));
    c_Element.appendChild(c_ProjFolderElement);
  }
  if (!c_GeneratePath.isEmpty()) {
    QDomElement c_GenerateElement = orc_Doc.createElement("generate");
    c_GenerateElement.appendChild(orc_Doc.createTextNode(c_GeneratePath));
    c_Element.appendChild(c_GenerateElement);
  }
  return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize C_OscTspApplication from QDomElement
   \param   orc_Element  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscTspApplication::FromQDomDocument(const QDomElement &orc_Element) {
  if (orc_Element.hasAttribute("is-programmable")) {
    q_IsProgrammable = orc_Element.attribute("is-programmable").toInt();
  }
  if (orc_Element.hasAttribute("process-id")) {
    u8_ProcessId =
        static_cast<uint8_t>(orc_Element.attribute("process-id").toInt());
  }
  if (orc_Element.hasAttribute("generates-halc-psi")) {
    q_GeneratesPsiFiles = orc_Element.attribute("generates-halc-psi").toInt();
  }
  if (orc_Element.hasAttribute("generated-code-version")) {
    u16_GenCodeVersion = static_cast<uint16_t>(
        orc_Element.attribute("generated-code-version").toInt());
  }

  QDomNode c_Node = orc_Element.firstChild();
  while (!c_Node.isNull()) {
    QDomElement c_Elem = c_Node.toElement();
    if (!c_Elem.isNull()) {
      if (c_Elem.tagName() == "display-name") {
        c_Name = c_Elem.text();
      } else if (c_Elem.tagName() == "comment") {
        c_Comment = c_Elem.text();
      } else if (c_Elem.tagName() == "ide-call") {
        c_IdeCall = c_Elem.text();
      } else if (c_Elem.tagName() == "code-generation") {
        if (c_Elem.hasAttribute("is-standard-opensyde")) {
          q_IsStandardSydeCoderCe =
              c_Elem.attribute("is-standard-opensyde").toInt();
        }
        QDomNode c_CgChild = c_Elem.firstChild();
        while (!c_CgChild.isNull()) {
          QDomElement c_CgElem = c_CgChild.toElement();
          if (!c_CgElem.isNull() && c_CgElem.tagName() == "path") {
            c_CodeGeneratorPath = c_CgElem.text();
          }
          c_CgChild = c_CgChild.nextSibling();
        }
      } else if (c_Elem.tagName() == "project-folder") {
        c_ProjectFolder = c_Elem.text();
      } else if (c_Elem.tagName() == "generate") {
        c_GeneratePath = c_Elem.text();
      } else if (c_Elem.tagName() == "result-paths") {
        QDomNode c_RpChild = c_Elem.firstChild();
        while (!c_RpChild.isNull()) {
          QDomElement c_RpElem = c_RpChild.toElement();
          if (!c_RpElem.isNull() && c_RpElem.tagName() == "output-file") {
            c_ResultPaths.append(c_RpElem.text());
          }
          c_RpChild = c_RpChild.nextSibling();
        }
      }
    }
    c_Node = c_Node.nextSibling();
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize C_OscTargetSupportPackageV2 to QDataStream
   \param   ro_DataStream  Output stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscTargetSupportPackageV2::ToQDataStream(QDataStream &ro_DataStream) const {
  ro_DataStream << c_DeviceName << c_Comment << static_cast<int32_t>(c_Applications.size());
  for (const C_OscTspApplication &c_App : c_Applications) {
    c_App.ToQDataStream(ro_DataStream);
  }
  ro_DataStream << static_cast<int32_t>(u8_ApplicationIndex)
                << static_cast<int32_t>(u8_MaxParallelTransmissions)
                << static_cast<int32_t>(u16_MaxMessageBufferTx)
                << static_cast<int32_t>(u16_MaxRoutingMessageBufferRx)
                << c_TemplatePath << c_HalcDefPath << c_HalcComment;
  // Serialize CodeExportSettings
  c_CodeExportSettings.ToQDataStream(ro_DataStream);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize C_OscTargetSupportPackageV2 from QDataStream
   \param   ro_DataStream  Input stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscTargetSupportPackageV2::FromQDataStream(QDataStream &ro_DataStream) {
  ro_DataStream >> c_DeviceName >> c_Comment;
  c_Applications.clear();
  int32_t s32_Count;
  ro_DataStream >> s32_Count;
  for (int32_t i = 0; i < s32_Count; ++i) {
    C_OscTspApplication c_App;
    c_App.FromQDataStream(ro_DataStream);
    c_Applications.append(c_App);
  }
  ro_DataStream >> u8_ApplicationIndex >> u8_MaxParallelTransmissions
      >> u16_MaxMessageBufferTx >> u16_MaxRoutingMessageBufferRx
      >> c_TemplatePath >> c_HalcDefPath >> c_HalcComment;
  c_CodeExportSettings.FromQDataStream(ro_DataStream);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize C_OscTargetSupportPackageV2 to QJsonObject
   \return  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscTargetSupportPackageV2::ToJsonObject() const {
  QJsonObject c_Obj;
  c_Obj["device-name"] = c_DeviceName;
  c_Obj["comment"] = c_Comment;
  c_Obj["dpd-application-index"] = static_cast<int32_t>(u8_ApplicationIndex);
  c_Obj["max-parallel-transmissions"] =
      static_cast<int32_t>(u8_MaxParallelTransmissions);
  c_Obj["max-tx-message-buffer"] = static_cast<int32_t>(u16_MaxMessageBufferTx);
  c_Obj["max-rx-routing-message-buffer"] =
      static_cast<int32_t>(u16_MaxRoutingMessageBufferRx);
  c_Obj["template-path"] = c_TemplatePath;
  c_Obj["halc-def-path"] = c_HalcDefPath;
  c_Obj["halc-comment"] = c_HalcComment;

  // Serialize applications
  QJsonArray c_AppsArray;
  for (const C_OscTspApplication &c_App : c_Applications) {
    c_AppsArray.append(c_App.ToJsonObject());
  }
  c_Obj["applications"] = c_AppsArray;

  // Serialize code export settings
  c_Obj["code-export-settings"] = c_CodeExportSettings.ToJsonObject();

  return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize C_OscTargetSupportPackageV2 from QJsonObject
   \param   orc_Object  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscTargetSupportPackageV2::FromJsonObject(const QJsonObject &orc_Object) {
  if (orc_Object.contains("device-name")) {
    c_DeviceName = orc_Object["device-name"].toString();
  }
  if (orc_Object.contains("comment")) {
    c_Comment = orc_Object["comment"].toString();
  }
  if (orc_Object.contains("dpd-application-index")) {
    u8_ApplicationIndex =
        static_cast<uint8_t>(orc_Object["dpd-application-index"].toInt());
  }
  if (orc_Object.contains("max-parallel-transmissions")) {
    u8_MaxParallelTransmissions = static_cast<uint8_t>(
        orc_Object["max-parallel-transmissions"].toInt());
  }
  if (orc_Object.contains("max-tx-message-buffer")) {
    u16_MaxMessageBufferTx = static_cast<uint16_t>(
        orc_Object["max-tx-message-buffer"].toInt());
  }
  if (orc_Object.contains("max-rx-routing-message-buffer")) {
    u16_MaxRoutingMessageBufferRx = static_cast<uint16_t>(
        orc_Object["max-rx-routing-message-buffer"].toInt());
  }
  if (orc_Object.contains("template-path")) {
    c_TemplatePath = orc_Object["template-path"].toString();
  }
  if (orc_Object.contains("halc-def-path")) {
    c_HalcDefPath = orc_Object["halc-def-path"].toString();
  }
  if (orc_Object.contains("halc-comment")) {
    c_HalcComment = orc_Object["halc-comment"].toString();
  }

  // Deserialize applications
  c_Applications.clear();
  if (orc_Object.contains("applications")) {
    QJsonArray c_AppsArray = orc_Object["applications"].toArray();
    for (const QJsonValue &c_Value : c_AppsArray) {
      C_OscTspApplication c_App;
      c_App.FromJsonObject(c_Value.toObject());
      c_Applications.append(c_App);
    }
  }

  // Deserialize code export settings
  if (orc_Object.contains("code-export-settings")) {
    c_CodeExportSettings.FromJsonObject(
        orc_Object["code-export-settings"].toObject());
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize C_OscTargetSupportPackageV2 to QDomElement
   \param   orc_Doc        XML document
   \param   orc_ElementName  Element name
   \return  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscTargetSupportPackageV2::ToQDomDocument(
    QDomDocument &orc_Doc, const QString &orc_ElementName) const {
  QDomElement c_Element = orc_Doc.createElement(orc_ElementName);

  // Add file version
  QDomElement c_VersionElement = orc_Doc.createElement("file-version");
  c_VersionElement.appendChild(orc_Doc.createTextNode("2"));
  c_Element.appendChild(c_VersionElement);

  // Device name
  QDomElement c_DeviceElement = orc_Doc.createElement("device-name");
  c_DeviceElement.appendChild(orc_Doc.createTextNode(c_DeviceName));
  c_Element.appendChild(c_DeviceElement);

  // Comment
  if (!c_Comment.isEmpty()) {
    QDomElement c_TspCommentElement = orc_Doc.createElement("tsp-comment");
    c_TspCommentElement.appendChild(orc_Doc.createTextNode(c_Comment));
    c_Element.appendChild(c_TspCommentElement);
  }

  // Code export settings
  QDomElement c_SettingsElement =
      c_CodeExportSettings.ToQDomDocument(orc_Doc, "code-export-settings");
  c_Element.appendChild(c_SettingsElement);

  // OpenSYDE server settings
  QDomElement c_ServerElement =
      orc_Doc.createElement("open-syde-server-settings");
  c_ServerElement.setAttribute("dpd-application-index",
                               static_cast<int32_t>(u8_ApplicationIndex));
  c_ServerElement.setAttribute("max-parallel-transmissions",
                               static_cast<int32_t>(u8_MaxParallelTransmissions));
  c_ServerElement.setAttribute("max-tx-message-buffer",
                               static_cast<int32_t>(u16_MaxMessageBufferTx));
  c_ServerElement.setAttribute("max-rx-routing-message-buffer",
                               static_cast<int32_t>(u16_MaxRoutingMessageBufferRx));
  c_Element.appendChild(c_ServerElement);

  // Template project
  QDomElement c_TemplateElement = orc_Doc.createElement("template-project");
  QDomElement c_TemplatePathElement = orc_Doc.createElement("template");
  c_TemplatePathElement.appendChild(orc_Doc.createTextNode(c_TemplatePath));
  c_TemplateElement.appendChild(c_TemplatePathElement);
  c_Element.appendChild(c_TemplateElement);

  // HALC definition
  if (!c_HalcDefPath.isEmpty()) {
    QDomElement c_HalcElement = orc_Doc.createElement("halc-definition");
    QDomElement c_HalcPathElement = orc_Doc.createElement("halc-path");
    c_HalcPathElement.appendChild(orc_Doc.createTextNode(c_HalcDefPath));
    c_HalcElement.appendChild(c_HalcPathElement);
    if (!c_HalcComment.isEmpty()) {
      QDomElement c_HalcCommentElement = orc_Doc.createElement("halc-comment");
      c_HalcCommentElement.appendChild(orc_Doc.createTextNode(c_HalcComment));
      c_HalcElement.appendChild(c_HalcCommentElement);
    }
    c_Element.appendChild(c_HalcElement);
  }

  // Applications
  for (const C_OscTspApplication &c_App : c_Applications) {
    QDomElement c_AppElement = c_App.ToQDomDocument(orc_Doc, "application");
    c_Element.appendChild(c_AppElement);
  }

  return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize C_OscTargetSupportPackageV2 from QDomElement
   \param   orc_Element  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscTargetSupportPackageV2::FromQDomDocument(const QDomElement &orc_Element) {
  // Clear existing data
  Clear();

  QDomNode c_Node = orc_Element.firstChild();
  while (!c_Node.isNull()) {
    QDomElement c_Elem = c_Node.toElement();
    if (!c_Elem.isNull()) {
      const QString c_TagName = c_Elem.tagName();
      if (c_TagName == "device-name") {
        c_DeviceName = c_Elem.text();
      } else if (c_TagName == "tsp-comment") {
        c_Comment = c_Elem.text();
      } else if (c_TagName == "code-export-settings") {
        c_CodeExportSettings.FromQDomDocument(c_Elem);
      } else if (c_TagName == "open-syde-server-settings") {
        if (c_Elem.hasAttribute("dpd-application-index")) {
          u8_ApplicationIndex = static_cast<uint8_t>(
              c_Elem.attribute("dpd-application-index").toInt());
        }
        if (c_Elem.hasAttribute("max-parallel-transmissions")) {
          u8_MaxParallelTransmissions = static_cast<uint8_t>(
              c_Elem.attribute("max-parallel-transmissions").toInt());
        }
        if (c_Elem.hasAttribute("max-tx-message-buffer")) {
          u16_MaxMessageBufferTx = static_cast<uint16_t>(
              c_Elem.attribute("max-tx-message-buffer").toInt());
        }
        if (c_Elem.hasAttribute("max-rx-routing-message-buffer")) {
          u16_MaxRoutingMessageBufferRx = static_cast<uint16_t>(
              c_Elem.attribute("max-rx-routing-message-buffer").toInt());
        }
      } else if (c_TagName == "template-project") {
        QDomNode c_TpChild = c_Elem.firstChild();
        while (!c_TpChild.isNull()) {
          QDomElement c_TpElem = c_TpChild.toElement();
          if (!c_TpElem.isNull() && c_TpElem.tagName() == "template") {
            c_TemplatePath = c_TpElem.text();
          }
          c_TpChild = c_TpChild.nextSibling();
        }
      } else if (c_TagName == "halc-definition") {
        QDomNode c_HChild = c_Elem.firstChild();
        while (!c_HChild.isNull()) {
          QDomElement c_HElem = c_HChild.toElement();
          if (!c_HElem.isNull()) {
            if (c_HElem.tagName() == "halc-path") {
              c_HalcDefPath = c_HElem.text();
            } else if (c_HElem.tagName() == "halc-comment") {
              c_HalcComment = c_HElem.text();
            }
          }
          c_HChild = c_HChild.nextSibling();
        }
      } else if (c_TagName == "application") {
        C_OscTspApplication c_App;
        c_App.FromQDomDocument(c_Elem);
        c_Applications.append(c_App);
      }
    }
    c_Node = c_Node.nextSibling();
  }
}
