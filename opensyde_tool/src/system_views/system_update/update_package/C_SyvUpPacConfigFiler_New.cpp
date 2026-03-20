//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Update package configuration file reader/writer (Multi-Format)

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include <QFileInfo>
#include <QJsonDocument>

#include "C_SyvUpPacConfigFiler.hpp"
#include "C_OscLoggingHandler.hpp"
#include "stwerrors.hpp"

using namespace stw::errors;
using namespace stw::opensyde_gui_logic;

// C_SyvUpPacConfigNodeApp serialization
void C_SyvUpPacConfigNodeApp::ToQDataStream(QDataStream& ro_Stream) const {
   ro_Stream << static_cast<int32_t>(e_Type) << c_Name << c_Path;
}

void C_SyvUpPacConfigNodeApp::FromQDataStream(QDataStream& ro_Stream) {
   int32_t s32_Type;
   ro_Stream >> s32_Type >> c_Name >> c_Path;
   e_Type = static_caststw::opensyde_core::C_OscNodeApplication::E_Type>(s32_Type);
}

QJsonObject C_SyvUpPacConfigNodeApp::ToJsonObject() const {
   QJsonObject c_Obj;
   c_Obj["type"] = stw::opensyde_core::C_OscNodeApplication::h_ApplicationToString(e_Type);
   c_Obj["name"] = c_Name;
   c_Obj["path"] = c_Path;
   return c_Obj;
}

void C_SyvUpPacConfigNodeApp::FromJsonObject(const QJsonObject& orc_Object) {
   if (orc_Object.contains("type")) {
      stw::opensyde_core::C_OscNodeApplication::h_StringToApplication(
         orc_Object["type"].toString(), e_Type);
   }
   if (orc_Object.contains("name")) c_Name = orc_Object["name"].toString();
   if (orc_Object.contains("path")) c_Path = orc_Object["path"].toString();
}

QDomElement C_SyvUpPacConfigNodeApp::ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_Name) const {
   QDomElement c_Element = orc_Doc.createElement(orc_Name);
   c_Element.setAttribute("type", stw::opensyde_core::C_OscNodeApplication::h_ApplicationToString(e_Type));
   if (!c_Name.isEmpty()) {
      QDomElement c_NameElem = orc_Doc.createElement("name");
      c_NameElem.appendChild(orc_Doc.createTextNode(c_Name));
      c_Element.appendChild(c_NameElem);
   }
   if (!c_Path.isEmpty()) {
      QDomElement c_PathElem = orc_Doc.createElement("path");
      c_PathElem.appendChild(orc_Doc.createTextNode(c_Path));
      c_Element.appendChild(c_PathElem);
   }
   return c_Element;
}

void C_SyvUpPacConfigNodeApp::FromQDomDocument(const QDomElement& orc_Element) {
   if (orc_Element.hasAttribute("type")) {
      stw::opensyde_core::C_OscNodeApplication::h_StringToApplication(
         orc_Element.attribute("type"), e_Type);
   }
   QDomNode c_Node = orc_Element.firstChild();
   while (!c_Node.isNull()) {
      QDomElement c_Elem = c_Node.toElement();
      if (!c_Elem.isNull()) {
         if (c_Elem.tagName() == "name") c_Name = c_Elem.text();
         else if (c_Elem.tagName() == "path") c_Path = c_Elem.text();
      }
      c_Node = c_Node.nextSibling();
   }
}

// C_SyvUpPacConfigNode serialization
void C_SyvUpPacConfigNode::ToQDataStream(QDataStream& ro_Stream) const {
   ro_Stream << c_Name << c_DeviceType << static_cast<int32_t>(c_AppConfigs.size());
   for (const auto& c_App : c_AppConfigs) c_App.ToQDataStream(ro_Stream);
   ro_Stream << c_ParamSetConfigs << c_FileConfigs << c_PemFilePath
             << static_cast<int32_t>(e_StateSecurity) << static_cast<int32_t>(e_StateDebugger);
}

