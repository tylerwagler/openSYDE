//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       HALC configuration standalone reader/writer (Multi-Format)

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "precomp_headers.hpp"
#include <QFileInfo>
#include <QJsonDocument>

#include "C_OscLoggingHandler.hpp"
#include "C_OscHalcConfigStandaloneFiler_New.hpp"
#include "stwerrors.hpp"

using namespace stw::errors;
using namespace stw::opensyde_core;

int32_t C_OscHalcConfigStandaloneFiler_New::h_LoadFile(
   C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path)
{
   return mh_DetectAndLoad(orc_Config, orc_Path);
}

int32_t C_OscHalcConfigStandaloneFiler_New::h_SaveFile(
   const C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QString c_LowerPath = orc_Path.toLower();

   if (c_LowerPath.endsWith(".bin")) {
      s32_Result = h_SaveBinary(orc_Config, orc_Path);
   }
   else if (c_LowerPath.endsWith(".json")) {
      s32_Result = h_SaveJson(orc_Config, orc_Path);
   }
   else {
      s32_Result = h_SaveXml(orc_Config, orc_Path);
   }
   return s32_Result;
}

int32_t C_OscHalcConfigStandaloneFiler_New::h_LoadBinary(
   C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QFile c_File(orc_Path);

   if (c_File.open(QIODevice::ReadOnly)) {
      QDataStream c_Stream(&c_File);
      c_Stream.setVersion(QDataStream::Qt_6_0);
      orc_Config.FromQDataStream(c_Stream);
      c_File.close();
   }
   else {
      osc_write_log_error("Loading HALC config standalone (binary)",
         "File \"" + orc_Path + "\" could not be opened.");
      s32_Result = C_RANGE;
   }
   return s32_Result;
}

int32_t C_OscHalcConfigStandaloneFiler_New::h_SaveBinary(
   const C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QFile c_File(orc_Path);

   if (c_File.open(QIODevice::WriteOnly)) {
      QDataStream c_Stream(&c_File);
      c_Stream.setVersion(QDataStream::Qt_6_0);
      orc_Config.ToQDataStream(c_Stream);
      c_File.close();
   }
   else {
      osc_write_log_error("Saving HALC config standalone (binary)",
         "File \"" + orc_Path + "\" could not be opened.");
      s32_Result = C_RD_WR;
   }
   return s32_Result;
}

int32_t C_OscHalcConfigStandaloneFiler_New::h_LoadFromMemoryBinary(
   C_OscHalcConfigStandalone &orc_Config, const QByteArray &orc_Data)
{
   QDataStream c_Stream(orc_Data);
   c_Stream.setVersion(QDataStream::Qt_6_0);
   return orc_Config.FromQDataStream(c_Stream);
}

QByteArray C_OscHalcConfigStandaloneFiler_New::h_SaveToMemoryBinary(
   const C_OscHalcConfigStandalone &orc_Config)
{
   QByteArray c_Data;
   QDataStream c_Stream(&c_Data, QIODevice::WriteOnly);
   c_Stream.setVersion(QDataStream::Qt_6_0);
   orc_Config.ToQDataStream(c_Stream);
   return c_Data;
}

int32_t C_OscHalcConfigStandaloneFiler_New::h_LoadJson(
   C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QFile c_File(orc_Path);

   if (c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QJsonParseError c_Error;
      QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll(), &c_Error);
      c_File.close();

      if (c_Error.error == QJsonParseError::NoError && c_Doc.isObject()) {
         orc_Config.FromJsonObject(c_Doc.object());
      }
      else {
         osc_write_log_error("Loading HALC config standalone (JSON)",
            "JSON parse error: " + c_Error.errorString());
         s32_Result = C_NOACT;
      }
   }
   else {
      osc_write_log_error("Loading HALC config standalone (JSON)",
         "File \"" + orc_Path + "\" could not be opened.");
      s32_Result = C_RANGE;
   }
   return s32_Result;
}

