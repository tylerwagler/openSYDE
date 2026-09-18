//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Byte-order helpers for wire formats (header only)

   Reassembling a multi-byte value from a byte buffer, or splitting one into bytes, used to be
   hand-written at every protocol site: four casts, three shifts and an index per value, with
   the index order the only thing telling big from little endian. Each such site is a chance for
   a transposed index that no compiler will notice. These helpers say the byte order in their
   name and are the one place the shifts live.

   All functions read or write exactly sizeof(value) bytes at the pointer and are constexpr, so
   a wire constant can be assembled at compile time.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCENDIAN_HPP
#define C_OSCENDIAN_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
///Read and write integers in a defined byte order
class C_OscEndian
{
public:
   //big endian: most significant byte first (network order, the openSYDE protocol, DoIP, Intel hex offsets)
   [[nodiscard]] static constexpr uint16_t h_GetU16Big(const uint8_t * const opu8_Data)
   {
      return static_cast<uint16_t>((static_cast<uint16_t>(opu8_Data[0]) << 8U) | opu8_Data[1]);
   }

   [[nodiscard]] static constexpr uint32_t h_GetU32Big(const uint8_t * const opu8_Data)
   {
      return (static_cast<uint32_t>(opu8_Data[0]) << 24U) | (static_cast<uint32_t>(opu8_Data[1]) << 16U) |
             (static_cast<uint32_t>(opu8_Data[2]) << 8U) | static_cast<uint32_t>(opu8_Data[3]);
   }

   [[nodiscard]] static constexpr uint64_t h_GetU64Big(const uint8_t * const opu8_Data)
   {
      return (static_cast<uint64_t>(h_GetU32Big(opu8_Data)) << 32U) | h_GetU32Big(&opu8_Data[4]);
   }

   static constexpr void h_SetU16Big(const uint16_t ou16_Value, uint8_t * const opu8_Data)
   {
      opu8_Data[0] = static_cast<uint8_t>(ou16_Value >> 8U);
      opu8_Data[1] = static_cast<uint8_t>(ou16_Value);
   }

   static constexpr void h_SetU32Big(const uint32_t ou32_Value, uint8_t * const opu8_Data)
   {
      opu8_Data[0] = static_cast<uint8_t>(ou32_Value >> 24U);
      opu8_Data[1] = static_cast<uint8_t>(ou32_Value >> 16U);
      opu8_Data[2] = static_cast<uint8_t>(ou32_Value >> 8U);
      opu8_Data[3] = static_cast<uint8_t>(ou32_Value);
   }

   static constexpr void h_SetU64Big(const uint64_t ou64_Value, uint8_t * const opu8_Data)
   {
      h_SetU32Big(static_cast<uint32_t>(ou64_Value >> 32U), opu8_Data);
      h_SetU32Big(static_cast<uint32_t>(ou64_Value), &opu8_Data[4]);
   }

   //little endian: least significant byte first (CANopen SDO payloads, the STW "low/high" CAN protocols,
   //the datapool hash inputs)
   [[nodiscard]] static constexpr uint16_t h_GetU16Little(const uint8_t * const opu8_Data)
   {
      return static_cast<uint16_t>((static_cast<uint16_t>(opu8_Data[1]) << 8U) | opu8_Data[0]);
   }

   [[nodiscard]] static constexpr uint32_t h_GetU32Little(const uint8_t * const opu8_Data)
   {
      return (static_cast<uint32_t>(opu8_Data[3]) << 24U) | (static_cast<uint32_t>(opu8_Data[2]) << 16U) |
             (static_cast<uint32_t>(opu8_Data[1]) << 8U) | static_cast<uint32_t>(opu8_Data[0]);
   }

   [[nodiscard]] static constexpr uint64_t h_GetU64Little(const uint8_t * const opu8_Data)
   {
      return (static_cast<uint64_t>(h_GetU32Little(&opu8_Data[4])) << 32U) | h_GetU32Little(opu8_Data);
   }

   static constexpr void h_SetU16Little(const uint16_t ou16_Value, uint8_t * const opu8_Data)
   {
      opu8_Data[0] = static_cast<uint8_t>(ou16_Value);
      opu8_Data[1] = static_cast<uint8_t>(ou16_Value >> 8U);
   }

   static constexpr void h_SetU32Little(const uint32_t ou32_Value, uint8_t * const opu8_Data)
   {
      opu8_Data[0] = static_cast<uint8_t>(ou32_Value);
      opu8_Data[1] = static_cast<uint8_t>(ou32_Value >> 8U);
      opu8_Data[2] = static_cast<uint8_t>(ou32_Value >> 16U);
      opu8_Data[3] = static_cast<uint8_t>(ou32_Value >> 24U);
   }

   static constexpr void h_SetU64Little(const uint64_t ou64_Value, uint8_t * const opu8_Data)
   {
      h_SetU32Little(static_cast<uint32_t>(ou64_Value), opu8_Data);
      h_SetU32Little(static_cast<uint32_t>(ou64_Value >> 32U), &opu8_Data[4]);
   }
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
