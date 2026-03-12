//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for parameter set raw node files (Multi-Format - Framework)

   Load / save parameter set raw node from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include "C_OscParamSetRawNodeFiler_New.hpp"
#include "C_OscFilerUtil.hpp"
#include <QFile>
#include <QFileInfo>

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler_New::h_LoadParamSetRawNodeFile(C_OscParamSetRawNode &orc_Node,
                                                                 const QString &orc_FilePath,
                                                                 const QString &orc_BasePath) {
   Q_UNUSED(orc_BasePath); // Not used in new framework
   return mh_DetectAndLoad(orc_Node, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler_New::h_SaveParamSetRawNodeFile(const C_OscParamSetRawNode &orc_Node,
                                                                 const QString &orc_FilePath,
                                                                 const QString &orc_BasePath) {
   Q_UNUSED(orc_BasePath); // Not used in new framework

   QFileInfo c_FileInfo(orc_FilePath);
   const QString c_Extension = c_FileInfo.suffix().toLower();

   if (c_Extension == "bin") {
      return h_SaveBinary(orc_Node, orc_FilePath);
   } else if (c_Extension == "json") {
      return h_SaveJson(orc_Node, orc_FilePath);
   } else if (c_Extension == "xml") {
      return h_SaveXml(orc_Node, orc_FilePath);
   } else {
      return C_CONFIG; // Invalid file extension
   }
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler_New::h_LoadBinary(C_OscParamSetRawNode &orc_Node,
                                                    const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadBinary<C_OscParamSetRawNode>(orc_Node, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler_New::h_SaveBinary(const C_OscParamSetRawNode &orc_Node,
                                                    const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveBinary<C_OscParamSetRawNode>(orc_Node, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler_New::h_LoadJson(C_OscParamSetRawNode &orc_Node,
                                                  const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadJson<C_OscParamSetRawNode>(orc_Node, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler_New::h_SaveJson(const C_OscParamSetRawNode &orc_Node,
                                                  const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveJson<C_OscParamSetRawNode>(orc_Node, orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler_New::h_LoadXml(C_OscParamSetRawNode &orc_Node,
                                                 const QString &orc_FilePath) {
   return C_OscFilerUtil::h_LoadXml<C_OscParamSetRawNode>(orc_Node, orc_FilePath, "paramSetRawNode");
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler_New::h_SaveXml(const C_OscParamSetRawNode &orc_Node,
                                                 const QString &orc_FilePath) {
   return C_OscFilerUtil::h_SaveXml<C_OscParamSetRawNode>(orc_Node, orc_FilePath, "paramSetRawNode");
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawNodeFiler_New::mh_DetectAndLoad(C_OscParamSetRawNode &orc_Node,
                                                        const QString &orc_FilePath) {
   QFileInfo c_FileInfo(orc_FilePath);
   const QString c_Extension = c_FileInfo.suffix().toLower();

   if (c_Extension == "bin") {
      return h_LoadBinary(orc_Node, orc_FilePath);
   } else if (c_Extension == "json") {
      return h_LoadJson(orc_Node, orc_FilePath);
   } else if (c_Extension == "xml") {
      return h_LoadXml(orc_Node, orc_FilePath);
   } else {
      return C_CONFIG; // Invalid file extension
   }
}

//----------------------------------------------------------------------------------------------------------------------
[[deprecated("Use format-specific methods")]]
int32_t C_OscParamSetRawNodeFiler_New::h_LoadFile(C_OscParamSetRawNode &orc_IoData,
                                                  const QString &orc_Path,
                                                  const QString &orc_BasePath) {
   Q_UNUSED(orc_BasePath); // Not used in new framework
   return h_LoadParamSetRawNodeFile(orc_IoData, orc_Path, QString());
}

//----------------------------------------------------------------------------------------------------------------------
[[deprecated("Use format-specific methods")]]
int32_t C_OscParamSetRawNodeFiler_New::h_SaveFile(const C_OscParamSetRawNode &orc_IoData,
                                                  const QString &orc_Path,
                                                  const QString &orc_BasePath,
                                                  QStringList *const opc_CreatedFiles) {
   Q_UNUSED(orc_BasePath); // Not used in new framework
   Q_UNUSED(opc_CreatedFiles); // Not used in new framework
   return h_SaveParamSetRawNodeFile(orc_IoData, orc_Path, QString());
}
