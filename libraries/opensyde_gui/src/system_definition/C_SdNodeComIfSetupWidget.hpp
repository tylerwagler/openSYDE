//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node Com Interface Setup (header)

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDNODECOMIFSETUPWIDGET_HPP
#define C_SDNODECOMIFSETUPWIDGET_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QRadioButton>

#include "stwtypes.hpp"
#include "C_OgePopUpContentBase.hpp"
#include "C_OscNodeComInterfaceSettings.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_SdNodeComIfSetupWidget;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
class C_SdNodeComIfSetupWidget :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_SdNodeComIfSetupWidget(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
                                     const uint32_t & oru32_NodeIndex, const uint32_t & oru32_BusIndex,
                                     const int32_t & ors32_SpecialInterface);
   ~C_SdNodeComIfSetupWidget(void) override;

   void InitStaticNames(void) const;
   uint8_t GetSelectedInterface(void) const;
   bool GetInteractionPossible(void) const;


protected:
   void m_OnEnterAccept(void) override;

private:
   //Avoid call
   C_SdNodeComIfSetupWidget(const C_SdNodeComIfSetupWidget &);
   C_SdNodeComIfSetupWidget & operator =(const C_SdNodeComIfSetupWidget &) &;

   Ui::C_SdNodeComIfSetupWidget * mpc_Ui;
   const uint32_t mu32_NodeIndex;
   const uint32_t mu32_BusIndex;
   const int32_t ms32_SpecialInterface;
   bool mq_InteractionPossible;

   void m_OkClicked(void);
   void m_CancelClicked(void);
   void m_InitFromData(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
