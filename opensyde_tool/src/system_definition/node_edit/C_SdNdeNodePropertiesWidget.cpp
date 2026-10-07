//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Properties dialog for node properties

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QCheckBox>
#include <QSpinBox>
#include <QFile>
#include <QFileInfo>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>

#include "C_Uti.hpp"
#include "C_PuiSdUtil.hpp"
#include "stwerrors.hpp"
#include "C_SdNdeNodePropertiesWidget.hpp"
#include "ui_C_SdNdeNodePropertiesWidget.h"
#include "C_OscUtils.hpp"
#include "C_SdNdeComIfSettingsTableDelegate.hpp"
#include "C_PuiSdHandler.hpp"
#include "TglUtils.hpp"
#include "C_SdUtil.hpp"
#include "C_OscNodeProperties.hpp"
#include <QCheckBox>
#include "C_OgeWiUtil.hpp"
#include "C_OgeChxTristateBase.hpp"
#include "C_OscNodeComInterfaceSettings.hpp"
#include "C_SdNdeIpAddressConfigurationWidget.hpp"
#include <QLabel>
#include "C_OgeWiCustomMessage.hpp"
#include "C_SdNdeNodeEditWidget.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::opensyde_core;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui_elements;
using namespace stw::scl;
using namespace stw::tgl;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const uint16_t mu16_NODE_IMG_WIDTH = 300;

const uint8_t mu8_FL_INDEX_OS = 0;
const uint8_t mu8_FL_INDEX_UDS = 1;
const uint8_t mu8_FL_INDEX_NOSUPPORT = 2;

