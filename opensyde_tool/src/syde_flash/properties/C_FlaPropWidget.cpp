//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base widget for all general properties

   Base widget for all general properties

   \copyright   Copyright 2023 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_UsHandler.hpp"
#include "C_FlaConNodeConfigPopup.hpp"

#include "C_FlaPropWidget.hpp"
#include "ui_C_FlaPropWidget.h"

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
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in,out]  opc_Parent    Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_FlaPropWidget::C_FlaPropWidget(QWidget * const opc_Parent) :
   QWidget(opc_Parent),
   mpc_Ui(new Ui::C_FlaPropWidget)
{
   this->mpc_Ui->setupUi(this);

   this->m_InitStaticNames();
   this->m_InitStaticGuiElements();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_FlaPropWidget::~C_FlaPropWidget()
{
   delete this->mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get node ID

   \retval   Node ID
*/
//----------------------------------------------------------------------------------------------------------------------
uint8_t C_FlaPropWidget::GetNodeId() const
{
   const uint8_t u8_Retval = static_cast<uint8_t>(this->mpc_Ui->pc_SpxNodeId->value());

   return u8_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get bitrate from combobox

   \retval   bitrate in kbit/s
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_FlaPropWidget::GetBitrate() const
{
   QString c_Text = this->mpc_Ui->pc_CbxBitrate->currentText();
   const uint32_t u32_Retval = static_cast<uint32_t>(c_Text.remove(" kbit/s").toInt());

   return u32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set configured Node ID in GUI

   \param[in]       ou8_NodeId   new configured Bitrate
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaPropWidget::SetNodeId(const uint8_t ou8_NodeId) const
{
   this->mpc_Ui->pc_SpxNodeId->setValue(ou8_NodeId);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set configured Bitrate in GUI

   \param[in]       ou32_Bitrate   new configured Bitrate
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaPropWidget::SetBitrate(const uint32_t ou32_Bitrate) const
{
   QString c_Bitrate = QString::number(ou32_Bitrate);

   c_Bitrate.append(" kbit/s");

   this->mpc_Ui->pc_CbxBitrate->setCurrentText(c_Bitrate);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save user settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaPropWidget::SaveUserSettings() const
{
   C_UsHandler::h_GetInstance()->SetPropNodeId(this->GetNodeId());
   C_UsHandler::h_GetInstance()->SetPropBitrate(this->GetBitrate());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load user settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaPropWidget::LoadUserSettings() const
{
   const uint8_t u8_NodeId = static_cast<uint8_t>(C_UsHandler::h_GetInstance()->GetPropNodeId());
   const uint32_t u32_Bitrate = C_UsHandler::h_GetInstance()->GetPropBitrate();

   SetNodeId(u8_NodeId);
   SetBitrate(u32_Bitrate);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all displayed static names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaPropWidget::m_InitStaticNames(void) const
{
   this->mpc_Ui->pc_WiTitle->SetTitle("Properties");
   this->mpc_Ui->pc_WiTitle->SetIconType(C_CamOgeWiSectionHeader::E_ButtonType::eNOBUTTON);

   this->mpc_Ui->pc_LabelNodeId->setText("Node ID");
   this->mpc_Ui->pc_LabelBitrate->setText("Bitrate");

   this->mpc_Ui->pc_LabelNodeId->SetToolTipInformation("Node ID",
                                                       "Current node ID of the node to be configured/updated.");

   this->mpc_Ui->pc_LabelBitrate->SetToolTipInformation("Bitrate",
                                                        "Current bitrate of the node to be configured/updated.");
   this->mpc_Ui->pc_SpxNodeId->SetToolTipInformation("Value Range",
                                                     "Minimum: 0\n"
                                                                            "Maximum: 125");
   this->mpc_Ui->pc_CbxBitrate->SetToolTipInformation("",
                                                      "",
                                                      C_NagToolTip::eDEFAULT);

   //fill combobox
   this->mpc_Ui->pc_CbxBitrate->addItem("100 kbit/s");
   this->mpc_Ui->pc_CbxBitrate->addItem("125 kbit/s");
   this->mpc_Ui->pc_CbxBitrate->addItem("250 kbit/s");
   this->mpc_Ui->pc_CbxBitrate->addItem("500 kbit/s");
   this->mpc_Ui->pc_CbxBitrate->addItem("800 kbit/s");
   this->mpc_Ui->pc_CbxBitrate->addItem("1000 kbit/s");
   this->mpc_Ui->pc_CbxBitrate->setCurrentText("125 kbit/s");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Initialize color, fonts, etc.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaPropWidget::m_InitStaticGuiElements() const
{
   this->mpc_Ui->pc_LabelNodeId->SetForegroundColor(8);
   this->mpc_Ui->pc_LabelNodeId->SetBackgroundColor(-1);
   this->mpc_Ui->pc_LabelNodeId->SetFontPixel(12);

   this->mpc_Ui->pc_LabelBitrate->SetForegroundColor(8);
   this->mpc_Ui->pc_LabelBitrate->SetBackgroundColor(-1);
   this->mpc_Ui->pc_LabelBitrate->SetFontPixel(12);
}
