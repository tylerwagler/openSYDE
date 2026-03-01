//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Migrated Filer for XApp properties using Qt-native serialization
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include <QFileInfo>
#include <QDomDocument>

#include "C_OscLoggingHandler.hpp"
#include "C_OscXappPropertiesFiler_New.hpp"
#include "C_OscXmlParserLog.hpp"
#include "C_OscSerializer.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"

using namespace stw::errors;
using namespace stw::opensyde_core;

const QString C_OscXappPropertiesFiler_New::hc_FILE_NAME = "xapp_properties.ini";

int32_t C_OscXappPropertiesFiler_New::h_LoadFile(C_OscXappProperties& orc_Config, const QString& orc_Path) {
   if (!QFileInfo(orc_Path).exists()) return C_RANGE;
   
   stw::opensyde_core::E_SerializationFormat e_Format = stw::opensyde_core::C_OscSerializer::DetectFormat(orc_Path);
   
   switch (e_Format) {
      case stw::opensyde_core::E_SerializationFormat::e_BINARY: {
         QByteArray c_Data;
         if (stw::opensyde_core::C_OscSerializer::LoadBinary(c_Data, orc_Path) != C_NO_ERR) return C_RD_WR;
         return h_LoadBinary(orc_Config, c_Data);
      }
      case stw::opensyde_core::E_SerializationFormat::e_JSON: {
         QJsonObject c_Object;
         if (stw::opensyde_core::C_OscSerializer::LoadJson(c_Object, orc_Path) != C_NO_ERR) return C_RD_WR;
         return h_LoadJson(orc_Config, c_Object);
      }
      case stw::opensyde_core::E_SerializationFormat::e_XML: {
         QDomDocument c_Doc;
         if (stw::opensyde_core::C_OscSerializer::LoadXml(c_Doc, orc_Path) != C_NO_ERR) return C_RD_WR;
         return h_LoadXml(orc_Config, c_Doc.documentElement());
      }
      default:
         return C_CONFIG;
   }
}

int32_t C_OscXappPropertiesFiler_New::h_SaveFile(const C_OscXappProperties& orc_Config, const QString& orc_Path) {
   stw::opensyde_core::E_SerializationFormat e_Format = stw::opensyde_core::C_OscSerializer::DetectFormat(orc_Path);
   
   switch (e_Format) {
      case stw::opensyde_core::E_SerializationFormat::e_BINARY:
         return stw::opensyde_core::C_OscSerializer::SaveBinary(h_SaveBinary(orc_Config), orc_Path);
      case stw::opensyde_core::E_SerializationFormat::e_JSON:
         return stw::opensyde_core::C_OscSerializer::SaveJson(h_SaveJson(orc_Config), orc_Path);
      case stw::opensyde_core::E_SerializationFormat::e_XML: {
         QDomDocument c_Doc;
         QDomElement c_Element = h_SaveXml(orc_Config, c_Doc);
         QDomElement c_Root = c_Doc.createElement("xapp-properties-root");
         c_Root.appendChild(c_Element);
         c_Doc.appendChild(c_Root);
         
         QFile c_File(orc_Path);
         if (!c_File.open(QIODevice::WriteOnly | QIODevice::Text)) return C_CONFIG;
         QTextStream c_Stream(&c_File);
         c_Stream.setEncoding(QStringConverter::Utf8);
         c_Stream << c_Doc.toString(2);
         c_File.close();
         return C_NO_ERR;
      }
      default:
         return C_CONFIG;
   }
}

int32_t C_OscXappPropertiesFiler_New::h_LoadBinary(C_OscXappProperties& orc_Config, const QByteArray& orc_Data) {
   QDataStream c_Stream(orc_Data);
   c_Stream.setVersion(QDataStream::Qt_6_0);
   return orc_Config.FromQDataStream(c_Stream);
}

QByteArray C_OscXappPropertiesFiler_New::h_SaveBinary(const C_OscXappProperties& orc_Config) {
   QByteArray c_Data;
   QDataStream c_Stream(&c_Data, QIODevice::WriteOnly);
   c_Stream.setVersion(QDataStream::Qt_6_0);
   orc_Config.ToQDataStream(c_Stream);
   return c_Data;
}

int32_t C_OscXappPropertiesFiler_New::h_LoadJson(C_OscXappProperties& orc_Config, const QJsonObject& orc_Object) {
   return orc_Config.FromJsonObject(orc_Object);
}

QJsonObject C_OscXappPropertiesFiler_New::h_SaveJson(const C_OscXappProperties& orc_Config) {
   return orc_Config.ToJsonObject();
}

int32_t C_OscXappPropertiesFiler_New::h_LoadXml(C_OscXappProperties& orc_Config, const QDomElement& orc_Element) {
   return orc_Config.FromQDomElement(orc_Element);
}

QDomElement C_OscXappPropertiesFiler_New::h_SaveXml(const C_OscXappProperties& orc_Config, QDomDocument& orc_Doc) {
   return orc_Config.ToQDomDocument(orc_Doc, "xapp-properties");
}

int32_t C_OscXappPropertiesFiler_New::h_LoadData(C_OscXappProperties& orc_Config, C_OscXmlParserBase& orc_XmlParser) {
   // Simplified legacy implementation
   orc_Config.u32_PollingIntervalMs = 10U;
   orc_Config.u32_DataRequestIntervalMs = 100U;
   orc_Config.e_ConnectedInterfaceType = C_OscSystemBus::eCAN;
   orc_Config.u8_ConnectedInterfaceNumber = 0U;
   return C_NO_ERR;
}

void C_OscXappPropertiesFiler_New::h_SaveData(const C_OscXappProperties& orc_Config, C_OscXmlParserBase& orc_XmlParser) {
   // Legacy API - simplified
   Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("xapp-properties") == "xapp-properties");
   orc_XmlParser.SelectNodeParent();
}
