//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated check box with unified functionality and variant-based styling
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeChxUnified.hpp"

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
C_OgeChxUnified::C_OgeChxUnified(QWidget * const opc_Parent) :
   C_OgeChxToolTipBase(opc_Parent),
   mq_Tristate(false),
   mu32_Index(0U),
   mu32_SubIndex(0U)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Constructor with tristate support
   \param[in] oq_Tristate Enable tristate mode
   \param[in,out] opc_Parent Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeChxUnified::C_OgeChxUnified(const bool oq_Tristate, QWidget * const opc_Parent) :
   C_OgeChxToolTipBase(opc_Parent),
   mq_Tristate(oq_Tristate),
   mu32_Index(0U),
   mu32_SubIndex(0U)
{
   if (oq_Tristate == true)
   {
      this->setTristate(true);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeChxUnified::~C_OgeChxUnified(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set checkbox variant and trigger QSS re-polish
   \param[in] orc_Variant Variant name for QSS targeting
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeChxUnified::SetCheckBoxVariant(const QString & orc_Variant)
{
   this->mc_CheckBoxVariant = orc_Variant;
   this->style()->unpolish(this);
   this->style()->polish(this);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current checkbox variant
   \return Current variant string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeChxUnified::GetCheckBoxVariant(void) const
{
   return this->mc_CheckBoxVariant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set tristate mode
   \param[in] oq_Tristate True to enable tristate mode
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeChxUnified::SetTristate(const bool oq_Tristate)
{
   this->mq_Tristate = oq_Tristate;
   this->setTristate(oq_Tristate);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if tristate mode is enabled
   \return True if tristate mode is enabled
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OgeChxUnified::IsTristate(void) const
{
   return this->mq_Tristate;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set indexes for identification
   \param[in] ou32_Index First index
   \param[in] ou32_SubIndex Second index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeChxUnified::SetIndexes(const uint32_t ou32_Index, const uint32_t ou32_SubIndex)
{
   this->mu32_Index = ou32_Index;
   this->mu32_SubIndex = ou32_SubIndex;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get indexes
   \param[out] oru32_Index First index
   \param[out] oru32_SubIndex Second index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeChxUnified::GetIndexes(uint32_t & oru32_Index, uint32_t & oru32_SubIndex) const
{
   oru32_Index = this->mu32_Index;
   oru32_SubIndex = this->mu32_SubIndex;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten resize event slot
   \param[in,out] opc_Event Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeChxUnified::resizeEvent(QResizeEvent * const opc_Event)
{
   // Only update indicator size if tristate mode is enabled
   if (this->mq_Tristate == true)
   {
      this->m_UpdateIndicatorSize();
   }
   C_OgeChxToolTipBase::resizeEvent(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle toggle change
   \param[in] oq_Checked New checked state
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeChxUnified::toggleChanged(const bool oq_Checked)
{
   C_OgeChxToolTipBase::toggleChanged(opc_Checked);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle checkbox toggled signal
   \param[in] oq_Checked New checked state
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeChxUnified::m_CheckBoxToggled(const bool oq_Checked)
{
   Q_EMIT this->SigCheckBoxToggled(this->mu32_Index, this->mu32_SubIndex, oq_Checked);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update indicator size for tristate checkboxes
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeChxUnified::m_UpdateIndicatorSize(void)
{
   const QString c_StyleSheet = "stw--opensyde_gui_elements--C_OgeChxUnified::indicator {" +
                                "width:" + QString::number(this->width()) + "px;" +
                                "height:" + QString::number(this->height()) + "px;" +
                                "}";

   this->setStyleSheet(c_StyleSheet);
}
