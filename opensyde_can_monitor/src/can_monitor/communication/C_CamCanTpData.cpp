//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       CAN-TP (ISO 15765-2) data structures (implementation)

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_CamCanTpData.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamCanTpSession::C_CamCanTpSession() :
   u32_SourceCanId(0U),
   e_Status(eIDLE),
   u8_ExpectedSn(0U),
   u16_TotalLength(0U),
   u16_BytesReceived(0U),
   u64_LastFrameTimeUs(0U),
   u32_FirstFrameTimeMs(0U),
   q_Error(false),
   c_ErrorDescription("")
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Reset session to idle state
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpSession::Reset(void)
{
   this->e_Status = eIDLE;
   this->u8_ExpectedSn = 0U;
   this->u16_TotalLength = 0U;
   this->u16_BytesReceived = 0U;
   this->c_Buffer.clear();
   this->u64_LastFrameTimeUs = 0U;
   this->u32_FirstFrameTimeMs = 0U;
   this->q_Error = false;
   this->c_ErrorDescription = "";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_CamCanTpResult::C_CamCanTpResult() :
   q_IsTpFrame(false),
   u8_PciType(0U),
   u8_SequenceNumber(0U),
   u16_TotalMessageLength(0U),
   u8_BlockSize(0U),
   u8_SeparationTime(0U),
   u32_SessionKey(0U),
   u8_SessionCount(0U),
   q_ReassemblyComplete(false),
   q_Error(false),
   c_ErrorDescription("")
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Clear result to default state
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CamCanTpResult::Clear(void)
{
   this->q_IsTpFrame = false;
   this->u8_PciType = 0U;
   this->u8_SequenceNumber = 0U;
   this->u16_TotalMessageLength = 0U;
   this->u8_BlockSize = 0U;
   this->u8_SeparationTime = 0U;
   this->u32_SessionKey = 0U;
   this->u8_SessionCount = 0U;
   this->q_ReassemblyComplete = false;
   this->q_Error = false;
   this->c_ErrorDescription = "";
}
