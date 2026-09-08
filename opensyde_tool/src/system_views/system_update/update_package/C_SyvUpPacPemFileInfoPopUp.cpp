//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Screen for pem file information

   Screen for pem file information

   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QFileInfo>
#include <system_error>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscSecurityPem.hpp"
#include "C_SyvUpPacPemFileInfoPopUp.hpp"
#include "ui_C_SyvUpPacPemFileInfoPopUp.h"
#include "C_SclStringCompat.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_core;
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

   \param[in,out]  orc_Parent    Reference to parent
   \param[in]      orc_Path      Path for parameter set
*/
//----------------------------------------------------------------------------------------------------------------------
C_SyvUpPacPemFileInfoPopUp::C_SyvUpPacPemFileInfoPopUp(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
                                                       const QString & orc_Path) :
   C_OgePopUpContentBase(orc_Parent, &orc_Parent),
   mpc_Ui(new Ui::C_SyvUpPacPemFileInfoPopUp)
{
   QString c_DisplayText;
   C_OscSecurityPem c_Pem;

   std::string c_ErrorMessage;
   const std::error_code c_Result = c_Pem.LoadFromFile(orc_Path.toStdString(), c_ErrorMessage);

   this->mpc_Ui->setupUi(this);

   InitStaticNames();

   this->mrc_ParentDialog.SetSubTitle(QFileInfo(orc_Path).fileName());

   if (!c_Result)
   {
      c_DisplayText += c_Pem.GetMetaInfos().c_str();
   }
   else
   {
      //lint -e{1946} Qt interface
      c_DisplayText += QString("Error loading file %1: %2").arg(orc_Path,
                                                                                        c_ErrorMessage.c_str());
   }
   this->mpc_Ui->pc_TextEditFlash_2->setText(c_DisplayText);
   this->mpc_Ui->pc_TextEditFlash_2->setReadOnly(true);

   // register the widget for showing
   this->mrc_ParentDialog.SetWidget(this);

   connect(this->mpc_Ui->pc_PushButtonOk, &QPushButton::clicked, this, &C_SyvUpPacPemFileInfoPopUp::m_OkClicked);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_SyvUpPacPemFileInfoPopUp::~C_SyvUpPacPemFileInfoPopUp(void)
{
   delete this->mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all displayed static names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvUpPacPemFileInfoPopUp::InitStaticNames(void) const
{
   this->mrc_ParentDialog.SetTitle("PEM File");
   this->mpc_Ui->pc_LabelHeadingPreview->setText("File Information");
   this->mpc_Ui->pc_PushButtonOk->setText("OK");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of Ok button click
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvUpPacPemFileInfoPopUp::m_OkClicked(void)
{
   this->mrc_ParentDialog.accept();
}
