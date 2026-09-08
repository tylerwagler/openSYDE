//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Export initialization module for DPD and DPH.

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCEXPORTOSYINIT_HPP
#define C_OSCEXPORTOSYINIT_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>
#include <vector>
#include <system_error>

#include "stwtypes.hpp"

#include <string>
#include <vector>
#include "C_OscNode.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
///Code exporter for server side DPD and DPH init structures
class C_OscExportOsyInit
{
public:
   static std::string h_GetFileName(void);
   static std::error_code h_CreateSourceCode(const std::string & orc_FilePath, const C_OscNode & orc_Node,
                                             const bool oq_RunsDpd, const uint16_t ou16_ApplicationIndex,
                                             const std::string & orc_ExportToolInfo = "");

protected:
   static bool mh_IsDpdInitRequired(const C_OscNodeComInterfaceSettings & orc_Settings);
   static bool mh_IsDpKnownToApp(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ApplicationIndex,
                                 const C_OscNode & orc_Node, const bool oq_RunsDpd);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
