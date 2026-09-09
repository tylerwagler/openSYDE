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
#include <cstdint>

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

/// Bridge a legacy int32_t STW error code into an error_code.
///
/// Most of the codebase still returns the bare int32_t convention, so migrated
/// code has to consume unmigrated callees. The 13 STW constants are numerically
/// identical to the Errc enumerators and form a closed set, which makes the
/// mapping exact — but writing `static_cast<Errc>(...)` at every call site hides
/// what is happening and would silently produce a bogus enumerator if a callee
/// ever returned something outside that set. This names the conversion and
/// funnels anything unrecognised to unknown_err instead.
inline std::error_code make_error_code_from_stw(const int32_t os32_StwError)
{
   Errc e_Error;

   switch (os32_StwError)
   {
   case 0:    e_Error = Errc::success;     break;
   case -1:   e_Error = Errc::unknown_err; break;
   case -2:   e_Error = Errc::warn;        break;
   case -3:   e_Error = Errc::default_;    break;
   case -4:   e_Error = Errc::busy;        break;
   case -5:   e_Error = Errc::range;       break;
   case -6:   e_Error = Errc::overflow;    break;
   case -7:   e_Error = Errc::rd_wr;       break;
   case -8:   e_Error = Errc::noact;       break;
   case -9:   e_Error = Errc::com;         break;
   case -10:  e_Error = Errc::config;      break;
   case -11:  e_Error = Errc::checksum;    break;
   case -12:  e_Error = Errc::timeout;     break;
   default:   e_Error = Errc::unknown_err; break;
   }
   return make_error_code(e_Error);
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
