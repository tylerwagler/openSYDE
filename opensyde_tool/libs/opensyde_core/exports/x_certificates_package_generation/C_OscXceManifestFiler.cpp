//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for XCE manifest files (Multi-Format - Framework)

   Load / save XCE manifest from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscXceManifestFiler.hpp"
#include "C_OscFilerUtil.hpp"
#include <QFile>
#include <QFileInfo>

using namespace stw::opensyde_core;
using namespace stw::errors;

// Define file name constant
const QString C_OscXceManifestFiler::hc_FILE_NAME = "manifest.syde_pkg";

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler::h_LoadXceManifestFile(C_OscXceManifest &orc_Manifest,
                                                         const QString &orc_FilePath,
                                                         const QString &orc_BasePath) {
   Q_UNUSED(orc_BasePath); // Not used in new framework
   return mh_DetectAndLoad(orc_Manifest, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler::h_SaveXceManifestFile(const C_OscXceManifest &orc_Manifest,
                                                         const QString &orc_FilePath,
                                                         const QString &orc_BasePath) {
   Q_UNUSED(orc_BasePath); // Not used in new framework

   QFileInfo c_FileInfo(orc_FilePath);
   const QString c_Extension = c_FileInfo.suffix().toLower();

   if (c_Extension == "bin") {
      return h_SaveBinary(orc_Manifest, orc_FilePath);
   } else if (c_Extension == "json") {
      return h_SaveJson(orc_Manifest, orc_FilePath);
   } else if (c_Extension == "xml") {
      return h_SaveXml(orc_Manifest, orc_FilePath);
   } else {
      return C_CONFIG; // Invalid file extension
   }
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler::h_LoadBinary(C_OscXceManifest &orc_Manifest,
                                                const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadSingleBinary<C_OscXceManifest>(orc_Manifest, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler::h_SaveBinary(const C_OscXceManifest &orc_Manifest,
                                                const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveSingleBinary<C_OscXceManifest>(orc_Manifest, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler::h_LoadJson(C_OscXceManifest &orc_Manifest,
                                              const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadSingleJson<C_OscXceManifest>(orc_Manifest, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler::h_SaveJson(const C_OscXceManifest &orc_Manifest,
                                              const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveSingleJson<C_OscXceManifest>(orc_Manifest, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler::h_LoadXml(C_OscXceManifest &orc_Manifest,
                                             const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadSingleXml<C_OscXceManifest>(orc_Manifest, orc_FilePath, "xceManifest");
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler::h_SaveXml(const C_OscXceManifest &orc_Manifest,
                                             const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveSingleXml<C_OscXceManifest>(orc_Manifest, orc_FilePath, "xceManifest");
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifestFiler::mh_DetectAndLoad(C_OscXceManifest &orc_Manifest,
                                                    const QString &orc_FilePath) {
   QFileInfo c_FileInfo(orc_FilePath);
   const QString c_Extension = c_FileInfo.suffix().toLower();

   if (c_Extension == "bin") {
      return h_LoadBinary(orc_Manifest, orc_FilePath);
   } else if (c_Extension == "json") {
      return h_LoadJson(orc_Manifest, orc_FilePath);
   } else if (c_Extension == "xml") {
      return h_LoadXml(orc_Manifest, orc_FilePath);
   } else {
      return C_CONFIG; // Invalid file extension
   }
}

//----------------------------------------------------------------------------------------------------------------------
[[deprecated("Use format-specific methods")]]
int32_t C_OscXceManifestFiler::h_LoadFile(C_OscXceManifest &orc_Manifest,
                                              const QString &orc_Path) {
   return h_LoadXceManifestFile(orc_Manifest, orc_Path, QString());
}

//----------------------------------------------------------------------------------------------------------------------
[[deprecated("Use format-specific methods")]]
int32_t C_OscXceManifestFiler::h_SaveFile(const C_OscXceManifest &orc_Manifest,
                                              const QString &orc_Path) {
   return h_SaveXceManifestFile(orc_Manifest, orc_Path, QString());
}
