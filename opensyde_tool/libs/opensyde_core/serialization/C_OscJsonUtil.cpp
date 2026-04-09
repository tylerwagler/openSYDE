//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON access helpers for the openSYDE filer subsystem (impl)

   See header for full description.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscJsonUtil.hpp"
#include "stwerrors.hpp"

#include <QJsonValue>

#include <cmath>
#include <limits>

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Module Globals ------------------------------------------------------------------------------------------------ */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Read a required string field

   \param[in]  orc_Json   JSON object containing the field
   \param[in]  orc_Key    Field name
   \param[out] orc_Value  Field value (untouched on error)

   \return
   C_NO_ERR   field present, type matches
   C_CONFIG   field missing or wrong type
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscJsonUtil::h_GetString(const QJsonObject & orc_Json, const QString & orc_Key, QString & orc_Value)
{
   const QJsonValue c_Val = orc_Json.value(orc_Key);
   if (!c_Val.isString())
   {
      return C_CONFIG;
   }
   orc_Value = c_Val.toString();
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Read a required boolean field

   \param[in]  orc_Json    JSON object containing the field
   \param[in]  orc_Key     Field name
   \param[out] orq_Value   Field value (untouched on error)

   \return
   C_NO_ERR   field present, type matches
   C_CONFIG   field missing or wrong type
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscJsonUtil::h_GetBool(const QJsonObject & orc_Json, const QString & orc_Key, bool & orq_Value)
{
   const QJsonValue c_Val = orc_Json.value(orc_Key);
   if (!c_Val.isBool())
   {
      return C_CONFIG;
   }
   orq_Value = c_Val.toBool();
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Internal: read a 64-bit integer JSON number, range-checked

   Returns C_CONFIG if the field is missing or not a JSON number, C_RANGE
   if the value is outside [os64_Min, os64_Max].
*/
//----------------------------------------------------------------------------------------------------------------------
static int32_t mh_GetInteger(const QJsonObject & orc_Json, const QString & orc_Key,
                             const int64_t os64_Min, const int64_t os64_Max, int64_t & ors64_Value)
{
   const QJsonValue c_Val = orc_Json.value(orc_Key);
   if (!c_Val.isDouble())
   {
      return C_CONFIG;
   }
   const int64_t s64_Raw = c_Val.toInteger();
   if ((s64_Raw < os64_Min) || (s64_Raw > os64_Max))
   {
      return C_RANGE;
   }
   ors64_Value = s64_Raw;
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscJsonUtil::h_GetU8(const QJsonObject & orc_Json, const QString & orc_Key, uint8_t & oru8_Value)
{
   int64_t s64_Tmp = 0;
   const int32_t s32_Result = mh_GetInteger(orc_Json, orc_Key, 0, 0xFF, s64_Tmp);
   if (s32_Result == C_NO_ERR)
   {
      oru8_Value = static_cast<uint8_t>(s64_Tmp);
   }
   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscJsonUtil::h_GetU16(const QJsonObject & orc_Json, const QString & orc_Key, uint16_t & oru16_Value)
{
   int64_t s64_Tmp = 0;
   const int32_t s32_Result = mh_GetInteger(orc_Json, orc_Key, 0, 0xFFFF, s64_Tmp);
   if (s32_Result == C_NO_ERR)
   {
      oru16_Value = static_cast<uint16_t>(s64_Tmp);
   }
   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscJsonUtil::h_GetU32(const QJsonObject & orc_Json, const QString & orc_Key, uint32_t & oru32_Value)
{
   int64_t s64_Tmp = 0;
   const int32_t s32_Result = mh_GetInteger(orc_Json, orc_Key, 0, 0xFFFFFFFFLL, s64_Tmp);
   if (s32_Result == C_NO_ERR)
   {
      oru32_Value = static_cast<uint32_t>(s64_Tmp);
   }
   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscJsonUtil::h_GetS8(const QJsonObject & orc_Json, const QString & orc_Key, int8_t & ors8_Value)
{
   int64_t s64_Tmp = 0;
   const int32_t s32_Result = mh_GetInteger(orc_Json, orc_Key, -0x80, 0x7F, s64_Tmp);
   if (s32_Result == C_NO_ERR)
   {
      ors8_Value = static_cast<int8_t>(s64_Tmp);
   }
   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscJsonUtil::h_GetS16(const QJsonObject & orc_Json, const QString & orc_Key, int16_t & ors16_Value)
{
   int64_t s64_Tmp = 0;
   const int32_t s32_Result = mh_GetInteger(orc_Json, orc_Key, -0x8000, 0x7FFF, s64_Tmp);
   if (s32_Result == C_NO_ERR)
   {
      ors16_Value = static_cast<int16_t>(s64_Tmp);
   }
   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscJsonUtil::h_GetS32(const QJsonObject & orc_Json, const QString & orc_Key, int32_t & ors32_Value)
{
   int64_t s64_Tmp = 0;
   const int32_t s32_Result = mh_GetInteger(orc_Json, orc_Key, -0x80000000LL, 0x7FFFFFFFLL, s64_Tmp);
   if (s32_Result == C_NO_ERR)
   {
      ors32_Value = static_cast<int32_t>(s64_Tmp);
   }
   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Read a required uint64_t field (string-encoded)

   uint64_t fields are stored as decimal strings to avoid the JSON-number-as-double precision loss
   for values above 2^53.

   \param[in]  orc_Json     JSON object
   \param[in]  orc_Key      Field name
   \param[out] oru64_Value  Field value (untouched on error)

   \return
   C_NO_ERR   field present, parses as uint64_t
   C_CONFIG   field missing or not a string
   C_RANGE    field is a string but not a valid uint64_t
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscJsonUtil::h_GetU64(const QJsonObject & orc_Json, const QString & orc_Key, uint64_t & oru64_Value)
{
   const QJsonValue c_Val = orc_Json.value(orc_Key);
   if (!c_Val.isString())
   {
      return C_CONFIG;
   }
   bool q_Ok = false;
   const uint64_t u64_Parsed = c_Val.toString().toULongLong(&q_Ok);
   if (!q_Ok)
   {
      return C_RANGE;
   }
   oru64_Value = u64_Parsed;
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Read a required int64_t field (string-encoded)

   See h_GetU64.
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscJsonUtil::h_GetS64(const QJsonObject & orc_Json, const QString & orc_Key, int64_t & ors64_Value)
{
   const QJsonValue c_Val = orc_Json.value(orc_Key);
   if (!c_Val.isString())
   {
      return C_CONFIG;
   }
   bool q_Ok = false;
   const int64_t s64_Parsed = c_Val.toString().toLongLong(&q_Ok);
   if (!q_Ok)
   {
      return C_RANGE;
   }
   ors64_Value = s64_Parsed;
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscJsonUtil::h_GetF32(const QJsonObject & orc_Json, const QString & orc_Key, float32_t & orf32_Value)
{
   const QJsonValue c_Val = orc_Json.value(orc_Key);
   if (!c_Val.isDouble())
   {
      return C_CONFIG;
   }
   orf32_Value = static_cast<float32_t>(c_Val.toDouble());
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscJsonUtil::h_GetF64(const QJsonObject & orc_Json, const QString & orc_Key, float64_t & orf64_Value)
{
   const QJsonValue c_Val = orc_Json.value(orc_Key);
   if (!c_Val.isDouble())
   {
      return C_CONFIG;
   }
   orf64_Value = c_Val.toDouble();
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscJsonUtil::h_GetObject(const QJsonObject & orc_Json, const QString & orc_Key, QJsonObject & orc_Value)
{
   const QJsonValue c_Val = orc_Json.value(orc_Key);
   if (!c_Val.isObject())
   {
      return C_CONFIG;
   }
   orc_Value = c_Val.toObject();
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscJsonUtil::h_GetArray(const QJsonObject & orc_Json, const QString & orc_Key, QJsonArray & orc_Value)
{
   const QJsonValue c_Val = orc_Json.value(orc_Key);
   if (!c_Val.isArray())
   {
      return C_CONFIG;
   }
   orc_Value = c_Val.toArray();
   return C_NO_ERR;
}

/* ===== Optional getters =========================================================================================== */

//----------------------------------------------------------------------------------------------------------------------
void C_OscJsonUtil::h_GetStringOr(const QJsonObject & orc_Json, const QString & orc_Key,
                                  const QString & orc_Default, QString & orc_Value)
{
   if (h_GetString(orc_Json, orc_Key, orc_Value) != C_NO_ERR)
   {
      orc_Value = orc_Default;
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscJsonUtil::h_GetBoolOr(const QJsonObject & orc_Json, const QString & orc_Key,
                                const bool oq_Default, bool & orq_Value)
{
   if (h_GetBool(orc_Json, orc_Key, orq_Value) != C_NO_ERR)
   {
      orq_Value = oq_Default;
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscJsonUtil::h_GetU32Or(const QJsonObject & orc_Json, const QString & orc_Key,
                               const uint32_t ou32_Default, uint32_t & oru32_Value)
{
   if (h_GetU32(orc_Json, orc_Key, oru32_Value) != C_NO_ERR)
   {
      oru32_Value = ou32_Default;
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscJsonUtil::h_GetU64Or(const QJsonObject & orc_Json, const QString & orc_Key,
                               const uint64_t ou64_Default, uint64_t & oru64_Value)
{
   if (h_GetU64(orc_Json, orc_Key, oru64_Value) != C_NO_ERR)
   {
      oru64_Value = ou64_Default;
   }
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscJsonUtil::h_GetF64Or(const QJsonObject & orc_Json, const QString & orc_Key,
                               const float64_t of64_Default, float64_t & orf64_Value)
{
   if (h_GetF64(orc_Json, orc_Key, orf64_Value) != C_NO_ERR)
   {
      orf64_Value = of64_Default;
   }
}

/* ===== Setters ==================================================================================================== */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Write a uint64_t field as a decimal string

   See h_GetU64 for the rationale (JSON-number-as-double precision loss above 2^53).

   \param[in,out] orc_Json     JSON object
   \param[in]     orc_Key      Field name
   \param[in]     ou64_Value   Value to write
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscJsonUtil::h_SetU64(QJsonObject & orc_Json, const QString & orc_Key, const uint64_t ou64_Value)
{
   orc_Json[orc_Key] = QString::number(ou64_Value);
}

//----------------------------------------------------------------------------------------------------------------------
void C_OscJsonUtil::h_SetS64(QJsonObject & orc_Json, const QString & orc_Key, const int64_t os64_Value)
{
   orc_Json[orc_Key] = QString::number(os64_Value);
}

/* ===== Float validation =========================================================================================== */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Whether a float64_t can be safely encoded as a JSON number

   JSON does not permit NaN or Infinity.  This helper exists so callers can range-check before
   serializing and decide whether to substitute, log, or return C_RANGE.

   \param[in] of64_Value  Value to check

   \return
   true   value is finite (safe for JSON)
   false  value is NaN or Inf
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscJsonUtil::h_IsJsonSafe(const float64_t of64_Value)
{
   return std::isfinite(of64_Value);
}

//----------------------------------------------------------------------------------------------------------------------
bool C_OscJsonUtil::h_IsJsonSafe(const float32_t of32_Value)
{
   return std::isfinite(of32_Value);
}
