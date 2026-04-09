//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscNodeDataPoolContent (impl)

   See header for the JSON shape.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscNodeDataPoolContentFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "stwerrors.hpp"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonValue>
#include <QList>
#include <QString>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Module Globals ------------------------------------------------------------------------------------------------ */

namespace
{
const C_OscJsonUtil::T_EnumEntry<C_OscNodeDataPoolContent::E_Type> hac_TypeTable[] = {
   { C_OscNodeDataPoolContent::eUINT8,   "uint8"   },
   { C_OscNodeDataPoolContent::eUINT16,  "uint16"  },
   { C_OscNodeDataPoolContent::eUINT32,  "uint32"  },
   { C_OscNodeDataPoolContent::eUINT64,  "uint64"  },
   { C_OscNodeDataPoolContent::eSINT8,   "sint8"   },
   { C_OscNodeDataPoolContent::eSINT16,  "sint16"  },
   { C_OscNodeDataPoolContent::eSINT32,  "sint32"  },
   { C_OscNodeDataPoolContent::eSINT64,  "sint64"  },
   { C_OscNodeDataPoolContent::eFLOAT32, "float32" },
   { C_OscNodeDataPoolContent::eFLOAT64, "float64" }
};
} // namespace

/* -- Implementation ------------------------------------------------------------------------------------------------ */

