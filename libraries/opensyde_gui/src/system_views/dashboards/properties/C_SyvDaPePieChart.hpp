//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for system view dashboard pie chart properties (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYVDAPEPIECHART_HPP
#define C_SYVDAPEPIECHART_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_SyvDaPePanelBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_SyvDaPePieChart;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
class C_SyvDaPePieChart :
   public C_SyvDaPePanelBase
{
   Q_OBJECT

public:
   explicit C_SyvDaPePieChart(C_SyvDaPeBase & orc_Parent);
   ~C_SyvDaPePieChart(void) override;

   void InitStaticNames(void) const;

   bool GetShowValue(void) const;
   bool GetShowUnit(void) const;

   void SetShowValue(const bool oq_Value) const;
   void SetShowUnit(const bool oq_Value) const;

private:
   //Avoid call
   C_SyvDaPePieChart(const C_SyvDaPePieChart &);
   C_SyvDaPePieChart & operator =(const C_SyvDaPePieChart &) &;

   void m_UpdatePreview(void);

   Ui::C_SyvDaPePieChart * mpc_Ui;
};
}
}

#endif
