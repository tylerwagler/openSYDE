//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for HALC configuration files (Multi-Format - Framework)

   Load / save HALC configuration data from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscHalcConfigFiler.hpp"
#include "C_OscFilerUtil.hpp"
#include <QFile>
#include <QFileInfo>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigFiler_New::h_LoadHalcConfigFile(C_OscHalcConfig &orc_Config,
                                                       const QString &orc_FilePath,
                                                       const QString &orc_BasePath) {
   Q_UNUSED(orc_BasePath); // Not used in new framework
   return mh_DetectAndLoad(orc_Config, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigFiler_New::h_SaveHalcConfigFile(const C_OscHalcConfig &orc_Config,
                                                       const QString &orc_FilePath,
                                                       const QString &orc_BasePath) {
   Q_UNUSED(orc_BasePath); // Not used in new framework

   QFileInfo c_FileInfo(orc_FilePath);
   const QString c_Extension = c_FileInfo.suffix().toLower();

   if (c_Extension == "bin") {
      return h_SaveBinary(orc_Config, orc_FilePath);
   } else if (c_Extension == "json") {
      return h_SaveJson(orc_Config, orc_FilePath);
   } else if (c_Extension == "xml") {
      return h_SaveXml(orc_Config, orc_FilePath);
   } else {
      return C_CONFIG; // Invalid file extension
   }
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigFiler_New::h_LoadBinary(C_OscHalcConfig &orc_Config,
                                               const QString &orc_FilePath) {
   QList<C_OscHalcConfig> c_List;
   int32_t s32_Retval = C_OscFilerUtil::h_LoadListBinary<C_OscHalcConfig>(c_List, orc_FilePath);
   if (s32_Retval == stw::errors::C_NO_ERR) {
      if (c_List.size() > 0) {
         orc_Config = c_List.first();
      }
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigFiler_New::h_SaveBinary(const C_OscHalcConfig &orc_Config,
                                               const QString &orc_FilePath) {
   QList<C_OscHalcConfig> c_List;
   c_List.append(orc_Config);
   return C_OscFilerUtil::h_SaveListBinary<C_OscHalcConfig>(c_List, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigFiler_New::h_LoadJson(C_OscHalcConfig &orc_Config,
                                             const QString &orc_FilePath) {
   QList<C_OscHalcConfig> c_List;
   int32_t s32_Retval = C_OscFilerUtil::h_LoadListJson<C_OscHalcConfig>(c_List, orc_FilePath);
   if (s32_Retval == stw::errors::C_NO_ERR) {
      if (c_List.size() > 0) {
         orc_Config = c_List.first();
      }
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigFiler_New::h_SaveJson(const C_OscHalcConfig &orc_Config,
                                             const QString &orc_FilePath) {
   QList<C_OscHalcConfig> c_List;
   c_List.append(orc_Config);
   return C_OscFilerUtil::h_SaveListJson<C_OscHalcConfig>(c_List, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigFiler_New::h_LoadXml(C_OscHalcConfig &orc_Config,
                                            const QString &orc_FilePath) {
   QList<C_OscHalcConfig> c_List;
   int32_t s32_Retval = C_OscFilerUtil::h_LoadListXml<C_OscHalcConfig>(c_List, orc_FilePath, "halcConfig", "config");
   if (s32_Retval == stw::errors::C_NO_ERR) {
      if (c_List.size() > 0) {
         orc_Config = c_List.first();
      }
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigFiler_New::h_SaveXml(const C_OscHalcConfig &orc_Config,
                                            const QString &orc_FilePath) {
   QList<C_OscHalcConfig> c_List;
   c_List.append(orc_Config);
   return C_OscFilerUtil::h_SaveListXml<C_OscHalcConfig>(c_List, orc_FilePath, "halcConfig", "config");
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcConfigFiler_New::mh_DetectAndLoad(C_OscHalcConfig &orc_Config,
                                                   const QString &orc_FilePath) {
   QFileInfo c_FileInfo(orc_FilePath);
   const QString c_Extension = c_FileInfo.suffix().toLower();

   if (c_Extension == "bin") {
      return h_LoadBinary(orc_Config, orc_FilePath);
   } else if (c_Extension == "json") {
      return h_LoadJson(orc_Config, orc_FilePath);
   } else if (c_Extension == "xml") {
      return h_LoadXml(orc_Config, orc_FilePath);
   } else {
      return C_CONFIG; // Invalid file extension
   }
}

//----------------------------------------------------------------------------------------------------------------------
[[deprecated("Use format-specific methods")]]
int32_t C_OscHalcConfigFiler_New::h_LoadFile(C_OscHalcConfig &orc_IoData,
                                             const QString &orc_Path,
                                             const QString &orc_BasePath) {
   return h_LoadHalcConfigFile(orc_IoData, orc_Path, orc_BasePath);
}

//----------------------------------------------------------------------------------------------------------------------
[[deprecated("Use format-specific methods")]]
int32_t C_OscHalcConfigFiler_New::h_SaveFile(const C_OscHalcConfig &orc_IoData,
                                             const QString &orc_Path,
                                             const QString &orc_BasePath,
                                             QStringList *const opc_CreatedFiles) {
   Q_UNUSED(orc_BasePath);
   Q_UNUSED(opc_CreatedFiles);
   return h_SaveHalcConfigFile(orc_IoData, orc_Path, orc_BasePath);
}
