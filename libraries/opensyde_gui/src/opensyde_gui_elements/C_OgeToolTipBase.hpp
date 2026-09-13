//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base class for all gui element classes with a tool tip (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGETOOLTIPBASE_HPP
#define C_OGETOOLTIPBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include <QEvent>
#include <QWidget>

#include "C_NagToolTip.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeToolTipBase
{
public:
   C_OgeToolTipBase();
   virtual ~C_OgeToolTipBase();

   void SetToolTipInformation(const QString & orc_Heading, const QString & orc_Content, const stw::opensyde_gui::C_NagToolTip::E_Type oe_Type =
                                 stw::opensyde_gui::C_NagToolTip::eDEFAULT);
   void ShowToolTipWhenDisabled(const bool oq_ShowToolTip);

protected:
   bool m_HandleToolTipEvent(QEvent * const opc_Event, QWidget & orc_Widget, bool & orq_Handled);

   //Hooks for the two signals. They cannot live here because this class is deliberately not a
   //QObject -- it is mixed into widgets that already derive from one. Defaults do nothing, which
   //is what the widgets that declare no such signal did before.
   virtual void m_EmitLastChanceToUpdateToolTip(void);
   virtual void m_EmitHideOtherToolTips(void);

   virtual void m_HideToolTip(void);
   stw::opensyde_gui::C_NagToolTip * m_GetToolTip(void);

   QString mc_ToolTipHeading;
   QString mc_ToolTipContent;
   stw::opensyde_gui::C_NagToolTip::E_Type me_ToolTipType;
   bool mq_ToolTipActive;
   bool mq_ShowToolTipWhenDisabled;

private:
   stw::opensyde_gui::C_NagToolTip * mpc_ToolTip;

   void m_CreateToolTip(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