void C_SyvUpPacConfigNode::FromQDataStream(QDataStream& ro_Stream) {
   int32_t s32_Count;
   ro_Stream >> c_Name >> c_DeviceType >> s32_Count;
   c_AppConfigs.clear();
   for (int32_t i = 0; i < s32_Count; ++i) {
      C_SyvUpPacConfigNodeApp c_App;
      c_App.FromQDataStream(ro_Stream);
      c_AppConfigs.append(c_App);
   }
   ro_Stream >> c_ParamSetConfigs >> c_FileConfigs >> c_PemFilePath;
   int32_t s32_Sec, s32_Debug;
   ro_Stream >> s32_Sec >> s32_Debug;
   e_StateSecurity = static_caststw::opensyde_core::C_OscViewNodeUpdate::E_StateSecurity>(s32_Sec);
   e_StateDebugger = static_caststw::opensyde_core::C_OscViewNodeUpdate::E_StateDebugger>(s32_Debug);
}

QJsonObject C_SyvUpPacConfigNode::ToJsonObject() const {
   QJsonObject c_Obj;
   c_Obj["name"] = c_Name;
   c_Obj["device-type"] = c_DeviceType;
   QJsonArray c_Apps;
   for (const auto& c_App : c_AppConfigs) c_Apps.append(c_App.ToJsonObject());
   c_Obj["apps"] = c_Apps;
   c_Obj["param-sets"] = QJsonValue::fromVariant(c_ParamSetConfigs);
   c_Obj["files"] = QJsonValue::fromVariant(c_FileConfigs);
   c_Obj["pem-file-path"] = c_PemFilePath;
   c_Obj["state-security"] = stw::opensyde_core::C_OscViewNodeUpdate::h_StateSecurityToString(e_StateSecurity);
   c_Obj["state-debugger"] = stw::opensyde_core::C_OscViewNodeUpdate::h_StateDebuggerToString(e_StateDebugger);
   return c_Obj;
}

void C_SyvUpPacConfigNode::FromJsonObject(const QJsonObject& orc_Object) {
   if (orc_Object.contains("name")) c_Name = orc_Object["name"].toString();
   if (orc_Object.contains("device-type")) c_DeviceType = orc_Object["device-type"].toString();
   if (orc_Object.contains("apps")) {
      c_AppConfigs.clear();
      for (const auto& c_Val : orc_Object["apps"].toArray()) {
         C_SyvUpPacConfigNodeApp c_App;
         c_App.FromJsonObject(c_Val.toObject());
         c_AppConfigs.append(c_App);
      }
   }
   if (orc_Object.contains("param-sets")) {
      c_ParamSetConfigs = orc_Object["param-sets"].toArray().toStringList();
   }
   if (orc_Object.contains("files")) {
      c_FileConfigs = orc_Object["files"].toArray().toStringList();
   }
   if (orc_Object.contains("pem-file-path")) c_PemFilePath = orc_Object["pem-file-path"].toString();
   if (orc_Object.contains("state-security")) {
      stw::opensyde_core::C_OscViewNodeUpdate::h_StringToStateSecurity(
         orc_Object["state-security"].toString(), e_StateSecurity);
   }
   if (orc_Object.contains("state-debugger")) {
      stw::opensyde_core::C_OscViewNodeUpdate::h_StringToStateDebugger(
         orc_Object["state-debugger"].toString(), e_StateDebugger);
   }
}