int32_t C_OscHalcConfigStandaloneFiler_New::h_SaveJson(
   const C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QFile c_File(orc_Path);

   if (c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QJsonObject c_Obj = orc_Config.ToJsonObject();
      c_File.write(QJsonDocument(c_Obj).toJson(QJsonDocument::Indented));
      c_File.close();
   }
   else {
      osc_write_log_error("Saving HALC config standalone (JSON)",
         "File \"" + orc_Path + "\" could not be opened.");
      s32_Result = C_RD_WR;
   }
   return s32_Result;
}

int32_t C_OscHalcConfigStandaloneFiler_New::h_LoadFromMemoryJson(
   C_OscHalcConfigStandalone &orc_Config, const QJsonObject &orc_Object)
{
   return orc_Config.FromJsonObject(orc_Object);
}

QJsonObject C_OscHalcConfigStandaloneFiler_New::h_SaveToMemoryJson(
   const C_OscHalcConfigStandalone &orc_Config)
{
   return orc_Config.ToJsonObject();
}

int32_t C_OscHalcConfigStandaloneFiler_New::h_LoadXml(
   C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QFile c_File(orc_Path);

   if (c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QDomDocument c_Doc;
      QString c_Error;
      int i_Line, i_Column;

      if (c_Doc.setContent(c_File.readAll(), &c_Error, &i_Line, &i_Column)) {
         orc_Config.FromQDomDocument(c_Doc.documentElement());
      }
      else {
         osc_write_log_error("Loading HALC config standalone (XML)",
            "XML parse error at line " + QString::number(i_Line) + ": " + c_Error);
         s32_Result = C_NOACT;
      }
      c_File.close();
   }
   else {
      osc_write_log_error("Loading HALC config standalone (XML)",
         "File \"" + orc_Path + "\" could not be opened.");
      s32_Result = C_RANGE;
   }
   return s32_Result;
}

int32_t C_OscHalcConfigStandaloneFiler_New::h_SaveXml(
   const C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QFile c_File(orc_Path);

   if (c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QDomDocument c_Doc;
      QDomElement c_Element = orc_Config.ToQDomDocument(c_Doc, "halc-config-standalone");
      c_Doc.appendChild(c_Element);
      c_File.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                   + c_Doc.toString(3).toUtf8());
      c_File.close();
   }
   else {
      osc_write_log_error("Saving HALC config standalone (XML)",
         "File \"" + orc_Path + "\" could not be opened.");
      s32_Result = C_RD_WR;
   }
   return s32_Result;
}

int32_t C_OscHalcConfigStandaloneFiler_New::h_LoadFromMemoryXml(
   C_OscHalcConfigStandalone &orc_Config, const QDomElement &orc_Element)
{
   return orc_Config.FromQDomDocument(orc_Element);
}

QDomElement C_OscHalcConfigStandaloneFiler_New::h_SaveToMemoryXml(
   const C_OscHalcConfigStandalone &orc_Config, QDomDocument &orc_Doc)
{
   return orc_Config.ToQDomDocument(orc_Doc, "halc-config-standalone");
}

int32_t C_OscHalcConfigStandaloneFiler_New::mh_DetectAndLoad(
   C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path)
{
   int32_t s32_Result = C_NO_ERR;
   QString c_LowerPath = orc_Path.toLower();

   if (QFileInfo(orc_Path).exists() && QFileInfo(orc_Path).isFile()) {
      if (c_LowerPath.endsWith(".bin")) {
         s32_Result = h_LoadBinary(orc_Config, orc_Path);
      }
      else if (c_LowerPath.endsWith(".json")) {
         s32_Result = h_LoadJson(orc_Config, orc_Path);
      }
      else {
         s32_Result = h_LoadXml(orc_Config, orc_Path);
      }
   }
   else {
      osc_write_log_error("Loading HALC config standalone",
         "File \"" + orc_Path + "\" does not exist.");
      s32_Result = C_RANGE;
   }
   return s32_Result;
}
