//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for system view dashboard data element update mode (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYVDAPEUPDATEMODECONFIGURATION_HPP
#define C_SYVDAPEUPDATEMODECONFIGURATION_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include "C_OgePopUpContentBase.hpp"
#include "C_PuiSvDbNodeDataPoolListElementId.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_SyvDaPeUpdateModeConfiguration;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
class C_SyvDaPeUpdateModeConfiguration :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_SyvDaPeUpdateModeConfiguration(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
                                             const uint32_t ou32_ViewIndex);
   ~C_SyvDaPeUpdateModeConfiguration(void) override;

   void InitStaticNames(void) const;
   void Select(const stw::opensyde_gui_logic::C_PuiSvDbNodeDataPoolListElementId & orc_Id) const;


private:
   //Avoid call
   C_SyvDaPeUpdateModeConfiguration(const C_SyvDaPeUpdateModeConfiguration &);
   C_SyvDaPeUpdateModeConfiguration & operator =(const C_SyvDaPeUpdateModeConfiguration &) &;

   Ui::C_SyvDaPeUpdateModeConfiguration * mpc_Ui;
   const uint32_t mu32_ViewIndex;

   void m_OkClicked(void);
   void m_CancelClicked(void);
   void m_ConfigureClicked(void) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
