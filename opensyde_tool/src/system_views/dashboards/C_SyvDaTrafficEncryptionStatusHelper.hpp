//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Util class for traffic encryption status functions
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYVDATRAFFICENCRYPTIONSTATUSHELPER_HPP
#define C_SYVDATRAFFICENCRYPTIONSTATUSHELPER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QMap>

#include <cstdint>
#include "C_PuiSvDbNodeDataPoolListElementId.hpp"
#include "C_OscComDriverProtocol.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SyvDaTrafficEncryptionStatusHelper
{
public:
   static QMap<uint32_t, bool> h_GetMappingNodeToTrafficEncryptionStatus(
      const uint32_t ou32_ViewIndex, const bool oq_ConnectionActiveStatus,
      const stw::opensyde_core::C_OscComDriverProtocol & orc_ComDriver);
   static bool h_GetViewNodeEncrypted(
      const stw::opensyde_gui_logic::C_PuiSvDbNodeDataPoolListElementId & orc_DataPoolElementId,
      const bool oq_ConnectionActiveStatus, const uint32_t ou32_ViewIndex, const QMap<uint32_t,
                                                                                      bool> & orc_MappingNodeToTrafficEncryptionStatus);

private:
   C_SyvDaTrafficEncryptionStatusHelper();

   static void mh_GetTrafficEncryptionStatusForNodeUsingBus(QMap<uint32_t,
                                                                 bool> & orc_MappingNodeToTrafficEncryptionStatus,
                                                            const uint32_t ou32_NodeIndex,
                                                            const uint32_t ou32_BusIndex,
                                                            const stw::opensyde_core::C_OscComDriverProtocol & orc_ComDriver);
   static void mh_GetTrafficEncryptionStatusForNodeUsingBusAndNodeId(QMap<uint32_t,
                                                                          bool> & orc_MappingNodeToTrafficEncryptionStatus, const uint32_t ou32_NodeIndex, const uint32_t ou32_BusIndex, const uint8_t ou8_NodeId, const stw::opensyde_core::C_OscComDriverProtocol & orc_ComDriver);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
