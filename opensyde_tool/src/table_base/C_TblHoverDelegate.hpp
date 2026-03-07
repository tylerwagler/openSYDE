//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Generic table hover-highlight delegate

   Reusable delegate that draws a colored border around the hovered row.
   Replaces several identical per-feature delegates (CANopen overview,
   CANopen PDO, HALC overview, Data Logger overview).

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_TBLHOVERDELEGATE_HPP
#define C_TBLHOVERDELEGATE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QStyledItemDelegate>
#include <QAbstractTableModel>
#include "stwtypes.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_TblHoverDelegate :
   public QStyledItemDelegate
{
public:
   C_TblHoverDelegate(QObject * const opc_Parent = NULL);

   void paint(QPainter * const opc_Painter, const QStyleOptionViewItem & orc_Option,
              const QModelIndex & orc_Index) const override;
   bool SetHoveredRow(const int32_t & ors32_Value);
   void SetModel(const QAbstractTableModel * const opc_Value);

private:
   const QAbstractTableModel * mpc_Model;
   int32_t ms32_HoveredRow;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
