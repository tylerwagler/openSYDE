//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       XML persistence of a node's UDS configuration

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODEUDSCONFIGFILER_HPP
#define C_OSCNODEUDSCONFIGFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>

#include "C_OscNodeUdsConfig.hpp"
#include "C_OscXmlParser.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscNodeUdsConfigFiler
{
public:
   static void h_SaveData(const C_OscNodeUdsConfig & orc_Config, C_OscXmlParserBase & orc_XmlParser);
   [[nodiscard]] static std::error_code h_LoadData(C_OscNodeUdsConfig & orc_Config, C_OscXmlParserBase & orc_XmlParser);

private:
   static void mh_SaveIdList(const char * const opcn_ListName, const char * const opcn_ItemName,
                             const std::vector<uint8_t> & orc_Ids, C_OscXmlParserBase & orc_XmlParser);
   [[nodiscard]] static std::error_code mh_LoadIdList(const char * const opcn_ListName, const char * const opcn_ItemName,
                                                      std::vector<uint8_t> & orc_Ids, C_OscXmlParserBase & orc_XmlParser);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
