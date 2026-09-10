#include "precomp_headers.hpp" //pre-compiled headers

#include <cstring>
#include "stw_can.hpp"
#include <system_error>

#include <cstdint>
#include "stwerrors.hpp"
#include "C_CanMonProtocol.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_CanMonProtocolCanOpen.hpp"
#include "C_CanMonProtocolGd.hpp"
#include "C_CanMonProtocolL2.hpp"
#include "C_CanMonProtocolJ1939.hpp"
#include "C_CanMonProtocolOpenSyde.hpp"
#include "C_CanMonProtocolUds.hpp"

#include <string>
#include <cstdio>
#include "C_SclStringUtil.hpp"
#include "C_SclIniFile.hpp"
#include "TglFile.hpp"
#include "TglUtils.hpp"

//---------------------------------------------------------------------------

using namespace stw::errors;
using namespace stw::cmon_protocol;
using namespace stw::scl;
using namespace stw::tgl;
using namespace stw::can;

//---------------------------------------------------------------------------

std::error_code C_CanMonProtocols::GetProtocolName(const e_CanMonL7Protocols oe_L7Protocol, std::string & orc_Description) const
{
   std::error_code c_Return = Errc::success;

   if (static_cast<int32_t>(oe_L7Protocol) < gs32_CMON_NUM_PROTOCOLS)
   {
      orc_Description = "L7-Protocol: " + mapc_Protocols[oe_L7Protocol]->GetProtocolName();
   }
   else
   {
      orc_Description = "Protocol: Unknown";
      c_Return = Errc::range;
   }
   return c_Return;
}

//---------------------------------------------------------------------------

std::string C_CanMonProtocols::MessageToString(const T_STWCAN_Msg_TX & orc_Msg) const
{
   T_STWCAN_Msg_RX c_Msg;

   c_Msg.u32_ID = orc_Msg.u32_ID;
   c_Msg.u8_XTD = orc_Msg.u8_XTD;
   c_Msg.u8_DLC = orc_Msg.u8_DLC;
   c_Msg.u8_RTR = orc_Msg.u8_RTR;
   (void)std::memcpy(c_Msg.au8_Data, orc_Msg.au8_Data, 8U);
   c_Msg.u64_TimeStamp = 0;
   return this->MessageToString(c_Msg);
}

//---------------------------------------------------------------------------

std::string C_CanMonProtocols::MessageToStringLog(const T_STWCAN_Msg_TX & orc_Msg) const
{
   T_STWCAN_Msg_RX c_Msg;

   c_Msg.u32_ID = orc_Msg.u32_ID;
   c_Msg.u8_XTD = orc_Msg.u8_XTD;
   c_Msg.u8_DLC = orc_Msg.u8_DLC;
   c_Msg.u8_RTR = orc_Msg.u8_RTR;
   (void)std::memcpy(c_Msg.au8_Data, orc_Msg.au8_Data, 8);
   c_Msg.u64_TimeStamp = 0U;
   return this->MessageToStringLog(c_Msg);
}

//---------------------------------------------------------------------------
//for logging to file; the layer 2 part has a different format than the one for the screen
// (mainly separating ";" inserted for easier parsing)
std::string C_CanMonProtocols::MessageToStringLog(const T_STWCAN_Msg_RX & orc_Msg) const
{
   const uint16_t u16_MAX_CHARS_RAW_DATA = 73U;

   std::string c_Text;
   int32_t s32_Index;
   uint8_t u8_Len;
   bool q_Decimal = GetDecimalMode();

   u8_Len = (orc_Msg.u8_DLC > 8) ? static_cast<uint8_t>(8U) : orc_Msg.u8_DLC;
   if (q_Decimal == true)
   {
      c_Text = PrintFormattedCompat("%8d; %s; %s; %d; ", orc_Msg.u32_ID, (orc_Msg.u8_XTD == 1U) ? "29B" : "11B",
                                  (orc_Msg.u8_RTR == 1U) ? "RTR" : "STD", orc_Msg.u8_DLC);
      //reserve the full line once so the per-byte appends below never reallocate
      c_Text.reserve(u16_MAX_CHARS_RAW_DATA + 16U);
      for (s32_Index = 0; s32_Index < u8_Len; s32_Index++)
      {
         char acn_Byte[8];
         (void)std::snprintf(acn_Byte, sizeof(acn_Byte), "%3d; ", orc_Msg.au8_Data[s32_Index]);
         c_Text += acn_Byte;
      }
   }
   else
   {
      c_Text = PrintFormattedCompat("0x%08X; %s; %s; %d; ", orc_Msg.u32_ID, (orc_Msg.u8_XTD == 1U) ? "29B" : "11B",
                                  (orc_Msg.u8_RTR == 1U) ? "RTR" : "STD", orc_Msg.u8_DLC);
      c_Text.reserve(u16_MAX_CHARS_RAW_DATA + 16U);
      for (s32_Index = 0; s32_Index < u8_Len; s32_Index++)
      {
         char acn_Byte[8];
         (void)std::snprintf(acn_Byte, sizeof(acn_Byte), "0x%02X; ", orc_Msg.au8_Data[s32_Index]);
         c_Text += acn_Byte;
      }
   }
   for (; s32_Index < 8; s32_Index++)
   {
      c_Text += ";";
   }

   if (me_ActiveProtocol != eCMON_L7_PROTOCOL_NONE)
   {
      if (c_Text.length() < u16_MAX_CHARS_RAW_DATA)
      {
         c_Text.append(static_cast<size_t>(u16_MAX_CHARS_RAW_DATA - c_Text.length()), ' ');
      }
      c_Text += (MessageToString(orc_Msg) + ";");
   }
   return c_Text;
}

