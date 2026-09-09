//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Popup dialog for global tool settings
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_NAGTOOLSETTINGSPOPUPDIALOG_HPP
#define C_NAGTOOLSETTINGSPOPUPDIALOG_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include "C_OgePopUpContentBase.hpp"
#include "C_OscSystemNameMaxCharLimitChangeReportItem.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_NagToolSettingsPopupDialog;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_NagToolSettingsPopupDialog :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_NagToolSettingsPopupDialog(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent);
   ~C_NagToolSettingsPopupDialog(void) override;

   void InitStaticNames(void) const;


private:
   Ui::C_NagToolSettingsPopupDialog * mpc_Ui;

   void m_OkClicked(void);
   void m_CancelClicked(void);
   void m_InitEnvironmentSection(void);
   void m_InitDeviceRootsSection(void);
   void m_AddDeviceRoot(void);
   void m_RemoveDeviceRoot(void);
   void m_MoveDeviceRootUp(void);
   void m_MoveDeviceRootDown(void);

   //Avoid call
   C_NagToolSettingsPopupDialog(const C_NagToolSettingsPopupDialog &);
   C_NagToolSettingsPopupDialog & operator =(const C_NagToolSettingsPopupDialog &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
