//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Message add undo command (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDBUEUNOMESSAGEADDCOMMAND_HPP
#define C_SDBUEUNOMESSAGEADDCOMMAND_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QStringList>
#include <QList>
#include "C_SdBueUnoMessageAddDeleteBaseCommand.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdBueUnoMessageAddCommand :
   public C_SdBueUnoMessageAddDeleteBaseCommand
{
public:
   C_SdBueUnoMessageAddCommand(
      const QList<stw::opensyde_core::C_OscCanMessageIdentificationIndices> & orc_MessageId,
      C_PuiSdNodeCanMessageSyncManager * const opc_MessageSyncManager,
      stw::opensyde_gui::C_SdBueMessageSelectorTreeWidget * const opc_MessageTreeWidget,
      const QString & orc_Text);

   void SetInitialData(const QList<stw::opensyde_core::C_OscCanMessage > & orc_Message,
                       const QList<QList<stw::opensyde_core::C_OscNodeDataPoolListElement> > & orc_OscSignalCommons, const QList<QList<C_PuiSdNodeDataPoolListElement> > & orc_UiSignalCommons, const QList<C_PuiSdNodeCanMessage> & orc_UiMessages, const QList<QStringList> & orc_OwnerNodeName, const QList<QList<uint32_t> > & orc_OwnerNodeInterfaceIndex, const QList<QList<uint32_t> > & orc_OwnerNodeDatapoolIndex, const QList<QList<bool> > & orc_OwnerIsTxFlag);

   QList<stw::opensyde_core::C_OscCanMessageIdentificationIndices> GetLastMessageIds(void) const;

   void redo(void) override;
   void undo(void) override;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
