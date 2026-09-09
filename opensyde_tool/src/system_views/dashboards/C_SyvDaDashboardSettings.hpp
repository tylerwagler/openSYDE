//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for system view dashboards settings

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYVDADASHBOARDSETTINGS_HPP
#define C_SYVDADASHBOARDSETTINGS_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include "C_OgePopUpContentBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_SyvDaDashboardSettings;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
class C_SyvDaDashboardSettings :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_SyvDaDashboardSettings(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
                                     const uint32_t ou32_ViewIndex);
   ~C_SyvDaDashboardSettings(void) override;
   void InitStaticNames(void) const;
   void Save(void) const;


private:
   Ui::C_SyvDaDashboardSettings * mpc_Ui;
   const uint32_t mu32_ViewIndex;
   static const int32_t mhs32_MIN;
   static const int32_t mhs32_MAX;
   static const int32_t mhs32_MIN_DISTANCE_BETWEEN;

   void m_Load(void) const;
   void m_OkClicked(void);
   void m_CancelClicked(void);
   void m_OnFastChanged(const int32_t os32_Value) const;
   void m_OnMediumChanged(const int32_t os32_Value) const;
   void m_OnSlowChanged(const int32_t os32_Value) const;
   //Avoid call
   C_SyvDaDashboardSettings(const C_SyvDaDashboardSettings &);
   C_SyvDaDashboardSettings & operator =(const C_SyvDaDashboardSettings &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