QDomElement C_SyvUpPacConfigNode::ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_Name) const {
   QDomElement c_Element = orc_Doc.createElement(orc_Name);
   c_Element.setAttribute("name", c_Name);
   c_Element.setAttribute("device-type", c_DeviceType);
   c_Element.setAttribute("state-security", stw::opensyde_core::C_OscViewNodeUpdate::h_StateSecurityToString(e_StateSecurity));
   c_Element.setAttribute("state-debugger", stw::opensyde_core::C_OscViewNodeUpdate::h_StateDebuggerToString(e_StateDebugger));
   
   QDomElement c_AppsElement = orc_Doc.createElement("apps");
   for (const auto& c_App : c_AppConfigs) {
      c_AppsElement.appendChild(c_App.ToQDomDocument(orc_Doc, "app"));
   }
   c_Element.appendChild(c_AppsElement);
   
   if (!c_ParamSetConfigs.isEmpty()) {
      QDomElement c_PSElement = orc_Doc.createElement("param-sets");
      for (const auto& c_PS : c_ParamSetConfigs) {
         QDomElement c_PSElem = orc_Doc.createElement("param-set");
         c_PSElem.appendChild(orc_Doc.createTextNode(c_PS));
         c_PSElement.appendChild(c_PSElem);
      }
      c_Element.appendChild(c_PSElement);
   }
   
   if (!c_FileConfigs.isEmpty()) {
      QDomElement c_FElement = orc_Doc.createElement("files");
      for (const auto& c_F : c_FileConfigs) {
         QDomElement c_FElem = orc_Doc.createElement("file");
         c_FElem.appendChild(orc_Doc.createTextNode(c_F));
         c_FElement.appendChild(c_FElem);
      }
      c_Element.appendChild(c_FElement);
   }
   
   if (!c_PemFilePath.isEmpty()) {
      QDomElement c_PemElement = orc_Doc.createElement("pem-file-path");
      c_PemElement.appendChild(orc_Doc.createTextNode(c_PemFilePath));
      c_Element.appendChild(c_PemElement);
   }
   
   return c_Element;
}

void C_SyvUpPacConfigNode::FromQDomDocument(const QDomElement& orc_Element) {
   if (orc_Element.hasAttribute("name")) c_Name = orc_Element.attribute("name");
   if (orc_Element.hasAttribute("device-type")) c_DeviceType = orc_Element.attribute("device-type");
   if (orc_Element.hasAttribute("state-security")) {
      stw::opensyde_core::C_OscViewNodeUpdate::h_StringToStateSecurity(
         orc_Element.attribute("state-security"), e_StateSecurity);
   }
   if (orc_Element.hasAttribute("state-debugger")) {
      stw::opensyde_core::C_OscViewNodeUpdate::h_StringToStateDebugger(
         orc_Element.attribute("state-debugger"), e_StateDebugger);
   }
   
   QDomNode c_Node = orc_Element.firstChild();
   while (!c_Node.isNull()) {
      QDomElement c_Elem = c_Node.toElement();
      if (!c_Elem.isNull()) {
         const QString c_Tag = c_Elem.tagName();
         if (c_Tag == "apps") {
            c_AppConfigs.clear();
            QDomNode c_AppNode = c_Elem.firstChild();
            while (!c_AppNode.isNull()) {
               QDomElement c_AppElem = c_AppNode.toElement();
               if (!c_AppElem.isNull() && c_AppElem.tagName() == "app") {
                  C_SyvUpPacConfigNodeApp c_App;
                  c_App.FromQDomDocument(c_AppElem);
                  c_AppConfigs.append(c_App);
               }
               c_AppNode = c_AppNode.nextSibling();
            }
         } else if (c_Tag == "param-sets") {
            c_ParamSetConfigs.clear();
            QDomNode c_PSNode = c_Elem.firstChild();
            while (!c_PSNode.isNull()) {
               QDomElement c_PSElem = c_PSNode.toElement();
               if (!c_PSElem.isNull() && c_PSElem.tagName() == "param-set") {
                  c_ParamSetConfigs.append(c_PSElem.text());
               }
               c_PSNode = c_PSNode.nextSibling();
            }
         } else if (c_Tag == "files") {
            c_FileConfigs.clear();
            QDomNode c_FNode = c_Elem.firstChild();
            while (!c_FNode.isNull()) {
               QDomElement c_FElem = c_FNode.toElement();
               if (!c_FElem.isNull() && c_FElem.tagName() == "file") {
                  c_FileConfigs.append(c_FElem.text());
               }
               c_FNode = c_FNode.nextSibling();
            }
         } else if (c_Tag == "pem-file-path") {
            c_PemFilePath = c_Elem.text();
         }
      }
      c_Node = c_Node.nextSibling();
   }
}

