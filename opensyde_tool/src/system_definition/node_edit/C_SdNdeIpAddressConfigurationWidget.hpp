//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for IP Address configuration (implementation)

   Widget for IP Address configuration

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDNDEIPADDRESSCONFIGURATIONWIDGET_HPP
#define C_SDNDEIPADDRESSCONFIGURATIONWIDGET_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QLineEdit>
#include <cstdint>
#include "C_OgePopUpContentBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_SdNdeIpAddressConfigurationWidget;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdNdeIpAddressConfigurationWidget :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_SdNdeIpAddressConfigurationWidget(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
                                                const uint32_t ou32_NodeIndex, const uint32_t ou32_ComIf);
   ~C_SdNdeIpAddressConfigurationWidget(void) override;

   void InitStaticNames(void) const;

   void SlotTabKey(const QLineEdit * const opc_LineEdit);


private:
   Ui::C_SdNdeIpAddressConfigurationWidget * mpc_Ui;
   const uint32_t mu32_NodeIndex;
   const uint32_t mu32_ComIf;

   void m_LoadData(void) const;
   void m_OkClicked(void);
   void m_CancelClicked(void);

   //Avoid call
   C_SdNdeIpAddressConfigurationWidget(const C_SdNdeIpAddressConfigurationWidget &);
   C_SdNdeIpAddressConfigurationWidget & operator =(const C_SdNdeIpAddressConfigurationWidget &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
