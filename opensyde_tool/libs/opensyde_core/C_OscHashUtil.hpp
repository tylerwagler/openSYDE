//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Utility for reducing CalcHash boilerplate via type-dispatched overloads

   Header-only utility providing CalcHashMembers() variadic template that hashes
   arbitrary member fields using C++17 fold expressions and if-constexpr dispatch.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHASHUTIL_HPP
#define C_OSCHASHUTIL_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <type_traits>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QHash>
#include <QMap>
#include "stwtypes/stwtypes.hpp"
#include "scl/C_SclChecksums.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
namespace hash_util
{

/* -- Types --------------------------------------------------------------------------------------------------------- */

// Type traits for container detection
template <typename T>
struct IsQList : std::false_type
{
};
template <typename T>
struct IsQList<QList<T>> : std::true_type
{
};
template <typename T>
struct IsQHash : std::false_type
{
};
template <typename K, typename V>
struct IsQHash<QHash<K, V>> : std::true_type
{
};
template <typename T>
struct IsQMap : std::false_type
{
};
template <typename K, typename V>
struct IsQMap<QMap<K, V>> : std::true_type
{
};

//----------------------------------------------------------------------------------------------------------------------
// QString: hash UTF-8 bytes (non-template overload, highest priority)
inline void CalcHashMember(const QString & orc_Value, uint32_t & oru32_Hash)
{
   const QByteArray c_Bytes = orc_Value.toUtf8();
   stw::scl::C_SclChecksums::CalcCRC32(c_Bytes.constData(), static_cast<uint32_t>(c_Bytes.size()), oru32_Hash);
}

//----------------------------------------------------------------------------------------------------------------------
// QByteArray: hash raw bytes (non-template overload, highest priority)
inline void CalcHashMember(const QByteArray & orc_Value, uint32_t & oru32_Hash)
{
   stw::scl::C_SclChecksums::CalcCRC32(orc_Value.constData(), static_cast<uint32_t>(orc_Value.size()), oru32_Hash);
}

//----------------------------------------------------------------------------------------------------------------------
// Generic template: dispatches via if constexpr to avoid forward-declaration ordering issues
template <typename T>
void CalcHashMember(const T & orc_Value, uint32_t & oru32_Hash)
{
   if constexpr (std::is_arithmetic_v<T> || std::is_enum_v<T>)
   {
      // Arithmetic and enum types: hash raw bytes
      stw::scl::C_SclChecksums::CalcCRC32(&orc_Value, sizeof(orc_Value), oru32_Hash);
   }
   else if constexpr (IsQList<T>::value)
   {
      // QList<T>: hash each element
      for (const auto & rc_Element : orc_Value)
      {
         CalcHashMember(rc_Element, oru32_Hash);
      }
   }
   else if constexpr (IsQHash<T>::value)
   {
      // QHash<K,V>: hash each key-value pair
      for (auto c_It = orc_Value.begin(); c_It != orc_Value.end(); ++c_It)
      {
         CalcHashMember(c_It.key(), oru32_Hash);
         CalcHashMember(c_It.value(), oru32_Hash);
      }
   }
   else if constexpr (IsQMap<T>::value)
   {
      // QMap<K,V>: hash each key-value pair
      for (auto c_It = orc_Value.begin(); c_It != orc_Value.end(); ++c_It)
      {
         CalcHashMember(c_It.key(), oru32_Hash);
         CalcHashMember(c_It.value(), oru32_Hash);
      }
   }
   else if constexpr (std::is_array_v<T>)
   {
      // C-array: hash raw bytes
      stw::scl::C_SclChecksums::CalcCRC32(orc_Value, sizeof(orc_Value), oru32_Hash);
   }
   else
   {
      // Objects with CalcHash(uint32_t&) const method: delegate
      orc_Value.CalcHash(oru32_Hash);
   }
}

//----------------------------------------------------------------------------------------------------------------------
// Variadic: hash all members via C++17 fold expression
template <typename... Ts>
void CalcHashMembers(uint32_t & oru32_Hash, const Ts &... orc_Members)
{
   (CalcHashMember(orc_Members, oru32_Hash), ...);
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

} // namespace hash_util
} // namespace opensyde_core
} // namespace stw

#endif // C_OSCHASHUTIL_HPP