// C_SyvUpPacConfig serialization
void C_SyvUpPacConfig::ToQDataStream(QDataStream& ro_Stream) const {
   ro_Stream << static_cast<int32_t>(c_NodeConfigs.size());
   for (const auto& c_Node : c_NodeConfigs) c_Node.ToQDataStream(ro_Stream);
}

void C_SyvUpPacConfig::FromQDataStream(QDataStream& ro_Stream) {
   int32_t s32_Count;
   ro_Stream >> s32_Count;
   c_NodeConfigs.clear();
   for (int32_t i = 0; i < s32_Count; ++i) {
      C_SyvUpPacConfigNode c_Node;
      c_Node.FromQDataStream(ro_Stream);
      c_NodeConfigs.append(c_Node);
   }
}

QJsonObject C_SyvUpPacConfig::ToJsonObject() const {
   QJsonObject c_Obj;
   QJsonArray c_Nodes;
   for (const auto& c_Node : c_NodeConfigs) c_Nodes.append(c_Node.ToJsonObject());
   c_Obj["nodes"] = c_Nodes;
   return c_Obj;
}

void C_SyvUpPacConfig::FromJsonObject(const QJsonObject& orc_Object) {
   c_NodeConfigs.clear();
   if (orc_Object.contains("nodes")) {
      for (const auto& c_Val : orc_Object["nodes"].toArray()) {
         C_SyvUpPacConfigNode c_Node;
         c_Node.FromJsonObject(c_Val.toObject());
         c_NodeConfigs.append(c_Node);
      }
   }
}

QDomElement C_SyvUpPacConfig::ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_Name) const {
   QDomElement c_Element = orc_Doc.createElement(orc_Name);
   QDomElement c_NodesElement = orc_Doc.createElement("nodes");
   for (const auto& c_Node : c_NodeConfigs) {
      c_NodesElement.appendChild(c_Node.ToQDomDocument(orc_Doc, "node"));
   }
   c_Element.appendChild(c_NodesElement);
   return c_Element;
}

void C_SyvUpPacConfig::FromQDomDocument(const QDomElement& orc_Element) {
   c_NodeConfigs.clear();
   QDomNode c_Node = orc_Element.firstChild();
   while (!c_Node.isNull()) {
      QDomElement c_Elem = c_Node.toElement();
      if (!c_Elem.isNull() && c_Elem.tagName() == "nodes") {
         QDomNode c_NodeNode = c_Elem.firstChild();
         while (!c_NodeNode.isNull()) {
            QDomElement c_NodeElem = c_NodeNode.toElement();
            if (!c_NodeElem.isNull() && c_NodeElem.tagName() == "node") {
               C_SyvUpPacConfigNode c_NodeConfig;
               c_NodeConfig.FromQDomDocument(c_NodeElem);
               c_NodeConfigs.append(c_NodeConfig);
            }
            c_NodeNode = c_NodeNode.nextSibling();
         }
      }
      c_Node = c_Node.nextSibling();
   }
}

// Filer implementation
int32_t C_SyvUpPacConfigFiler_New::h_LoadFile(C_SyvUpPacConfig& orc_Config, const QString& orc_Path) {
   return mh_DetectAndLoad(orc_Config, orc_Path);
}

int32_t C_SyvUpPacConfigFiler_New::h_SaveFile(const C_SyvUpPacConfig& orc_Config, const QString& orc_Path) {
   int32_t s32_Result = C_NO_ERR;
   QString c_Lower = orc_Path.toLower();
   if (c_Lower.endsWith(".bin")) s32_Result = h_SaveBinary(orc_Config, orc_Path);
   else if (c_Lower.endsWith(".json")) s32_Result = h_SaveJson(orc_Config, orc_Path);
   else s32_Result = h_SaveXml(orc_Config, orc_Path);
   return s32_Result;
}

int32_t C_SyvUpPacConfigFiler_New::h_LoadBinary(C_SyvUpPacConfig& orc_Config, const QString& orc_Path) {
   QFile c_File(orc_Path);
   if (c_File.open(QIODevice::ReadOnly)) {
      QDataStream c_Stream(&c_File);
      c_Stream.setVersion(QDataStream::Qt_6_0);
      orc_Config.FromQDataStream(c_Stream);
      c_File.close();
      return C_NO_ERR;
   }
   osc_write_log_error("Loading update package config (binary)", "File not found: " + orc_Path);
   return C_RANGE;
}

