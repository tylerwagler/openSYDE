//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for application related information (implementation)

   Data class for application related information

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "stwerrors.hpp"

#include "C_OscNodeApplication.hpp"

#include "C_OscFilerUtil.hpp"
#include "C_OscHashUtil.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_SclChecksums.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::errors;
using namespace stw::opensyde_core;
using namespace stw::scl;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */

/* -- Module Global Variables
 * ---------------------------------------------------------------------------------------
 */

namespace
{
const C_OscFilerUtil::EnumEntry<C_OscNodeApplication::E_Type> mac_APPLICATION_TYPE_TABLE[] = {
   {C_OscNodeApplication::eBINARY, "binary"},
   {C_OscNodeApplication::ePROGRAMMABLE_APPLICATION, "programming_application"},
   {C_OscNodeApplication::ePARAMETER_SET_HALC, "parameter_set_halc"}
};
}

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscNodeApplication::C_OscNodeApplication(void)
    : e_Type(eBINARY), c_Name(""), c_Comment(""), q_Active(true),
      u8_ProcessId(0), c_ProjectPath(""), c_IdeCall(""),
      c_CodeGeneratorPath(""), c_GeneratePath(""), u16_GenCodeVersion(0) {
  // default: one output file
  c_ResultPaths.resize(1);
  c_ResultPaths[0] = "";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.
   It is not endian-safe, so it should only be used on the same system it is
   created on.

   \param[in,out] oru32_HashValue    Hash value with initial [in] value and
   result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeApplication::CalcHash(uint32_t &oru32_HashValue) const {
   hash_util::CalcHashMembers(oru32_HashValue,
                              this->e_Type, this->c_Name, this->c_Comment,
                              this->q_Active, this->u8_ProcessId,
                              this->c_ProjectPath, this->c_IdeCall,
                              this->c_CodeGeneratorPath, this->c_GeneratePath,
                              this->u16_GenCodeVersion, this->c_ResultPaths);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Transform application type to string

   \param[in] ore_Application Application type

   \return
   Stringified application type
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscNodeApplication::h_ApplicationToString(
    const C_OscNodeApplication::E_Type &ore_Application) {
  return C_OscFilerUtil::h_EnumToString(ore_Application, mac_APPLICATION_TYPE_TABLE);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Transform string to application type

   \param[in]  orc_String String to interpret
   \param[out] ore_Type   Application type
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeApplication::h_StringToApplication(
    const QString &orc_String, C_OscNodeApplication::E_Type &ore_Type) {
  if (C_OscFilerUtil::h_StringToEnum(orc_String, mac_APPLICATION_TYPE_TABLE, ore_Type) != stw::errors::C_NO_ERR) {
    ore_Type = C_OscNodeApplication::eBINARY;
    osc_write_log_warning(
        "Loading node definition",
        "Invalid value for application type. Unknown type: " + orc_String +
            ". "
            "Type is replaced with: " +
            h_ApplicationToString(C_OscNodeApplication::eBINARY));
  }
}
