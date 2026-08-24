//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for signature
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSUPSIGNATUREFILER_HPP
#define C_OSCSUPSIGNATUREFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscSupSignatureFiler
{
public:
   static int32_t h_CreateSignatureFile(const std::string & orc_Path,
                                        const std::string & orc_Signature);
   static int32_t h_LoadSignatureFile(const std::string & orc_Path, std::string & orc_Signature);
   static std::string h_GetSignatureFileName(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
