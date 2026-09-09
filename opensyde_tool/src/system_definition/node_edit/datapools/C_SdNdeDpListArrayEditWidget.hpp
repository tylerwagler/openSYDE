//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data set array edit widget (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDNDEDPLISTARRAYEDITWIDGET_HPP
#define C_SDNDEDPLISTARRAYEDITWIDGET_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include <QUndoCommand>
#include <cstdint>
#include "C_OgePopUpContentBase.hpp"
#include "C_SdNdeDpUtil.hpp"
#include "C_SdNdeDpListModelViewManager.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

namespace Ui
{
class C_SdNdeDpListArrayEditWidget;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdNdeDpListArrayEditWidget :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_SdNdeDpListArrayEditWidget(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
                                         const uint32_t & oru32_NodeIndex, const uint32_t & oru32_DataPoolIndex,
                                         const uint32_t & oru32_ListIndex, const uint32_t & oru32_ElementIndex,
                                         const stw::opensyde_gui_logic::C_SdNdeDpUtil::E_ArrayEditType & ore_ArrayEditType,
                                         const uint32_t & oru32_DataSetIndex);
   ~C_SdNdeDpListArrayEditWidget(void) override;
   void InitStaticNames(void) const;
   QUndoCommand * TakeUndoCommand(void) const;
   void SetModelViewManager(stw::opensyde_gui_logic::C_SdNdeDpListModelViewManager * const opc_Value) const;


private:
   Ui::C_SdNdeDpListArrayEditWidget * const mpc_Ui;
   const uint32_t mu32_NodeIndex;
   const uint32_t mu32_DataPoolIndex;
   const uint32_t mu32_ListIndex;
   const uint32_t mu32_ElementIndex;
   const stw::opensyde_gui_logic::C_SdNdeDpUtil::E_ArrayEditType me_ArrayEditType;
   const uint32_t mu32_DataSetIndex;

   void m_OkClicked(void);
   void m_CancelClicked(void);
   //Avoid call
   C_SdNdeDpListArrayEditWidget(const C_SdNdeDpListArrayEditWidget &);
   C_SdNdeDpListArrayEditWidget & operator =(const C_SdNdeDpListArrayEditWidget &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
