//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       STW error category for std::error_code (implementation)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OscErrorCategory.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Return the category name.
 *
 *   \return  Pointer to a null-terminated string "STW".
 */
//----------------------------------------------------------------------------------------------------------------------
const char * STWErrorCategory::name() const noexcept
{
   return "STW";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Return the human-readable message for a given error value.
 *
 *   \param[in]  ev  Integer error value (one of the Errc enumerators).
 *
 *   \return  Descriptive string, e.g. "C_NO_ERR", "C_CONFIG".
 */
//----------------------------------------------------------------------------------------------------------------------
std::string STWErrorCategory::message(const int ev) const
{
   switch (ev)
   {
   case static_cast<int>(Errc::success):
      return "C_NO_ERR";
   case static_cast<int>(Errc::unknown_err):
      return "C_UNKNOWN_ERR";
   case static_cast<int>(Errc::warn):
      return "C_WARN";
   case static_cast<int>(Errc::default_):
      return "C_DEFAULT";
   case static_cast<int>(Errc::busy):
      return "C_BUSY";
   case static_cast<int>(Errc::range):
      return "C_RANGE";
   case static_cast<int>(Errc::overflow):
      return "C_OVERFLOW";
   case static_cast<int>(Errc::rd_wr):
      return "C_RD_WR";
   case static_cast<int>(Errc::noact):
      return "C_NOACT";
   case static_cast<int>(Errc::com):
      return "C_COM";
   case static_cast<int>(Errc::config):
      return "C_CONFIG";
   case static_cast<int>(Errc::checksum):
      return "C_CHECKSUM";
   case static_cast<int>(Errc::timeout):
      return "C_TIMEOUT";
   default:
      return "C_UNKNOWN_ERR";
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Singleton accessor.
 *
 *   \return  Reference to the global STWErrorCategory instance.
 */
//----------------------------------------------------------------------------------------------------------------------
const STWErrorCategory & STWErrorCategory::Instance()
{
   static const STWErrorCategory c_Instance;
   return c_Instance;
}