namespace
{
//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Encode the value field for a scalar (non-array) content
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonValue mh_SaveScalar(const C_OscNodeDataPoolContent & orc_Content)
{
   switch (orc_Content.GetType())
   {
   case C_OscNodeDataPoolContent::eUINT8:
      return static_cast<qint64>(orc_Content.GetValueU8());
   case C_OscNodeDataPoolContent::eUINT16:
      return static_cast<qint64>(orc_Content.GetValueU16());
   case C_OscNodeDataPoolContent::eUINT32:
      return static_cast<qint64>(orc_Content.GetValueU32());
   case C_OscNodeDataPoolContent::eUINT64:
      return QString::number(orc_Content.GetValueU64());
   case C_OscNodeDataPoolContent::eSINT8:
      return static_cast<qint64>(orc_Content.GetValueS8());
   case C_OscNodeDataPoolContent::eSINT16:
      return static_cast<qint64>(orc_Content.GetValueS16());
   case C_OscNodeDataPoolContent::eSINT32:
      return static_cast<qint64>(orc_Content.GetValueS32());
   case C_OscNodeDataPoolContent::eSINT64:
      return QString::number(orc_Content.GetValueS64());
   case C_OscNodeDataPoolContent::eFLOAT32:
      return static_cast<double>(orc_Content.GetValueF32());
   case C_OscNodeDataPoolContent::eFLOAT64:
      return orc_Content.GetValueF64();
   default:
      return QJsonValue();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Encode the value field for an array content

   uint8 arrays use Base64 (the storage is QByteArray, this is the natural compact form).
   uint64/sint64 arrays use string-encoded elements (precision).
   Everything else uses JSON-number arrays.
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonValue mh_SaveArray(const C_OscNodeDataPoolContent & orc_Content)
{
   switch (orc_Content.GetType())
   {
   case C_OscNodeDataPoolContent::eUINT8:
      return QString::fromLatin1(orc_Content.GetValueArrU8().toBase64());
   case C_OscNodeDataPoolContent::eUINT16:
      {
         QJsonArray c_Array;
         const QList<uint16_t> c_Values = orc_Content.GetValueArrU16();
         for (const uint16_t u16_Value : c_Values)
         {
            c_Array.append(static_cast<qint64>(u16_Value));
         }
         return c_Array;
      }
   case C_OscNodeDataPoolContent::eUINT32:
      {
         QJsonArray c_Array;
         const QList<uint32_t> c_Values = orc_Content.GetValueArrU32();
         for (const uint32_t u32_Value : c_Values)
         {
            c_Array.append(static_cast<qint64>(u32_Value));
         }
         return c_Array;
      }
   case C_OscNodeDataPoolContent::eUINT64:
      {
         QJsonArray c_Array;
         const QList<uint64_t> c_Values = orc_Content.GetValueArrU64();
         for (const uint64_t u64_Value : c_Values)
         {
            c_Array.append(QString::number(u64_Value));
         }
         return c_Array;
      }
   case C_OscNodeDataPoolContent::eSINT8:
      {
         QJsonArray c_Array;
         const QList<int8_t> c_Values = orc_Content.GetValueArrS8();
         for (const int8_t s8_Value : c_Values)
         {
            c_Array.append(static_cast<qint64>(s8_Value));
         }
         return c_Array;
      }
   case C_OscNodeDataPoolContent::eSINT16:
      {
         QJsonArray c_Array;
         const QList<int16_t> c_Values = orc_Content.GetValueArrS16();
         for (const int16_t s16_Value : c_Values)
         {
            c_Array.append(static_cast<qint64>(s16_Value));
         }
         return c_Array;
      }
   case C_OscNodeDataPoolContent::eSINT32:
      {
         QJsonArray c_Array;
         const QList<int32_t> c_Values = orc_Content.GetValueArrS32();
         for (const int32_t s32_Value : c_Values)
         {
            c_Array.append(static_cast<qint64>(s32_Value));
         }
         return c_Array;
      }
   case C_OscNodeDataPoolContent::eSINT64:
      {
         QJsonArray c_Array;
         const QList<int64_t> c_Values = orc_Content.GetValueArrS64();
         for (const int64_t s64_Value : c_Values)
         {
            c_Array.append(QString::number(s64_Value));
         }
         return c_Array;
      }
   case C_OscNodeDataPoolContent::eFLOAT32:
      {
         QJsonArray c_Array;
         const QList<float32_t> c_Values = orc_Content.GetValueArrF32();
         for (const float32_t f32_Value : c_Values)
         {
            c_Array.append(static_cast<double>(f32_Value));
         }
         return c_Array;
      }
   case C_OscNodeDataPoolContent::eFLOAT64:
      {
         QJsonArray c_Array;
         const QList<float64_t> c_Values = orc_Content.GetValueArrF64();
         for (const float64_t f64_Value : c_Values)
         {
            c_Array.append(f64_Value);
         }
         return c_Array;
      }
   default:
      return QJsonArray();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Decode a scalar value field into the content object
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t mh_LoadScalar(const QJsonValue & orc_Value, C_OscNodeDataPoolContent & orc_Content)
{
   switch (orc_Content.GetType())
   {
   case C_OscNodeDataPoolContent::eUINT8:
      if (!orc_Value.isDouble())
      {
         return C_CONFIG;
      }
      orc_Content.SetValueU8(static_cast<uint8_t>(orc_Value.toInteger()));
      break;
   case C_OscNodeDataPoolContent::eUINT16:
      if (!orc_Value.isDouble())
      {
         return C_CONFIG;
      }
      orc_Content.SetValueU16(static_cast<uint16_t>(orc_Value.toInteger()));
      break;
   case C_OscNodeDataPoolContent::eUINT32:
      if (!orc_Value.isDouble())
      {
         return C_CONFIG;
      }
      orc_Content.SetValueU32(static_cast<uint32_t>(orc_Value.toInteger()));
      break;
   case C_OscNodeDataPoolContent::eUINT64:
      {
         if (!orc_Value.isString())
         {
            return C_CONFIG;
         }
         bool q_Ok = false;
         const uint64_t u64_Parsed = orc_Value.toString().toULongLong(&q_Ok);
         if (!q_Ok)
         {
            return C_RANGE;
         }
         orc_Content.SetValueU64(u64_Parsed);
      }
      break;
   case C_OscNodeDataPoolContent::eSINT8:
      if (!orc_Value.isDouble())
      {
         return C_CONFIG;
      }
      orc_Content.SetValueS8(static_cast<int8_t>(orc_Value.toInteger()));
      break;
   case C_OscNodeDataPoolContent::eSINT16:
      if (!orc_Value.isDouble())
      {
         return C_CONFIG;
      }
      orc_Content.SetValueS16(static_cast<int16_t>(orc_Value.toInteger()));
      break;
   case C_OscNodeDataPoolContent::eSINT32:
      if (!orc_Value.isDouble())
      {
         return C_CONFIG;
      }
      orc_Content.SetValueS32(static_cast<int32_t>(orc_Value.toInteger()));
      break;
   case C_OscNodeDataPoolContent::eSINT64:
      {
         if (!orc_Value.isString())
         {
            return C_CONFIG;
         }
         bool q_Ok = false;
         const int64_t s64_Parsed = orc_Value.toString().toLongLong(&q_Ok);
         if (!q_Ok)
         {
            return C_RANGE;
         }
         orc_Content.SetValueS64(s64_Parsed);
      }
      break;
   case C_OscNodeDataPoolContent::eFLOAT32:
      if (!orc_Value.isDouble())
      {
         return C_CONFIG;
      }
      orc_Content.SetValueF32(static_cast<float32_t>(orc_Value.toDouble()));
      break;
   case C_OscNodeDataPoolContent::eFLOAT64:
      if (!orc_Value.isDouble())
      {
         return C_CONFIG;
      }
      orc_Content.SetValueF64(orc_Value.toDouble());
      break;
   default:
      return C_CONFIG;
   }
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Decode an array value field into the content object
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t mh_LoadArray(const QJsonValue & orc_Value, C_OscNodeDataPoolContent & orc_Content)
{
   if (orc_Content.GetType() == C_OscNodeDataPoolContent::eUINT8)
   {
      if (!orc_Value.isString())
      {
         return C_CONFIG;
      }
      const QByteArray c_Decoded = QByteArray::fromBase64(orc_Value.toString().toLatin1());
      orc_Content.SetArraySize(static_cast<uint32_t>(c_Decoded.size()));
      orc_Content.SetValueArrU8(c_Decoded);
      return C_NO_ERR;
   }

   if (!orc_Value.isArray())
   {
      return C_CONFIG;
   }
   const QJsonArray c_Array = orc_Value.toArray();
   orc_Content.SetArraySize(static_cast<uint32_t>(c_Array.size()));

   switch (orc_Content.GetType())
   {
   case C_OscNodeDataPoolContent::eUINT16:
      {
         QList<uint16_t> c_Values;
         c_Values.reserve(c_Array.size());
         for (const QJsonValue & rc_Item : c_Array)
         {
            if (!rc_Item.isDouble())
            {
               return C_CONFIG;
            }
            c_Values.append(static_cast<uint16_t>(rc_Item.toInteger()));
         }
         orc_Content.SetValueArrU16(c_Values);
      }
      break;
   case C_OscNodeDataPoolContent::eUINT32:
      {
         QList<uint32_t> c_Values;
         c_Values.reserve(c_Array.size());
         for (const QJsonValue & rc_Item : c_Array)
         {
            if (!rc_Item.isDouble())
            {
               return C_CONFIG;
            }
            c_Values.append(static_cast<uint32_t>(rc_Item.toInteger()));
         }
         orc_Content.SetValueArrU32(c_Values);
      }
      break;
   case C_OscNodeDataPoolContent::eUINT64:
      {
         QList<uint64_t> c_Values;
         c_Values.reserve(c_Array.size());
         for (const QJsonValue & rc_Item : c_Array)
         {
            if (!rc_Item.isString())
            {
               return C_CONFIG;
            }
            bool q_Ok = false;
            const uint64_t u64_Parsed = rc_Item.toString().toULongLong(&q_Ok);
            if (!q_Ok)
            {
               return C_RANGE;
            }
            c_Values.append(u64_Parsed);
         }
         orc_Content.SetValueArrU64(c_Values);
      }
      break;
   case C_OscNodeDataPoolContent::eSINT8:
      {
         QList<int8_t> c_Values;
         c_Values.reserve(c_Array.size());
         for (const QJsonValue & rc_Item : c_Array)
         {
            if (!rc_Item.isDouble())
            {
               return C_CONFIG;
            }
            c_Values.append(static_cast<int8_t>(rc_Item.toInteger()));
         }
         orc_Content.SetValueArrS8(c_Values);
      }
      break;
   case C_OscNodeDataPoolContent::eSINT16:
      {
         QList<int16_t> c_Values;
         c_Values.reserve(c_Array.size());
         for (const QJsonValue & rc_Item : c_Array)
         {
            if (!rc_Item.isDouble())
            {
               return C_CONFIG;
            }
            c_Values.append(static_cast<int16_t>(rc_Item.toInteger()));
         }
         orc_Content.SetValueArrS16(c_Values);
      }
      break;
   case C_OscNodeDataPoolContent::eSINT32:
      {
         QList<int32_t> c_Values;
         c_Values.reserve(c_Array.size());
         for (const QJsonValue & rc_Item : c_Array)
         {
            if (!rc_Item.isDouble())
            {
               return C_CONFIG;
            }
            c_Values.append(static_cast<int32_t>(rc_Item.toInteger()));
         }
         orc_Content.SetValueArrS32(c_Values);
      }
      break;
   case C_OscNodeDataPoolContent::eSINT64:
      {
         QList<int64_t> c_Values;
         c_Values.reserve(c_Array.size());
         for (const QJsonValue & rc_Item : c_Array)
         {
            if (!rc_Item.isString())
            {
               return C_CONFIG;
            }
            bool q_Ok = false;
            const int64_t s64_Parsed = rc_Item.toString().toLongLong(&q_Ok);
            if (!q_Ok)
            {
               return C_RANGE;
            }
            c_Values.append(s64_Parsed);
         }
         orc_Content.SetValueArrS64(c_Values);
      }
      break;
   case C_OscNodeDataPoolContent::eFLOAT32:
      {
         QList<float32_t> c_Values;
         c_Values.reserve(c_Array.size());
         for (const QJsonValue & rc_Item : c_Array)
         {
            if (!rc_Item.isDouble())
            {
               return C_CONFIG;
            }
            c_Values.append(static_cast<float32_t>(rc_Item.toDouble()));
         }
         orc_Content.SetValueArrF32(c_Values);
      }
      break;
   case C_OscNodeDataPoolContent::eFLOAT64:
      {
         QList<float64_t> c_Values;
         c_Values.reserve(c_Array.size());
         for (const QJsonValue & rc_Item : c_Array)
         {
            if (!rc_Item.isDouble())
            {
               return C_CONFIG;
            }
            c_Values.append(rc_Item.toDouble());
         }
         orc_Content.SetValueArrF64(c_Values);
      }
      break;
   default:
      return C_CONFIG;
   }
   return C_NO_ERR;
}
} // namespace

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save a content value to a JSON object

   \param[in]  orc_Content   Content to serialize

   \return JSON object with type, is_array, value fields
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscNodeDataPoolContentFiler::save(const C_OscNodeDataPoolContent & orc_Content)
{
   QJsonObject c_Json;
   c_Json["type"] = C_OscJsonUtil::h_EnumToString(orc_Content.GetType(), hac_TypeTable);
   c_Json["is_array"] = orc_Content.GetArray();
   if (orc_Content.GetArray())
   {
      c_Json["value"] = mh_SaveArray(orc_Content);
   }
   else
   {
      c_Json["value"] = mh_SaveScalar(orc_Content);
   }
   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load a content value from a JSON object

   \param[in]  orc_Json      JSON object as produced by save()
   \param[out] orc_Content   Content to populate

   \return
   C_NO_ERR   loaded
   C_CONFIG   missing/invalid required field
   C_RANGE    string-encoded integer out of range
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPoolContentFiler::load(const QJsonObject & orc_Json, C_OscNodeDataPoolContent & orc_Content)
{
   QString c_TypeStr;
   int32_t s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "type", c_TypeStr);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   C_OscNodeDataPoolContent::E_Type e_Type = C_OscNodeDataPoolContent::eUINT8;
   s32_Result = C_OscJsonUtil::h_StringToEnum(c_TypeStr, hac_TypeTable, e_Type);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   bool q_IsArray = false;
   s32_Result = C_OscJsonUtil::h_GetBool(orc_Json, "is_array", q_IsArray);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   if (!orc_Json.contains("value"))
   {
      return C_CONFIG;
   }

   orc_Content.SetType(e_Type);
   orc_Content.SetArray(q_IsArray);

   if (q_IsArray)
   {
      return mh_LoadArray(orc_Json.value("value"), orc_Content);
   }
   return mh_LoadScalar(orc_Json.value("value"), orc_Content);
}
