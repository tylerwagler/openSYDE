//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Connects the openSYDE protocol driver to a node with GUI specific functionality

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYVCOMDATADEALER_HPP
#define C_SYVCOMDATADEALER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QMap>
#include <QList>
#include <system_error>

#include "stwtypes.hpp"

#include "C_OscErrorCategory.hpp"
#include "C_OscDataDealerNvmSafe.hpp"

#include "C_OscNode.hpp"
#include "C_OscDiagProtocolBase.hpp"

#include "C_PuiSvDbDataElementHandler.hpp"
#include "C_OscNodeDataPoolListElementId.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SyvComDataDealer :
   public stw::opensyde_core::C_OscDataDealerNvmSafe
{
public:
   C_SyvComDataDealer(void);
   C_SyvComDataDealer(stw::opensyde_core::C_OscNode * const opc_Node, const uint32_t ou32_NodeIndex,
                      stw::opensyde_core::C_OscDiagProtocolBase * const opc_DiagProtocol);
   ~C_SyvComDataDealer(void) noexcept override;

   void RegisterWidget(C_PuiSvDbDataElementHandler * const opc_Widget);

   std::error_code DataPoolReadWithWidget(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                          const uint16_t ou16_ElementIndex,
                                          C_PuiSvDbDataElementHandler * const opc_DashboardWidget,
                                          uint8_t * const opu8_NrCode);
   std::error_code DataPoolRead(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                const uint16_t ou16_ElementIndex, uint8_t * const opu8_NrCode) override;
   std::error_code NvmRead(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                           const uint16_t ou16_ElementIndex, uint8_t * const opu8_NrCode) override;
   std::error_code NvmReadList(const uint32_t ou32_DataPoolIndex, const uint32_t ou32_ListIndex,
                               uint8_t * const opu8_NrCode) override;

protected:
   void m_OnReadDataPoolEventReceivedForWidget(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                               const uint16_t ou16_ElementIndex,
                                               C_PuiSvDbDataElementHandler * const opc_DashboardWidget);
   void m_OnReadDataPoolEventReceived(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                      const uint16_t ou16_ElementIndex) override;
   void m_OnReadDataPoolEventErrorReceived(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                           const uint16_t ou16_ElementIndex, const uint8_t ou8_ErrorCode) override;
   void m_OnReadDataPoolNvmEventReceived(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                         const uint16_t ou16_ElementIndex) override;

private:
   QMap<stw::opensyde_core::C_OscNodeDataPoolListElementId, QList<C_PuiSvDbDataElementHandler *> > mc_AllWidgets;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
