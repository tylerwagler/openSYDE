//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON access helpers for the openSYDE filer subsystem (header)

   Type-safe getters and setters for QJsonObject keys, plus enum<->string
   conversion helpers.  Used by every filer in the JSON-only filer subsystem.

   The helpers exist for two reasons:
     1. JSON numbers are doubles internally, so uint64_t round-trips lose
        precision above 2^53.  h_GetU64 / h_SetU64 encode uint64_t as a
        decimal string to dodge this.
     2. JSON loaders need uniform "missing key" / "wrong type" error
        handling.  The h_Get* helpers return C_CONFIG on either failure
        and never log; callers decide what context to log.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCJSONUTIL_HPP
#define C_OSCJSONUTIL_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

#include <cstddef>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

/// JSON access helpers for filers
class C_OscJsonUtil
{
public:
   /* ===== Required getters =================================================================================== */
   /* Return C_NO_ERR on success.  Return C_CONFIG if the key is missing or has the wrong JSON type.
      The output reference is left untouched on error. */

   static int32_t h_GetString(const QJsonObject & orc_Json, const QString & orc_Key, QString & orc_Value);
   static int32_t h_GetBool(const QJsonObject & orc_Json, const QString & orc_Key, bool & orq_Value);
   static int32_t h_GetU8(const QJsonObject & orc_Json, const QString & orc_Key, uint8_t & oru8_Value);
   static int32_t h_GetU16(const QJsonObject & orc_Json, const QString & orc_Key, uint16_t & oru16_Value);
   static int32_t h_GetU32(const QJsonObject & orc_Json, const QString & orc_Key, uint32_t & oru32_Value);
   static int32_t h_GetU64(const QJsonObject & orc_Json, const QString & orc_Key, uint64_t & oru64_Value);
   static int32_t h_GetS8(const QJsonObject & orc_Json, const QString & orc_Key, int8_t & ors8_Value);
   static int32_t h_GetS16(const QJsonObject & orc_Json, const QString & orc_Key, int16_t & ors16_Value);
   static int32_t h_GetS32(const QJsonObject & orc_Json, const QString & orc_Key, int32_t & ors32_Value);
   static int32_t h_GetS64(const QJsonObject & orc_Json, const QString & orc_Key, int64_t & ors64_Value);
   static int32_t h_GetF32(const QJsonObject & orc_Json, const QString & orc_Key, float32_t & orf32_Value);
   static int32_t h_GetF64(const QJsonObject & orc_Json, const QString & orc_Key, float64_t & orf64_Value);
   static int32_t h_GetObject(const QJsonObject & orc_Json, const QString & orc_Key, QJsonObject & orc_Value);
   static int32_t h_GetArray(const QJsonObject & orc_Json, const QString & orc_Key, QJsonArray & orc_Value);

   /* ===== Optional getters =================================================================================== */
   /* Always succeed.  When the key is absent or has the wrong type, the default value is written instead. */

   static void h_GetStringOr(const QJsonObject & orc_Json, const QString & orc_Key,
                             const QString & orc_Default, QString & orc_Value);
   static void h_GetBoolOr(const QJsonObject & orc_Json, const QString & orc_Key,
                           const bool oq_Default, bool & orq_Value);
   static void h_GetU32Or(const QJsonObject & orc_Json, const QString & orc_Key,
                          const uint32_t ou32_Default, uint32_t & oru32_Value);
   static void h_GetU64Or(const QJsonObject & orc_Json, const QString & orc_Key,
                          const uint64_t ou64_Default, uint64_t & oru64_Value);
   static void h_GetF64Or(const QJsonObject & orc_Json, const QString & orc_Key,
                          const float64_t of64_Default, float64_t & orf64_Value);

   /* ===== Setters ============================================================================================ */
   /* QJsonObject's operator[] handles QString / bool / int / double natively.  These setters exist only for
      types that need special encoding (uint64_t -> string) or that don't have a clean QJsonValue conversion. */

   static void h_SetU64(QJsonObject & orc_Json, const QString & orc_Key, const uint64_t ou64_Value);
   static void h_SetS64(QJsonObject & orc_Json, const QString & orc_Key, const int64_t os64_Value);

   /* ===== Float validation =================================================================================== */
   /* JSON does not permit NaN or Inf.  Callers that may produce such values should check before serializing
      and either substitute or return C_RANGE.  This helper centralizes the check. */

   static bool h_IsJsonSafe(const float64_t of64_Value);
   static bool h_IsJsonSafe(const float32_t of32_Value);

   /* ===== Enum <-> string mapping ============================================================================ */
   /* Per the filer pattern, all enum fields are serialized as strings (robust to enum reordering).  Each enum
      type provides a static mapping table; these helpers do the lookup. */

   template <typename E>
   struct T_EnumEntry
   {
      E e_Value;
      const char * pc_String;
   };

   template <typename E, std::size_t N>
   static QString h_EnumToString(const E & ore_Value, const T_EnumEntry<E> (& orac_Table)[N])
   {
      for (std::size_t u_Index = 0U; u_Index < N; ++u_Index)
      {
         if (orac_Table[u_Index].e_Value == ore_Value)
         {
            return QString::fromLatin1(orac_Table[u_Index].pc_String);
         }
      }
      return QString();
   }

   template <typename E, std::size_t N>
   static int32_t h_StringToEnum(const QString & orc_String, const T_EnumEntry<E> (& orac_Table)[N],
                                 E & ore_Value);
};

} // namespace opensyde_core
} // namespace stw

/* -- Template implementations -------------------------------------------------------------------------------------- */

#include "stwerrors.hpp"

namespace stw
{
namespace opensyde_core
{

template <typename E, std::size_t N>
int32_t C_OscJsonUtil::h_StringToEnum(const QString & orc_String, const T_EnumEntry<E> (& orac_Table)[N],
                                      E & ore_Value)
{
   for (std::size_t u_Index = 0U; u_Index < N; ++u_Index)
   {
      if (orc_String == QLatin1String(orac_Table[u_Index].pc_String))
      {
         ore_Value = orac_Table[u_Index].e_Value;
         return stw::errors::C_NO_ERR;
      }
   }
   return stw::errors::C_RANGE;
}

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCJSONUTIL_HPP
