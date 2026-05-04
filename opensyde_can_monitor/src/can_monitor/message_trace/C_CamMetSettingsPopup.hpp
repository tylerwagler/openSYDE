//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for trace settings popup dialog
   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CAMMETSETTINGSPOPUP_HPP
#define C_CAMMETSETTINGSPOPUP_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include "stwtypes.hpp"

#include "C_OgePopUpContentBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_CamMetSettingsPopup;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_CamMetSettingsPopup :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_CamMetSettingsPopup(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent);
   ~C_CamMetSettingsPopup(void) override;

   // Set for initialization
   void SetValues(const bool oq_DisplayTimestampAbsoluteTimeOfDay, const uint32_t ou32_TraceBufferSize) const;

   // Get
   bool GetDisplayTimestampAbsoluteTimeOfDay(void) const;
   uint32_t GetTraceBufferSize(void) const;


private:
   Ui::C_CamMetSettingsPopup * mpc_Ui;

   //Avoid call
   C_CamMetSettingsPopup(const C_CamMetSettingsPopup &);
   C_CamMetSettingsPopup & operator =(const C_CamMetSettingsPopup &) &;

   void m_InitStaticNames(void) const;
   void m_InitStaticGuiElements(void) const;
   void m_OnCancel(void) const;
   void m_OnOk(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
