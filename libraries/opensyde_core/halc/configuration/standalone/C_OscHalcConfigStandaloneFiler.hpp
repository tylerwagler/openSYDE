//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for standalone HALC config
   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCCONFIGSTANDALONEFILER_HPP
#define C_OSCHALCCONFIGSTANDALONEFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>

#include "C_OscXmlParser.hpp"
#include "C_OscHalcConfigStandalone.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcConfigStandaloneFiler
{
public:
   C_OscHalcConfigStandaloneFiler(void);

   static std::error_code h_LoadFileStandalone(C_OscHalcConfigStandalone & orc_IoData, const std::string & orc_Path);
   static std::error_code h_SaveFileStandalone(const C_OscHalcConfigStandalone & orc_IoData,
                                               const std::string & orc_Path);
   static std::error_code h_LoadDataStandalone(C_OscHalcConfigStandalone & orc_IoData,
                                               C_OscXmlParserBase & orc_XmlParser);
   static std::error_code h_SaveDataStandalone(const C_OscHalcConfigStandalone & orc_IoData,
                                               C_OscXmlParserBase & orc_XmlParser);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
