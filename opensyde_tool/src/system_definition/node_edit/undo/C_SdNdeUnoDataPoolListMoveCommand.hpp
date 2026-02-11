//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data pool list move undo command (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDNDEUNODATAPOOLLISTMOVECOMMAND_HPP
#define C_SDNDEUNODATAPOOLLISTMOVECOMMAND_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include "stwtypes.hpp"
#include <QList>
#include "C_SdNdeUnoDataPoolListAddDeleteBaseCommand.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdNdeUnoDataPoolListMoveCommand :
   public C_SdNdeUnoDataPoolListAddDeleteBaseCommand
{
public:
   C_SdNdeUnoDataPoolListMoveCommand(const uint32_t & oru32_NodeIndex, const uint32_t & oru32_DataPoolIndex,
                                     stw::opensyde_gui::C_SdNdeDpListsTreeWidget * const opc_DataPoolListsTreeWidget,
                                     const QList<uint32_t> & orc_SourceRows,
                                     const QList<uint32_t> & orc_TargetRows,
                                     QUndoCommand * const opc_Parent = NULL);
   void redo(void) override;
   void undo(void) override;

private:
   QList<uint32_t> mc_SourceRows;
   QList<uint32_t> mc_TargetRows;

   void m_DoMoveRows(const QList<uint32_t> & orc_SelectedIndices,
                     const QList<uint32_t> & orc_TargetIndices);
   void m_MoveItems(const QList<uint32_t> & orc_ContiguousIndices, const uint32_t ou32_TargetIndex) const;
   void m_MoveItem(const uint32_t ou32_SourceIndex, const uint32_t ou32_TargetIndex) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
