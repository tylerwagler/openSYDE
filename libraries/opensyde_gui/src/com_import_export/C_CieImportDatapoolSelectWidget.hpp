//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for selecting a datapool while DBC/EDS/DCF file import.
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CIEIMPORTDATAPOOLSELECTWIDGET_HPP
#define C_CIEIMPORTDATAPOOLSELECTWIDGET_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OgePopUpContentBase.hpp"
#include "C_OscCanProtocol.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_CieImportDatapoolSelectWidget;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_CieImportDatapoolSelectWidget :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_CieImportDatapoolSelectWidget(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
                                            const uint32_t ou32_NodeIndex,
                                            const stw::opensyde_core::C_OscCanProtocol::E_Type oe_ComProtocolType);
   ~C_CieImportDatapoolSelectWidget(void) override;

   void InitStaticNames(void) const;
   uint32_t GetSelectedDatapoolIndex(void) const;


protected:
   void m_OnEnterAccept(void) override;

private:
   Ui::C_CieImportDatapoolSelectWidget * mpc_Ui;
   const uint32_t mu32_NodeIndex;
   stw::opensyde_core::C_OscCanProtocol::E_Type me_ComProtocolType;

   void m_OkClicked(void);
   void m_CancelClicked(void) const;
   void m_InitComboBox(void) const;

   //Avoid call
   C_CieImportDatapoolSelectWidget(const C_CieImportDatapoolSelectWidget &);
   C_CieImportDatapoolSelectWidget & operator =(const C_CieImportDatapoolSelectWidget &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
