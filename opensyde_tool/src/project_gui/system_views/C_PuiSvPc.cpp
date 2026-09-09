//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       System view PC item data element (implementation)

   Holds the user-configured CAN adapter for this PC (SocketCAN ifname on Linux, PEAK
   channel+bitrate on Windows). Replaces the legacy DLL-type/path scheme.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstdint>
#include "C_SclChecksums.hpp"
#include "C_PuiSvPc.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
C_PuiSvPc::C_PuiSvPc(void) :
   C_PuiBsBox(),
   mc_AdapterConfig(C_OscCanAdapterConfig::h_GetPlatformDefault())
{
}

//----------------------------------------------------------------------------------------------------------------------
void C_PuiSvPc::CalcHash(uint32_t & oru32_HashValue) const
{
   stw::scl::C_SclChecksums::CalcCRC32(&this->mc_AdapterConfig.e_BackendKind,
                                       sizeof(this->mc_AdapterConfig.e_BackendKind), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(this->mc_AdapterConfig.c_ChannelId.c_str(),
                                       this->mc_AdapterConfig.c_ChannelId.length(), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->mc_AdapterConfig.u32_BitrateBps,
                                       sizeof(this->mc_AdapterConfig.u32_BitrateBps), oru32_HashValue);
   this->mc_ConnectionData.CalcHash(oru32_HashValue);

   C_PuiBsBox::CalcHash(oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
const C_PuiBsLineBase & C_PuiSvPc::GetConnectionData(void) const
{
   return this->mc_ConnectionData;
}

//----------------------------------------------------------------------------------------------------------------------
void C_PuiSvPc::SetConnectionData(const C_PuiBsLineBase & orc_Value)
{
   this->mc_ConnectionData = orc_Value;
}

//----------------------------------------------------------------------------------------------------------------------
const C_OscCanAdapterConfig & C_PuiSvPc::GetAdapterConfig(void) const
{
   return this->mc_AdapterConfig;
}

//----------------------------------------------------------------------------------------------------------------------
void C_PuiSvPc::SetAdapterConfig(const C_OscCanAdapterConfig & orc_Config)
{
   this->mc_AdapterConfig = orc_Config;
}

//----------------------------------------------------------------------------------------------------------------------
void C_PuiSvPc::SetBox(const C_PuiBsBox & orc_Box)
{
   this->c_UiPosition = orc_Box.c_UiPosition;
   this->f64_Height = orc_Box.f64_Height;
   this->f64_Width = orc_Box.f64_Width;
   this->f64_ZetOrder = orc_Box.f64_ZetOrder;
}
