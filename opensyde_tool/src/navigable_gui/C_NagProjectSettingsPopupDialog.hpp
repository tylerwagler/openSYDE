//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Name char length settings popup dialog
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_NAGPROJECTSETTINGSPOPUPDIALOG_HPP
#define C_NAGPROJECTSETTINGSPOPUPDIALOG_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include "C_OgePopUpContentBase.hpp"
#include "C_OscSystemNameMaxCharLimitChangeReportItem.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_NagProjectSettingsPopupDialog;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_NagProjectSettingsPopupDialog :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_NagProjectSettingsPopupDialog(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent);
   ~C_NagProjectSettingsPopupDialog(void) override;

   void InitStaticNames(void) const;
   bool ApplyMaxCharLimitSettings(void);


private:
   Ui::C_NagProjectSettingsPopupDialog * mpc_Ui;
   uint32_t mu32_CurrentSpinBoxValue;

   void m_OkClicked(void);
   void m_CancelClicked(void);
   void m_ValueChanged(void);
   bool m_AskUserForModifyName(
      const std::list<stw::opensyde_core::C_OscSystemNameMaxCharLimitChangeReportItem> & orc_ChangedItems);
   void m_ShowUnUsedFiles(void);

   //Avoid call
   C_NagProjectSettingsPopupDialog(const C_NagProjectSettingsPopupDialog &);
   C_NagProjectSettingsPopupDialog & operator =(const C_NagProjectSettingsPopupDialog &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
