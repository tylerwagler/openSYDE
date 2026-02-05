//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for configuring CAN bitrate in SYDEflash (implementation)

   Widget for configuring CAN bitrate in SYDEflash

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_FlaBitrateWidget.hpp"
#include "ui_C_FlaBitrateWidget.h"

#include "C_OgeWiUtil.hpp"
#include "C_UsHandler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_elements;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor

   Set up GUI with all elements.

   \param[in,out]  opc_Parent    Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_FlaBitrateWidget::C_FlaBitrateWidget(QWidget * const opc_Parent) :
   C_OgeWiOnlyBackground(opc_Parent),
   mpc_Ui(new Ui::C_FlaBitrateWidget)
{
   this->mpc_Ui->setupUi(this);

   // initialize GUI elements
   this->m_InitUi();

   // Connect bitrate changed signal
   connect(this->mpc_Ui->pc_ComboBoxBitrate, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
           this, &C_FlaBitrateWidget::m_OnBitrateChanged);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_FlaBitrateWidget::~C_FlaBitrateWidget()
{
   delete this->mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load all user settings.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaBitrateWidget::LoadUserSettings(void)
{
   // Load bitrate from user settings (uses PropBitrate from SYDEflash settings)
   const int32_t s32_Bitrate = C_UsHandler::h_GetInstance()->GetPropBitrate();
   this->m_SetBitrateComboBox(s32_Bitrate);

   // Load and apply expand state
   const bool q_Expanded = C_UsHandler::h_GetInstance()->GetWiDllConfigExpanded();
   this->mpc_Ui->pc_WiHeader->SetExpanded(q_Expanded);
   this->m_OnExpand(q_Expanded);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get the currently selected CAN bitrate from the combo box

   \return
   Current bitrate in kBit/s
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_FlaBitrateWidget::GetSelectedBitrate(void) const
{
   // Map combo box indices to bitrates
   static const int32_t has32_Bitrates[] = {10, 20, 50, 100, 125, 250, 500, 800, 1000};
   const int32_t s32_Index = this->mpc_Ui->pc_ComboBoxBitrate->currentIndex();

   if ((s32_Index >= 0) && (s32_Index < static_cast<int32_t>(sizeof(has32_Bitrates) / sizeof(int32_t))))
   {
      return has32_Bitrates[s32_Index];
   }
   // Default to 500 kBit/s if invalid
   return 500;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Initialize GUI elements
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaBitrateWidget::m_InitUi(void)
{
   // initialize colors
   this->SetBackgroundColor(5);

   C_OgeWiUtil::h_ApplyStylesheetProperty(this->mpc_Ui->pc_FrameBottom, "HasColor8Background", true);
   C_OgeWiUtil::h_ApplyStylesheetProperty(this->mpc_Ui->pc_FrameTop, "HasColor8Background", true);

   // initialize title widget
   this->mpc_Ui->pc_WiHeader->SetTitle("Bitrate");
   this->mpc_Ui->pc_WiHeader->SetIcon("://images/IconConfig.svg");
   this->mpc_Ui->pc_WiHeader->SetToggle(false);
   this->mpc_Ui->pc_WiHeader->ShowExpandButton(true);

   // Connect header signals
   connect(this->mpc_Ui->pc_WiHeader, &C_CamOgeWiSettingSubSection::SigExpandSection,
           this, &C_FlaBitrateWidget::m_OnExpand);
   connect(this->mpc_Ui->pc_WiHeader, &C_CamOgeWiSettingSubSection::SigHide,
           this, &C_FlaBitrateWidget::SigHide);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Slot for header expand/collapse

   \param[in]  oq_Expand   true: expand, false: collapse
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaBitrateWidget::m_OnExpand(const bool oq_Expand)
{
   this->mpc_Ui->pc_WiContent->setVisible(oq_Expand);

   // Save expand state
   C_UsHandler::h_GetInstance()->SetWiDllConfigExpanded(oq_Expand);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set the bitrate combo box selection based on bitrate value

   \param[in]  os32_Bitrate   Bitrate in kBit/s
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaBitrateWidget::m_SetBitrateComboBox(const int32_t os32_Bitrate)
{
   // Map bitrate values to combo box indices
   int32_t s32_Index = 6; // Default to 500 kBit/s

   switch (os32_Bitrate)
   {
   case 10:
      s32_Index = 0;
      break;
   case 20:
      s32_Index = 1;
      break;
   case 50:
      s32_Index = 2;
      break;
   case 100:
      s32_Index = 3;
      break;
   case 125:
      s32_Index = 4;
      break;
   case 250:
      s32_Index = 5;
      break;
   case 500:
      s32_Index = 6;
      break;
   case 800:
      s32_Index = 7;
      break;
   case 1000:
      s32_Index = 8;
      break;
   default:
      s32_Index = 6; // Default to 500 kBit/s
      break;
   }

   this->mpc_Ui->pc_ComboBoxBitrate->setCurrentIndex(s32_Index);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Slot for bitrate changed signal
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaBitrateWidget::m_OnBitrateChanged(void)
{
   const int32_t s32_Bitrate = this->GetSelectedBitrate();

   // Save to user settings immediately (uses PropBitrate for SYDEflash)
   C_UsHandler::h_GetInstance()->SetPropBitrate(s32_Bitrate);

   // Emit signals
   Q_EMIT this->SigBitrateChanged(s32_Bitrate);
   Q_EMIT this->SigCanDllConfigured(); // For compatibility with C_FlaSetWidget
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle communication start/stop

   \param[in]  oq_Online   true: started, false: stopped
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaBitrateWidget::OnCommunicationStarted(const bool oq_Online)
{
   // Disable combobox when online
   this->mpc_Ui->pc_ComboBoxBitrate->setEnabled(!oq_Online);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Prepare widget for popup view in collapsed state resp. expanded view

   \param[in]  oq_Expand   true: prepare for expanded, false: prepare for collapsed popup
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaBitrateWidget::PrepareForExpanded(const bool oq_Expand) const
{
   Q_UNUSED(oq_Expand)
   // Header doesn't expand, so nothing to do
}
