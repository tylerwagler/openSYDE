//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle PEM content
   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSECURITYPEM_HPP
#define C_OSCSECURITYPEM_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>
#include <system_error>

#include "stwtypes.hpp"
#include "C_OscSecurityPemBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscSecurityPem :
   public C_OscSecurityPemBase
{
public:
   C_OscSecurityPem();

   static std::error_code h_ExtractModulusAndExponentFromFile(const std::string & orc_FileName,
                                                      std::vector<uint8_t> & orc_Modulus,
                                                      std::vector<uint8_t> & orc_Exponent,
                                                      std::string & orc_ErrorMessage);
   static std::error_code h_ExtractModulusAndExponent(const std::vector<uint8_t> & orc_PubKeyTextDecoded,
                                              std::vector<uint8_t> & orc_Modulus, std::vector<uint8_t> & orc_Exponent,
                                              std::string & orc_ErrorMessage);

protected:
   virtual std::error_code m_ReadPrivateKey(const std::vector<uint8_t> & orc_FileContent,
                                            std::string & orc_ErrorMessage);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
