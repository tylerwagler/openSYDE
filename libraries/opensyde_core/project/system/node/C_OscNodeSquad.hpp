//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       The squad of nodes. In case of multiple CPUs it has all indexes of the sub nodes and has its base name

   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODESQUAD_HPP
#define C_OSCNODESQUAD_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>

#include "stwtypes.hpp"
#include <string>

#include "C_OscNode.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscNodeSquad
{
public:
   C_OscNodeSquad(void);
   virtual ~C_OscNodeSquad(void);

   static std::string h_CombineNames(const std::string & orc_MainDeviceName,
                                               const std::string & orc_SubDeviceName);
   int32_t SetBaseName(std::vector<C_OscNode> & orc_Nodes, const std::string & orc_NodeBaseName);
   static bool h_CheckIsMultiDevice(const uint32_t ou32_NodeIndex,
                                    const std::vector<stw::opensyde_core::C_OscNodeSquad> & orc_AvailableGroups,
                                    uint32_t * const opu32_GroupIndex = nullptr);

   static const std::string hc_SEPARATOR; ///< Default separator for GUI and device type checks
   std::string c_BaseName;                ///< Base name for all sub nodes
   std::vector<uint32_t> c_SubNodeIndexes;          ///< Indexes of all containing sub nodes in the squad
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
