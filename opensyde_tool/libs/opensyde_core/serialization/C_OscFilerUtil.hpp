//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Template utility functions for multi-format file serialization

   Header-only free function templates that encapsulate the common
   binary / JSON / XML file I/O patterns used across all filers.

   Type T requirements (list helpers):
   - int32_t FromQDataStream(QDataStream&)
   - int32_t ToQDataStream(QDataStream&) const
   - int32_t FromJsonObject(const QJsonObject&)
   - QJsonObject ToJsonObject() const
   - int32_t FromQDomElement(const QDomElement&)
   - QDomElement ToQDomElement(QDomDocument&, const QString&) const

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCFILERUTIL_HPP
#define C_OSCFILERUTIL_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscLoggingHandler.hpp"

#include <QDataStream>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QList>
#include <QString>
#include <QTextStream>

#include <cstddef>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw {
namespace opensyde_core {
namespace C_OscFilerUtil {

/* ===== List helpers =============================================================================================== */

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Load list from binary file

   Optional version header: when ou16_ExpectedVersion > 0, a uint16_t is read
   first and verified.  Then a uint32_t item count followed by T items.

   \param[out] orc_List              Target list (cleared before filling)
   \param[in]  orc_Path              File path
   \param[in]  ou16_ExpectedVersion  Expected version (0 = no version header)

