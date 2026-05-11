//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Dialog for choosing and configuring the PC CAN adapter

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYVSEDLLCONFIGURATIONDIALOG_HPP
#define C_SYVSEDLLCONFIGURATIONDIALOG_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include "C_OscCanAdapterConfig.hpp"
#include "C_OgePopUpContentBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_SyvSeDllConfigurationDialog;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SyvSeDllConfigurationDialog :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_SyvSeDllConfigurationDialog(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent);
   ~C_SyvSeDllConfigurationDialog() override;

   void InitText() const;
   void SetAdapterConfig(const stw::opensyde_core::C_OscCanAdapterConfig & orc_Config);
   void SetBitrate(const uint64_t ou64_Bitrate);
   stw::opensyde_core::C_OscCanAdapterConfig GetAdapterConfig(void) const;

private:
   //Avoid call
   C_SyvSeDllConfigurationDialog(const C_SyvSeDllConfigurationDialog &);
   C_SyvSeDllConfigurationDialog & operator =(const C_SyvSeDllConfigurationDialog &) &;

   void m_OkClicked(void) const;
   void m_CancelClicked(void) const;
   void m_TestConnectionClicked(void) const;

   void m_ApplyPlatformLayout(void) const;
   void m_PopulateSocketCanInterfaces(void) const;
   static QStringList mh_DiscoverSocketCanInterfaces(void);

   Ui::C_SyvSeDllConfigurationDialog * mpc_Ui;

   stw::opensyde_core::C_OscCanAdapterConfig mc_AdapterConfig;
   uint64_t mu64_Bitrate;
};
}
}

#endif
