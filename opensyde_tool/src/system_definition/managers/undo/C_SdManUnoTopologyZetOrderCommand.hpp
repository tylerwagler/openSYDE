//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Topology specific Z order undo command (header)

   See cpp file for detailed description

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDMANUNOTOPOLOGYZETORDERCOMMAND_HPP
#define C_SDMANUNOTOPOLOGYZETORDERCOMMAND_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include "C_SebUnoZetOrderCommand.hpp"
#include "C_GiLiBusConnector.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdManUnoTopologyZetOrderCommand :
   public C_SebUnoZetOrderCommand
{
public:
   C_SdManUnoTopologyZetOrderCommand(QGraphicsScene * const opc_Scene, const std::vector<uint64_t> & orc_Ids,
                                     const std::vector<double> & orc_NewZetValues,
                                     QUndoCommand * const opc_Parent = nullptr);
   ~C_SdManUnoTopologyZetOrderCommand(void) override;

   static void h_CheckZetOrderPriority(const QList<QGraphicsItem *> & orc_SelectedItems,
                                       const QList<QGraphicsItem *> & orc_Items, QMap<QGraphicsItem *,
                                                                                      double> & orc_NewZetValues);

private:
   static void mh_CheckZetOrderOfBusConnectors(stw::opensyde_gui::C_GiLiBusConnector * const opc_Item,
                                               QMap<QGraphicsItem *,
                                                    double> & orc_NewZetValues);
   static void mh_InsertOrReplaceByMinimum(QMap<QGraphicsItem *, double> & orc_Map, QGraphicsItem * const opc_Key,
                                           const double & orf64_Value);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
