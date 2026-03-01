//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for C_OscViewData with Qt-native serialization
   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscViewDataFiler.hpp"
#include "C_OscViewData.hpp"
#include "stwtypes.hpp"
#include "stwerrors.hpp"

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
/*! \brief  Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscViewDataFiler::C_OscViewDataFiler() {
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from file with auto-detection of format based on extension

   \param[out] orc_Views List of views to load into
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_LoadFile(QList<C_OscViewData> &orc_Views,
                                        const QString &orc_Path) {
   using namespace stw::errors;
   
   // Detect format based on extension
   QString c_LowerPath = orc_Path.toLower();
   if (c_LowerPath.endsWith(".bin")) {
      return h_LoadBinary(orc_Views, orc_Path);
   } else if (c_LowerPath.endsWith(".json")) {
      return h_LoadJson(orc_Views, orc_Path);
   } else {
      // Default to XML
      return h_LoadXml(orc_Views, orc_Path);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to file with auto-detection of format based on extension

   \param[in] orc_Views List of views to save
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_SaveFile(const QList<C_OscViewData> &orc_Views,
                                        const QString &orc_Path) {
   using namespace stw::errors;
   
   // Detect format based on extension
   QString c_LowerPath = orc_Path.toLower();
   if (c_LowerPath.endsWith(".bin")) {
      return h_SaveBinary(orc_Views, orc_Path);
   } else if (c_LowerPath.endsWith(".json")) {
      return h_SaveJson(orc_Views, orc_Path);
   } else {
      // Default to XML
      return h_SaveXml(orc_Views, orc_Path);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from binary file

   \param[out] orc_Views List of views to load into
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_LoadBinary(QList<C_OscViewData> &orc_Views,
                                          const QString &orc_Path) {
   using namespace stw::errors;
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }
   
   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_5_15);
   
   // Read version
   uint16_t u16_Version = 0;
   c_Stream >> u16_Version;
   
   if (u16_Version != 1) {
      return C_CONFIG;
   }
   
   // Read count
   uint32_t u32_Count = 0;
   c_Stream >> u32_Count;
   
   orc_Views.clear();
   for (uint32_t i = 0; i < u32_Count; ++i) {
      C_OscViewData c_View;
      c_View.FromQDataStream(c_Stream);
      orc_Views.append(c_View);
   }
   
   if (c_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to binary file

   \param[in] orc_Views List of views to save
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_SaveBinary(const QList<C_OscViewData> &orc_Views,
                                          const QString &orc_Path) {
   using namespace stw::errors;
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }
   
   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_5_15);
   
   // Write version
   uint16_t u16_Version = 1;
   c_Stream << u16_Version;
   
   // Write count
   uint32_t u32_Count = static_cast<uint32_t>(orc_Views.size());
   c_Stream << u32_Count;
   
   // Write each view
   for (const C_OscViewData& c_View : orc_Views) {
      c_View.ToQDataStream(c_Stream);
   }
   
   if (c_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from JSON file

   \param[out] orc_Views List of views to load into
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_LoadJson(QList<C_OscViewData> &orc_Views,
                                        const QString &orc_Path) {
   using namespace stw::errors;
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }
   
   QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll());
   if (c_Doc.isNull() || !c_Doc.isArray()) {
      return C_CONFIG;
   }
   
   QJsonArray c_Array = c_Doc.array();
   orc_Views.clear();
   
   for (const QJsonValue& c_Value : c_Array) {
      if (c_Value.isObject()) {
         C_OscViewData c_View;
         c_View.FromJsonObject(c_Value.toObject());
         orc_Views.append(c_View);
      }
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to JSON file

   \param[in] orc_Views List of views to save
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_SaveJson(const QList<C_OscViewData> &orc_Views,
                                        const QString &orc_Path) {
   using namespace stw::errors;
   
   QJsonArray c_Array;
   for (const C_OscViewData& c_View : orc_Views) {
      c_Array.append(c_View.ToJsonObject());
   }
   
   QJsonDocument c_Doc(c_Array);
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }
   
   c_File.write(c_Doc.toJson(QJsonDocument::Compact));
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from XML file

   \param[out] orc_Views List of views to load into
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_LoadXml(QList<C_OscViewData> &orc_Views,
                                       const QString &orc_Path) {
   using namespace stw::errors;
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }
   
   QDomDocument c_Doc;
   if (!c_Doc.setContent(&c_File)) {
      return C_CONFIG;
   }
   
   QDomElement c_RootElement = c_Doc.documentElement();
   if (c_RootElement.tagName() != "views") {
      return C_CONFIG;
   }
   
   orc_Views.clear();
   QDomNode c_Node = c_RootElement.firstChild();
   
   while (!c_Node.isNull()) {
      if (c_Node.isElement() && c_Node.toElement().tagName() == "view") {
         C_OscViewData c_View;
         c_View.FromQDomElement(c_Node.toElement());
         orc_Views.append(c_View);
      }
      c_Node = c_Node.nextSibling();
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to XML file

   \param[in] orc_Views List of views to save
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewDataFiler::h_SaveXml(const QList<C_OscViewData> &orc_Views,
                                       const QString &orc_Path) {
   using namespace stw::errors;
   
   QDomDocument c_Doc("views");
   QDomElement c_RootElement = c_Doc.createElement("views");
   c_Doc.appendChild(c_RootElement);
   
   for (const C_OscViewData& c_View : orc_Views) {
      QDomElement c_ViewElement = c_View.ToQDomDocument(c_Doc, "view");
      c_RootElement.appendChild(c_ViewElement);
   }
   
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }
   
   QTextStream c_Stream(&c_File);
   c_Doc.save(c_Stream, 2);
   
   return C_NO_ERR;
}
