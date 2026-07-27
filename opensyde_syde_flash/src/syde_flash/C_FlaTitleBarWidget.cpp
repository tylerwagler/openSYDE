//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Title bar widget for SYDEflash

   Title bar widget for SYDEflash

   \copyright   Copyright 2023 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeWiUtil.hpp"
#include "C_FlaConNodeConfigPopup.hpp"

#include "C_FlaTitleBarWidget.hpp"
#include "ui_C_FlaTitleBarWidget.h"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;
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
C_FlaTitleBarWidget::C_FlaTitleBarWidget(QWidget * const opc_Parent) :
   C_OgeTitleBarWidget(opc_Parent),
   mpc_Ui(new Ui::C_FlaTitleBarWidget)
{
   this->mpc_Ui->setupUi(this);
   this->SetBackgroundColor(10);

   C_OgeWiUtil::h_ApplyStylesheetProperty(this->mpc_Ui->pc_Frame, "HasColor8Background", true);

   // initialize static names
   this->InitStaticNames();

   this->m_LoadStwLogo(this->mpc_Ui->pc_LogoLabel);

   // button icons
   this->mpc_Ui->pc_PushButtonUpdate->setIconSize(QSize(22, 22));
   this->mpc_Ui->pc_PushButtonSearch->setIconSize(QSize(24, 24));
   this->mpc_Ui->pc_PushButtonConfigure->setIconSize(QSize(24, 24));
   this->mpc_Ui->pc_PushButtonHelp->setIconSize(QSize(24, 24));
   this->mpc_Ui->pc_PushButtonUpdate->SetSvg("://images/IconUpdate.svg", "://images/IconUpdateDisabled.svg");
   this->mpc_Ui->pc_PushButtonSearch->SetSvg("://images/IconSearch.svg", "://images/IconSearchDisable.svg");
   this->mpc_Ui->pc_PushButtonConfigure->SetSvg("://images/IconConfigure.svg", "://images/IconConfigureDisabled.svg");
   this->mpc_Ui->pc_PushButtonHelp->SetSvg("://images/IconHelp.svg");
   this->mpc_Ui->pc_PushButtonHelp->setText("");
   this->mpc_Ui->pc_PushButtonHelp->SetIconOnly(true);
   connect(this->mpc_Ui->pc_PushButtonUpdate, &QPushButton::clicked, this, &C_FlaTitleBarWidget::SigUpdateNode);
   connect(this->mpc_Ui->pc_PushButtonSearch, &QPushButton::clicked, this, &C_FlaTitleBarWidget::SigSearchNode);
   connect(this->mpc_Ui->pc_PushButtonConfigure, &QPushButton::clicked, this, &C_FlaTitleBarWidget::SigConfigureNode);
   connect(this->mpc_Ui->pc_PushButtonAbout, &QPushButton::clicked, this, &C_FlaTitleBarWidget::m_ShowAbout);
   connect(this->mpc_Ui->pc_PushButtonHelp, &QPushButton::clicked, this, &C_FlaTitleBarWidget::m_TriggerHelp);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_FlaTitleBarWidget::~C_FlaTitleBarWidget()
{
   delete this->mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Initialize all displayed static names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaTitleBarWidget::InitStaticNames(void) const
{
   // initialize button texts
   this->mpc_Ui->pc_PushButtonAbout->setText("About"); // no icon therefore no icon-only mode
   this->m_SetButtonsText(false);

   //tooltips
   this->mpc_Ui->pc_PushButtonHelp->SetToolTipInformation("Help",
                                                          "Open user manual for help.");
   this->mpc_Ui->pc_PushButtonAbout->SetToolTipInformation("About",
                                                           "Show information about SYDEflash.");

   this->mpc_Ui->pc_PushButtonUpdate->SetToolTipInformation("Update Node",
                                                            "Flash selected HEX files on connected node.");
   this->mpc_Ui->pc_PushButtonSearch->SetToolTipInformation("Search Node",
                                                            "Scan network for connected openSYDE nodes.");
   this->mpc_Ui->pc_PushButtonConfigure->SetToolTipInformation("Configure Node",
                                                               "Apply new node ID and bitrate "
                                                                                      "to connected node.");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Enable/disable action buttons while update

   \param[in]  oq_Enabled  true: enable, false: disable
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaTitleBarWidget::EnableActions(const bool oq_Enabled)
{
   this->mpc_Ui->pc_PushButtonUpdate->setEnabled(oq_Enabled);
   this->mpc_Ui->pc_PushButtonSearch->setEnabled(oq_Enabled);
   this->mpc_Ui->pc_PushButtonConfigure->setEnabled(oq_Enabled);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Overwritten resize event slot

   Here: remove or add button text depending on size

   \param[in,out]  opc_Event  Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaTitleBarWidget::resizeEvent(QResizeEvent * const opc_Event)
{
   C_OgeTitleBarWidget::resizeEvent(opc_Event);
   this->m_SetButtonsText(this->width() < 910);
}

//----------------------------------------------------------------------------------------------------------------------
QString C_FlaTitleBarWidget::m_GetAppName(void) const
{
   return "SYDEflash";
}

//----------------------------------------------------------------------------------------------------------------------
QString C_FlaTitleBarWidget::m_GetLogoPath(void) const
{
   return ":/images/SYDEflash_logo.png";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set buttons text or remove them if only icon should be visible (in small width mode)

   \param[in]  oq_IconOnly    Flag to indicate icon only vs icon with text
*/
//----------------------------------------------------------------------------------------------------------------------
void C_FlaTitleBarWidget::m_SetButtonsText(const bool oq_IconOnly) const
{
   this->mpc_Ui->pc_PushButtonUpdate->SetIconOnly(oq_IconOnly);
   this->mpc_Ui->pc_PushButtonSearch->SetIconOnly(oq_IconOnly);
   this->mpc_Ui->pc_PushButtonConfigure->SetIconOnly(oq_IconOnly);

   if (oq_IconOnly == true)
   {
      const uint32_t u32_SMALL_MAXIMUM_SIZE = 34;
      this->mpc_Ui->pc_PushButtonUpdate->setText("");
      this->mpc_Ui->pc_PushButtonSearch->setText("");
      this->mpc_Ui->pc_PushButtonConfigure->setText("");

      this->mpc_Ui->pc_PushButtonUpdate->setMaximumWidth(u32_SMALL_MAXIMUM_SIZE);
      this->mpc_Ui->pc_PushButtonSearch->setMaximumWidth(u32_SMALL_MAXIMUM_SIZE);
      this->mpc_Ui->pc_PushButtonConfigure->setMaximumWidth(u32_SMALL_MAXIMUM_SIZE);
   }
   else
   {
      this->mpc_Ui->pc_PushButtonUpdate->setText("Update Node");
      this->mpc_Ui->pc_PushButtonSearch->setText("Search Node");
      this->mpc_Ui->pc_PushButtonConfigure->setText("Configure Node");

      // we cannot change Qt constant but it is still better than using the hard coded magic number 16777215
      this->mpc_Ui->pc_PushButtonUpdate->setMaximumWidth(QWIDGETSIZE_MAX);    //lint !e893 !e9130 !e9136
      this->mpc_Ui->pc_PushButtonSearch->setMaximumWidth(QWIDGETSIZE_MAX);    //lint !e893 !e9130 !e9136
      this->mpc_Ui->pc_PushButtonConfigure->setMaximumWidth(QWIDGETSIZE_MAX); //lint !e893 !e9130 !e9136
   }
}
