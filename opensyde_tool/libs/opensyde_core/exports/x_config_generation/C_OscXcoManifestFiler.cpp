//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for XCO manifest files (Multi-Format - Framework)

   Load / save XCO manifest from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscXcoManifestFiler.hpp"
#include "C_OscFilerUtil.hpp"
#include <QFile>
#include <QFileInfo>

using namespace stw::opensyde_core;
using namespace stw::errors;

// Define file name constant
const QString C_OscXcoManifestFiler_New::hc_FILE_NAME = "manifest.syde_cfg";

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXcoManifestFiler_New::h_LoadXcoManifestFile(QList<C_OscXcoManifest> &orc_Manifests,
                                                         const QString &orc_FilePath,
                                                         const QString &orc_BasePath) {
   Q_UNUSED(orc_BasePath); // Not used in new framework
   return mh_DetectAndLoad(orc_Manifests, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXcoManifestFiler_New::h_SaveXcoManifestFile(const QList<C_OscXcoManifest> &orc_Manifests,
                                                         const QString &orc_FilePath,
                                                         const QString &orc_BasePath) {
   Q_UNUSED(orc_BasePath); // Not used in new framework

   QFileInfo c_FileInfo(orc_FilePath);
   const QString c_Extension = c_FileInfo.suffix().toLower();

   if (c_Extension == "bin") {
      return h_SaveBinary(orc_Manifests, orc_FilePath);
   } else if (c_Extension == "json") {
      return h_SaveJson(orc_Manifests, orc_FilePath);
   } else if (c_Extension == "xml") {
      return h_SaveXml(orc_Manifests, orc_FilePath);
   } else {
      return C_CONFIG; // Invalid file extension
   }
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXcoManifestFiler_New::h_LoadBinary(QList<C_OscXcoManifest> &orc_Manifests,
                                                const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadListBinary<C_OscXcoManifest>(orc_Manifests, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXcoManifestFiler_New::h_SaveBinary(const QList<C_OscXcoManifest> &orc_Manifests,
                                                const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveListBinary<C_OscXcoManifest>(orc_Manifests, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXcoManifestFiler_New::h_LoadJson(QList<C_OscXcoManifest> &orc_Manifests,
                                              const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadListJson<C_OscXcoManifest>(orc_Manifests, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXcoManifestFiler_New::h_SaveJson(const QList<C_OscXcoManifest> &orc_Manifests,
                                              const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveListJson<C_OscXcoManifest>(orc_Manifests, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXcoManifestFiler_New::h_LoadXml(QList<C_OscXcoManifest> &orc_Manifests,
                                             const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadListXml<C_OscXcoManifest>(orc_Manifests, orc_FilePath, "xcoManifest", "manifest");
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXcoManifestFiler_New::h_SaveXml(const QList<C_OscXcoManifest> &orc_Manifests,
                                             const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveListXml<C_OscXcoManifest>(orc_Manifests, orc_FilePath, "xcoManifest", "manifest");
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXcoManifestFiler_New::mh_DetectAndLoad(QList<C_OscXcoManifest> &orc_Manifests,
                                                    const QString &orc_FilePath) {
   QFileInfo c_FileInfo(orc_FilePath);
   const QString c_Extension = c_FileInfo.suffix().toLower();

   if (c_Extension == "bin") {
      return h_LoadBinary(orc_Manifests, orc_FilePath);
   } else if (c_Extension == "json") {
      return h_LoadJson(orc_Manifests, orc_FilePath);
   } else if (c_Extension == "xml") {
      return h_LoadXml(orc_Manifests, orc_FilePath);
   } else {
      return C_CONFIG; // Invalid file extension
   }
}

//----------------------------------------------------------------------------------------------------------------------
[[deprecated("Use format-specific methods")]]
int32_t C_OscXcoManifestFiler_New::h_LoadFile(QList<C_OscXcoManifest> &orc_Manifests,
                                              const QString &orc_Path) {
   return h_LoadXcoManifestFile(orc_Manifests, orc_Path, QString());
}

//----------------------------------------------------------------------------------------------------------------------
[[deprecated("Use format-specific methods")]]
int32_t C_OscXcoManifestFiler_New::h_SaveFile(const QList<C_OscXcoManifest> &orc_Manifests,
                                              const QString &orc_Path) {
   return h_SaveXcoManifestFile(orc_Manifests, orc_Path, QString());
}
