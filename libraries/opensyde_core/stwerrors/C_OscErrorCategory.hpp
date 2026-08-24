//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       STW error category for std::error_code

   Defines Errc enum and STWErrorCategory for integrating the STW error
   code convention (C_NO_ERR, C_CONFIG, etc.) with std::error_code.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSC_ERROR_CATEGORY_HPP
#define C_OSC_ERROR_CATEGORY_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>
#include "stwtypes.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace errors
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/// Error codes matching the existing STW int32_t convention.
/// Implicitly convertible to std::error_code via ADL make_error_code.
enum class Errc
{
   success      = 0,
   unknown_err  = -1,
   warn         = -2,
   default_     = -3,   ///< "default" is a reserved keyword
   busy         = -4,
   range        = -5,
   overflow     = -6,
   rd_wr        = -7,
   noact        = -8,
   com          = -9,
   config       = -10,
   checksum     = -11,
   timeout      = -12,
};

/// std::error_category implementation for STW error codes.
class STWErrorCategory : public std::error_category
{
public:
   const char * name() const noexcept override;
   std::string message(const int ev) const override;

   /// Singleton access (error_category is typically used via pointer identity)
   static const STWErrorCategory & Instance();

private:
   STWErrorCategory() = default;
};

// -----------------------------------------------------------------------
// Free functions (ADL)

/// Convert Errc to std::error_code.
inline std::error_code make_error_code(const Errc e)
{
   return std::error_code(static_cast<int>(e), STWErrorCategory::Instance());
}

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
} // namespace errors
} // namespace stw

// Specialisation so that Errc works transparently with std::error_code.
namespace std
{
template <>
struct is_error_code_enum<stw::errors::Errc> : true_type
{
};
} // namespace std

#endif
