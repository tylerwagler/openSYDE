//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       File handler for target support package file data.
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCTARGETSUPPORTPACKAGEFILER_HPP
#define C_OSCTARGETSUPPORTPACKAGEFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include <string>
#include <system_error>
#include "C_OscTargetSupportPackage.hpp"
#include "C_OscXmlParser.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscTargetSupportPackageFiler
{
public:
   static std::error_code h_Load(stw::opensyde_core::C_OscTargetSupportPackage & orc_TargetSupportPackage,
                                 std::string & orc_NodePath, const std::string & orc_Path);

private:
   static std::error_code mh_Load(C_OscTargetSupportPackage & orc_TargetSupportPackage, std::string & orc_NodePath,
                                  C_OscXmlParserBase & orc_XmlParser);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
