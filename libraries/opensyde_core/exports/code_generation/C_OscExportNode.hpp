//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Export code of an openSYDE node (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCEXPORTNODE_HPP
#define C_OSCEXPORTNODE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>

#include "stwtypes.hpp"
#include "C_OscNode.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
///Code exporter for server code: Datapools, comm configuration, DPD and DPH initialization
class C_OscExportNode
{
public:
   static std::error_code h_CreateSourceCode(const C_OscNode & orc_Node, const uint16_t ou16_ApplicationIndex,
                                             const std::string & orc_Path,
                                             std::vector<std::string> & orc_Files,
                                             const std::string & orc_ExportToolName = "",
                                             const std::string & orc_ExportToolVersion = "");

protected:
   static std::error_code mh_CreateOsyInitCode(const C_OscNode & orc_Node, const uint16_t ou16_ApplicationIndex,
                                               const std::string & orc_Path,
                                               std::vector<std::string> & orc_Files,
                                               const std::string & orc_ExportToolInfo = "");
   static std::error_code mh_CreateDatapoolCode(const C_OscNode & orc_Node, const uint16_t ou16_ApplicationIndex,
                                                const std::string & orc_Path,
                                                std::vector<std::string> & orc_Files,
                                                const std::string & orc_ExportToolInfo = "");
   static std::error_code mh_CreateCommStackCode(const C_OscNode & orc_Node, const uint16_t ou16_ApplicationIndex,
                                                 const std::string & orc_Path,
                                                 std::vector<std::string> & orc_Files,
                                                 const std::string & orc_ExportToolInfo = "");
   static std::error_code mh_CreateHalConfigCode(const C_OscNode & orc_Node, const uint16_t ou16_ApplicationIndex,
                                                 const std::string & orc_Path,
                                                 std::vector<std::string> & orc_Files,
                                                 const std::string & orc_ExportToolInfo = "");
   static std::error_code mh_CreateHalNvmData(const C_OscNode & orc_Node, const uint16_t ou16_ApplicationIndex,
                                              const std::string & orc_Path,
                                              std::vector<std::string> & orc_Files,
                                              const std::string & orc_ExportToolName,
                                              const std::string & orc_ExportToolVersion);

   static std::error_code mh_CheckPrerequisites(const C_OscNode & orc_Node);
   static std::error_code mh_GetAdaptedComDataPool(const C_OscNode & orc_Node, const uint32_t ou32_DataPoolIndex,
                                                   C_OscNodeDataPool & orc_DataPool);
   static std::error_code mh_AdaptComDataPool(const C_OscNode & orc_Node, const uint32_t ou32_DataPoolIndex,
                                              C_OscNodeDataPool & orc_DataPool);
   static std::error_code mh_GetHalDataPoolIndices(const stw::opensyde_core::C_OscNode & orc_Node, std::
                                                   map<bool, int32_t> & orc_HalcDataPools);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
