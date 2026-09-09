//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       std::error_code support for hex file errors

   C_HexFile historically reported errors as a packed uint32_t: the high nibble
   identified the error and the low 28 bits carried context (a line number for
   the syntax-class errors). That is two pieces of information in one integer,
   which is why callers had to mask with ERR_MASK before comparing.

   std::error_code is deliberately payload-free, so the two are separated here:
   the identity becomes a HexFileErrc in this category, and the line number is
   retrieved from C_HexFile::GetLastErrorLineNumber(). That mirrors how the
   offending address for a record overlay was already exposed, via
   GetLastOverlayErrorAddress().

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_HEXFILEERRORCATEGORYHPP
#define C_HEXFILEERRORCATEGORYHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>
#include <system_error>
#include <type_traits>

#include <cstdint>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace hex_file
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/// Hex file error identities. The values match the high nibble of the legacy
/// packed codes shifted down, so the mapping is order-independent and obvious.
enum class HexFileErrc
{
   success            = 0,  ///< NO_ERR
   no_eof_record      = 1,  ///< WRN_NO_EOF_RECORD, a warning: data is usable
   record_overlay     = 2,  ///< WRN_RECORD_OVERLAY, a warning: see GetLastOverlayErrorAddress()
   hexline_syntax     = 8,  ///< ERR_HEXLINE_SYNTAX, see GetLastErrorLineNumber()
   hexline_checksum   = 9,  ///< ERR_HEXLINE_CHECKSUM, see GetLastErrorLineNumber()
   hexline_command    = 10, ///< ERR_HEXLINE_COMMAND, see GetLastErrorLineNumber()
   no_data            = 11, ///< ERR_NO_DATA
   not_enough_memory  = 14, ///< ERR_NOT_ENOUGH_MEMORY
   cant_open_file     = 15, ///< ERR_CANT_OPEN_FILE
   undefined          = 255 ///< unrecognised legacy code
};

/// std::error_category implementation for hex file errors.
class C_HexFileErrorCategory :
   public std::error_category
{
public:
   const char * name() const noexcept override;
   std::string message(const int ox_Value) const override;

   static const C_HexFileErrorCategory & h_Instance(void);

private:
   C_HexFileErrorCategory() = default;
};

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Function Prototypes ------------------------------------------------------------------------------------------- */

/// Convert HexFileErrc to std::error_code (found via ADL).
inline std::error_code make_error_code(const HexFileErrc oe_Error)
{
   return std::error_code(static_cast<int>(oe_Error), C_HexFileErrorCategory::h_Instance());
}

/// Split a legacy packed uint32_t into its error identity.
/// The low 28 bits are context, not identity, and are dropped here — callers
/// read them back from the C_HexFile accessors.
std::error_code h_HexFileErrorFromLegacy(const uint32_t ou32_LegacyCode);

/// Extract the context bits (line number) from a legacy packed code.
uint32_t h_HexFileLineFromLegacy(const uint32_t ou32_LegacyCode);

/* -- Implementation ------------------------------------------------------------------------------------------------ */
}
}

namespace std
{
/// Enables implicit conversion from HexFileErrc to std::error_code.
template <>
struct is_error_code_enum<stw::hex_file::HexFileErrc> :
   true_type
{
};
}

#endif
