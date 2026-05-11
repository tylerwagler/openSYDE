//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       PEAK adapter — Windows-native PCANBasic interface (header)

   See cpp file for detailed description

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANPEAKADAPTERHPP
#define C_OSCCANPEAKADAPTERHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include "C_CanDispatcher.hpp"
#include "C_SclString.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
class C_OscCanPeakAdapter :
   public stw::can::C_CanDispatcher
{
private:
   C_OscCanPeakAdapter(const C_OscCanPeakAdapter & orc_Source);               //not implemented
   C_OscCanPeakAdapter & operator = (const C_OscCanPeakAdapter & orc_Source); //not implemented

   uint16_t mu16_Channel;            ///< 1..16 (mapped to PCAN_USBBUS1..16 internally)
   uint32_t mu32_BitrateKbits;       ///< configured bitrate; honored by CAN_Init()
   uint16_t mu16_PcanHandle;         ///< TPCANHandle value passed to PCANBasic (cached)
   bool mq_Initialized;              ///< true after CAN_Initialize succeeded

   static uint16_t mh_ChannelToPcanHandle(const uint16_t ou16_Channel);
   static uint16_t mh_BitrateKbitsToPcanBaud(const uint32_t ou32_BitrateKbits);

protected:
   virtual int32_t m_CAN_Read_Msg(stw::can::T_STWCAN_Msg_RX & orc_Message);

public:
   C_OscCanPeakAdapter(const uint16_t ou16_Channel, const uint32_t ou32_BitrateKbits);
   virtual ~C_OscCanPeakAdapter(void);

   //C_CanBase interface
   virtual int32_t CAN_Init(void);
   virtual int32_t CAN_Init(const int32_t os32_BitrateKBitS);
   virtual int32_t CAN_Exit(void);
   virtual int32_t CAN_Reset(void);
   virtual int32_t CAN_Send_Msg(const stw::can::T_STWCAN_Msg_TX & orc_Message);
   virtual int32_t CAN_Get_System_Time(uint64_t & oru64_SystemTimeUs) const;

   uint16_t GetChannel(void) const;
   uint32_t GetBitrateKbits(void) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