//---------------------------------------------------------------------------

std::string C_CanMonProtocols::MessageToString(const T_STWCAN_Msg_RX & orc_Msg) const
{
   std::string c_Text;

   c_Text = "";
   if (static_cast<int32_t>(me_ActiveProtocol) < gs32_CMON_NUM_PROTOCOLS)
   {
      c_Text = mapc_Protocols[me_ActiveProtocol]->MessageToString(orc_Msg);
   }
   if (c_Text == "")
   {
      //get L2 representation
      c_Text = mapc_Protocols[eCMON_L7_PROTOCOL_NONE]->MessageToString(orc_Msg);
   }
   return c_Text;
}

//---------------------------------------------------------------------------

std::string C_CanMonProtocols::MessageToString(const T_STWCAN_Msg_RX & orc_Message, const uint32_t ou32_Count) const
{
   std::string c_Text;
   std::string c_Help;

   c_Text = this->MessageToString(orc_Message);
   //count
   c_Help = PrintFormattedCompat("%7d  ", ou32_Count);
   return (c_Help + c_Text);
}

//---------------------------------------------------------------------------

C_CanMonProtocols::C_CanMonProtocols() :
   mq_Decimal(false),
   me_ActiveProtocol(eCMON_L7_PROTOCOL_NONE)
{
   //set shortcut pointers to be able to iterate:
   mapc_Protocols[eCMON_L7_PROTOCOL_NONE]      = &mc_ProtocolL2;
   mapc_Protocols[eCMON_L7_PROTOCOL_CAN_OPEN]  = &mc_ProtocolCanOpen;
   mapc_Protocols[eCMON_L7_PROTOCOL_GD]        = &mc_ProtocolGd;
   mapc_Protocols[eCMON_L7_PROTOCOL_J1939]     = &mc_ProtocolJ1939;
   mapc_Protocols[eCMON_L7_PROTOCOL_OPEN_SYDE] = &mc_ProtocolOpenSyde;
   mapc_Protocols[eCMON_L7_PROTOCOL_CAN_TP]    = &mc_ProtocolL2; // L2 interpretation
   mapc_Protocols[eCMON_L7_PROTOCOL_UDS]       = &mc_ProtocolUds;
}

//---------------------------------------------------------------------------

C_CanMonProtocols::~C_CanMonProtocols(void)
{
}

//---------------------------------------------------------------------------

e_CanMonL7Protocols C_CanMonProtocols::GetProtocolMode(void) const
{
   return me_ActiveProtocol;
}

//---------------------------------------------------------------------------

std::error_code C_CanMonProtocols::SetProtocolMode(const e_CanMonL7Protocols oe_L7Protocol)
{
   me_ActiveProtocol = oe_L7Protocol;
   return Errc::success;
}

//---------------------------------------------------------------------------

bool C_CanMonProtocols::GetDecimalMode(void) const
{
   return mq_Decimal;
}

//---------------------------------------------------------------------------

