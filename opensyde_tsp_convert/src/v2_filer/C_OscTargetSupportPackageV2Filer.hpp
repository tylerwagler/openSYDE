//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       File handler for target support package V2 file data.

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCTARGETSUPPORTPACKAGEV2FILER_HPP
#define C_OSCTARGETSUPPORTPACKAGEV2FILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include <string>
#include <system_error>
#include "C_OscTargetSupportPackageV2.hpp"
#include "C_OscXmlParser.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscTargetSupportPackageV2Filer
{
public:
   static std::error_code h_Load(stw::opensyde_core::C_OscTargetSupportPackageV2 & orc_TargetSupportPackage,
                                 const std::string & orc_Path);

private:
   static std::error_code mh_Load(C_OscTargetSupportPackageV2 & orc_TargetSupportPackage,
                                  C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_ParseApplication(C_OscTargetSupportPackageV2 & orc_TargetSupportPackage,
                                              C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_ParseHalcSection(C_OscTargetSupportPackageV2 & orc_TargetSupportPackage,
                                              C_OscXmlParserBase & orc_XmlParser);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
