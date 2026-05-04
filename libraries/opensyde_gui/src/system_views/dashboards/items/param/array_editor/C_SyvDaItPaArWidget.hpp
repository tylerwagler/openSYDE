//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data set array edit widget (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYVDAITPAARWIDGET_HPP
#define C_SYVDAITPAARWIDGET_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include "C_OgePopUpContentBase.hpp"
#include "C_PuiSvDbDataElementHandler.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_SyvDaItPaArWidget;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
class C_SyvDaItPaArWidget :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_SyvDaItPaArWidget(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
                                const uint32_t & oru32_ElementIndex,
                                stw::opensyde_gui_logic::C_PuiSvDbDataElementHandler * const opc_DataWidget,
                                const bool oq_EcuValues);
   ~C_SyvDaItPaArWidget(void) override;
   void InitStaticNames(void);


private:
   Ui::C_SyvDaItPaArWidget * const mpc_Ui;
   const bool mq_EcuValues;
   const uint32_t mu32_ElementIndex;
   stw::opensyde_gui_logic::C_PuiSvDbDataElementHandler * const mpc_DataWidget;

   void m_OkClicked(void);
   void m_CancelClicked(void);
   //Avoid call
   C_SyvDaItPaArWidget(const C_SyvDaItPaArWidget &);
   C_SyvDaItPaArWidget & operator =(const C_SyvDaItPaArWidget &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
