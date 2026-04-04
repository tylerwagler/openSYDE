//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for view data files (Multi-Format - Framework)

   Load / save view data from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscViewDataFiler.hpp"
#include "C_OscFilerUtil.hpp"
#include <QFile>
#include <QFileInfo>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_LoadViewDataFile(QList<C_OscViewData> &orc_Views,
                                                   const QString &orc_FilePath,
                                                   const QString &orc_BasePath) {
   Q_UNUSED(orc_BasePath); // Not used in new framework
   return mh_DetectAndLoad(orc_Views, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_SaveViewDataFile(const QList<C_OscViewData> &orc_Views,
                                                   const QString &orc_FilePath,
                                                   const QString &orc_BasePath) {
   Q_UNUSED(orc_BasePath); // Not used in new framework

   QFileInfo c_FileInfo(orc_FilePath);
   const QString c_Extension = c_FileInfo.suffix().toLower();

   if (c_Extension == "bin") {
      return h_SaveBinary(orc_Views, orc_FilePath);
   } else if (c_Extension == "json") {
      return h_SaveJson(orc_Views, orc_FilePath);
   } else if (c_Extension == "xml") {
      return h_SaveXml(orc_Views, orc_FilePath);
   } else {
      return C_CONFIG; // Invalid file extension
   }
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_LoadBinary(QList<C_OscViewData> &orc_Views,
                                             const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadListBinary<C_OscViewData>(orc_Views, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_SaveBinary(const QList<C_OscViewData> &orc_Views,
                                             const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveListBinary<C_OscViewData>(orc_Views, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_LoadJson(QList<C_OscViewData> &orc_Views,
                                           const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadListJson<C_OscViewData>(orc_Views, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_SaveJson(const QList<C_OscViewData> &orc_Views,
                                           const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveListJson<C_OscViewData>(orc_Views, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_LoadXml(QList<C_OscViewData> &orc_Views,
                                          const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadListXml<C_OscViewData>(orc_Views, orc_FilePath, "viewData", "view");
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_SaveXml(const QList<C_OscViewData> &orc_Views,
                                          const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveListXml<C_OscViewData>(orc_Views, orc_FilePath, "viewData", "view");
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::mh_DetectAndLoad(QList<C_OscViewData> &orc_Views,
                                                 const QString &orc_FilePath) {
   QFileInfo c_FileInfo(orc_FilePath);
   const QString c_Extension = c_FileInfo.suffix().toLower();

   if (c_Extension == "bin") {
      return h_LoadBinary(orc_Views, orc_FilePath);
   } else if (c_Extension == "json") {
      return h_LoadJson(orc_Views, orc_FilePath);
   } else if (c_Extension == "xml") {
      return h_LoadXml(orc_Views, orc_FilePath);
   } else {
      return C_CONFIG; // Invalid file extension
   }
}

//----------------------------------------------------------------------------------------------------------------------
[[deprecated("Use format-specific methods")]]
int32_t C_OscViewDataFiler::h_LoadFile(QList<C_OscViewData> &orc_Views,
                                           const QString &orc_Path) {
   return h_LoadViewDataFile(orc_Views, orc_Path, QString());
}

//----------------------------------------------------------------------------------------------------------------------
[[deprecated("Use format-specific methods")]]
int32_t C_OscViewDataFiler::h_SaveFile(const QList<C_OscViewData> &orc_Views,
                                           const QString &orc_Path) {
   return h_SaveViewDataFile(orc_Views, orc_Path, QString());
}
