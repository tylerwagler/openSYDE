//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Popup dialog for security options

   Let the user select to activate/deactivate/do not change security options like authentication
   debugger and traffic encryption.

   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "TglUtils.hpp"
#include "C_SyvUpPacSecurityOptionsPopUp.hpp"
#include "ui_C_SyvUpPacSecurityOptionsPopUp.h"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui_elements;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const int32_t C_SyvUpPacSecurityOptionsPopUp::mhs32_SEC_INDEX_NO_CHANGE = 0;
const int32_t C_SyvUpPacSecurityOptionsPopUp::mhs32_SEC_INDEX_ACTIVATE = 1;
const int32_t C_SyvUpPacSecurityOptionsPopUp::mhs32_SEC_INDEX_DEACTIVATE = 2;
const int32_t C_SyvUpPacSecurityOptionsPopUp::mhs32_DEB_INDEX_NO_CHANGE = 0;
const int32_t C_SyvUpPacSecurityOptionsPopUp::mhs32_DEB_INDEX_ACTIVATE = 1;
const int32_t C_SyvUpPacSecurityOptionsPopUp::mhs32_DEB_INDEX_DEACTIVATE = 2;
const int32_t C_SyvUpPacSecurityOptionsPopUp::mhs32_TEN_INDEX_NO_CHANGE = 0;
const int32_t C_SyvUpPacSecurityOptionsPopUp::mhs32_TEN_INDEX_ACTIVATE = 1;
const int32_t C_SyvUpPacSecurityOptionsPopUp::mhs32_TEN_INDEX_DEACTIVATE = 2;

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in,out]  orc_Parent                      Reference to parent
   \param[in]      oe_StateSecureAuthentication    State authentication
   \param[in]      oe_StateDebugger                State debugger
   \param[in]      oe_StateTrafficEncryption       State encryption
