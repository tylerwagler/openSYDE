//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base class for all gui element classes with a tool tip (implementation)

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QEvent>
#include <QHelpEvent>
#include "C_OgeToolTipBase.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_elements;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeToolTipBase::C_OgeToolTipBase() :
   mc_ToolTipHeading(""),
   mc_ToolTipContent(""),
   me_ToolTipType(stw::opensyde_gui::C_NagToolTip::eDEFAULT),
   mq_ToolTipActive(false),
   mq_ShowToolTipWhenDisabled(false),
   mpc_ToolTip(nullptr)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeToolTipBase::~C_OgeToolTipBase()
{
   delete mpc_ToolTip;
   mpc_ToolTip = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets the strings for the tooltip

   \param[in]     orc_Heading     String with heading
   \param[in]     orc_Content     String with content
   \param[in]     oe_Type         Optional tooltip type
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeToolTipBase::SetToolTipInformation(const QString & orc_Heading, const QString & orc_Content,
                                             const stw::opensyde_gui::C_NagToolTip::E_Type oe_Type)
{
   this->mc_ToolTipHeading = orc_Heading;
   this->mc_ToolTipContent = orc_Content;
   this->me_ToolTipType = oe_Type;

   if ((this->mc_ToolTipContent != "") || (this->mc_ToolTipHeading != ""))
   {
      this->mq_ToolTipActive = true;
   }
   else
   {
      this->mq_ToolTipActive = false;
      if ((this->mpc_ToolTip != nullptr) &&
          (this->mpc_ToolTip->isVisible() == true))
      {
         this->mpc_ToolTip->hide();
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets the flag for showing the tooltip even when the element is disabled

   \param[in]       oq_ShowToolTip     Flag for showing tooltip
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeToolTipBase::ShowToolTipWhenDisabled(const bool oq_ShowToolTip)
{
   this->mq_ShowToolTipWhenDisabled = oq_ShowToolTip;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Hides the tool tip
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeToolTipBase::m_HideToolTip(void)
{
   if (this->mpc_ToolTip != nullptr)
   {
      this->mpc_ToolTip->hide();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get the current tool tip

   \return
   Current tool tip
*/
//----------------------------------------------------------------------------------------------------------------------
stw::opensyde_gui::C_NagToolTip * C_OgeToolTipBase::m_GetToolTip(void)
{
   m_CreateToolTip();
   return this->mpc_ToolTip;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Creates an instance for mpc_ToolTip if not already exist
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeToolTipBase::m_CreateToolTip(void)
{
   if (mpc_ToolTip == nullptr)
   {
      mpc_ToolTip = new stw::opensyde_gui::C_NagToolTip();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle a widget event that may concern the tool tip

   The tool tip handling every C_Oge*ToolTipBase widget needs, factored out of the twelve
   identical event() overrides that used to carry it. The widgets cannot share this by
   inheritance because each derives from a different Qt class, which is what makes a mixin
   the right shape here.

   The widget is passed in rather than reached through "this" because this class is not a
   QWidget; it is mixed into one.

   \param[in,out]  opc_Event      Event identification and information
   \param[in,out]  orc_Widget     The widget this base is mixed into
   \param[out]     orq_Handled    True if the tool tip logic claimed the event. When false the
                                  caller must delegate to its own Qt base class, which only it
                                  can name.

   \return
   Result to report for the event, meaningful only when orq_Handled is true
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OgeToolTipBase::m_HandleToolTipEvent(QEvent * const opc_Event, QWidget & orc_Widget, bool & orq_Handled)
{
   bool q_Return = false;

   orq_Handled = false;

   if (this->mq_ToolTipActive == true)
   {
      if (opc_Event->type() == QEvent::ToolTip)
      {
         //show tooltip
         if (this->m_GetToolTip()->isVisible() == false)
         {
            QHelpEvent * const pc_HelpEvent = dynamic_cast<QHelpEvent * const>(opc_Event);

            if (pc_HelpEvent != nullptr)
            {
               orc_Widget.setMouseTracking(true);

               this->m_EmitLastChanceToUpdateToolTip();

               this->m_GetToolTip()->SetHeading(this->mc_ToolTipHeading);
               this->m_GetToolTip()->SetContent(this->mc_ToolTipContent);
               this->m_GetToolTip()->SetType(this->me_ToolTipType);
               this->m_GetToolTip()->show();
               this->m_GetToolTip()->DoMove(pc_HelpEvent->globalPos());
            }
         }
         //Accept event because of Qt dynamic tooltip time based on the fact if there was a tooltip in this widget
         // already
         this->m_EmitHideOtherToolTips();
         opc_Event->accept();
         q_Return = true;
         orq_Handled = true;
      }
      else if (opc_Event->type() == QEvent::Leave)
      {
         //hide on leave
         this->m_HideToolTip();

         orc_Widget.setMouseTracking(false);

         opc_Event->accept();
         q_Return = true;
         orq_Handled = true;
      }
      else
      {
         //Not a tool tip event; the caller delegates
      }
   }

   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Emit SigLastChanceToUpdateToolTip, for the widgets that declare it

   Does nothing by default. Only C_OgeLabToolTipBase declares this signal, and only it
   overrides this.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeToolTipBase::m_EmitLastChanceToUpdateToolTip(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Emit SigHideOtherToolTips, for the widgets that declare it

   Does nothing by default. The signal is declared by five widgets and connected in exactly one
   place -- the Datapool selector -- so the widgets that stay silent here are the ones that were
   silent before, not an oversight.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeToolTipBase::m_EmitHideOtherToolTips(void)
{
}
