//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for XAPP properties files (Multi-Format - Framework)

   Load / save XAPP properties from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscXappPropertiesFiler.hpp"
#include "C_OscFilerUtil.hpp"
#include <QFile>
#include <QFileInfo>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXappPropertiesFiler::h_LoadXappPropertiesFile(QList<C_OscXappProperties> &orc_Properties,
                                                               const QString &orc_FilePath,
                                                               const QString &orc_BasePath) {
   Q_UNUSED(orc_BasePath); // Not used in new framework
   return mh_DetectAndLoad(orc_Properties, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXappPropertiesFiler::h_SaveXappPropertiesFile(const QList<C_OscXappProperties> &orc_Properties,
                                                               const QString &orc_FilePath,
                                                               const QString &orc_BasePath) {
   Q_UNUSED(orc_BasePath); // Not used in new framework

   QFileInfo c_FileInfo(orc_FilePath);
   const QString c_Extension = c_FileInfo.suffix().toLower();

   if (c_Extension == "bin") {
      return h_SaveBinary(orc_Properties, orc_FilePath);
   } else if (c_Extension == "json") {
      return h_SaveJson(orc_Properties, orc_FilePath);
   } else if (c_Extension == "xml") {
      return h_SaveXml(orc_Properties, orc_FilePath);
   } else {
      return C_CONFIG; // Invalid file extension
   }
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXappPropertiesFiler::h_LoadBinary(QList<C_OscXappProperties> &orc_Properties,
                                                   const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadListBinary<C_OscXappProperties>(orc_Properties, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXappPropertiesFiler::h_SaveBinary(const QList<C_OscXappProperties> &orc_Properties,
                                                   const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveListBinary<C_OscXappProperties>(orc_Properties, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXappPropertiesFiler::h_LoadJson(QList<C_OscXappProperties> &orc_Properties,
                                                 const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadListJson<C_OscXappProperties>(orc_Properties, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXappPropertiesFiler::h_SaveJson(const QList<C_OscXappProperties> &orc_Properties,
                                                 const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveListJson<C_OscXappProperties>(orc_Properties, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXappPropertiesFiler::h_LoadXml(QList<C_OscXappProperties> &orc_Properties,
                                                const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadListXml<C_OscXappProperties>(orc_Properties, orc_FilePath, "xappProperties", "property");
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXappPropertiesFiler::h_SaveXml(const QList<C_OscXappProperties> &orc_Properties,
                                                const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveListXml<C_OscXappProperties>(orc_Properties, orc_FilePath, "xappProperties", "property");
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXappPropertiesFiler::mh_DetectAndLoad(QList<C_OscXappProperties> &orc_Properties,
                                                       const QString &orc_FilePath) {
   QFileInfo c_FileInfo(orc_FilePath);
   const QString c_Extension = c_FileInfo.suffix().toLower();

   if (c_Extension == "bin") {
      return h_LoadBinary(orc_Properties, orc_FilePath);
   } else if (c_Extension == "json") {
      return h_LoadJson(orc_Properties, orc_FilePath);
   } else if (c_Extension == "xml") {
      return h_LoadXml(orc_Properties, orc_FilePath);
   } else {
      return C_CONFIG; // Invalid file extension
   }
}

//----------------------------------------------------------------------------------------------------------------------
[[deprecated("Use format-specific methods")]]
int32_t C_OscXappPropertiesFiler::h_LoadFile(QList<C_OscXappProperties> &orc_Properties,
                                                 const QString &orc_Path) {
   return h_LoadXappPropertiesFile(orc_Properties, orc_Path, QString());
}

//----------------------------------------------------------------------------------------------------------------------
[[deprecated("Use format-specific methods")]]
int32_t C_OscXappPropertiesFiler::h_SaveFile(const QList<C_OscXappProperties> &orc_Properties,
                                                 const QString &orc_Path) {
   return h_SaveXappPropertiesFile(orc_Properties, orc_Path, QString());
}