*/
//----------------------------------------------------------------------------------------------------------------------
C_SyvUpPacSecurityOptionsPopUp::C_SyvUpPacSecurityOptionsPopUp(
   stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
   const C_OscViewNodeUpdate::E_StateSecureAuthentication oe_StateSecureAuthentication,
   const C_OscViewNodeUpdate::E_StateDebugger oe_StateDebugger,
   const C_OscViewNodeUpdate::E_StateTrafficEncryption oe_StateTrafficEncryption, const QString & orc_NodeName) :
   C_OgePopUpContentBase(orc_Parent, &orc_Parent),
   mpc_Ui(new Ui::C_SyvUpPacSecurityOptionsPopUp),
   mc_NodeName(orc_NodeName)
{
   this->mpc_Ui->setupUi(this);

   InitStaticNames();

   this->m_InitComboBoxSec(oe_StateSecureAuthentication);
   this->m_InitComboBoxDeb(oe_StateDebugger);
   this->m_InitComboBoxTen(oe_StateTrafficEncryption);

   // register the widget for showing
   this->mrc_ParentDialog.SetWidget(this);

   connect(this->mpc_Ui->pc_PushButtonOk, &QPushButton::clicked, this, &C_SyvUpPacSecurityOptionsPopUp::m_OkClicked);
   connect(this->mpc_Ui->pc_PushButtonCancel, &QPushButton::clicked, this,
           &C_SyvUpPacSecurityOptionsPopUp::m_CancelClicked);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_SyvUpPacSecurityOptionsPopUp::~C_SyvUpPacSecurityOptionsPopUp(void)
{
   delete this->mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all displayed static names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvUpPacSecurityOptionsPopUp::InitStaticNames(void) const
{
   const QString c_ToolTipContent = "No Change: Do nothing\n"
                                                           "Activate: Enable %1 on next system update\n"
                                                           "Deactivate: Disable %1 on next system update\n";

   this->mrc_ParentDialog.SetTitle(mc_NodeName);
   this->mrc_ParentDialog.SetSubTitle("Security Settings");
   this->mpc_Ui->pc_LabelHeading->setText("Services");
   this->mpc_Ui->pc_LabelAuth->setText("Activate Authentication");
   this->mpc_Ui->pc_LabelDebugger->setText("Activate Debugger");
   this->mpc_Ui->pc_LabelTrafficEncryption->setText("Activate Traffic Encryption");

   this->mpc_Ui->pc_PushButtonCancel->setText("Cancel");
   this->mpc_Ui->pc_PushButtonOk->setText("OK");

   //Combo Box
   this->mpc_Ui->pc_ComboBoxAuth->addItem("No Change");
   this->mpc_Ui->pc_ComboBoxAuth->addItem("Activate");
   this->mpc_Ui->pc_ComboBoxAuth->addItem("Deactivate");
   this->mpc_Ui->pc_ComboBoxDebugger->addItem("No Change");
   this->mpc_Ui->pc_ComboBoxDebugger->addItem("Activate");
   this->mpc_Ui->pc_ComboBoxDebugger->addItem("Deactivate");
   this->mpc_Ui->pc_ComboBoxTrafficEncryption->addItem("No Change");
   this->mpc_Ui->pc_ComboBoxTrafficEncryption->addItem("Activate");
   this->mpc_Ui->pc_ComboBoxTrafficEncryption->addItem("Deactivate");

   //ToolTips
   this->mpc_Ui->pc_LabelAuth->SetToolTipInformation(
      "Activate Authentication",
      c_ToolTipContent.arg("authentication"));
   this->mpc_Ui->pc_LabelDebugger->SetToolTipInformation(
      "Activate Debugger",
      c_ToolTipContent.arg("debugger"));
   this->mpc_Ui->pc_LabelTrafficEncryption->SetToolTipInformation(
      "Activate Traffic Encryption",
      c_ToolTipContent.arg("traffic encryption"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get combo box sec state

   \return
   Combo box sec state
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscViewNodeUpdate::E_StateSecureAuthentication C_SyvUpPacSecurityOptionsPopUp::GetComboBoxSecState() const
{
   C_OscViewNodeUpdate::E_StateSecureAuthentication e_Retval = C_OscViewNodeUpdate::eST_SEC_NO_CHANGE;
   switch (this->mpc_Ui->pc_ComboBoxAuth->currentIndex())
   {
   case C_SyvUpPacSecurityOptionsPopUp::mhs32_SEC_INDEX_NO_CHANGE:
      e_Retval = C_OscViewNodeUpdate::eST_SEC_NO_CHANGE;
      break;
   case C_SyvUpPacSecurityOptionsPopUp::mhs32_SEC_INDEX_ACTIVATE:
      e_Retval = C_OscViewNodeUpdate::eST_SEC_ACTIVATE;
      break;
   case C_SyvUpPacSecurityOptionsPopUp::mhs32_SEC_INDEX_DEACTIVATE:
      e_Retval = C_OscViewNodeUpdate::eST_SEC_DEACTIVATE;
      break;
   default:
      tgl_assert(false);
      break;
   }
   return e_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get combo box deb state

   \return
   Combo box deb state
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscViewNodeUpdate::E_StateDebugger C_SyvUpPacSecurityOptionsPopUp::GetComboBoxDebState() const
{
   C_OscViewNodeUpdate::E_StateDebugger e_Retval = C_OscViewNodeUpdate::eST_DEB_NO_CHANGE;
   switch (this->mpc_Ui->pc_ComboBoxDebugger->currentIndex())
   {
   case C_SyvUpPacSecurityOptionsPopUp::mhs32_DEB_INDEX_NO_CHANGE:
      e_Retval = C_OscViewNodeUpdate::eST_DEB_NO_CHANGE;
      break;
   case C_SyvUpPacSecurityOptionsPopUp::mhs32_DEB_INDEX_ACTIVATE:
      e_Retval = C_OscViewNodeUpdate::eST_DEB_ACTIVATE;
      break;
   case C_SyvUpPacSecurityOptionsPopUp::mhs32_DEB_INDEX_DEACTIVATE:
      e_Retval = C_OscViewNodeUpdate::eST_DEB_DEACTIVATE;
      break;
   default:
      tgl_assert(false);
      break;
   }
   return e_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get combo box ten state

   \return
   Combo box ten state
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscViewNodeUpdate::E_StateTrafficEncryption C_SyvUpPacSecurityOptionsPopUp::GetComboBoxTenState() const
{
   C_OscViewNodeUpdate::E_StateTrafficEncryption e_Retval = C_OscViewNodeUpdate::eST_TEN_NO_CHANGE;
   switch (this->mpc_Ui->pc_ComboBoxTrafficEncryption->currentIndex())
   {
   case C_SyvUpPacSecurityOptionsPopUp::mhs32_TEN_INDEX_NO_CHANGE:
      e_Retval = C_OscViewNodeUpdate::eST_TEN_NO_CHANGE;
      break;
   case C_SyvUpPacSecurityOptionsPopUp::mhs32_TEN_INDEX_ACTIVATE:
      e_Retval = C_OscViewNodeUpdate::eST_TEN_ACTIVATE;
      break;
   case C_SyvUpPacSecurityOptionsPopUp::mhs32_TEN_INDEX_DEACTIVATE:
      e_Retval = C_OscViewNodeUpdate::eST_TEN_DEACTIVATE;
      break;
   default:
      tgl_assert(false);
      break;
   }
   return e_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of Ok button click
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvUpPacSecurityOptionsPopUp::m_OkClicked(void)
{
   this->mrc_ParentDialog.accept();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of Cancel button click
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvUpPacSecurityOptionsPopUp::m_CancelClicked()
{
   this->mrc_ParentDialog.reject();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Init combo box sec

   \param[in]  oe_StateSecureAuthentication  State security
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvUpPacSecurityOptionsPopUp::m_InitComboBoxSec(
   const C_OscViewNodeUpdate::E_StateSecureAuthentication oe_StateSecureAuthentication)
{
   //lint -e{9042} Warning wanted if new cases are added
   switch (oe_StateSecureAuthentication)
   {
   case C_OscViewNodeUpdate::eST_SEC_NO_CHANGE:
      this->mpc_Ui->pc_ComboBoxAuth->setCurrentIndex(C_SyvUpPacSecurityOptionsPopUp::mhs32_SEC_INDEX_NO_CHANGE);
      break;
   case C_OscViewNodeUpdate::eST_SEC_ACTIVATE:
      this->mpc_Ui->pc_ComboBoxAuth->setCurrentIndex(C_SyvUpPacSecurityOptionsPopUp::mhs32_SEC_INDEX_ACTIVATE);
      break;
   case C_OscViewNodeUpdate::eST_SEC_DEACTIVATE:
      this->mpc_Ui->pc_ComboBoxAuth->setCurrentIndex(C_SyvUpPacSecurityOptionsPopUp::mhs32_SEC_INDEX_DEACTIVATE);
      break;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Init combo box deb

   \param[in]  oe_StateDebugger  State debugger
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvUpPacSecurityOptionsPopUp::m_InitComboBoxDeb(const C_OscViewNodeUpdate::E_StateDebugger oe_StateDebugger)
{
   //lint -e{9042} Warning wanted if new cases are added
   switch (oe_StateDebugger)
   {
   case C_OscViewNodeUpdate::eST_DEB_NO_CHANGE:
      this->mpc_Ui->pc_ComboBoxDebugger->setCurrentIndex(C_SyvUpPacSecurityOptionsPopUp::mhs32_DEB_INDEX_NO_CHANGE);
      break;
   case C_OscViewNodeUpdate::eST_DEB_ACTIVATE:
      this->mpc_Ui->pc_ComboBoxDebugger->setCurrentIndex(C_SyvUpPacSecurityOptionsPopUp::mhs32_DEB_INDEX_ACTIVATE);
      break;
   case C_OscViewNodeUpdate::eST_DEB_DEACTIVATE:
      this->mpc_Ui->pc_ComboBoxDebugger->setCurrentIndex(C_SyvUpPacSecurityOptionsPopUp::mhs32_DEB_INDEX_DEACTIVATE);
      break;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Init combo box ten

   \param[in]  oe_StateTrafficEncryption  State Traffic Encryption
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvUpPacSecurityOptionsPopUp::m_InitComboBoxTen(
   const C_OscViewNodeUpdate::E_StateTrafficEncryption oe_StateTrafficEncryption)
{
   //lint -e{9042} Warning wanted if new cases are added
   switch (oe_StateTrafficEncryption)
   {
   case C_OscViewNodeUpdate::eST_TEN_NO_CHANGE:
      this->mpc_Ui->pc_ComboBoxTrafficEncryption->setCurrentIndex(
         C_SyvUpPacSecurityOptionsPopUp::mhs32_TEN_INDEX_NO_CHANGE);
      break;
   case C_OscViewNodeUpdate::eST_TEN_ACTIVATE:
      this->mpc_Ui->pc_ComboBoxTrafficEncryption->setCurrentIndex(
         C_SyvUpPacSecurityOptionsPopUp::mhs32_TEN_INDEX_ACTIVATE);
      break;
   case C_OscViewNodeUpdate::eST_TEN_DEACTIVATE:
      this->mpc_Ui->pc_ComboBoxTrafficEncryption->setCurrentIndex(
         C_SyvUpPacSecurityOptionsPopUp::mhs32_TEN_INDEX_DEACTIVATE);
      break;
   }
}
