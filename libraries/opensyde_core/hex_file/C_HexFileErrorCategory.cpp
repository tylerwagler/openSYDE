//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       std::error_code support for hex file errors

   see header in .hpp file for details.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <string>
#include <system_error>

#include "stwtypes.hpp"
#include "C_HexFileErrorCategory.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::hex_file;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/// High nibble of the legacy packed code identifies the error.
static const uint32_t mhu32_LEGACY_ERR_MASK = 0xF0000000UL;
/// Low 28 bits carry context (a line number for the syntax-class errors).
static const uint32_t mhu32_LEGACY_CONTEXT_MASK = 0x0FFFFFFFUL;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Category name

   \return
   Category name
*/
//----------------------------------------------------------------------------------------------------------------------
const char * C_HexFileErrorCategory::name() const noexcept
{
   return "stw.hex_file";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Human readable message for an error value

   Deliberately describes the error only. Where a line number or address applies
   it is retrieved from the C_HexFile instance that produced the error, because
   an error_category has no access to that context.

   \param[in]  ox_Value    Error value

   \return
   Message text
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_HexFileErrorCategory::message(const int ox_Value) const
{
   std::string c_Text;

   switch (static_cast<HexFileErrc>(ox_Value))
   {
   case HexFileErrc::success:
      c_Text = "No error";
      break;
   case HexFileErrc::no_eof_record:
      c_Text = "No EOF record";
      break;
   case HexFileErrc::record_overlay:
      c_Text = "Address used twice";
      break;
   case HexFileErrc::hexline_syntax:
      c_Text = "Wrong syntax in hex line";
      break;
   case HexFileErrc::hexline_checksum:
      c_Text = "Wrong checksum in hex line";
      break;
   case HexFileErrc::hexline_command:
      c_Text = "Wrong command in hex line";
      break;
   case HexFileErrc::no_data:
      c_Text = "No data";
      break;
   case HexFileErrc::not_enough_memory:
      c_Text = "Out of memory";
      break;
   case HexFileErrc::cant_open_file:
      c_Text = "File not found";
      break;
   case HexFileErrc::undefined:
   default:
      c_Text = "Undefined error";
      break;
   }
   return c_Text;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Singleton access

   \return
   The category instance
*/
//----------------------------------------------------------------------------------------------------------------------
const C_HexFileErrorCategory & C_HexFileErrorCategory::h_Instance(void)
{
   static const C_HexFileErrorCategory hc_Instance;

   return hc_Instance;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert a legacy packed code to an error_code

   \param[in]  ou32_LegacyCode   Legacy packed code as returned by the old API

   \return
   Corresponding error_code in the hex file category
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code stw::hex_file::h_HexFileErrorFromLegacy(const uint32_t ou32_LegacyCode)
{
   HexFileErrc e_Error;

   if (ou32_LegacyCode == 0UL)
   {
      e_Error = HexFileErrc::success;
   }
   else
   {
      //identity lives in the high nibble; the rest is context
      switch (ou32_LegacyCode & mhu32_LEGACY_ERR_MASK)
      {
      case 0x10000000UL:
         e_Error = HexFileErrc::no_eof_record;
         break;
      case 0x20000000UL:
         e_Error = HexFileErrc::record_overlay;
         break;
      case 0x80000000UL:
         e_Error = HexFileErrc::hexline_syntax;
         break;
      case 0x90000000UL:
         e_Error = HexFileErrc::hexline_checksum;
         break;
      case 0xA0000000UL:
         e_Error = HexFileErrc::hexline_command;
         break;
      case 0xB0000000UL:
         e_Error = HexFileErrc::no_data;
         break;
      case 0xE0000000UL:
         e_Error = HexFileErrc::not_enough_memory;
         break;
      case 0xF0000000UL:
         e_Error = HexFileErrc::cant_open_file;
         break;
      default:
         e_Error = HexFileErrc::undefined;
         break;
      }
   }
   return make_error_code(e_Error);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Extract the context bits from a legacy packed code

   Meaningful only for the syntax-class errors, where it is a line number.

   \param[in]  ou32_LegacyCode   Legacy packed code

   \return
   Context value (line number), 0 when the code carries none
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t stw::hex_file::h_HexFileLineFromLegacy(const uint32_t ou32_LegacyCode)
{
   uint32_t u32_Line = 0UL;

   switch (ou32_LegacyCode & mhu32_LEGACY_ERR_MASK)
   {
   case 0x80000000UL: //syntax
   case 0x90000000UL: //checksum
   case 0xA0000000UL: //command
      u32_Line = ou32_LegacyCode & mhu32_LEGACY_CONTEXT_MASK;
      break;
   default:
      u32_Line = 0UL;
      break;
   }
   return u32_Line;
}