const int32_t C_SdNdeNodePropertiesWidget::mhs32_PR_INDEX_DISABLED = 0;
const int32_t C_SdNdeNodePropertiesWidget::mhs32_PR_INDEX_ENABLED = 1;

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in,out]  opc_Parent    Reference to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeNodePropertiesWidget::C_SdNdeNodePropertiesWidget(QWidget * const opc_Parent) :
   QWidget(opc_Parent),
   mpc_Ui(new Ui::C_SdNdeNodePropertiesWidget),
   mu32_NodeIndex(0),
   mu32_BusIndex(0),
   mc_BusName("")
{
   // init UI
   mpc_Ui->setupUi(this);

   //style of subnode information
   this->mpc_Ui->pc_LabSubNodeTitle->SetBackgroundColor(0);
   this->mpc_Ui->pc_LabSubNodeTitle->SetForegroundColor(4);
   this->mpc_Ui->pc_LabSubNodeTitle->SetFontPixel(13);
   this->mpc_Ui->pc_LabSubNodeName->SetBackgroundColor(11);
   this->mpc_Ui->pc_LabSubNodeName->SetForegroundColor(1);
   this->mpc_Ui->pc_LabSubNodeName->SetFontPixel(13);

   //allow to open link to manufacturer page in system standard browser
   this->mpc_Ui->pc_LabelProductPageLink->setOpenExternalLinks(true);

   InitStaticNames();

   //table setups
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(static_cast<int32_t> (
                                                                      C_SdNdeComIfSettingsTableDelegate::eINTERFACE))->
   setTextAlignment(static_cast<int32_t> (Qt::AlignLeft));
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(static_cast<int32_t> (
                                                                      C_SdNdeComIfSettingsTableDelegate::eCONNECTION))->
   setTextAlignment(static_cast<int32_t> (Qt::AlignLeft));
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(static_cast<int32_t> (
                                                                      C_SdNdeComIfSettingsTableDelegate::eNODEID))->
   setTextAlignment(static_cast<int32_t> (Qt::AlignHCenter));
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(static_cast<int32_t> (
                                                                      C_SdNdeComIfSettingsTableDelegate::eJ1939_ADDRESS))
   ->setTextAlignment(static_cast<int32_t> (Qt::AlignHCenter));
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(static_cast<int32_t> (
                                                                      C_SdNdeComIfSettingsTableDelegate::eIPADDRESS))->
   setTextAlignment(static_cast<int32_t> (Qt::AlignHCenter));
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(static_cast<int32_t> (
                                                                      C_SdNdeComIfSettingsTableDelegate::eUPDATE))->
   setTextAlignment(static_cast<int32_t> (Qt::AlignHCenter));
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(static_cast<int32_t> (
                                                                      C_SdNdeComIfSettingsTableDelegate::eROUTING))->
   setTextAlignment(static_cast<int32_t> (Qt::AlignHCenter));
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(static_cast<int32_t> (
                                                                      C_SdNdeComIfSettingsTableDelegate::eDIAGNOSTIC))->
   setTextAlignment(static_cast<int32_t> (Qt::AlignHCenter));

   //set min column width (necessary for "Linked to..." strech column
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeader()->setMinimumSectionSize(150);

   //setup column size mode and size
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeader()->setSectionResizeMode(static_cast<int32_t> (
                                                                                          C_SdNdeComIfSettingsTableDelegate
                                                                                          ::eINTERFACE),
                                                                                       QHeaderView::Fixed);

   this->mpc_Ui->pc_TableWidgetComIfSettings->setColumnWidth(static_cast<int32_t> (C_SdNdeComIfSettingsTableDelegate::
                                                                                   eINTERFACE), 150);

   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeader()->setSectionResizeMode(static_cast<int32_t> (
                                                                                          C_SdNdeComIfSettingsTableDelegate
                                                                                          ::eCONNECTION),
                                                                                       QHeaderView::Stretch);

   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeader()->setSectionResizeMode(static_cast<int32_t> (
                                                                                          C_SdNdeComIfSettingsTableDelegate
                                                                                          ::eNODEID),
                                                                                       QHeaderView::Fixed);
   this->mpc_Ui->pc_TableWidgetComIfSettings->setColumnWidth(static_cast<int32_t> (C_SdNdeComIfSettingsTableDelegate::
                                                                                   eNODEID), 150);

   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeader()->setSectionResizeMode(static_cast<int32_t> (
                                                                                          C_SdNdeComIfSettingsTableDelegate
                                                                                          ::eJ1939_ADDRESS),
                                                                                       QHeaderView::Fixed);
   this->mpc_Ui->pc_TableWidgetComIfSettings->setColumnWidth(static_cast<int32_t> (C_SdNdeComIfSettingsTableDelegate::
                                                                                   eJ1939_ADDRESS), 150);

   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeader()->setSectionResizeMode(static_cast<int32_t> (
                                                                                          C_SdNdeComIfSettingsTableDelegate
                                                                                          ::eIPADDRESS),
                                                                                       QHeaderView::Fixed);
   this->mpc_Ui->pc_TableWidgetComIfSettings->setColumnWidth(static_cast<int32_t> (C_SdNdeComIfSettingsTableDelegate::
                                                                                   eIPADDRESS), 150);

   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeader()->setSectionResizeMode(static_cast<int32_t> (
                                                                                          C_SdNdeComIfSettingsTableDelegate
                                                                                          ::eUPDATE),
                                                                                       QHeaderView::Fixed);
   this->mpc_Ui->pc_TableWidgetComIfSettings->setColumnWidth(static_cast<int32_t> (C_SdNdeComIfSettingsTableDelegate::
                                                                                   eUPDATE), 150);

   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeader()->setSectionResizeMode(static_cast<int32_t> (
                                                                                          C_SdNdeComIfSettingsTableDelegate
                                                                                          ::eROUTING),
                                                                                       QHeaderView::Fixed);
   this->mpc_Ui->pc_TableWidgetComIfSettings->setColumnWidth(static_cast<int32_t> (C_SdNdeComIfSettingsTableDelegate::
                                                                                   eROUTING), 150);

   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeader()->setSectionResizeMode(static_cast<int32_t> (
                                                                                          C_SdNdeComIfSettingsTableDelegate
                                                                                          ::eDIAGNOSTIC),
                                                                                       QHeaderView::Fixed);
   this->mpc_Ui->pc_TableWidgetComIfSettings->setColumnWidth(static_cast<int32_t> (C_SdNdeComIfSettingsTableDelegate::
                                                                                   eDIAGNOSTIC), 150);

   //Name restriction
   this->mpc_Ui->pc_LineEditNodeName->setMaxLength(C_PuiSdHandler::h_GetInstance()->GetNameMaxCharLimit());

   // connects
   connect(this->mpc_Ui->pc_LineEditNodeName, &QLineEdit::textChanged, this,
           &C_SdNdeNodePropertiesWidget::m_CheckNodeName);
   connect(this->mpc_Ui->pc_LineEditNodeName, &QLineEdit::editingFinished, this,
           &C_SdNdeNodePropertiesWidget::m_TrimNodeName);
   connect(this->mpc_Ui->pc_TableWidgetComIfSettings, &QTableWidget::cellChanged, this,
           &C_SdNdeNodePropertiesWidget::m_CheckComInterface);

   connect(this->mpc_Ui->pc_TableWidgetComIfSettings, &QTableWidget::cellClicked, this,
           &C_SdNdeNodePropertiesWidget::m_HandleCellClick);
   //lint -e{929} Cast required to avoid ambiguous signal of qt interface
   connect(this->mpc_Ui->pc_ComboBoxProtocol,
           static_cast<void (QComboBox::*)(int32_t)>(&QComboBox::currentIndexChanged), this,
           &C_SdNdeNodePropertiesWidget::m_SupportedProtocolChange);
   connect(this->mpc_Ui->pc_ComboBoxXAppSupport,
           static_cast<void (QComboBox::*)(int32_t)>(&QComboBox::currentIndexChanged), this,
           &C_SdNdeNodePropertiesWidget::m_XappSupportChange);

   // see m_BusBitrateClicked for details
   this->mc_Timer.setSingleShot(true);
   this->mc_Timer.setInterval(100);
   connect(&this->mc_Timer, &QTimer::timeout, this, &C_SdNdeNodePropertiesWidget::m_OpenBus);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_SdNdeNodePropertiesWidget::~C_SdNdeNodePropertiesWidget(void)
{
   delete mpc_Ui;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all displayed static names
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::InitStaticNames(void) const
{
   QString c_InterfaceString;
   QString c_ConnectString;

   const int32_t s32_COL_INTERFACE = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eINTERFACE);
   const int32_t s32_COL_CONNECTION = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eCONNECTION);
   const int32_t s32_COL_NODE_ID = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eNODEID);
   const int32_t s32_COL_IP_ADDRESS = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eIPADDRESS);
   const int32_t s32_COL_UPDATE = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eUPDATE);
   const int32_t s32_COL_ROUTING = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eROUTING);
   const int32_t s32_COL_DIAGNOSTIC = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eDIAGNOSTIC);

   this->mpc_Ui->pc_LabSubNodeTitle->setText("Sub-Node");
   this->mpc_Ui->pc_LabelName->setText("Name");
   this->mpc_Ui->pc_LabelComment->setText("Comment");
   this->mpc_Ui->pc_LabelConfiguration->setText("Configuration");
   this->mpc_Ui->pc_LabelProtocol->setText("Protocol Support");
   this->mpc_Ui->pc_LabelProgramming->setText("Programming Support");
   this->mpc_Ui->pc_LabelXAppSupport->setText("X.App Support");
   this->mpc_Ui->pc_LabelComIfSettings->setText("Communication Interfaces Settings");

   this->mpc_Ui->pc_ComboBoxProtocol->addItem("openSYDE");
   this->mpc_Ui->pc_ComboBoxProtocol->addItem("UDS");
   this->mpc_Ui->pc_ComboBoxProtocol->addItem("None");

   //UDS addressing (shown when the protocol is UDS)
   this->mpc_Ui->pc_LabelUdsAddressing->setText("UDS Addressing");
   this->mpc_Ui->pc_LabelUdsRequestId->setText("Request ID");
   this->mpc_Ui->pc_LabelUdsResponseId->setText("Response ID");
   this->mpc_Ui->pc_LabelUdsFunctionalId->setText("Functional ID");
   this->mpc_Ui->pc_LabelUdsExtendedId->setText("29 bit identifiers");
   this->mpc_Ui->pc_LabelUdsPadFrames->setText("Pad frames to 8 bytes");
   this->mpc_Ui->pc_LabelUdsExtendedId->SetToolTipInformation(
      "29 bit identifiers", "Checked: the identifiers above are 29 bit (extended). Unchecked: 11 bit.");
   this->mpc_Ui->pc_LabelUdsPadFrames->SetToolTipInformation(
      "Pad frames", "Checked: every frame to the node is padded to 8 data bytes, as most UDS servers require.");
   this->mpc_Ui->pc_LabelUdsRequestId->SetToolTipInformation(
      "Request ID", "CAN identifier the tool sends requests on (client to node), e.g. 0x7E0.");
   this->mpc_Ui->pc_LabelUdsResponseId->SetToolTipInformation(
      "Response ID", "CAN identifier the node answers on (node to client), e.g. 0x7E8.");
   this->mpc_Ui->pc_LabelUdsFunctionalId->SetToolTipInformation(
      "Functional ID", "CAN identifier for requests to every node on the bus, e.g. 0x7DF. 0 if the node has none.");
   this->mpc_Ui->pc_CheckBoxUdsExtendedId->SetToolTipInformation(
      "29 bit identifiers", "Checked: the identifiers above are 29 bit (extended). Unchecked: 11 bit.");
   this->mpc_Ui->pc_CheckBoxUdsPadFrames->SetToolTipInformation(
      "Pad frames", "Checked: every frame to the node is padded to 8 data bytes, as most UDS servers require.");
   this->mpc_Ui->pc_SpinBoxUdsRequestId->setPrefix("0x");
   this->mpc_Ui->pc_SpinBoxUdsResponseId->setPrefix("0x");
   this->mpc_Ui->pc_SpinBoxUdsFunctionalId->setPrefix("0x");
   this->mpc_Ui->pc_SpinBoxUdsRequestId->setDisplayIntegerBase(16);
   this->mpc_Ui->pc_SpinBoxUdsResponseId->setDisplayIntegerBase(16);
   this->mpc_Ui->pc_SpinBoxUdsFunctionalId->setDisplayIntegerBase(16);
   this->m_SetUdsIdRange(true);

   this->mpc_Ui->pc_ComboBoxProgramming->addItem("Disabled");
   this->mpc_Ui->pc_ComboBoxProgramming->addItem("Enabled");

   this->mpc_Ui->pc_ComboBoxXAppSupport->addItem("Disabled");
   this->mpc_Ui->pc_ComboBoxXAppSupport->addItem("Enabled");

   //table column text
   //fake padding with Spaces. No other solution known so far
   c_InterfaceString = "          ";
   c_InterfaceString.append("Interface");
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(s32_COL_INTERFACE)->setText(c_InterfaceString);

   //fake padding with Spaces. No other solution known so far
   c_ConnectString = "  ";
   c_ConnectString.append("Linked to...");
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(s32_COL_CONNECTION)->setText(c_ConnectString);
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(s32_COL_NODE_ID)->setText("Node ID");
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(
      static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eJ1939_ADDRESS))->setText("J1939 Address");
   this->mpc_Ui->pc_TableWidgetComIfSettings->SetToolTipHeadingAt(
      static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eJ1939_ADDRESS), Qt::Horizontal, "J1939 Address",
      "The node's J1939 source address on this bus (0x00..0xFD). It is written into the source address byte of every "
      "J1939 message the node sends, so the messages follow the node rather than each carrying its own address.\n"
      "0xFE is the null address: the node has no J1939 address on this bus and its messages are left as they are.");
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(s32_COL_IP_ADDRESS)->setText("IP Address");
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(s32_COL_UPDATE)->setText("Usable for Update");
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(s32_COL_ROUTING)->setText("Usable for Routing");
   this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeaderItem(s32_COL_DIAGNOSTIC)->setText("Usable for Dashboard");

   this->mpc_Ui->pc_TextEditComment->setPlaceholderText("Add your comment here ...");

   //Tool tips
   this->mpc_Ui->pc_LabSubNodeTitle->SetToolTipInformation("Sub-Node",
                                                           "Name of the Sub-Node.");

   this->mpc_Ui->pc_LabelName->SetToolTipInformation("Name",
                                                     static_cast<QString>("Symbolic node name. Unique within Network Topology.\n"
                                                                             "\nFollowing C naming conventions are required:"
                                                                             "\n - must not be empty"
                                                                             "\n - must not start with digits"
                                                                             "\n - only alphanumeric characters and \"_\""
                                                                             "\n - should not be longer than %1 (= project setting) characters").arg(
                                                        C_PuiSdHandler::h_GetInstance()->GetNameMaxCharLimit()));

   this->mpc_Ui->pc_LabelComment->SetToolTipInformation("Comment",
                                                        "Comment for this node.");

   this->mpc_Ui->pc_LabelProgramming->SetToolTipInformation("Programming Support",
                                                            "This property shows if the device is user programmable."
                                                               "\nDefined in read only *.syde_devdef file."
                                                               "\n\nIf enabled, the source code generation feature can "
                                                               "be activated for Data Blocks .");
   this->mpc_Ui->pc_LabelXAppSupport->SetToolTipInformation("X.App Support",
                                                            "Node properties option, available only for file-based targets.\n\n"
                                                               "If enabled:\n"
                                                               "- The X.App configuration support in Data Blocks is enabled\n"
                                                               "- The tab Data Logger is enabled");

   this->mpc_Ui->pc_LabelProtocol->SetToolTipInformation("Protocol Support",
                                                         "Type of Flashloader and diagnostic server.\n"
                                                            "Options:\n"
                                                            "   - openSYDE: openSYDE server and openSYDE Flashloader support\n"
                                                            "   - none: no STW protocol support (e.g.: 3rd party node)\n"
                                                            "\nSupported protocols defined in read only "
                                                            "*.syde_devdef file.");

   this->mpc_Ui->pc_TableWidgetComIfSettings->SetToolTipHeadingAt(s32_COL_INTERFACE, Qt::Horizontal,
                                                                  "Interface",
                                                                  "Name of communication interface (CAN/ETHERNET).");

   this->mpc_Ui->pc_TableWidgetComIfSettings->SetToolTipHeadingAt(s32_COL_CONNECTION, Qt::Horizontal,
                                                                  "Linked to...",
                                                                  "Name of bus to which the interface is linked to.");

   this->mpc_Ui->pc_TableWidgetComIfSettings->SetToolTipHeadingAt(s32_COL_NODE_ID, Qt::Horizontal,
                                                                  "Node ID",
                                                                  "Node ID is unique on connected bus. The ID is "
                                                                     "used for addressing in the communication "
                                                                     "protocol. \nThis property is configured for all "
                                                                     "connected interfaces on device while \"Device "
                                                                     "configuration\" (SYSTEM COMMISSIONING/Setup).");

   this->mpc_Ui->pc_TableWidgetComIfSettings->SetToolTipHeadingAt(s32_COL_IP_ADDRESS, Qt::Horizontal,
                                                                  "IP Address",
                                                                  "IP address settings: IP address and subnet mask"
                                                                     "\nThese properties are configured for all "
                                                                     "connected interfaces on device while \"Device "
                                                                     "configuration\" (SYSTEM COMMISSIONING/Setup)");

   this->mpc_Ui->pc_TableWidgetComIfSettings->SetToolTipHeadingAt(s32_COL_UPDATE, Qt::Horizontal,
                                                                  "Usable for Update",
                                                                  "If enabled, the interface is usable for System Update. "
                                                                     "(SYSTEM COMMISSIONING - Update)"
                                                                     "\n\nThis property is just "
                                                                     "a configuration for openSYDE PC tool, "
                                                                     "it is NOT configured on device.");

   this->mpc_Ui->pc_TableWidgetComIfSettings->SetToolTipHeadingAt(s32_COL_DIAGNOSTIC, Qt::Horizontal,
                                                                  "Usable for Dashboard",
                                                                  "If enabled, the interface is usable for Dashboard "
                                                                     "(Access of Datapool data elements via diagnostic protocol). "
                                                                     "(SYSTEM COMMISSIONING - Dashboards)"
                                                                     "\n\nThis property is just "
                                                                     "a configuration for openSYDE PC tool, "
                                                                     "it is NOT configured on device.");

   this->mpc_Ui->pc_TableWidgetComIfSettings->SetToolTipHeadingAt(s32_COL_ROUTING, Qt::Horizontal,
                                                                  "Usable for Routing",
                                                                  "If enabled, the interface is usable for Routing. "
                                                                     "\nAttention: This property is intended as additive "
                                                                     "feature in addition to \"System Update\" and \"Dashboards\" "
                                                                     "\nUse cases: SYSTEM COMMISSIONING - Update "
                                                                     "SYSTEM COMMISSIONING - Dashboards.\n\nThis "
                                                                     "property is just a configuration for openSYDE tool, "
                                                                     "it is NOT configured on device.");

}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Selects the node name in the text edit for fast editing
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::SelectName(void) const
{
   this->mpc_Ui->pc_LineEditNodeName->setFocus();
   this->mpc_Ui->pc_LineEditNodeName->selectAll();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Node ID setter

   Sets the private node id of widget

   \param[in]  ou32_NodeIndex    new node id
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::SetNodeId(const uint32_t ou32_NodeIndex)
{
   this->mu32_NodeIndex = ou32_NodeIndex;

   //load node data
   this->m_LoadFromData();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load node information

   Load node information from core data using node index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_LoadFromData(void)
{
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);

   //Disconnects for RegisterChange
   disconnect(this->mpc_Ui->pc_LineEditNodeName, &QLineEdit::editingFinished, this,
              &C_SdNdeNodePropertiesWidget::m_RegisterNameChange);
   disconnect(this->mpc_Ui->pc_TextEditComment, &QTextEdit::textChanged, this,
              &C_SdNdeNodePropertiesWidget::m_RegisterChange);
   disconnect(this->mpc_Ui->pc_TableWidgetComIfSettings, &QTableWidget::cellChanged, this,
              &C_SdNdeNodePropertiesWidget::m_RegisterErrorChange);
   disconnect(this->mpc_Ui->pc_TableWidgetComIfSettings, &QTableWidget::cellChanged, this,
              &C_SdNdeNodePropertiesWidget::m_CheckComInterface);
   //lint -e{929} Cast required to avoid ambiguous signal of qt interface
   disconnect(this->mpc_Ui->pc_ComboBoxProtocol,
              static_cast<void (QComboBox::*)(int32_t)>(&QComboBox::currentIndexChanged), this,
              &C_SdNdeNodePropertiesWidget::m_SupportedProtocolChange);
   disconnect(this->mpc_Ui->pc_ComboBoxXAppSupport,
              static_cast<void (QComboBox::*)(int32_t)>(&QComboBox::currentIndexChanged), this,
              &C_SdNdeNodePropertiesWidget::m_XappSupportChange);
   disconnect(this->mpc_Ui->pc_SpinBoxUdsRequestId, static_cast<void (QSpinBox::*)(int32_t)>(&QSpinBox::valueChanged),
              this, &C_SdNdeNodePropertiesWidget::m_UdsAddressingChanged);
   disconnect(this->mpc_Ui->pc_SpinBoxUdsResponseId, static_cast<void (QSpinBox::*)(int32_t)>(&QSpinBox::valueChanged),
              this, &C_SdNdeNodePropertiesWidget::m_UdsAddressingChanged);
   disconnect(this->mpc_Ui->pc_SpinBoxUdsFunctionalId,
              static_cast<void (QSpinBox::*)(int32_t)>(&QSpinBox::valueChanged),
              this, &C_SdNdeNodePropertiesWidget::m_UdsAddressingChanged);
   disconnect(this->mpc_Ui->pc_CheckBoxUdsExtendedId, &QCheckBox::toggled, this,
              &C_SdNdeNodePropertiesWidget::m_UdsExtendedIdChanged);
   disconnect(this->mpc_Ui->pc_CheckBoxUdsPadFrames, &QCheckBox::toggled, this,
              &C_SdNdeNodePropertiesWidget::m_UdsAddressingChanged);

   tgl_assert(pc_Node != nullptr);
   if (pc_Node != nullptr)
   {
      QPixmap c_ImgNode;
      QString c_ComIfName;
      QString c_BusName;
      const C_OscDeviceDefinition * const pc_DevDef = pc_Node->pc_DeviceDefinition;
      const uint32_t u32_SubDeviceIndex = pc_Node->u32_SubDeviceIndex;
      tgl_assert(pc_DevDef != nullptr);

      //create delegate
      this->mpc_Ui->pc_TableWidgetComIfSettings->setItemDelegate(new C_SdNdeComIfSettingsTableDelegate(this,
                                                                                                       this->
                                                                                                       mu32_NodeIndex));

      if (pc_DevDef != nullptr)
      {
         tgl_assert(u32_SubDeviceIndex < pc_DevDef->c_SubDevices.size());
         if (u32_SubDeviceIndex < pc_DevDef->c_SubDevices.size())
         {
            QFileInfo c_FileInfoDevImg;
            bool q_FileExists;
            uint32_t u32_NodeSquadIndex;

            //node name
            this->mpc_Ui->pc_LineEditNodeName->setText(C_PuiSdUtil::h_GetNodeBaseNameOrName(this->mu32_NodeIndex));

            //sub node info
            if (C_PuiSdHandler::h_GetInstance()->GetNodeSquadIndexWithNodeIndex(this->mu32_NodeIndex,
                                                                                u32_NodeSquadIndex) == C_NO_ERR)
            {
               //squad node
               this->mpc_Ui->pc_SubNodeInfoWidget->setVisible(true);
               this->mpc_Ui->pc_LabSubNodeName->setText(C_PuiSdUtil::h_GetSubNodeDeviceName(this->mu32_NodeIndex));
            }
            else
            {
               //no squad node
               this->mpc_Ui->pc_SubNodeInfoWidget->setVisible(false);
            }

            //comment
            this->mpc_Ui->pc_TextEditComment->setText(pc_Node->c_Properties.c_Comment.c_str());

            //protocol: the node's own setting. The device definition seeds it when the node is placed and decides
            //whether openSYDE may be chosen at all; UDS and "None" are always available.
            {
               const C_OscSubDeviceDefinition & rc_SubDevice = pc_DevDef->c_SubDevices[u32_SubDeviceIndex];
               const bool q_OpenSydeSupported = (rc_SubDevice.q_FlashloaderOpenSydeEthernet == true) ||
                                                (rc_SubDevice.q_FlashloaderOpenSydeCan == true) ||
                                                (rc_SubDevice.q_DiagnosticProtocolOpenSydeEthernet == true) ||
                                                (rc_SubDevice.q_DiagnosticProtocolOpenSydeCan == true);
               QStandardItemModel * const pc_Model =
                  dynamic_cast<QStandardItemModel *>(this->mpc_Ui->pc_ComboBoxProtocol->model());
               if ((pc_Model != nullptr) && (pc_Model->item(mu8_FL_INDEX_OS) != nullptr))
               {
                  pc_Model->item(mu8_FL_INDEX_OS)->setEnabled(q_OpenSydeSupported);
               }
               this->mpc_Ui->pc_ComboBoxProtocol->setEnabled(true);
               if ((pc_Node->c_Properties.e_FlashLoader == C_OscNodeProperties::eFL_OPEN_SYDE) ||
                   (pc_Node->c_Properties.e_DiagnosticServer == C_OscNodeProperties::eDS_OPEN_SYDE))
               {
                  this->mpc_Ui->pc_ComboBoxProtocol->setCurrentIndex(mu8_FL_INDEX_OS);
               }
               else if ((pc_Node->c_Properties.e_FlashLoader == C_OscNodeProperties::eFL_UDS) ||
                        (pc_Node->c_Properties.e_DiagnosticServer == C_OscNodeProperties::eDS_UDS))
               {
                  this->mpc_Ui->pc_ComboBoxProtocol->setCurrentIndex(mu8_FL_INDEX_UDS);
               }
               else
               {
                  this->mpc_Ui->pc_ComboBoxProtocol->setCurrentIndex(mu8_FL_INDEX_NOSUPPORT);
               }
            }

            //UDS addressing: the values are always loaded, the block is shown for a UDS node only
            this->m_SetUdsIdRange(pc_Node->c_UdsConfig.q_ExtendedId);
            this->mpc_Ui->pc_CheckBoxUdsExtendedId->setChecked(pc_Node->c_UdsConfig.q_ExtendedId);
            this->mpc_Ui->pc_CheckBoxUdsPadFrames->setChecked(pc_Node->c_UdsConfig.q_PadFrames);
            this->mpc_Ui->pc_SpinBoxUdsRequestId->setValue(static_cast<int32_t>(pc_Node->c_UdsConfig.u32_RequestId));
            this->mpc_Ui->pc_SpinBoxUdsResponseId->setValue(static_cast<int32_t>(pc_Node->c_UdsConfig.u32_ResponseId));
            this->mpc_Ui->pc_SpinBoxUdsFunctionalId->setValue(
               static_cast<int32_t>(pc_Node->c_UdsConfig.u32_FunctionalId));
            this->mpc_Ui->pc_WidgetUds->setVisible(this->mpc_Ui->pc_ComboBoxProtocol->currentIndex() ==
                                                   mu8_FL_INDEX_UDS);

            //programming
            if (pc_DevDef->c_SubDevices[u32_SubDeviceIndex].q_ProgrammingSupport == true)
            {
               this->mpc_Ui->pc_ComboBoxProgramming->setCurrentIndex(C_SdNdeNodePropertiesWidget::mhs32_PR_INDEX_ENABLED);
            }
            else
            {
               this->mpc_Ui->pc_ComboBoxProgramming->setCurrentIndex(
                  C_SdNdeNodePropertiesWidget::mhs32_PR_INDEX_DISABLED);
            }

            //X-App Support
            if (pc_DevDef->c_SubDevices[u32_SubDeviceIndex].q_FlashloaderOpenSydeIsFileBased == true)
            {
               this->mpc_Ui->pc_LabelXAppSupport->setVisible(true);
               this->mpc_Ui->pc_ComboBoxXAppSupport->setVisible(true);
            }
            else
            {
               this->mpc_Ui->pc_LabelXAppSupport->setVisible(false);
               this->mpc_Ui->pc_ComboBoxXAppSupport->setVisible(false);
            }

            if (pc_Node->c_Properties.q_XappSupport == true)
            {
               this->mpc_Ui->pc_ComboBoxXAppSupport->setCurrentIndex(C_SdNdeNodePropertiesWidget::mhs32_PR_INDEX_ENABLED);
            }
            else
            {
               this->mpc_Ui->pc_ComboBoxXAppSupport->setCurrentIndex(
                  C_SdNdeNodePropertiesWidget::mhs32_PR_INDEX_DISABLED);
            }

            //load device picture
            c_FileInfoDevImg.setFile(pc_DevDef->c_ImagePath.c_str());
            q_FileExists = (c_FileInfoDevImg.exists() && c_FileInfoDevImg.isFile());

            //check if file exists
            if (q_FileExists == true)
            {
               c_ImgNode.load(pc_DevDef->c_ImagePath.c_str());
               c_ImgNode = c_ImgNode.scaled(mu16_NODE_IMG_WIDTH, c_ImgNode.height(), //second parameter is not relevant
                                            Qt::KeepAspectRatio, Qt::SmoothTransformation);
               this->mpc_Ui->pc_LabelNoImageAvailable->setVisible(false);
               this->mpc_Ui->pc_DatapoolTypeImage->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
            }
            else
            {
               //no image available
               const QIcon c_Icon("://images/system_definition/Image_Grey.svg");
               c_ImgNode = c_Icon.pixmap(QSize(mu16_NODE_IMG_WIDTH / 2, mu16_NODE_IMG_WIDTH / 2));
               this->mpc_Ui->pc_LabelNoImageAvailable->setVisible(true);
               this->mpc_Ui->pc_DatapoolTypeImage->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
            }

            //company logo and product page link
            if ((pc_DevDef->c_ManufacturerDisplayValue.empty() == false) &&
                (pc_DevDef->c_ManufacturerDisplayValue != "Sensor-Technik Wiedemann GmbH"))
            {
               QFileInfo c_FileInfoCompLogo;
               c_FileInfoCompLogo.setFile(pc_DevDef->c_CompanyLogoLink.c_str());
               q_FileExists = (c_FileInfoCompLogo.exists() && c_FileInfoCompLogo.isFile());

               if (q_FileExists == true)
               {
                  QPixmap c_ImgCompLogo;
                  c_ImgCompLogo.load(pc_DevDef->c_CompanyLogoLink.c_str());
                  c_ImgCompLogo = c_ImgCompLogo.scaled((c_ImgCompLogo.width() / 10), (c_ImgCompLogo.height() / 10),
                                                       Qt::KeepAspectRatio,
                                                       Qt::SmoothTransformation);
                  this->mpc_Ui->pc_CompanyLogo->setPixmap(c_ImgCompLogo);
               }

               this->mpc_Ui->pc_LabelProductPageLink->SetLink(
                  "Visit Product Page", pc_DevDef->c_ProductPageLink.c_str());
               this->mpc_Ui->pc_LabelProductPageLink->SetToolTipInformation(
                  pc_DevDef->c_ManufacturerDisplayValue.c_str(), pc_DevDef->c_ProductPageLink.c_str());
               this->mpc_Ui->pc_WidgetCompLogo->setVisible(true);
            }
            else
            {
               this->mpc_Ui->pc_WidgetCompLogo->setVisible(false);
            }

            this->mpc_Ui->pc_DatapoolTypeImage->setPixmap(c_ImgNode);

            //load device type name
            this->mpc_Ui->pc_LabelNodeType->setText(pc_DevDef->GetDisplayName().c_str());

            //clear table
            this->mpc_Ui->pc_TableWidgetComIfSettings->setRowCount(0);

            //insert can+ethernet count of rows
            this->mpc_Ui->pc_TableWidgetComIfSettings->setRowCount(static_cast<int32_t> (pc_DevDef->u8_NumCanBusses) +
                                                                   static_cast<int32_t> (pc_DevDef->u8_NumEthernetBusses));

            {
               //set new table max size (Header = 38 + row*40)
               const int32_t s32_MaxHeight = 38 + (this->mpc_Ui->pc_TableWidgetComIfSettings->rowCount() * 40);
               //Minimum is slightly off the optimal value (3 rows are visible)
               const int32_t s32_MinHeight = 38 +
                                             (std::min(3, this->mpc_Ui->pc_TableWidgetComIfSettings->rowCount()) * 40);
               this->mpc_Ui->pc_TableWidgetComIfSettings->setMinimumHeight(s32_MinHeight);
               this->mpc_Ui->pc_TableWidgetComIfSettings->setMaximumHeight(s32_MaxHeight);
            }

            //insert com ifs
            for (uint8_t u8_ComIfCnt = 0;
                 u8_ComIfCnt <
                 (static_cast<int32_t> (pc_DevDef->u8_NumCanBusses) +
                  static_cast<int32_t> (pc_DevDef->u8_NumEthernetBusses));
                 ++u8_ComIfCnt)
            {
               bool q_InterfaceIsConnected = false;

               const int32_t s32_COL_INTERFACE = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eINTERFACE);
               const int32_t s32_COL_CONNECTION = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eCONNECTION);
               const int32_t s32_COL_NODE_ID = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eNODEID);
               const int32_t s32_COL_IP_ADDRESS = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eIPADDRESS);
               const int32_t s32_COL_UPDATE = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eUPDATE);
               const int32_t s32_COL_ROUTING = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eROUTING);
               const int32_t s32_COL_DIAGNOSTIC = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eDIAGNOSTIC);
               bool q_IsUpdateAvailable;
               bool q_IsRoutingAvailable;
               bool q_IsDiagnosisAvailable;
               bool q_IsRoutingEnabled = true;
               if (u8_ComIfCnt < pc_DevDef->u8_NumCanBusses)
               {
                  q_IsUpdateAvailable =
                     pc_DevDef->c_SubDevices[u32_SubDeviceIndex].IsUpdateAvailable(C_OscSystemBus::eCAN);
                  q_IsRoutingAvailable = pc_Node->IsRoutingAvailable(C_OscSystemBus::eCAN);
                  q_IsDiagnosisAvailable = pc_Node->IsDiagnosisAvailable(C_OscSystemBus::eCAN);
               }
               else
               {
                  q_IsUpdateAvailable = pc_DevDef->c_SubDevices[u32_SubDeviceIndex].IsUpdateAvailable(
                     C_OscSystemBus::eETHERNET);
                  q_IsRoutingAvailable = pc_Node->IsRoutingAvailable(C_OscSystemBus::eETHERNET);
                  q_IsDiagnosisAvailable = pc_Node->IsDiagnosisAvailable(C_OscSystemBus::eETHERNET);
               }

               QString c_IpAddressString;

               /**********************************************************************************************************/
               //INTERFACE
               {
                  QCheckBox * const pc_ChkIf = new QCheckBox(this);
                  pc_ChkIf->setProperty("styleRole", "chx-tristate-transparent-error");
                  this->mpc_Ui->pc_TableWidgetComIfSettings->setCellWidget(u8_ComIfCnt, s32_COL_INTERFACE, pc_ChkIf);
               }
               //disable
               this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u8_ComIfCnt,
                                                                     s32_COL_INTERFACE)->setEnabled(false);

               //set name
               if (u8_ComIfCnt < pc_DevDef->u8_NumCanBusses)
               {
                  //its a CAN interface
                  c_ComIfName =
                     C_PuiSdUtil::h_GetInterfaceName(C_OscSystemBus::eCAN, static_cast<uint8_t>(u8_ComIfCnt));
               }
               else
               {
                  //its an Ethernet interface
                  c_ComIfName =
                     C_PuiSdUtil::h_GetInterfaceName(C_OscSystemBus::eETHERNET,
                                                     static_cast<uint8_t>(u8_ComIfCnt -
                                                                          static_cast<int32_t> (pc_DevDef->
                                                                                                u8_NumCanBusses)));
               }

               dynamic_cast<QCheckBox *> (this->mpc_Ui->pc_TableWidgetComIfSettings->
                                          cellWidget(u8_ComIfCnt, s32_COL_INTERFACE))->setText(c_ComIfName);

               /**********************************************************************************************************/
               //CONNECTED TO
               {
                  QLabel * const pc_LabelConn = new QLabel(this);
                  pc_LabelConn->setProperty("styleRole", "node-prop-com-if-table");
                  this->mpc_Ui->pc_TableWidgetComIfSettings->setCellWidget(u8_ComIfCnt, s32_COL_CONNECTION,
                                                                           pc_LabelConn);
               }

               //set bus name
               if (pc_Node->c_Properties.c_ComInterfaces[u8_ComIfCnt].GetBusConnected() == true)
               {
                  const C_OscSystemBus * const pc_Bus = C_PuiSdHandler::h_GetInstance()->GetOscBus(
                     pc_Node->c_Properties.c_ComInterfaces[u8_ComIfCnt].u32_BusIndex);
                  //get bus name
                  if (pc_Bus != nullptr)
                  {
                     c_BusName = pc_Bus->c_Name.c_str();

                     if (pc_Bus->e_Type == C_OscSystemBus::eCAN)
                     {
                        // add the bitrate
                        c_BusName += " (" + QString::number(pc_Bus->u64_BitRate / 1000ULL) + " kbit/s)";
                     }

                     // Is this bus usable for routing
                     q_IsRoutingEnabled = pc_Bus->q_UseableForRouting;
                  }
               }
               else
               {
                  c_BusName = "-"; // "not connected";
               }

               dynamic_cast<QLabel *> (this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(
                                          u8_ComIfCnt, s32_COL_CONNECTION))->setText(c_BusName);

               if (c_BusName != "-")
               {
                  this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u8_ComIfCnt, s32_COL_CONNECTION)->setEnabled(
                     true);
                  C_OgeWiUtil::h_ApplyStylesheetProperty(this->mpc_Ui->pc_TableWidgetComIfSettings
                                                         ->cellWidget(u8_ComIfCnt, s32_COL_CONNECTION),
                                                         "COMIF_TableCell_Hyperlink", true);
               }
               //if the node isn't connected to a bus, the link to bus definition shall not work
               else
               {
                  this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u8_ComIfCnt, s32_COL_CONNECTION)->setEnabled(
                     false);
               }

               /**********************************************************************************************************/
               //NODE ID
               this->mpc_Ui->pc_TableWidgetComIfSettings->setItem(u8_ComIfCnt, s32_COL_NODE_ID,
                                                                  new QTableWidgetItem(""));
               //align center
               this->mpc_Ui->pc_TableWidgetComIfSettings->item(u8_ComIfCnt, s32_COL_NODE_ID)
               ->setTextAlignment(static_cast<int32_t> (Qt::AlignCenter));
               //set node value
               this->mpc_Ui->pc_TableWidgetComIfSettings->item(u8_ComIfCnt, s32_COL_NODE_ID)->
               setText(QString::number(pc_Node->c_Properties.c_ComInterfaces[u8_ComIfCnt].u8_NodeId));

               this->mpc_Ui->pc_TableWidgetComIfSettings->item(u8_ComIfCnt, s32_COL_NODE_ID)->setFlags(
                  Qt::ItemIsEnabled | Qt::ItemIsEditable);
               /**********************************************************************************************************/
               //J1939 ADDRESS (CAN interfaces only)
               {
                  const int32_t s32_COL_J1939 = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eJ1939_ADDRESS);
                  QTableWidgetItem * const pc_Item = new QTableWidgetItem("");
                  pc_Item->setTextAlignment(static_cast<int32_t> (Qt::AlignCenter));
                  if (u8_ComIfCnt < pc_DevDef->u8_NumCanBusses)
                  {
                     pc_Item->setText("0x" + QString::number(
                                         pc_Node->c_Properties.c_ComInterfaces[u8_ComIfCnt].u8_J1939SourceAddress,
                                         16).toUpper());
                     pc_Item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
                  }
                  else
                  {
                     pc_Item->setText("-");
                     pc_Item->setFlags(Qt::ItemIsEnabled);
                  }
                  this->mpc_Ui->pc_TableWidgetComIfSettings->setItem(u8_ComIfCnt, s32_COL_J1939, pc_Item);
               }
               /**********************************************************************************************************/
               //IP Address
               {
                  QLabel * const pc_LabelIp = new QLabel(this);
                  pc_LabelIp->setProperty("styleRole", "node-prop-com-if-table");
                  this->mpc_Ui->pc_TableWidgetComIfSettings->setCellWidget(u8_ComIfCnt, s32_COL_IP_ADDRESS,
                                                                           pc_LabelIp);
               }

               //set IP Address
               if (u8_ComIfCnt >= static_cast<int32_t> (pc_DevDef->u8_NumCanBusses))
               {
                  c_IpAddressString.append(QString::number(pc_Node->c_Properties.c_ComInterfaces[u8_ComIfCnt].c_Ip.
                                                           au8_IpAddress[0]));
                  c_IpAddressString.append(".");
                  c_IpAddressString.append(QString::number(pc_Node->c_Properties.c_ComInterfaces[u8_ComIfCnt].c_Ip.
                                                           au8_IpAddress[1]));
                  c_IpAddressString.append(".");
                  c_IpAddressString.append(QString::number(pc_Node->c_Properties.c_ComInterfaces[u8_ComIfCnt].c_Ip.
                                                           au8_IpAddress[2]));
                  c_IpAddressString.append(".");
                  c_IpAddressString.append(QString::number(pc_Node->c_Properties.c_ComInterfaces[u8_ComIfCnt].c_Ip.
                                                           au8_IpAddress[3]));
               }
               else
               {
                  c_IpAddressString = "-";
               }

               dynamic_cast<QLabel *> (this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(
                                          u8_ComIfCnt, s32_COL_IP_ADDRESS))->setText(c_IpAddressString);

               dynamic_cast<QLabel *> (this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(
                                          u8_ComIfCnt, s32_COL_IP_ADDRESS))->setAlignment(Qt::AlignCenter);

               //apply enabled/disabled style
               //if CAN, disable per default
               if (u8_ComIfCnt < pc_DevDef->u8_NumCanBusses)
               {
                  this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u8_ComIfCnt,
                                                                        s32_COL_IP_ADDRESS)->setEnabled(false);
               }
               else
               {
                  this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u8_ComIfCnt,
                                                                        s32_COL_IP_ADDRESS)->setEnabled(true);
                  //apply special style
                  C_OgeWiUtil::h_ApplyStylesheetProperty(this->mpc_Ui->pc_TableWidgetComIfSettings
                                                         ->cellWidget(u8_ComIfCnt,
                                                                      s32_COL_IP_ADDRESS), "COMIF_TableCell_Hyperlink",
                                                         true);
               }

               //hide the ip address column for devices without Ethernet interfaces
               if (pc_DevDef->u8_NumEthernetBusses == 0)
               {
                  this->mpc_Ui->pc_TableWidgetComIfSettings->horizontalHeader()->hideSection(static_cast<int32_t> (
                                                                                                C_SdNdeComIfSettingsTableDelegate
                                                                                                ::eIPADDRESS));
               }

               /**********************************************************************************************************/
               // UPDATE
               {
                  C_OgeChxTristateBase * const pc_ChkUpdate = new C_OgeChxTristateBase(this);
                  pc_ChkUpdate->setProperty("styleRole", "chx-tristate");
                  this->mpc_Ui->pc_TableWidgetComIfSettings->setCellWidget(u8_ComIfCnt, s32_COL_UPDATE, pc_ChkUpdate);
               }
               //set node value
               if (q_IsUpdateAvailable == true)
               {
                  //defensive move
                  if ((pc_DevDef->u8_NumCanBusses) > 0)
                  {
                     dynamic_cast<C_OgeChxTristateBase *> (this->mpc_Ui->pc_TableWidgetComIfSettings
                                                       ->cellWidget(u8_ComIfCnt, s32_COL_UPDATE))
                     ->setChecked(pc_Node->c_Properties.c_ComInterfaces[u8_ComIfCnt].q_IsUpdateEnabled);
                  }
                  else
                  {
                     tgl_assert((pc_DevDef->u8_NumCanBusses - 1) <= 0);
                  }
               }
               else
               {
                  //disable
                  this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u8_ComIfCnt,
                                                                        s32_COL_UPDATE)->setEnabled(false);
               }
               //connect to RegisterChange
               connect(dynamic_cast<C_OgeChxTristateBase *> (this->mpc_Ui->pc_TableWidgetComIfSettings
                                                         ->cellWidget(u8_ComIfCnt,
                                                                      s32_COL_UPDATE)), &QCheckBox::stateChanged, this,
                       &C_SdNdeNodePropertiesWidget::m_RegisterChange);

               /**********************************************************************************************************/
               // ROUTING
               {
                  C_OgeChxTristateBase * const pc_ChkRouting = new C_OgeChxTristateBase(this);
                  pc_ChkRouting->setProperty("styleRole", "chx-tristate");
                  this->mpc_Ui->pc_TableWidgetComIfSettings->setCellWidget(u8_ComIfCnt, s32_COL_ROUTING, pc_ChkRouting);
               }

               if (q_IsRoutingAvailable == true)
               {
                  //set node value
                  dynamic_cast<C_OgeChxTristateBase *> (this->mpc_Ui->pc_TableWidgetComIfSettings
                                                    ->cellWidget(u8_ComIfCnt, s32_COL_ROUTING))
                  ->setChecked(pc_Node->c_Properties.c_ComInterfaces[u8_ComIfCnt].q_IsRoutingEnabled);

                  if (q_IsRoutingEnabled == false)
                  {
                     // Routing for the bus has been disabled
                     this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u8_ComIfCnt, s32_COL_ROUTING)->
                     setEnabled(false);
                  }
               }

               else
               {
                  //disable
                  this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u8_ComIfCnt, s32_COL_ROUTING)->
                  setEnabled(false);
               }

               //connect to RegisterChange
               connect(dynamic_cast<C_OgeChxTristateBase *> (this->mpc_Ui->pc_TableWidgetComIfSettings
                                                         ->cellWidget(u8_ComIfCnt,
                                                                      s32_COL_ROUTING)), &QCheckBox::stateChanged, this,
                       &C_SdNdeNodePropertiesWidget::m_RegisterChange);

               /**********************************************************************************************************/
               // DIAGNOSTIC
               {
                  C_OgeChxTristateBase * const pc_ChkDiag = new C_OgeChxTristateBase(this);
                  pc_ChkDiag->setProperty("styleRole", "chx-tristate");
                  this->mpc_Ui->pc_TableWidgetComIfSettings->setCellWidget(u8_ComIfCnt, s32_COL_DIAGNOSTIC, pc_ChkDiag);
               }

               if (q_IsDiagnosisAvailable == true)
               {
                  //set node value
                  dynamic_cast<C_OgeChxTristateBase *> (this->mpc_Ui->pc_TableWidgetComIfSettings
                                                    ->cellWidget(u8_ComIfCnt, s32_COL_DIAGNOSTIC))
                  ->setChecked(pc_Node->c_Properties.c_ComInterfaces[u8_ComIfCnt].q_IsDiagnosisEnabled);
               }
               else
               {
                  //disable
                  this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u8_ComIfCnt, s32_COL_DIAGNOSTIC)->
                  setEnabled(false);
                  dynamic_cast<C_OgeChxTristateBase *> (this->mpc_Ui->pc_TableWidgetComIfSettings
                                                    ->cellWidget(u8_ComIfCnt, s32_COL_DIAGNOSTIC))->setChecked(false);
               }

               //connect to RegisterChange
               connect(dynamic_cast<C_OgeChxTristateBase *> (
                          this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u8_ComIfCnt, s32_COL_DIAGNOSTIC)),
                       &QCheckBox::stateChanged, this, &C_SdNdeNodePropertiesWidget::m_RegisterChange);

               //hide rows if they are not connected (necessary for sub nodes)

               //check if interface is connected
               if (u8_ComIfCnt >= static_cast<int32_t> (pc_DevDef->u8_NumCanBusses))
               {
                  //ETH
                  q_InterfaceIsConnected =
                     pc_DevDef->c_SubDevices[u32_SubDeviceIndex].
                     IsConnected(C_OscSystemBus::eETHERNET,
                                 static_cast<uint8_t>(u8_ComIfCnt - static_cast<int32_t>(pc_DevDef->u8_NumCanBusses)));
               }
               else
               {
                  //CAN
                  q_InterfaceIsConnected =
                     pc_DevDef->c_SubDevices[u32_SubDeviceIndex].IsConnected(C_OscSystemBus::eCAN,
                                                                             static_cast<uint8_t>(u8_ComIfCnt));
               }

               //hide row if not connected
               if (q_InterfaceIsConnected == false)
               {
                  this->mpc_Ui->pc_TableWidgetComIfSettings->hideRow(u8_ComIfCnt);
               }
            }
         }
      }
   }

   // Initial check of the data. Row is irrelevant because all rows will be checked of the column.
   this->m_CheckComInterface(0U, static_cast<int32_t> (C_SdNdeComIfSettingsTableDelegate::eNODEID));

   //connects for RegisterChange
   connect(this->mpc_Ui->pc_LineEditNodeName, &QLineEdit::editingFinished, this,
           &C_SdNdeNodePropertiesWidget::m_RegisterNameChange);
   connect(this->mpc_Ui->pc_TextEditComment, &QTextEdit::textChanged, this,
           &C_SdNdeNodePropertiesWidget::m_RegisterChange);
   connect(this->mpc_Ui->pc_TableWidgetComIfSettings, &QTableWidget::cellChanged, this,
           &C_SdNdeNodePropertiesWidget::m_RegisterErrorChange);
   connect(this->mpc_Ui->pc_TableWidgetComIfSettings, &QTableWidget::cellChanged, this,
           &C_SdNdeNodePropertiesWidget::m_CheckComInterface);
   //lint -e{929} Cast required to avoid ambiguous signal of qt interface
   connect(this->mpc_Ui->pc_ComboBoxProtocol,
           static_cast<void (QComboBox::*)(int32_t)>(&QComboBox::currentIndexChanged), this,
           &C_SdNdeNodePropertiesWidget::m_SupportedProtocolChange);
   connect(this->mpc_Ui->pc_ComboBoxXAppSupport,
           static_cast<void (QComboBox::*)(int32_t)>(&QComboBox::currentIndexChanged), this,
           &C_SdNdeNodePropertiesWidget::m_XappSupportChange);
   connect(this->mpc_Ui->pc_SpinBoxUdsRequestId, static_cast<void (QSpinBox::*)(int32_t)>(&QSpinBox::valueChanged),
           this, &C_SdNdeNodePropertiesWidget::m_UdsAddressingChanged);
   connect(this->mpc_Ui->pc_SpinBoxUdsResponseId, static_cast<void (QSpinBox::*)(int32_t)>(&QSpinBox::valueChanged),
           this, &C_SdNdeNodePropertiesWidget::m_UdsAddressingChanged);
   connect(this->mpc_Ui->pc_SpinBoxUdsFunctionalId, static_cast<void (QSpinBox::*)(int32_t)>(&QSpinBox::valueChanged),
           this, &C_SdNdeNodePropertiesWidget::m_UdsAddressingChanged);
   connect(this->mpc_Ui->pc_CheckBoxUdsExtendedId, &QCheckBox::toggled, this,
           &C_SdNdeNodePropertiesWidget::m_UdsExtendedIdChanged);
   connect(this->mpc_Ui->pc_CheckBoxUdsPadFrames, &QCheckBox::toggled, this,
           &C_SdNdeNodePropertiesWidget::m_UdsAddressingChanged);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save ui data to node

   Is called from outside
      - on system definition save
      - on page change
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::SaveToData(void)
{
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);

   if (pc_Node != nullptr)
   {
      const C_OscDeviceDefinition * const pc_DevDef = pc_Node->pc_DeviceDefinition;
      const uint32_t u32_SubDeviceIndex = pc_Node->u32_SubDeviceIndex;
      tgl_assert(pc_DevDef != nullptr);

      if (pc_DevDef != nullptr)
      {
         tgl_assert(u32_SubDeviceIndex < pc_DevDef->c_SubDevices.size());
         if (u32_SubDeviceIndex < pc_DevDef->c_SubDevices.size())
         {
            QString c_Name;
            QString c_Comment;
            C_OscNodeProperties::E_DiagnosticServerProtocol e_DiagnosticServer;
            C_OscNodeProperties::E_FlashLoaderProtocol e_FlashLoader;
            std::vector<uint8_t> c_NodeIds;
            std::vector<uint8_t> c_J1939Addresses;
            std::vector<bool> c_UpdateFlags;
            std::vector<bool> c_RoutingFlags;
            std::vector<bool> c_DiagnosisFlags;
            const int32_t s32_COL_NODE_ID = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eNODEID);
            const int32_t s32_COL_UPDATE = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eUPDATE);
            const int32_t s32_COL_ROUTING = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eROUTING);
            const int32_t s32_COL_DIAGNOSTIC = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eDIAGNOSTIC);

            //save data

            //name
            //Only accept new name if not in conflict
            if (C_PuiSdHandler::h_GetInstance()->CheckNodeNameAvailable(
                   this->mpc_Ui->pc_LineEditNodeName->text().toStdString(), &this->mu32_NodeIndex, nullptr))
            {
               c_Name = this->mpc_Ui->pc_LineEditNodeName->text();
            }
            else
            {
               //Restore previous name
               uint32_t u32_NodeSquadIndex;

               if (C_PuiSdHandler::h_GetInstance()->GetNodeSquadIndexWithNodeIndex(this->mu32_NodeIndex,
                                                                                   u32_NodeSquadIndex) == C_NO_ERR)
               {
                  //squad node
                  const stw::opensyde_core::C_OscNodeSquad * const pc_NodeSquad =
                     C_PuiSdHandler::h_GetInstance()->GetOscNodeSquadConst(u32_NodeSquadIndex);
                  if (pc_NodeSquad != nullptr)
                  {
                     //name (base name)
                     c_Name = pc_NodeSquad->c_BaseName.c_str();
                  }
               }
               else
               {
                  //name
                  c_Name = pc_Node->c_Properties.c_Name.c_str();
               }
            }

            //comment
            c_Comment = this->mpc_Ui->pc_TextEditComment->toPlainText();

            switch (this->mpc_Ui->pc_ComboBoxProtocol->currentIndex())
            {
            case mu8_FL_INDEX_OS:
               e_FlashLoader = C_OscNodeProperties::eFL_OPEN_SYDE;
               e_DiagnosticServer = C_OscNodeProperties::eDS_OPEN_SYDE;
               break;
            case mu8_FL_INDEX_UDS:
               e_FlashLoader = C_OscNodeProperties::eFL_UDS;
               e_DiagnosticServer = C_OscNodeProperties::eDS_UDS;
               break;
            default:
               //Not supported
               e_FlashLoader = C_OscNodeProperties::eFL_NONE;
               e_DiagnosticServer = C_OscNodeProperties::eDS_NONE;
               break;
            }

            //com interface settings
            for (uint16_t u16_ComIfCnt = 0;
                 (u16_ComIfCnt < pc_Node->c_Properties.c_ComInterfaces.size()) &&
                 (u16_ComIfCnt < this->mpc_Ui->pc_TableWidgetComIfSettings->rowCount());
                 ++u16_ComIfCnt)
            {
               bool q_NewValue;
               const C_OscNodeComInterfaceSettings & rc_CurInterface =
                  pc_Node->c_Properties.c_ComInterfaces[u16_ComIfCnt];
               const bool q_IsUpdateAvailable = pc_DevDef->c_SubDevices[u32_SubDeviceIndex].IsUpdateAvailable(
                  rc_CurInterface.e_InterfaceType);
               const bool q_IsRoutingAvailable = pc_Node->IsRoutingAvailable(rc_CurInterface.e_InterfaceType);
               const bool q_IsDiagnosisAvailable = pc_Node->IsDiagnosisAvailable(rc_CurInterface.e_InterfaceType);
               //node id
               c_NodeIds.push_back(
                  static_cast<uint8_t>((this->mpc_Ui->pc_TableWidgetComIfSettings->
                                        item(u16_ComIfCnt, s32_COL_NODE_ID)->text().toInt())));
               //J1939 source address (CAN only; Ethernet rows carry the null address)
               if (rc_CurInterface.e_InterfaceType == C_OscSystemBus::eCAN)
               {
                  c_J1939Addresses.push_back(
                     static_cast<uint8_t>(this->mpc_Ui->pc_TableWidgetComIfSettings->item(
                                             u16_ComIfCnt,
                                             static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eJ1939_ADDRESS))->
                                          text().toInt(nullptr, 0))); //base 0: accepts "0x.." as well as decimal
               }
               else
               {
                  c_J1939Addresses.push_back(C_OscNodeComInterfaceSettings::hu8_J1939_NULL_ADDRESS);
               }

               //update
               if (q_IsUpdateAvailable == true)
               {
                  //set state only if the cell is enabled (connected + available)
                  if (this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u16_ComIfCnt,
                                                                            s32_COL_UPDATE)->isEnabled() == true)
                  {
                     q_NewValue =
                        dynamic_cast<C_OgeChxTristateBase *> (this->mpc_Ui->pc_TableWidgetComIfSettings
                                                          ->cellWidget(u16_ComIfCnt, s32_COL_UPDATE))->isChecked();
                  }
                  else
                  {
                     q_NewValue = false;
                  }
               }
               else
               {
                  q_NewValue = false;
               }
               c_UpdateFlags.push_back(q_NewValue);

               //routing
               if (q_IsRoutingAvailable == true)
               {
                  //set state only if the cell is enabled (connected + available)
                  if (this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u16_ComIfCnt,
                                                                            s32_COL_ROUTING)->isEnabled() == true)
                  {
                     q_NewValue =
                        dynamic_cast<C_OgeChxTristateBase *> (this->mpc_Ui->pc_TableWidgetComIfSettings
                                                          ->cellWidget(u16_ComIfCnt, s32_COL_ROUTING))->isChecked();
                  }
                  else
                  {
                     q_NewValue = false;
                  }
               }
               else
               {
                  q_NewValue = false;
               }
               c_RoutingFlags.push_back(q_NewValue);

               //diagnosis
               if (q_IsDiagnosisAvailable == true)
               {
                  //set state only if the cell is enabled (connected + available)
                  if (this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u16_ComIfCnt,
                                                                            s32_COL_DIAGNOSTIC)->isEnabled() == true)
                  {
                     q_NewValue =
                        dynamic_cast<C_OgeChxTristateBase *> (this->mpc_Ui->pc_TableWidgetComIfSettings
                                                          ->cellWidget(u16_ComIfCnt, s32_COL_DIAGNOSTIC))->isChecked();
                  }
                  else
                  {
                     q_NewValue = false;
                  }
               }
               else
               {
                  q_NewValue = false;
               }
               c_DiagnosisFlags.push_back(q_NewValue);
            }

            //save new node
            C_PuiSdHandler::h_GetInstance()->SetOscNodePropertiesDetailed(this->mu32_NodeIndex, c_Name, c_Comment,
                                                                          e_DiagnosticServer, e_FlashLoader,
                                                                          c_NodeIds, c_UpdateFlags, c_RoutingFlags,
                                                                          c_DiagnosisFlags);

            //J1939 addressing: stored on the interfaces and written into the J1939 Tx identifiers
            C_PuiSdHandler::h_GetInstance()->SetOscNodeJ1939SourceAddresses(this->mu32_NodeIndex, c_J1939Addresses);

            //UDS addressing (the rest of the UDS configuration is kept as it is)
            {
               C_OscNodeUdsConfig c_UdsConfig = pc_Node->c_UdsConfig;
               c_UdsConfig.u32_RequestId = static_cast<uint32_t>(this->mpc_Ui->pc_SpinBoxUdsRequestId->value());
               c_UdsConfig.u32_ResponseId = static_cast<uint32_t>(this->mpc_Ui->pc_SpinBoxUdsResponseId->value());
               c_UdsConfig.u32_FunctionalId = static_cast<uint32_t>(this->mpc_Ui->pc_SpinBoxUdsFunctionalId->value());
               c_UdsConfig.q_ExtendedId = this->mpc_Ui->pc_CheckBoxUdsExtendedId->isChecked();
               c_UdsConfig.q_PadFrames = this->mpc_Ui->pc_CheckBoxUdsPadFrames->isChecked();
               C_PuiSdHandler::h_GetInstance()->SetOscNodeUdsConfig(this->mu32_NodeIndex, c_UdsConfig);
            }

            //send signal SigNodePropChanged (trigger to adapt canopen config)
            Q_EMIT (this->SigNodePropChanged());
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reacts on changing protocol
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_SupportedProtocolChange(void)
{
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);

   //the UDS addressing block belongs to the UDS protocol only
   this->mpc_Ui->pc_WidgetUds->setVisible(this->mpc_Ui->pc_ComboBoxProtocol->currentIndex() == mu8_FL_INDEX_UDS);

   // Save the data
   this->m_RegisterChange();

   // Update the com interface routing and dashboard settings in the table
   if (pc_Node != nullptr)
   {
      const C_OscDeviceDefinition * const pc_DevDef = pc_Node->pc_DeviceDefinition;
      const C_OscNodeProperties c_NodeProp = pc_Node->c_Properties;

      tgl_assert(pc_DevDef != nullptr);
      if (pc_DevDef != nullptr)
      {
         for (uint16_t u16_ComIfCnt = 0U;
              u16_ComIfCnt <
              (static_cast<uint16_t>(pc_DevDef->u8_NumCanBusses) +
               static_cast<uint16_t>(pc_DevDef->u8_NumEthernetBusses));
              ++u16_ComIfCnt)
         {
            const C_OscNodeComInterfaceSettings & rc_CurInterface = pc_Node->c_Properties.c_ComInterfaces[u16_ComIfCnt];
            const int32_t s32_COL_ROUTING = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eROUTING);
            const int32_t s32_COL_UPDATE = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eUPDATE);
            const int32_t s32_COL_DIAGNOSTIC = static_cast<int32_t>(C_SdNdeComIfSettingsTableDelegate::eDIAGNOSTIC);
            const bool q_IsRoutingAvailable = pc_Node->IsRoutingAvailable(rc_CurInterface.e_InterfaceType);
            const bool q_IsDiagAvailable = pc_Node->IsDiagnosisAvailable(rc_CurInterface.e_InterfaceType);

            C_OgeChxTristateBase * const pc_TristateRouting =
               dynamic_cast<C_OgeChxTristateBase *>(this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u16_ComIfCnt,
                                                                                                      s32_COL_ROUTING));
            C_OgeChxTristateBase * const pc_TristateUpdate =
               dynamic_cast<C_OgeChxTristateBase *>(this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u16_ComIfCnt,
                                                                                                      s32_COL_UPDATE));
            C_OgeChxTristateBase * const pc_TristateDiag =
               dynamic_cast<C_OgeChxTristateBase *>(this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u16_ComIfCnt,
                                                                                                      s32_COL_DIAGNOSTIC));

            if (pc_TristateRouting != nullptr)
            {
               this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u16_ComIfCnt, s32_COL_ROUTING)->setEnabled(
                  q_IsRoutingAvailable);
               this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u16_ComIfCnt, s32_COL_DIAGNOSTIC)->setEnabled(
                  q_IsDiagAvailable);

               this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(u16_ComIfCnt, s32_COL_UPDATE)->setEnabled(true);
               pc_TristateUpdate->setChecked(true);
               (void)c_NodeProp;

               if (q_IsRoutingAvailable == true)
               {
                  //set node value
                  pc_TristateRouting->setChecked(rc_CurInterface.q_IsRoutingEnabled);
               }
               else
               {
                  // Setting is disabled, so it has to be unchecked too
                  pc_TristateRouting->setChecked(false);
               }

               if (q_IsDiagAvailable == true)
               {
                  //set node value
                  pc_TristateDiag->setChecked(rc_CurInterface.q_IsDiagnosisEnabled);
               }
               else
               {
                  // Setting is disabled, so it has to be unchecked too
                  pc_TristateDiag->setChecked(false);
               }
            }
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check node name

   - check input
   - show/hide invalid icon
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_CheckNodeName(void)
{
   //check
   const std::string c_Text = this->mpc_Ui->pc_LineEditNodeName->text().toStdString();
   const bool q_NameIsUnique = C_PuiSdHandler::h_GetInstance()->CheckNodeNameAvailable(c_Text, &this->mu32_NodeIndex,
                                                                                       nullptr);
   const bool q_NameIsValid = C_OscUtils::h_CheckValidCeName(c_Text);

   //set invalid text property
   C_OgeWiUtil::h_ApplyStylesheetProperty(this->mpc_Ui->pc_LineEditNodeName, "Valid", q_NameIsUnique && q_NameIsValid);

   if ((q_NameIsUnique == true) && (q_NameIsValid == true))
   {
      this->mpc_Ui->pc_LineEditNodeName->SetToolTipInformation("",
                                                               "",
                                                               C_NagToolTip::eDEFAULT);
   }
   else
   {
      const QString c_Heading = "Node Name";
      QString c_Content;
      if (q_NameIsUnique == false)
      {
         c_Content += "- is already in use\n";
      }
      if (q_NameIsValid == false)
      {
         c_Content += "- is empty or contains invalid characters.\n";
      }
      this->mpc_Ui->pc_LineEditNodeName->SetToolTipInformation(c_Heading, c_Content, C_NagToolTip::eERROR);
   }

   Q_EMIT this->SigChanged();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Trim node name

   Remove whitespaces at the beginning and end of the string
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_TrimNodeName(void) const
{
   this->mpc_Ui->pc_LineEditNodeName->setText(this->mpc_Ui->pc_LineEditNodeName->text().trimmed());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Register Change

   Function where ui elements register a change. Change will be sent via a signal
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_RegisterChange(void)
{
   SaveToData();
   //signal
   Q_EMIT this->SigChanged();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A UDS identifier or the padding flag was edited
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_UdsAddressingChanged(void)
{
   this->m_RegisterChange();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   The identifier format was switched between 11 and 29 bit

   The identifier spin boxes take the matching range; a value beyond 11 bit is clamped by the spin box.

   \param[in]  oq_Checked   true: 29 bit identifiers
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_UdsExtendedIdChanged(const bool oq_Checked)
{
   this->m_SetUdsIdRange(oq_Checked);
   this->m_RegisterChange();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set the range of the three UDS identifier spin boxes

   \param[in]  oq_ExtendedId   true: 0..0x1FFFFFFF; false: 0..0x7FF
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_SetUdsIdRange(const bool oq_ExtendedId) const
{
   const int32_t s32_Max = oq_ExtendedId ? 0x1FFFFFFF : 0x7FF;

   this->mpc_Ui->pc_SpinBoxUdsRequestId->SetMinimumCustom(0);
   this->mpc_Ui->pc_SpinBoxUdsRequestId->SetMaximumCustom(s32_Max);
   this->mpc_Ui->pc_SpinBoxUdsResponseId->SetMinimumCustom(0);
   this->mpc_Ui->pc_SpinBoxUdsResponseId->SetMaximumCustom(s32_Max);
   this->mpc_Ui->pc_SpinBoxUdsFunctionalId->SetMinimumCustom(0);
   this->mpc_Ui->pc_SpinBoxUdsFunctionalId->SetMaximumCustom(s32_Max);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Register error change

   Function where ui elements register an error change. Change will be sent via a signal
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_RegisterErrorChange()
{
   m_RegisterChange();
   //signal
   Q_EMIT this->SigErrorChange();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Register name change
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_RegisterNameChange(void)
{
   //This function can somehow be called twice, so ... let's avoid that!
   static bool hq_InProgress = false;

   if (hq_InProgress == false)
   {
      std::vector<std::string> c_ExistingNames;
      hq_InProgress = true;

      if (C_PuiSdHandler::h_GetInstance()->CheckNodeNameAvailable(
             this->mpc_Ui->pc_LineEditNodeName->text().toStdString(),
             &this->mu32_NodeIndex,
             &c_ExistingNames) == false)
      {
         const QString c_Description =
            static_cast<QString>("A node with the name \"%1\" already exists. Choose another name.").
            arg(this->mpc_Ui->pc_LineEditNodeName->text());
         QString c_Details;
         C_OgeWiCustomMessage c_Message(this, C_OgeWiCustomMessage::eERROR);
         c_Message.SetHeading("Node naming");
         c_Message.SetDescription(c_Description);
         c_Details.append("Used node names:\n");
         for (uint32_t u32_ItExistingName = 0UL; u32_ItExistingName < c_ExistingNames.size(); ++u32_ItExistingName)
         {
            const std::string & rc_Name = c_ExistingNames[u32_ItExistingName];
            c_Details.append(static_cast<QString>("\"%1\"\n").arg(rc_Name.c_str()));
         }
         c_Message.SetDetails(c_Details);
         c_Message.SetCustomMinHeight(220, 350);
         c_Message.Execute();
         //Restore previous name
         {
            const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);
            if (pc_Node != nullptr)
            {
               uint32_t u32_NodeSquadIndex;

               if (C_PuiSdHandler::h_GetInstance()->GetNodeSquadIndexWithNodeIndex(this->mu32_NodeIndex,
                                                                                   u32_NodeSquadIndex) == C_NO_ERR)
               {
                  //squad node
                  const stw::opensyde_core::C_OscNodeSquad * const pc_NodeSquad =
                     C_PuiSdHandler::h_GetInstance()->GetOscNodeSquadConst(u32_NodeSquadIndex);
                  if (pc_NodeSquad != nullptr)
                  {
                     //name (base name)
                     this->mpc_Ui->pc_LineEditNodeName->setText(pc_NodeSquad->c_BaseName.c_str());
                  }
               }
               else
               {
                  //name
                  this->mpc_Ui->pc_LineEditNodeName->setText(pc_Node->c_Properties.c_Name.c_str());
               }
            }
         }
      }
      else
      {
         const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);
         m_TrimNodeName();
         m_RegisterChange();

         if (pc_Node != nullptr)
         {
            Q_EMIT (this->SigNameChanged("NETWORK TOPOLOGY", pc_Node->c_Properties.c_Name.c_str(), false));
         }
      }
      hq_InProgress = false; //lint !e838 its static and could be used on strange second call
   }
}
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check com interface id

   - check input
   - valid/invalid text

   \param[in]  ou32_Row       Row
   \param[in]  ou32_Column    Column
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_CheckComInterface(const uint32_t, const uint32_t ou32_Column) const
{
   const int32_t s32_COL_NODE_ID = static_cast<int32_t> (C_SdNdeComIfSettingsTableDelegate::eNODEID);
   const int32_t s32_COL_IP = static_cast<int32_t> (C_SdNdeComIfSettingsTableDelegate::eIPADDRESS);

   //node id or ip change?
   if ((ou32_Column == s32_COL_NODE_ID) || (ou32_Column == s32_COL_IP))
   {
      int32_t s32_RowCounter; // Equals com interface number.

      bool q_IdValid;
      bool q_IpValid;

      for (s32_RowCounter = 0U; s32_RowCounter < this->mpc_Ui->pc_TableWidgetComIfSettings->model()->rowCount();
           ++s32_RowCounter)
      {
         //get node id from cell
         const uint8_t u8_NodeId =
            static_cast<uint8_t> (this->mpc_Ui->pc_TableWidgetComIfSettings->item(s32_RowCounter,
                                                                                  s32_COL_NODE_ID)->text().toInt());

         //get ip address from cell
         const QString c_Ip =
            dynamic_cast<QLabel *>(this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(s32_RowCounter,
                                                                                         s32_COL_IP))
            ->text();
         //convert string into vector for core logic
         const QStringList c_IpBytesTmp = c_Ip.split('.');
         std::vector<int32_t> c_IpBytes;
         for (int32_t s32_It = 0; s32_It < c_IpBytesTmp.size(); ++s32_It)
         {
            c_IpBytes.push_back(static_cast<int32_t>(c_IpBytesTmp[s32_It].toInt()));
         }

         //check node id and ip address of current row for conflicts
         this->m_GetInterfaceStatus(this->mu32_NodeIndex, s32_RowCounter, u8_NodeId, c_IpBytes, q_IdValid, q_IpValid);

         //handle coloring of cell, when properties are conflicting
         this->m_HandlePropertyConflict(s32_RowCounter, s32_COL_NODE_ID, s32_COL_IP, q_IdValid, q_IpValid);

         //handle error icon and tool-tips with already used properties
         this->m_HandleErrorFeedback(s32_RowCounter, q_IdValid, q_IpValid);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Wrapper for core logic. Checks for id and ip collision on given interface.

   \param[in]    ou32_NodeIndex        Node index
   \param[in]    os32_InterfaceIndex   Interface index
   \param[in]    ou8_NodeId            Node id to investigate
   \param[in]    orc_Ip                Ip address to investigate
   \param[out]   orq_IdValid           Storage for result of node id check
   \param[out]   orq_IpValid           Storage for result of ip address check

*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_GetInterfaceStatus(const uint32_t ou32_NodeIndex, const int32_t os32_InterfaceIndex,
                                                       const uint8_t ou8_NodeId, const std::vector<int32_t> & orc_Ip,
                                                       bool & orq_IdValid, bool & orq_IpValid) const
{
   //check if node id is valid
   orq_IdValid =
      C_PuiSdHandler::h_GetInstance()->GetOscSystemDefinitionConst().CheckInterfaceIsAvailable(
         ou32_NodeIndex,
         os32_InterfaceIndex,
         ou8_NodeId);

   //check if ip is valid — only Ethernet interfaces have meaningful IPs. CAN rows default
   //to 0.0.0.0 and would otherwise collide with each other in CheckIpAddressIsValid,
   //producing a spurious "Interface: IP Address invalid" tooltip on every CAN row.
   const C_OscNode * const pc_NodeForType = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(ou32_NodeIndex);
   if ((pc_NodeForType != nullptr) &&
       (static_cast<uint32_t>(os32_InterfaceIndex) < pc_NodeForType->c_Properties.c_ComInterfaces.size()) &&
       (pc_NodeForType->c_Properties.c_ComInterfaces[os32_InterfaceIndex].e_InterfaceType ==
        C_OscSystemBus::eETHERNET))
   {
      orq_IpValid =
         C_PuiSdHandler::h_GetInstance()->GetOscSystemDefinitionConst().CheckIpAddressIsValid(
            ou32_NodeIndex,
            os32_InterfaceIndex, orc_Ip);
   }
   else
   {
      orq_IpValid = true;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Function marks specified cell content red if a property is invalid (id/ip collision)

   \param[in]      os32_Row      Table row
   \param[in]      os32_ColumnId Table column for node id
   \param[in]      os32_ColumnIp Table column for ip address
   \param[in]      oq_IdValid    Status of node id. (false: conflict!)
   \param[in]      oq_IpValid    Status of ip address. (false: conflict!)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_HandlePropertyConflict(const int32_t os32_Row, const int32_t os32_ColumnId,
                                                           const int32_t os32_ColumnIp, const bool oq_IdValid,
                                                           const bool oq_IpValid) const
{
   if (oq_IdValid == false)
   {
      //invalid
      this->mpc_Ui->pc_TableWidgetComIfSettings->model()->setData(
         this->mpc_Ui->pc_TableWidgetComIfSettings->model()->index(os32_Row, os32_ColumnId),
         mc_STYLE_GUIDE_COLOR_24, static_cast<int32_t> (Qt::ForegroundRole));
   }
   else
   {
      //valid
      this->mpc_Ui->pc_TableWidgetComIfSettings->model()->setData(
         this->mpc_Ui->pc_TableWidgetComIfSettings->model()->index(os32_Row, os32_ColumnId),
         mc_STYLE_GUIDE_COLOR_6, static_cast<int32_t> (Qt::ForegroundRole));
   }

   if (oq_IpValid == false)
   {
      //invalid
      this->mpc_Ui->pc_TableWidgetComIfSettings->model()->setData(
         this->mpc_Ui->pc_TableWidgetComIfSettings->model()->index(os32_Row, os32_ColumnIp),
         mc_STYLE_GUIDE_COLOR_24, static_cast<int32_t> (Qt::ForegroundRole));
   }
   else
   {
      //valid
      this->mpc_Ui->pc_TableWidgetComIfSettings->model()->setData(
         this->mpc_Ui->pc_TableWidgetComIfSettings->model()->index(os32_Row, os32_ColumnIp),
         mc_STYLE_GUIDE_COLOR_6, static_cast<int32_t> (Qt::ForegroundRole));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handles different error cases, sets tool-tip and error icon and prepares content for tool-tip

   \param[in]   os32_InterfaceIndex    Interface index
   \param[in]   oq_IdValid             True: Conflicting ID
   \param[in]   oq_IpValid             True: Conflicting IP
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_HandleErrorFeedback(const int32_t os32_InterfaceIndex, const bool oq_IdValid,
                                                        const bool oq_IpValid) const
{
   bool q_ShowIcon = false;

   if ((oq_IdValid == true) && (oq_IpValid == true))
   {
      this->mpc_Ui->pc_TableWidgetComIfSettings->SetToolTipAt(os32_InterfaceIndex,
                                                              static_cast<uint32_t> (
                                                                 C_SdNdeComIfSettingsTableDelegate
                                                                 ::eINTERFACE), "", "",
                                                              C_NagToolTip::eDEFAULT);
   }
   else
   {
      const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);
      if (pc_Node != nullptr)
      {
         if (os32_InterfaceIndex < static_cast<int32_t>(pc_Node->c_Properties.c_ComInterfaces.size()))
         {
            const C_OscNodeComInterfaceSettings & rc_Interface =
               pc_Node->c_Properties.c_ComInterfaces[os32_InterfaceIndex];
            if (rc_Interface.GetBusConnected() == true)
            {
               const C_OscSystemBus * const pc_Bus = C_PuiSdHandler::h_GetInstance()->GetOscBus(
                  rc_Interface.u32_BusIndex);
               if (pc_Bus != nullptr)
               {
                  QString c_TooltipContent;

                  //only ids are conflicting
                  if ((oq_IdValid == false) && (oq_IpValid == true))
                  {
                     q_ShowIcon = true;
                     const std::vector<uint32_t> c_UsedIds = C_SdUtil::h_GetUsedNodeIdsForBusUniqueAndSortedAscending(
                        rc_Interface.u32_BusIndex, this->mu32_NodeIndex, static_cast<int32_t>(os32_InterfaceIndex));
                     const QString c_Heading = "Interface: Node ID invalid";
                     c_TooltipContent = C_SdUtil::h_InitUsedIdsString(c_UsedIds,
                                                                      pc_Bus->c_Name.c_str(),
                                                                      "bus");
                     this->mpc_Ui->pc_TableWidgetComIfSettings->SetToolTipAt(os32_InterfaceIndex,
                                                                             static_cast<uint32_t> (
                                                                                C_SdNdeComIfSettingsTableDelegate
                                                                                ::eINTERFACE), c_Heading,
                                                                             c_TooltipContent, C_NagToolTip::eERROR);
                  }
                  //only ips are conflicting
                  else if ((oq_IpValid == false) && (oq_IdValid == true))
                  {
                     q_ShowIcon = true;
                     const std::vector<std::vector<uint8_t> > c_Ips = C_SdUtil::h_GetAllUsedIpAddressesForBus(
                        rc_Interface.u32_BusIndex, this->mu32_NodeIndex,
                        static_cast<int32_t>(os32_InterfaceIndex));
                     const QString c_Heading = "Interface: IP Address invalid";
                     c_TooltipContent = C_SdUtil::h_InitUsedIpsString(c_Ips,
                                                                      pc_Bus->c_Name.c_str(),
                                                                      "bus");

                     this->mpc_Ui->pc_TableWidgetComIfSettings->SetToolTipAt(os32_InterfaceIndex,
                                                                             static_cast<uint32_t> (
                                                                                C_SdNdeComIfSettingsTableDelegate
                                                                                ::eINTERFACE), c_Heading,
                                                                             c_TooltipContent, C_NagToolTip::eERROR);
                  }
                  //both id and ip are conflicting
                  else if ((oq_IdValid == false) && (oq_IpValid == false))
                  {
                     q_ShowIcon = true;
                     const std::vector<uint32_t> c_UsedIds = C_SdUtil::h_GetUsedNodeIdsForBusUniqueAndSortedAscending(
                        rc_Interface.u32_BusIndex, this->mu32_NodeIndex, static_cast<int32_t>(os32_InterfaceIndex));
                     const std::vector<std::vector<uint8_t> > c_Ips = C_SdUtil::h_GetAllUsedIpAddressesForBus(
                        rc_Interface.u32_BusIndex, this->mu32_NodeIndex, static_cast<int32_t>(os32_InterfaceIndex));
                     const QString c_Heading = "Interface: Property invalid";
                     c_TooltipContent = C_SdUtil::h_InitUsedIdsString(c_UsedIds,
                                                                      pc_Bus->c_Name.c_str(),
                                                                      "bus");
                     c_TooltipContent += "\n";
                     c_TooltipContent += C_SdUtil::h_InitUsedIpsString(c_Ips,
                                                                       pc_Bus->c_Name.c_str(),
                                                                       "bus");

                     this->mpc_Ui->pc_TableWidgetComIfSettings->SetToolTipAt(os32_InterfaceIndex,
                                                                             static_cast<uint32_t> (
                                                                                C_SdNdeComIfSettingsTableDelegate
                                                                                ::eINTERFACE), c_Heading,
                                                                             c_TooltipContent, C_NagToolTip::eERROR);
                  }
                  else
                  {
                     //nothing to do here
                  }
               }
            }
         }
      }
   }

   //handle invalid icon
   dynamic_cast<QCheckBox *> (this->mpc_Ui->pc_TableWidgetComIfSettings
                              ->cellWidget(os32_InterfaceIndex,
                                           static_cast<int32_t>
                                           (C_SdNdeComIfSettingsTableDelegate::
                                            eINTERFACE)))
   ->setChecked(q_ShowIcon);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle cell click

   Decides which column in Communication Interfaces Settings Table was clicked and which action to perform after click.
   If a IP-Address is clicked, a pop up to edit the IP-Address will show up.
   If a Bus-Bitrate was clicked, Tool jumps to "Edit Bus Properties" Screen.

   \param[in]  ou32_Row       Table Row
   \param[in]  ou32_Column    Table Column
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_HandleCellClick(const uint32_t ou32_Row, const uint32_t ou32_Column)
{
   const int32_t s32_COL_IP_ADDRESS = static_cast<int32_t> (C_SdNdeComIfSettingsTableDelegate::eIPADDRESS);
   const int32_t s32_COL_BUS_BITRATE = static_cast<int32_t> (C_SdNdeComIfSettingsTableDelegate::eCONNECTION);
   const bool q_EnabledCellIp = this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(ou32_Row,
                                                                                      s32_COL_IP_ADDRESS)->isEnabled();
   const bool q_EnabledCellBus = this->mpc_Ui->pc_TableWidgetComIfSettings->cellWidget(ou32_Row,
                                                                                       s32_COL_BUS_BITRATE)->isEnabled();

   if ((ou32_Column == s32_COL_IP_ADDRESS) && (q_EnabledCellIp == true))
   {
      this->m_IpAddressClick(ou32_Row);
   }
   else if ((ou32_Column == s32_COL_BUS_BITRATE) && (q_EnabledCellBus == true))
   {
      this->m_BusBitrateClick(ou32_Row);
   }
   else
   {
      //nothing to do
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   m_IpAddressClick

   open Pop Up for clicked IP Cell

   \param[in]  ou32_Row    Row
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_IpAddressClick(const uint32_t ou32_Row)
{
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);

   if (pc_Node != nullptr)
   {
      //Set parent for better hierarchy handling via window manager
      const QPointer<C_OgePopUpDialog> c_New = new C_OgePopUpDialog(this->parentWidget(), this->parentWidget());
      new C_SdNdeIpAddressConfigurationWidget(*c_New, this->mu32_NodeIndex, ou32_Row);
      const QSize c_SIZE(600, 416);

      //Resize
      c_New->SetSize(c_SIZE);

      if (c_New->exec() == static_cast<int32_t>(QDialog::Accepted))
      {
         //refresh table
         this->m_LoadFromData();
      }
      //Hide overlay after dialog is not relevant anymore
      if (c_New.isNull() == false)
      {
         c_New->HideOverlay();
         c_New->deleteLater();
      }
   } //lint !e429  //no memory leak because of the parent of pc_New and pc_Dialog and the Qt memory management
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  collects info about the bus, which got clicked in table and sends signal to change screen. Timer is needed

 *          to delay the signal, that a cell was clicked and the screen should change to bus properties. Without this
 *          opensyde crashes, because of unexpected behaviour of the table. It seems that the table still wants to
 *          perform actions after signal is out and screen is changed (while parent widget is already deleted).

   \param[in]  ou32_Row    Row
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_BusBitrateClick(const uint32_t ou32_Row)
{
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);
   const C_OscNodeProperties c_Prop = pc_Node->c_Properties;

   const std::vector<C_OscNodeComInterfaceSettings> & rc_ComInterfaces = c_Prop.c_ComInterfaces;

   if (ou32_Row < rc_ComInterfaces.size())
   {
      //check to which buses the node is connected
      if (rc_ComInterfaces[ou32_Row].GetBusConnected() == true)
      {
         const uint32_t u32_BusIndex = rc_ComInterfaces[ou32_Row].u32_BusIndex;
         //get name of connected bus
         const C_OscSystemBus * const pc_Bus = C_PuiSdHandler::h_GetInstance()->GetOscBus(
            rc_ComInterfaces[ou32_Row].u32_BusIndex);
         tgl_assert(pc_Bus != nullptr);
         if (pc_Bus != nullptr)
         {
            const QString c_BusName = pc_Bus->c_Name.c_str();

            this->mu32_BusIndex = u32_BusIndex;
            this->mc_BusName = c_BusName;
            this->mc_Timer.start();
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Slot for Timer::timeout to walk around the screen change on table-click.

 *          See m_BusBitrateClicked for details.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_OpenBus(void)
{
   Q_EMIT (this->SigBusBitrateClicked(this->mu32_BusIndex, this->mc_BusName));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot for X-App Support index change and registering the change

   \param[in]  os32_Index  Combobox index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SdNdeNodePropertiesWidget::m_XappSupportChange(const int32_t os32_Index)
{
   const bool q_IsXappSupported = (os32_Index == mhs32_PR_INDEX_ENABLED);
   bool q_Continue = true;
   const int32_t s32_ResetIndex =
      (os32_Index == mhs32_PR_INDEX_DISABLED) ? mhs32_PR_INDEX_ENABLED : mhs32_PR_INDEX_DISABLED;
   QStringList c_ConcernedDataBlocks;
   QStringList c_ConcernedLogJobs;
   bool q_FileGenDatablockExists = false;
   bool q_LogJobExists = false;
   const C_OscNode * const pc_Node = C_PuiSdHandler::h_GetInstance()->GetOscNodeConst(this->mu32_NodeIndex);

   // Check if file generation Data Blocks exist
   if (pc_Node != nullptr)
   {
      for (uint32_t u32_ItApp = 0; u32_ItApp < pc_Node->c_Applications.size(); ++u32_ItApp)
      {
         const C_OscNodeApplication & rc_App = pc_Node->c_Applications[u32_ItApp];
         if (rc_App.e_Type == C_OscNodeApplication::ePROGRAMMABLE_APPLICATION)
         {
            q_FileGenDatablockExists = true;
            c_ConcernedDataBlocks.append(rc_App.c_Name.c_str());
         }
      }
   }

   // If disabling, check if active log jobs exist
   if ((os32_Index == mhs32_PR_INDEX_DISABLED) && (pc_Node != nullptr))
   {
      if (pc_Node->c_DataLoggerJobs.size() > 0)
      {
         q_LogJobExists = true;
         for (uint32_t u32_ItLogJobs = 0; u32_ItLogJobs < pc_Node->c_DataLoggerJobs.size(); ++u32_ItLogJobs)
         {
            c_ConcernedLogJobs.append(pc_Node->c_DataLoggerJobs[u32_ItLogJobs].c_Properties.c_Name.c_str());
         }
      }
   }

   // Ask user
   if ((q_FileGenDatablockExists == true) || (q_LogJobExists == true))
   {
      QString c_Description;
      QString c_Details;
      C_OgeWiCustomMessage c_Message(this, C_OgeWiCustomMessage::eQUESTION);
      const QString c_EnableDisable =
         (os32_Index == mhs32_PR_INDEX_ENABLED) ? "Enable" : "Disable";

      c_Description = "Do you really want to " + c_EnableDisable.toLower() +
                      " X.App Support?";

      if (q_FileGenDatablockExists == true)
      {
         c_Description += " All existing Data Blocks with enabled file "
                                                 "generation will be deleted.";
         c_Details += "The following Data Blocks will be deleted:\n" +
                      c_ConcernedDataBlocks.join("\n") + "\n\n";
      }

      if (q_LogJobExists == true)
      {
         c_Description += " All log jobs will be deleted.";
         c_Details += "The following log jobs will be deleted:\n" +
                      c_ConcernedLogJobs.join("\n");
      }

      c_Message.SetHeading(c_EnableDisable + " X.App Support");
      c_Message.SetDescription(c_Description);
      c_Message.SetDetails(c_Details);

      c_Message.SetOkButtonText(c_EnableDisable + " X.App Support");
      c_Message.SetNoButtonText("Cancel");

      if (c_Message.Execute() != C_OgeWiCustomMessage::eYES)
      {
         q_Continue = false;
      }
   }

   if (q_Continue == true)
   {
      // Save new flag and trigger Data Block / Log Job deletion
      tgl_assert(C_PuiSdHandler::h_GetInstance()->SetOscNodePropertyXappSupport(
                    this->mu32_NodeIndex, q_IsXappSupported) == C_NO_ERR);

      // Trigger adaption of data logger and Data Blocks
      Q_EMIT (this->SigNodeXappSupportChanged());
   }
   else
   {
      // Reset combobox (disconnect to not call this method again)
      disconnect(this->mpc_Ui->pc_ComboBoxXAppSupport,
                 static_cast<void (QComboBox::*)(int32_t)>(&QComboBox::currentIndexChanged), this,
                 &C_SdNdeNodePropertiesWidget::m_XappSupportChange);
      this->mpc_Ui->pc_ComboBoxXAppSupport->setCurrentIndex(s32_ResetIndex);
      connect(this->mpc_Ui->pc_ComboBoxXAppSupport,
              static_cast<void (QComboBox::*)(int32_t)>(&QComboBox::currentIndexChanged), this,
              &C_SdNdeNodePropertiesWidget::m_XappSupportChange);
   }
}
