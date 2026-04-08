//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscCanSignal (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanSignalFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "stwerrors.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Module Globals ------------------------------------------------------------------------------------------------ */

namespace
{
const C_OscJsonUtil::T_EnumEntry<C_OscCanSignal::E_ByteOrderType> hac_ByteOrderTable[] = {
   { C_OscCanSignal::eBYTE_ORDER_INTEL,    "intel"    },
   { C_OscCanSignal::eBYTE_ORDER_MOTOROLA, "motorola" }
};

const C_OscJsonUtil::T_EnumEntry<C_OscCanSignal::E_MultiplexerType> hac_MultiplexerTable[] = {
   { C_OscCanSignal::eMUX_DEFAULT,            "default"            },
   { C_OscCanSignal::eMUX_MULTIPLEXER_SIGNAL, "multiplexer_signal" },
   { C_OscCanSignal::eMUX_MULTIPLEXED_SIGNAL, "multiplexed_signal" }
};
} // namespace

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscCanSignalFiler::save(const C_OscCanSignal & orc_Signal)
{
   QJsonObject c_Json;

   c_Json["byte_order"] = C_OscJsonUtil::h_EnumToString(orc_Signal.e_ComByteOrder, hac_ByteOrderTable);
   c_Json["bit_length"] = static_cast<qint64>(orc_Signal.u16_ComBitLength);
   c_Json["bit_start"] = static_cast<qint64>(orc_Signal.u16_ComBitStart);
   c_Json["data_element_index"] = static_cast<qint64>(orc_Signal.u32_ComDataElementIndex);

   c_Json["multiplexer_type"] = C_OscJsonUtil::h_EnumToString(orc_Signal.e_MultiplexerType, hac_MultiplexerTable);
   c_Json["multiplex_value"] = static_cast<qint64>(orc_Signal.u16_MultiplexValue);

   c_Json["can_open_object_dictionary_index"] =
      static_cast<qint64>(orc_Signal.u16_CanOpenManagerObjectDictionaryIndex);
   c_Json["can_open_object_dictionary_sub_index"] =
      static_cast<qint64>(orc_Signal.u8_CanOpenManagerObjectDictionarySubIndex);

   c_Json["j1939_suspect_parameter_number"] = static_cast<qint64>(orc_Signal.u32_J1939SuspectParameterNumber);

   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscCanSignalFiler::load(const QJsonObject & orc_Json, C_OscCanSignal & orc_Signal)
{
   QString c_EnumStr;

   int32_t s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "byte_order", c_EnumStr);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_StringToEnum(c_EnumStr, hac_ByteOrderTable, orc_Signal.e_ComByteOrder);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetU16(orc_Json, "bit_length", orc_Signal.u16_ComBitLength);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU16(orc_Json, "bit_start", orc_Signal.u16_ComBitStart);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "data_element_index", orc_Signal.u32_ComDataElementIndex);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetString(orc_Json, "multiplexer_type", c_EnumStr);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_StringToEnum(c_EnumStr, hac_MultiplexerTable, orc_Signal.e_MultiplexerType);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetU16(orc_Json, "multiplex_value", orc_Signal.u16_MultiplexValue);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetU16(orc_Json, "can_open_object_dictionary_index",
                                        orc_Signal.u16_CanOpenManagerObjectDictionaryIndex);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }
   s32_Result = C_OscJsonUtil::h_GetU8(orc_Json, "can_open_object_dictionary_sub_index",
                                       orc_Signal.u8_CanOpenManagerObjectDictionarySubIndex);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   s32_Result = C_OscJsonUtil::h_GetU32(orc_Json, "j1939_suspect_parameter_number",
                                        orc_Signal.u32_J1939SuspectParameterNumber);
   if (s32_Result != C_NO_ERR)
   {
      return s32_Result;
   }

   return C_NO_ERR;
}
