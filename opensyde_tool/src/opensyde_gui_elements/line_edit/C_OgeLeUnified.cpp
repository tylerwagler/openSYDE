//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated line edit with unified functionality and mode-based behavior
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeLeUnified.hpp"

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
   \param[in,out] opc_Parent Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeLeUnified::C_OgeLeUnified(QWidget * const opc_Parent) :
   C_OgeLeToolTipBase(opc_Parent),
   ms32_BackgroundColor(0),
   me_Mode(E_Mode::STANDARD),
   mq_EscapeSignalEnabled(false),
   mq_IPFormattingEnabled(false),
   mq_DarkTheme(false)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Constructor with mode
   \param[in] e_Mode Initial mode
   \param[in,out] opc_Parent Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeLeUnified::C_OgeLeUnified(const E_Mode e_Mode, QWidget * const opc_Parent) :
   C_OgeLeToolTipBase(opc_Parent),
   ms32_BackgroundColor(0),
   me_Mode(e_Mode),
   mq_EscapeSignalEnabled(false),
   mq_IPFormattingEnabled(false),
   mq_DarkTheme(false)
{
   // Configure based on mode
   switch (e_Mode)
   {
   case E_Mode::NAVIGATION:
      this->mq_EscapeSignalEnabled = true;
      break;

   case E_Mode::SEARCH:
      this->mq_DarkTheme = false;  // Default to light theme
      break;

   case E_Mode::IP_ADDRESS:
      this->mq_IPFormattingEnabled = true;
      break;

   default:
      break;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeLeUnified::~C_OgeLeUnified(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set line edit variant and trigger QSS re-polish
   \param[in] orc_Variant Variant name for QSS targeting
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLeUnified::SetLineEditVariant(const QString & orc_Variant)
{
   this->mc_LineEditVariant = orc_Variant;
   this->style()->unpolish(this);
   this->style()->polish(this);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current line edit variant
   \return Current variant string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeLeUnified::GetLineEditVariant(void) const
{
   return this->mc_LineEditVariant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set background color (stylesheet color index)
   \param[in] os32_Value New value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLeUnified::SetBackgroundColor(const int32_t os32_Value)
{
   this->ms32_BackgroundColor = os32_Value;
   stw::opensyde_gui_elements::C_OgeWiUtil::h_ApplyStylesheetProperty(this, "Background", os32_Value);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get background color
   \return Current background color value
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OgeLeUnified::GetBackgroundColor(void) const
{
   return this->ms32_BackgroundColor;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set line edit mode
   \param[in] e_Mode New mode
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLeUnified::SetLineEditMode(const E_Mode e_Mode)
{
   this->me_Mode = e_Mode;

   // Configure based on mode
   switch (e_Mode)
   {
   case E_Mode::NAVIGATION:
      this->mq_EscapeSignalEnabled = true;
      break;

   case E_Mode::SEARCH:
      // Search mode doesn't change escape signal
      break;

   case E_Mode::IP_ADDRESS:
      this->mq_IPFormattingEnabled = true;
      break;

   default:
      this->mq_EscapeSignalEnabled = false;
      this->mq_IPFormattingEnabled = false;
      break;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current line edit mode
   \return Current mode
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeLeUnified::E_Mode C_OgeLeUnified::GetLineEditMode(void) const
{
   return this->me_Mode;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set search theme (dark or light)
   \param[in] oq_Dark True for dark theme, false for light theme
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLeUnified::SetSearchTheme(const bool oq_Dark)
{
   this->mq_DarkTheme = oq_Dark;
   this->m_OnSearchThemeChanged(oq_Dark);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set escape signal enabled state
   \param[in] oq_Enable True to enable escape signal
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLeUnified::SetEscapeSignalEnabled(const bool oq_Enable)
{
   this->mq_EscapeSignalEnabled = oq_Enable;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set IP formatting enabled state
   \param[in] oq_Enable True to enable IP formatting
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLeUnified::SetIPFormattingEnabled(const bool oq_Enable)
{
   this->mq_IPFormattingEnabled = oq_Enable;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten key press event slot
   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLeUnified::keyPressEvent(QKeyEvent * const opc_Event)
{
   if ((this->mq_EscapeSignalEnabled == true) &&
       (opc_Event->key() == static_cast<int32_t>(Qt::Key_Escape)))
   {
      //Signal parent
      Q_EMIT this->SigEscape();
      //Don't call base and accept to properly signal a handled escape
      opc_Event->accept();
   }
   else
   {
      C_OgeLeToolTipBase::keyPressEvent(opc_Event);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overridden focus in event slot
   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLeUnified::focusInEvent(QFocusEvent * const opc_Event)
{
   if (this->mq_IPFormattingEnabled == true)
   {
      this->setText(this->text().trimmed());
      Q_EMIT SignalFocusIn(this);
   }

   C_OgeLeToolTipBase::focusInEvent(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overridden focus out event slot
   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLeUnified::focusOutEvent(QFocusEvent * const opc_Event)
{
   if (this->mq_IPFormattingEnabled == true)
   {
      this->setText(this->text().trimmed());

      if (opc_Event->reason() == Qt::TabFocusReason)
      {
         Q_EMIT (SignalTabKey(this));
      }

      Q_EMIT (SignalFocusOut(this));
   }

   C_OgeLeToolTipBase::focusOutEvent(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten mouse press event slot
   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLeUnified::mousePressEvent(QMouseEvent * const opc_Event)
{
   if (this->me_Mode == E_Mode::COMBO_BOX)
   {
      // Trigger combo box pop up if parent is a combo box
      stw::opensyde_gui_elements::C_OgeCbxText * const pc_Parent =
         dynamic_cast<stw::opensyde_gui_elements::C_OgeCbxText *>(this->parent());

      C_OgeLeToolTipBase::mousePressEvent(opc_Event);

      if (pc_Parent != NULL)
      {
         pc_Parent->SuppressHide(); // suppress call to hidePopup
         pc_Parent->showPopup();
      }
   }
   else
   {
      C_OgeLeToolTipBase::mousePressEvent(opc_Event);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle search theme change
   \param[in] oq_Dark True for dark theme
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLeUnified::m_OnSearchThemeChanged(const bool oq_Dark)
{
   if (oq_Dark == true)
   {
      this->setStyleSheet("stw--opensyde_gui_elements--C_OgeLeUnified {"
                          "background-color:transparent;"
                          "color:rgb(211, 211, 216);"
                          "}");
   }
   else
   {
      this->setStyleSheet("");
   }
}
