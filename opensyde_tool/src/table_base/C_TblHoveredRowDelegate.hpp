//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Table delegate drawing an icon and a hovered-row border (header)

   See cpp file for detailed description

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_TBLHOVEREDROWDELEGATE_HPP
#define C_TBLHOVEREDROWDELEGATE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QStyledItemDelegate>
#include <QAbstractTableModel>
#include <cstdint>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_TblHoveredRowDelegate :
   public QStyledItemDelegate
{
public:
   explicit C_TblHoveredRowDelegate(QObject * const opc_Parent = nullptr);

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