int32_t C_SyvUpPacConfigFiler_New::h_SaveBinary(const C_SyvUpPacConfig& orc_Config, const QString& orc_Path) {
   QFile c_File(orc_Path);
   if (c_File.open(QIODevice::WriteOnly)) {
      QDataStream c_Stream(&c_File);
      c_Stream.setVersion(QDataStream::Qt_6_0);
      orc_Config.ToQDataStream(c_Stream);
      c_File.close();
      return C_NO_ERR;
   }
   osc_write_log_error("Saving update package config (binary)", "Cannot write: " + orc_Path);
   return C_RD_WR;
}

int32_t C_SyvUpPacConfigFiler_New::h_LoadJson(C_SyvUpPacConfig& orc_Config, const QString& orc_Path) {
   QFile c_File(orc_Path);
   if (c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QJsonParseError c_Error;
      QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll(), &c_Error);
      c_File.close();
      if (c_Error.error == QJsonParseError::NoError && c_Doc.isObject()) {
         orc_Config.FromJsonObject(c_Doc.object());
         return C_NO_ERR;
      }
      osc_write_log_error("Loading update package config (JSON)", c_Error.errorString());
      return C_NOACT;
   }
   osc_write_log_error("Loading update package config (JSON)", "File not found: " + orc_Path);
   return C_RANGE;
}

int32_t C_SyvUpPacConfigFiler_New::h_SaveJson(const C_SyvUpPacConfig& orc_Config, const QString& orc_Path) {
   QFile c_File(orc_Path);
   if (c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
      c_File.write(QJsonDocument(orc_Config.ToJsonObject()).toJson(QJsonDocument::Indented));
      c_File.close();
      return C_NO_ERR;
   }
   osc_write_log_error("Saving update package config (JSON)", "Cannot write: " + orc_Path);
   return C_RD_WR;
}

int32_t C_SyvUpPacConfigFiler_New::h_LoadXml(C_SyvUpPacConfig& orc_Config, const QString& orc_Path) {
   QFile c_File(orc_Path);
   if (c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QDomDocument c_Doc;
      QString c_Error;
      int i_Line, i_Column;
      if (c_Doc.setContent(c_File.readAll(), &c_Error, &i_Line, &i_Column)) {
         orc_Config.FromQDomDocument(c_Doc.documentElement());
         c_File.close();
         return C_NO_ERR;
      }
      osc_write_log_error("Loading update package config (XML)", "Parse error: " + c_Error);
      return C_NOACT;
   }
   osc_write_log_error("Loading update package config (XML)", "File not found: " + orc_Path);
   return C_RANGE;
}

int32_t C_SyvUpPacConfigFiler_New::h_SaveXml(const C_SyvUpPacConfig& orc_Config, const QString& orc_Path) {
   QFile c_File(orc_Path);
   if (c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QDomDocument c_Doc;
      QDomElement c_Element = orc_Config.ToQDomDocument(c_Doc, "update-package-config");
      c_Doc.appendChild(c_Element);
      c_File.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + c_Doc.toString(3).toUtf8());
      c_File.close();
      return C_NO_ERR;
   }
   osc_write_log_error("Saving update package config (XML)", "Cannot write: " + orc_Path);
   return C_RD_WR;
}

int32_t C_SyvUpPacConfigFiler_New::mh_DetectAndLoad(C_SyvUpPacConfig& orc_Config, const QString& orc_Path) {
   if (!QFileInfo(orc_Path).exists()) {
      osc_write_log_error("Loading update package config", "File not found: " + orc_Path);
      return C_RANGE;
   }
   QString c_Lower = orc_Path.toLower();
   if (c_Lower.endsWith(".bin")) return h_LoadBinary(orc_Config, orc_Path);
   if (c_Lower.endsWith(".json")) return h_LoadJson(orc_Config, orc_Path);
   return h_LoadXml(orc_Config, orc_Path);
}
