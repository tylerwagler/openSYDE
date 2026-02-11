//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Signal add undo command (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDBUEUNOSIGNALADDCOMMAND_HPP
#define C_SDBUEUNOSIGNALADDCOMMAND_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QList>
#include "C_SdBueUnoSignalAddDeleteBaseCommand.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdBueUnoSignalAddCommand :
   public C_SdBueUnoSignalAddDeleteBaseCommand
{
public:
   C_SdBueUnoSignalAddCommand(
      const QList<stw::opensyde_core::C_OscCanMessageIdentificationIndices> & orc_MessageId,
      const QList<uint32_t> & orc_SignalIndex, const QList<uint16_t> & orc_StartBit,
      const QList<stw::opensyde_core::C_OscCanSignal::E_MultiplexerType> & orc_MultiplexerType,
      const QList<uint16_t> & orc_MultiplexerValue,
      C_PuiSdNodeCanMessageSyncManager * const opc_MessageSyncManager,
      stw::opensyde_gui::C_SdBueMessageSelectorTreeWidget * const opc_MessageTreeWidget,
      const QString & orc_Text);

   void SetInitialData(const QList<stw::opensyde_core::C_OscCanSignal> & orc_Signal,
                       const QList<stw::opensyde_core::C_OscNodeDataPoolListElement> & orc_OscSignalCommon,
                       const QList<C_PuiSdNodeDataPoolListElement> & orc_UiSignalCommon,
                       const QList<C_PuiSdNodeCanSignal> & orc_UiSignal,
                       const QList<stw::opensyde_core::C_OscCanProtocol::E_Type> & orc_ProtocolType);

   void redo(void) override;
   void undo(void) override;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
