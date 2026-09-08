//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Translate L2 CAN message to textual interpretation

   \copyright   Copyright 2002 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CCMONPROTOCOLBASEHPP
#define CCMONPROTOCOLBASEHPP

#include <system_error>

#include "C_OscErrorCategory.hpp"

#include "stwtypes.hpp"
#include "stw_can.hpp"
#include <string>
#include "C_SclIniFile.hpp"

//----------------------------------------------------------------------------------------------------------------------

namespace stw
{
namespace cmon_protocol
{
//----------------------------------------------------------------------------------------------------------------------

//Base class for all protocol interpretations
//Provides basic functions to help converting a CAN message structure into text.
//Also provides an abstract interface for protocol-specific conversions.
class C_CanMonProtocolBase
{
protected:
   bool mq_Decimal;

   //general:
   std::string m_GetValueDecHex(const uint32_t ou32_Value) const;
   std::string m_GetByteAsStringFormat(const uint8_t ou8_Value) const;
   std::string m_GetWordAsStringFormat(const uint16_t ou16_Value) const;
   static uint32_t mh_BytesToDwordLowHigh(const uint8_t oau8_Bytes[4]);
   static uint16_t mh_BytesToWordLowHigh(const uint8_t oau8_Bytes[2]);
   static uint16_t mh_BytesToWordHighLow(const uint8_t oau8_Bytes[2]);
   static uint32_t mh_BytesToDwordHighLow(const uint8_t oau8_Bytes[4]);
   static std::string mh_SerialNumberToString(const uint8_t * const opu8_SerialNumber);

public:
   C_CanMonProtocolBase(void);
   virtual ~C_CanMonProtocolBase(void);

   //-----------------------------------------------------------------------------
   /*!
      \brief    Return CAN message in string interpretation

      Actual conversion routine to be overloaded by inheriting class

      \param[in]       orc_Msg    CAN message to convert to string

      \return
      CAN message in string representation
   */
   //-----------------------------------------------------------------------------
   virtual std::string MessageToString(const stw::can::T_STWCAN_Msg_RX & orc_Msg) const = 0;

   //-----------------------------------------------------------------------------
   /*!
      \brief    Return string representation of protocol name

      To be overloaded by inheriting class.

      \return
      string representation of protocol name
   */
   //-----------------------------------------------------------------------------
   virtual std::string GetProtocolName(void) const = 0;

   virtual void SetDecimal(const bool oq_Decimal);

   //overload if we have something to save (use protocol abbreviation as prefix for directives !)
   virtual std::error_code SaveParamsToIni(stw::scl::C_SclIniFile & orc_IniFile, const std::string & orc_Section);
   virtual std::error_code LoadParamsFromIni(stw::scl::C_SclIniFile & orc_IniFile, const std::string & orc_Section);
};

//----------------------------------------------------------------------------------------------------------------------
}
}

#endif