std::error_code C_CanMonProtocols::SetDecimalMode(const bool oq_Decimal)
{
   int32_t s32_Loop;

   mq_Decimal = oq_Decimal;

   //set in all  protocol instances:
   for (s32_Loop = 0; s32_Loop < gs32_CMON_NUM_PROTOCOLS; s32_Loop++)
   {
      mapc_Protocols[s32_Loop]->SetDecimal(oq_Decimal);
   }

   return Errc::success;
}

//-----------------------------------------------------------------------------
/*!
   \brief   Write protocol specific parameters to INI

   Write protocol specific parameters for all known protocols to a .INI file.
   Can be read back later with "LoadProtocolParametersFromINI".
   All parameters are written regardless of the active L7-protocol mode.
   No leading ".\\" will be added if no full path is given !

   \param[in]     orc_FileName    full file path to the .ini file
   \param[in]     orc_Section     INI file section to write to

   \return
   C_NO_ERR  -> written
   C_RD_WR   -> could not write
*/
//-----------------------------------------------------------------------------
std::error_code C_CanMonProtocols::SaveProtocolParametersToIni(const std::string & orc_FileName,
                                                       const std::string & orc_Section)
const
{
   std::error_code c_Return = Errc::success;

   try
   {
      C_SclIniFile c_IniFile(orc_FileName);
      //The pre-migration code summed the per-protocol results and mapped any
      //non-zero total to C_RD_WR. Record the first failure instead, but keep
      //writing every protocol so behaviour is unchanged.
      for (int32_t s32_Loop = 0; s32_Loop < gs32_CMON_NUM_PROTOCOLS; s32_Loop++)
      {
         if (mapc_Protocols[s32_Loop]->SaveParamsToIni(c_IniFile, orc_Section))
         {
            c_Return = Errc::rd_wr;
         }
      }
   }
   catch (...)
   {
      c_Return = Errc::rd_wr;
   }
   return c_Return;
}

//-----------------------------------------------------------------------------
/*!
   \brief   Read protocol specific parameters from INI

   Load protocol specific parameters for all known protocols from a .INI file.
   All parameters are read regardless of the active L7-protocol mode.
   No leading ".\\" will be added if no full path is given !

   \param[in]     orc_FileName    full file path to the .ini file
   \param[in]     orc_Section     INI file section to read from

   \return
   C_NO_ERR  -> written
   C_RD_WR   -> could not read (file does not exist)
*/
//-----------------------------------------------------------------------------
std::error_code C_CanMonProtocols::LoadProtocolParametersFromIni(const std::string & orc_FileName,
                                                         const std::string & orc_Section)
const
{
   std::error_code c_Return = Errc::success;

   if (TglFileExists(orc_FileName) == false)
   {
      c_Return = Errc::rd_wr;
   }
   else
   {
      C_SclIniFile c_IniFile(orc_FileName);

      //see SaveProtocolParametersToIni: any per-protocol failure maps to rd_wr,
      //and every protocol is still read
      for (int32_t s32_Loop = 0; s32_Loop < gs32_CMON_NUM_PROTOCOLS; s32_Loop++)
      {
         if (mapc_Protocols[s32_Loop]->LoadParamsFromIni(c_IniFile, orc_Section))
         {
            c_Return = Errc::rd_wr;
         }
      }
   }
   return c_Return;
}

//-----------------------------------------------------------------------------
/*!
   \brief   Return micro-second time stamp as string

   \param[in]     ou64_TimeStampUs  timestamps [us]
   \param[in]     oq_LeftFillBlanks true: fill left with " "
                                    false: fill left with "0"

   \return
   Formatted timestamp ("mmmmmmmmmm.uuu").
*/
//-----------------------------------------------------------------------------
std::string C_CanMonProtocols::FormatTimeStamp(const uint64_t ou64_TimeStampUs, const bool oq_LeftFillBlanks)
{
   std::string c_Time;

   if (oq_LeftFillBlanks == false)
   {
      c_Time = PrintFormattedCompat("%013llu", ou64_TimeStampUs);
   }
   else
   {
      if (ou64_TimeStampUs >= 1000)
      {
         c_Time = PrintFormattedCompat("%13llu", ou64_TimeStampUs);
      }
      else
      {
         //we need at least 4 characters so we don't get strings list " . 12" but "0.012"
         c_Time = PrintFormattedCompat("         %04llu", ou64_TimeStampUs);
      }
   }
   (void)InsertCompat(c_Time, ".", 11);
   return c_Time;
}

//---------------------------------------------------------------------------
