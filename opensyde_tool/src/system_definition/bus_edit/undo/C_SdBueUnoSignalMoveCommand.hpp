//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Signal move undo command (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDBUEUNOSIGNALMOVECOMMAND_HPP
#define C_SDBUEUNOSIGNALMOVECOMMAND_HPP

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

class C_SdBueUnoSignalMoveCommand :
   public C_SdBueUnoSignalAddDeleteBaseCommand
{
public:
   C_SdBueUnoSignalMoveCommand(
      const QList<stw::opensyde_core::C_OscCanMessageIdentificationIndices> & orc_SourceMessageId,
      const QList<uint32_t> & orc_SourceSignalIndex,
      const QList<stw::opensyde_core::C_OscCanMessageIdentificationIndices> & orc_TargetMessageId,
      const QList<uint32_t> & orc_TargetSignalIndex,
      C_PuiSdNodeCanMessageSyncManager * const opc_MessageSyncManager,
      stw::opensyde_gui::C_SdBueMessageSelectorTreeWidget * const opc_MessageTreeWidget,
      QUndoCommand * const opc_Parent = NULL);

   void redo(void) override;
   void undo(void) override;

private:
   void m_Move(const QList<uint64_t> & orc_SourceUniqueId, const QList<uint32_t> & orc_SourceSignalIndex,
               const QList<uint64_t> & orc_TargetUniqueId, const QList<uint32_t> & orc_TargetSignalIndex);

   QList<uint64_t> mc_SourceUniqueId;
   const QList<uint32_t> mc_SourceSignalIndex;
   QList<uint64_t> mc_TargetUniqueId;
   const QList<uint32_t> mc_TargetSignalIndex;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