   \return
   C_NO_ERR   success
   C_RD_WR    cannot open / stream error
   C_CONFIG   version mismatch or item deserialization error
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_LoadListBinary(QList<T> & orc_List, const QString & orc_Path,
                         const uint16_t ou16_ExpectedVersion = 0)
{
   using namespace stw::errors;

   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly))
   {
      return C_RD_WR;
   }

   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_6_0);

   // Optional version header
   if (ou16_ExpectedVersion > 0U)
   {
      uint16_t u16_Version = 0U;
      c_Stream >> u16_Version;
      if (u16_Version != ou16_ExpectedVersion)
      {
         return C_CONFIG;
      }
   }

   // Item count
   uint32_t u32_Count = 0U;
   c_Stream >> u32_Count;

   orc_List.clear();
   orc_List.reserve(static_cast<int32_t>(u32_Count));

   for (uint32_t u32_I = 0U; u32_I < u32_Count; ++u32_I)
   {
      T c_Item;
      const int32_t s32_Err = c_Item.FromQDataStream(c_Stream);
      if (s32_Err != C_NO_ERR)
      {
         return s32_Err;
      }
      orc_List.append(c_Item);
   }

   if (c_Stream.status() != QDataStream::Ok)
   {
      return C_RD_WR;
   }
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Save list to binary file

   \param[in]  orc_List     Source list
   \param[in]  orc_Path     File path
   \param[in]  ou16_Version Version to write (0 = no version header)

   \return
   C_NO_ERR   success
   C_RD_WR    cannot open / stream error
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_SaveListBinary(const QList<T> & orc_List, const QString & orc_Path,
                         const uint16_t ou16_Version = 0)
{
   using namespace stw::errors;

   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly))
   {
      return C_RD_WR;
   }

   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_6_0);

   if (ou16_Version > 0U)
   {
      c_Stream << ou16_Version;
   }

   c_Stream << static_cast<uint32_t>(orc_List.size());

   for (const T & rc_Item : orc_List)
   {
      const int32_t s32_Err = rc_Item.ToQDataStream(c_Stream);
      if (s32_Err != C_NO_ERR)
      {
         return s32_Err;
      }
   }

   if (c_Stream.status() != QDataStream::Ok)
   {
      return C_RD_WR;
   }
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Load list from JSON file (bare JSON array)

   \param[out] orc_List  Target list
   \param[in]  orc_Path  File path

   \return
   C_NO_ERR   success
   C_RD_WR    cannot open
   C_CONFIG   parse error / unexpected format / item error
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_LoadListJson(QList<T> & orc_List, const QString & orc_Path)
{
   using namespace stw::errors;

   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly))
   {
      return C_RD_WR;
   }

   QJsonParseError c_ParseError;
   const QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll(), &c_ParseError);
   c_File.close();

   if (c_ParseError.error != QJsonParseError::NoError)
   {
      return C_CONFIG;
   }
   if (!c_Doc.isArray())
   {
      return C_CONFIG;
   }

   const QJsonArray c_Array = c_Doc.array();
   orc_List.clear();
   orc_List.reserve(c_Array.size());

   for (const QJsonValue & rc_Value : c_Array)
   {
      if (!rc_Value.isObject())
      {
         return C_CONFIG;
      }
      T c_Item;
      const int32_t s32_Err = c_Item.FromJsonObject(rc_Value.toObject());
      if (s32_Err != C_NO_ERR)
      {
         return s32_Err;
      }
      orc_List.append(c_Item);
   }
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Save list to JSON file (bare JSON array)

   \param[in]  orc_List  Source list
   \param[in]  orc_Path  File path

   \return
   C_NO_ERR   success
   C_RD_WR    cannot open
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_SaveListJson(const QList<T> & orc_List, const QString & orc_Path)
{
   using namespace stw::errors;

   QJsonArray c_Array;
   for (const T & rc_Item : orc_List)
   {
      c_Array.append(rc_Item.ToJsonObject());
   }

   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly))
   {
      return C_RD_WR;
   }

   c_File.write(QJsonDocument(c_Array).toJson(QJsonDocument::Indented));
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Load list from XML file (QDomDocument)

   Expected structure:  <orc_RootTag> <orc_ItemTag>...</orc_ItemTag> ... </orc_RootTag>

   \param[out] orc_List     Target list
   \param[in]  orc_Path     File path
   \param[in]  orc_RootTag  Expected root element tag name
   \param[in]  orc_ItemTag  Expected child element tag name

   \return
   C_NO_ERR   success
   C_RD_WR    cannot open
   C_CONFIG   parse error / wrong root tag / item error
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_LoadListXml(QList<T> & orc_List, const QString & orc_Path,
                      const QString & orc_RootTag, const QString & orc_ItemTag)
{
   using namespace stw::errors;

   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly))
   {
      return C_RD_WR;
   }

   QDomDocument c_Doc;
   if (!c_Doc.setContent(&c_File))
   {
      return C_CONFIG;
   }

   const QDomElement c_Root = c_Doc.documentElement();
   if (c_Root.tagName() != orc_RootTag)
   {
      return C_CONFIG;
   }

   orc_List.clear();
   QDomNode c_Node = c_Root.firstChild();
   while (!c_Node.isNull())
   {
      if (c_Node.isElement() && (c_Node.toElement().tagName() == orc_ItemTag))
      {
         T c_Item;
         const int32_t s32_Err = c_Item.FromQDomElement(c_Node.toElement());
         if (s32_Err != C_NO_ERR)
         {
            return s32_Err;
         }
         orc_List.append(c_Item);
      }
      c_Node = c_Node.nextSibling();
   }
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Save list to XML file (QDomDocument)

   \param[in]  orc_List     Source list
   \param[in]  orc_Path     File path
   \param[in]  orc_RootTag  Root element tag name
   \param[in]  orc_ItemTag  Child element tag name

   \return
   C_NO_ERR   success
   C_RD_WR    cannot open
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_SaveListXml(const QList<T> & orc_List, const QString & orc_Path,
                      const QString & orc_RootTag, const QString & orc_ItemTag)
{
   using namespace stw::errors;

   QDomDocument c_Doc(orc_RootTag);
   QDomElement c_Root = c_Doc.createElement(orc_RootTag);
   c_Doc.appendChild(c_Root);

   for (const T & rc_Item : orc_List)
   {
      c_Root.appendChild(rc_Item.ToQDomElement(c_Doc, orc_ItemTag));
   }

   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly))
   {
      return C_RD_WR;
   }

   QTextStream c_Stream(&c_File);
   c_Doc.save(c_Stream, 2);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Load list from file with auto-detection of format based on extension

   .bin  -> binary,  .json -> JSON,  anything else -> XML

   \param[out] orc_List              Target list
   \param[in]  orc_Path              File path
   \param[in]  orc_RootTag           XML root tag (also used as XML doctype)
   \param[in]  orc_ItemTag           XML item tag
   \param[in]  ou16_ExpectedVersion  Binary version (0 = no version header)

   \return Error code (see format-specific helpers)
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_LoadListFile(QList<T> & orc_List, const QString & orc_Path,
                       const QString & orc_RootTag, const QString & orc_ItemTag,
                       const uint16_t ou16_ExpectedVersion = 0)
{
   const QString c_Ext = QFileInfo(orc_Path).suffix().toLower();

   if (c_Ext == "bin")
   {
      return h_LoadListBinary(orc_List, orc_Path, ou16_ExpectedVersion);
   }
   else if (c_Ext == "json")
   {
      return h_LoadListJson(orc_List, orc_Path);
   }
   else
   {
      return h_LoadListXml(orc_List, orc_Path, orc_RootTag, orc_ItemTag);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Save list to file with auto-detection of format based on extension

   \param[in]  orc_List      Source list
   \param[in]  orc_Path      File path
   \param[in]  orc_RootTag   XML root tag
   \param[in]  orc_ItemTag   XML item tag
   \param[in]  ou16_Version  Binary version (0 = no version header)

   \return Error code (see format-specific helpers)
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_SaveListFile(const QList<T> & orc_List, const QString & orc_Path,
                       const QString & orc_RootTag, const QString & orc_ItemTag,
                       const uint16_t ou16_Version = 0)
{
   const QString c_Ext = QFileInfo(orc_Path).suffix().toLower();

   if (c_Ext == "bin")
   {
      return h_SaveListBinary(orc_List, orc_Path, ou16_Version);
   }
   else if (c_Ext == "json")
   {
      return h_SaveListJson(orc_List, orc_Path);
   }
   else
   {
      return h_SaveListXml(orc_List, orc_Path, orc_RootTag, orc_ItemTag);
   }
}

/* ===== Single-object helpers ====================================================================================== */

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Load single object from binary file

   \param[out] orc_Object  Target object
   \param[in]  orc_Path    File path

   \return
   C_NO_ERR   success
   C_RD_WR    cannot open / stream error
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_LoadSingleBinary(T & orc_Object, const QString & orc_Path)
{
   using namespace stw::errors;

   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly))
   {
      return C_RD_WR;
   }

   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_6_0);

   return orc_Object.FromQDataStream(c_Stream);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Save single object to binary file

   \param[in]  orc_Object  Source object
   \param[in]  orc_Path    File path

   \return
   C_NO_ERR   success
   C_RD_WR    cannot open / stream error
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_SaveSingleBinary(const T & orc_Object, const QString & orc_Path)
{
   using namespace stw::errors;

   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly))
   {
      return C_RD_WR;
   }

   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_6_0);

   return orc_Object.ToQDataStream(c_Stream);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Load single object from JSON file

   \param[out] orc_Object  Target object
   \param[in]  orc_Path    File path

   \return
   C_NO_ERR   success
   C_RD_WR    cannot open
   C_CONFIG   parse error / not an object
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_LoadSingleJson(T & orc_Object, const QString & orc_Path)
{
   using namespace stw::errors;

   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly))
   {
      return C_RD_WR;
   }

   QJsonParseError c_ParseError;
   const QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll(), &c_ParseError);
   c_File.close();

   if (c_ParseError.error != QJsonParseError::NoError)
   {
      return C_CONFIG;
   }
   if (!c_Doc.isObject())
   {
      return C_CONFIG;
   }

   return orc_Object.FromJsonObject(c_Doc.object());
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Save single object to JSON file

   \param[in]  orc_Object  Source object
   \param[in]  orc_Path    File path

   \return
   C_NO_ERR   success
   C_RD_WR    cannot open
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_SaveSingleJson(const T & orc_Object, const QString & orc_Path)
{
   using namespace stw::errors;

   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly))
   {
      return C_RD_WR;
   }

   c_File.write(QJsonDocument(orc_Object.ToJsonObject()).toJson(QJsonDocument::Indented));
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Load single object from XML file

   \param[out] orc_Object   Target object
   \param[in]  orc_Path     File path
   \param[in]  orc_RootTag  Expected root element tag (validated but not required to match)

   \return
   C_NO_ERR   success
   C_RD_WR    cannot open
   C_CONFIG   parse error
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_LoadSingleXml(T & orc_Object, const QString & orc_Path,
                        const QString & orc_RootTag)
{
   Q_UNUSED(orc_RootTag)
   using namespace stw::errors;

   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly))
   {
      return C_RD_WR;
   }

   QDomDocument c_Doc;
   if (!c_Doc.setContent(&c_File))
   {
      return C_CONFIG;
   }

   return orc_Object.FromQDomElement(c_Doc.documentElement());
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Save single object to XML file

   \param[in]  orc_Object   Source object
   \param[in]  orc_Path     File path
   \param[in]  orc_RootTag  Root element tag name

   \return
   C_NO_ERR   success
   C_RD_WR    cannot open
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_SaveSingleXml(const T & orc_Object, const QString & orc_Path,
                        const QString & orc_RootTag)
{
   using namespace stw::errors;

   QDomDocument c_Doc(orc_RootTag);
   c_Doc.appendChild(orc_Object.ToQDomElement(c_Doc, orc_RootTag));

   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly))
   {
      return C_RD_WR;
   }

   QTextStream c_Stream(&c_File);
   c_Doc.save(c_Stream, 2);
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Load single object from file (auto-detect format)

   \param[out] orc_Object   Target object
   \param[in]  orc_Path     File path
   \param[in]  orc_RootTag  XML root tag

   \return Error code (see format-specific helpers)
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_LoadSingleFile(T & orc_Object, const QString & orc_Path,
                         const QString & orc_RootTag)
{
   const QString c_Ext = QFileInfo(orc_Path).suffix().toLower();

   if (c_Ext == "bin")
   {
      return h_LoadSingleBinary(orc_Object, orc_Path);
   }
   else if (c_Ext == "json")
   {
      return h_LoadSingleJson(orc_Object, orc_Path);
   }
   else
   {
      return h_LoadSingleXml(orc_Object, orc_Path, orc_RootTag);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Save single object to file (auto-detect format)

   \param[in]  orc_Object   Source object
   \param[in]  orc_Path     File path
   \param[in]  orc_RootTag  XML root tag

   \return Error code (see format-specific helpers)
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename T>
int32_t h_SaveSingleFile(const T & orc_Object, const QString & orc_Path,
                         const QString & orc_RootTag)
{
   const QString c_Ext = QFileInfo(orc_Path).suffix().toLower();

   if (c_Ext == "bin")
   {
      return h_SaveSingleBinary(orc_Object, orc_Path);
   }
   else if (c_Ext == "json")
   {
      return h_SaveSingleJson(orc_Object, orc_Path);
   }
   else
   {
      return h_SaveSingleXml(orc_Object, orc_Path, orc_RootTag);
   }
}

/* ===== Enum / string conversion helpers =========================================================================== */

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Entry mapping an enum value to its string representation
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename E>
struct EnumEntry
{
   E e_Value;
   const char * pc_String;
};

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Convert enum value to string using a lookup table

   \param[in]  ore_Value     Enum value to convert
   \param[in]  orac_Table    Array of EnumEntry mapping enum values to strings
   \param[in]  opc_Default   String to return if enum value is not found (default: "invalid")

   \return
   String representation of the enum value, or opc_Default if not found
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename E, std::size_t N>
QString h_EnumToString(const E & ore_Value, const EnumEntry<E> (&orac_Table)[N],
                       const char * const opc_Default = "invalid")
{
   for (std::size_t u_I = 0U; u_I < N; ++u_I)
   {
      if (orac_Table[u_I].e_Value == ore_Value)
      {
         return orac_Table[u_I].pc_String;
      }
   }
   return opc_Default;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief  Convert string to enum value using a lookup table

   If orc_LogCategory is non-empty, an error is logged on failure.

   \param[in]   orc_String        String to interpret
   \param[in]   orac_Table        Array of EnumEntry mapping enum values to strings
   \param[out]  ore_Result        Resulting enum value (unchanged on error)
   \param[in]   orc_LogCategory   Log category for error message (empty = no logging)
   \param[in]   orc_FieldName     Field name for error message context

   \return
   C_NO_ERR   no error
   C_RANGE    String unknown
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename E, std::size_t N>
int32_t h_StringToEnum(const QString & orc_String, const EnumEntry<E> (&orac_Table)[N],
                       E & ore_Result,
                       const QString & orc_LogCategory = "",
                       const QString & orc_FieldName = "")
{
   for (std::size_t u_I = 0U; u_I < N; ++u_I)
   {
      if (orc_String == orac_Table[u_I].pc_String)
      {
         ore_Result = orac_Table[u_I].e_Value;
         return stw::errors::C_NO_ERR;
      }
   }
   if (!orc_LogCategory.isEmpty())
   {
      osc_write_log_error(orc_LogCategory,
                          "Invalid value for \"" + orc_FieldName + "\": " + orc_String);
   }
   return stw::errors::C_RANGE;
}

} // namespace C_OscFilerUtil
} // namespace opensyde_core
} // namespace stw

#endif // C_OSCFILERUTIL_HPP
