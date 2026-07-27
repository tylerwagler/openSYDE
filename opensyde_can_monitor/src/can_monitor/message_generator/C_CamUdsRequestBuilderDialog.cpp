//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Dialog for building UDS (ISO 14229) diagnostic requests (implementation)

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QRegularExpression>

#include "C_CamUdsRequestBuilderDialog.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

#define SID_DSC  (0x10U)
#define SID_ER   (0x11U)
#define SID_RDBI (0x22U)
#define SID_RMBA (0x23U)
#define SID_WDBI (0x2EU)
#define SID_IOCBI (0x2FU)
#define SID_SA   (0x27U)
#define SID_CC   (0x28U)
#define SID_RC   (0x31U)
#define SID_RD   (0x34U)
#define SID_RU   (0x35U)
#define SID_TD   (0x36U)
#define SID_RTE  (0x37U)
#define SID_WMBA (0x3DU)
#define SID_TP   (0x3EU)
#define SID_ATP  (0x83U)
#define SID_CDTC (0x85U)
#define SID_LC   (0x87U)

enum FieldPage
{
   PAGE_SUBFUNC = 0,
   PAGE_SUBFUNC_DID,
   PAGE_DID,
   PAGE_DID_DATA,
   PAGE_SUBFUNC_DATA,
   PAGE_ROUTINE,
   PAGE_NO_PARAMS,
   PAGE_DATA_ONLY
};

struct UdsServiceEntry
{
   uint8_t    u8_Sid;
   const char * pc_Name;
   FieldPage  e_Page;
};

static const UdsServiceEntry mha_Services[] = {
   {SID_DSC,  "DiagnosticSessionControl",         PAGE_SUBFUNC},
   {SID_ER,   "ECUReset",                          PAGE_SUBFUNC},
   {SID_SA,   "SecurityAccess",                    PAGE_SUBFUNC_DATA},
   {SID_CC,   "CommunicationControl",              PAGE_SUBFUNC},
   {SID_TP,   "TesterPresent",                     PAGE_SUBFUNC},
   {SID_ATP,  "AccessTimingParameter",             PAGE_SUBFUNC},
   {SID_CDTC, "ControlDTCSettings",                PAGE_SUBFUNC},
   {SID_LC,   "LinkControl",                       PAGE_SUBFUNC},
   {SID_RDBI, "ReadDataByIdentifier",              PAGE_DID},
   {SID_WDBI, "WriteDataByIdentifier",             PAGE_DID_DATA},
   {SID_IOCBI,"InputOutputControlByIdentifier",    PAGE_DID_DATA},
   {SID_RC,   "RoutineControl",                    PAGE_ROUTINE},
   {SID_RD,   "RequestDownload",                   PAGE_DATA_ONLY},
   {SID_RU,   "RequestUpload",                     PAGE_DATA_ONLY},
   {SID_TD,   "TransferData",                      PAGE_DATA_ONLY},
   {SID_RTE,  "RequestTransferExit",               PAGE_NO_PARAMS},
   {SID_RMBA, "ReadMemoryByAddress",               PAGE_DATA_ONLY},
   {SID_WMBA, "WriteMemoryByAddress",              PAGE_DATA_ONLY},
};

static const int32_t mhs32_NUM_SERVICES = sizeof(mha_Services) / sizeof(mha_Services[0]);

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

static QByteArray mh_ParseHex(const QString & orc_Text)
{
   QByteArray c_Result;
   const QStringList c_Parts = orc_Text.trimmed().split(QRegularExpression("[\\s,;]+"), Qt::SkipEmptyParts);
   for (const QString & rc_Part : c_Parts)
   {
      bool q_Ok;
      uint8_t u8_Val = static_cast<uint8_t>(rc_Part.toUInt(&q_Ok, 16));
      if (q_Ok)
      {
         c_Result.append(static_cast<char>(u8_Val));
      }
   }
   return c_Result;
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

C_CamUdsRequestBuilderDialog::C_CamUdsRequestBuilderDialog(QWidget * const opc_Parent) :
   QDialog(opc_Parent)
{
   this->m_SetupUi();
}

C_CamUdsRequestBuilderDialog::~C_CamUdsRequestBuilderDialog(void)
{
}

void C_CamUdsRequestBuilderDialog::m_SetupUi(void)
{
   this->setWindowTitle("UDS Diagnostic Request Builder");
   this->setMinimumWidth(520);
   this->setModal(true);

   QVBoxLayout * const pc_Layout = new QVBoxLayout(this);

   // ---- Service ----
   QGroupBox * const pc_ServiceBox = new QGroupBox("1. Select Service");
   QHBoxLayout * const pc_ServiceLayout = new QHBoxLayout(pc_ServiceBox);
   this->mpc_ComboBoxService = new QComboBox();
   this->mpc_ComboBoxService->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
   for (int32_t i = 0; i < mhs32_NUM_SERVICES; i++)
   {
      this->mpc_ComboBoxService->addItem(mha_Services[i].pc_Name);
   }
   pc_ServiceLayout->addWidget(this->mpc_ComboBoxService);
   pc_Layout->addWidget(pc_ServiceBox);

   // ---- Parameters (stacked) ----
   this->mpc_GroupParams = new QGroupBox("2. Configure Parameters");
   QVBoxLayout * const pc_ParamOuter = new QVBoxLayout(this->mpc_GroupParams);
   this->mpc_StackFields = new QStackedWidget();
   pc_ParamOuter->addWidget(this->mpc_StackFields);

   // Page 0: Sub-function only
   this->mpc_PageSubFunc = new QWidget();
   QFormLayout * const pc_SfLayout = new QFormLayout(this->mpc_PageSubFunc);
   this->mpc_SpinSubFunc = new QSpinBox();
   this->mpc_SpinSubFunc->setRange(0, 255);
   this->mpc_SpinSubFunc->setDisplayIntegerBase(16);
   this->mpc_SpinSubFunc->setPrefix("0x");
   this->mpc_LabelSubFunc = new QLabel("Sub-function:");
   pc_SfLayout->addRow(this->mpc_LabelSubFunc, this->mpc_SpinSubFunc);
   this->mpc_StackFields->addWidget(this->mpc_PageSubFunc);

   // Page 1: Sub-function + DID
   this->mpc_PageSubFuncDid = new QWidget();
   QFormLayout * const pc_SfdLayout = new QFormLayout(this->mpc_PageSubFuncDid);
   this->mpc_SpinSubFuncDid = new QSpinBox();
   this->mpc_SpinSubFuncDid->setRange(0, 255);
   this->mpc_SpinSubFuncDid->setDisplayIntegerBase(16);
   this->mpc_SpinSubFuncDid->setPrefix("0x");
   this->mpc_LabelSubFuncDid = new QLabel("Sub-function:");
   pc_SfdLayout->addRow(this->mpc_LabelSubFuncDid, this->mpc_SpinSubFuncDid);
   this->mpc_SpinDid = new QSpinBox();
   this->mpc_SpinDid->setRange(0, 65535);
   this->mpc_SpinDid->setDisplayIntegerBase(16);
   this->mpc_SpinDid->setPrefix("0x");
   this->mpc_LabelDid = new QLabel("Data Identifier:");
   pc_SfdLayout->addRow(this->mpc_LabelDid, this->mpc_SpinDid);
   this->mpc_StackFields->addWidget(this->mpc_PageSubFuncDid);

   // Page 2: DID only
   this->mpc_PageDid = new QWidget();
   QFormLayout * const pc_DLayout = new QFormLayout(this->mpc_PageDid);
   this->mpc_SpinDidOnly = new QSpinBox();
   this->mpc_SpinDidOnly->setRange(0, 65535);
   this->mpc_SpinDidOnly->setDisplayIntegerBase(16);
   this->mpc_SpinDidOnly->setPrefix("0x");
   this->mpc_LabelDidOnly = new QLabel("Data Identifier:");
   pc_DLayout->addRow(this->mpc_LabelDidOnly, this->mpc_SpinDidOnly);
   this->mpc_StackFields->addWidget(this->mpc_PageDid);

   // Page 3: DID + data
   this->mpc_PageDidData = new QWidget();
   QFormLayout * const pc_DdLayout = new QFormLayout(this->mpc_PageDidData);
   this->mpc_SpinDidData = new QSpinBox();
   this->mpc_SpinDidData->setRange(0, 65535);
   this->mpc_SpinDidData->setDisplayIntegerBase(16);
   this->mpc_SpinDidData->setPrefix("0x");
   this->mpc_LabelDidData = new QLabel("Data Identifier:");
   pc_DdLayout->addRow(this->mpc_LabelDidData, this->mpc_SpinDidData);
   this->mpc_EditDidData = new QLineEdit();
   this->mpc_EditDidData->setPlaceholderText("e.g. 01 02 AB");
   this->mpc_LabelDidDataBytes = new QLabel("Data bytes (hex):");
   pc_DdLayout->addRow(this->mpc_LabelDidDataBytes, this->mpc_EditDidData);
   this->mpc_StackFields->addWidget(this->mpc_PageDidData);

   // Page 4: Sub-function + data
   this->mpc_PageSubFuncData = new QWidget();
   QFormLayout * const pc_Sfd2Layout = new QFormLayout(this->mpc_PageSubFuncData);
   this->mpc_SpinSubFuncData = new QSpinBox();
   this->mpc_SpinSubFuncData->setRange(0, 255);
   this->mpc_SpinSubFuncData->setDisplayIntegerBase(16);
   this->mpc_SpinSubFuncData->setPrefix("0x");
   this->mpc_LabelSubFuncData = new QLabel("Sub-function:");
   pc_Sfd2Layout->addRow(this->mpc_LabelSubFuncData, this->mpc_SpinSubFuncData);
   this->mpc_EditSubFuncData = new QLineEdit();
   this->mpc_EditSubFuncData->setPlaceholderText("e.g. 11 22 33");
   this->mpc_LabelSubFuncDataBytes = new QLabel("Data bytes (hex):");
   pc_Sfd2Layout->addRow(this->mpc_LabelSubFuncDataBytes, this->mpc_EditSubFuncData);
   this->mpc_StackFields->addWidget(this->mpc_PageSubFuncData);

   // Page 5: Routine control
   this->mpc_PageRoutine = new QWidget();
   QFormLayout * const pc_RtLayout = new QFormLayout(this->mpc_PageRoutine);
   this->mpc_SpinRoutineSubFunc = new QSpinBox();
   this->mpc_SpinRoutineSubFunc->setRange(0, 255);
   this->mpc_SpinRoutineSubFunc->setDisplayIntegerBase(16);
   this->mpc_SpinRoutineSubFunc->setPrefix("0x");
   this->mpc_LabelRoutineSubFunc = new QLabel("Sub-function (1=start, 2=stop, 3=reqRes):");
   pc_RtLayout->addRow(this->mpc_LabelRoutineSubFunc, this->mpc_SpinRoutineSubFunc);
   this->mpc_SpinRid = new QSpinBox();
   this->mpc_SpinRid->setRange(0, 65535);
   this->mpc_SpinRid->setDisplayIntegerBase(16);
   this->mpc_SpinRid->setPrefix("0x");
   this->mpc_LabelRid = new QLabel("Routine ID:");
   pc_RtLayout->addRow(this->mpc_LabelRid, this->mpc_SpinRid);
   this->mpc_EditRoutineData = new QLineEdit();
   this->mpc_EditRoutineData->setPlaceholderText("e.g. 01 02 (optional)");
   this->mpc_LabelRoutineData = new QLabel("Data bytes (hex):");
   pc_RtLayout->addRow(this->mpc_LabelRoutineData, this->mpc_EditRoutineData);
   this->mpc_StackFields->addWidget(this->mpc_PageRoutine);

   // Page 6: No parameters
   this->mpc_PageNoParams = new QWidget();
   QVBoxLayout * const pc_NpLayout = new QVBoxLayout(this->mpc_PageNoParams);
   QLabel * const pc_NpLabel = new QLabel("This service requires no additional parameters.");
   pc_NpLayout->addWidget(pc_NpLabel);
   this->mpc_StackFields->addWidget(this->mpc_PageNoParams);

   // Page 7: Data only
   this->mpc_PageDataOnly = new QWidget();
   QFormLayout * const pc_DoLayout = new QFormLayout(this->mpc_PageDataOnly);
   this->mpc_EditDataOnly = new QLineEdit();
   this->mpc_EditDataOnly->setPlaceholderText("e.g. 00 44 00 01 00 00 00 08");
   this->mpc_LabelDataOnly = new QLabel("Data bytes (hex):");
   pc_DoLayout->addRow(this->mpc_LabelDataOnly, this->mpc_EditDataOnly);
   this->mpc_StackFields->addWidget(this->mpc_PageDataOnly);

   pc_Layout->addWidget(this->mpc_GroupParams);

   // ---- CAN ID ----
   QGroupBox * const pc_IdBox = new QGroupBox("3. CAN Identifier");
   QHBoxLayout * const pc_IdLayout = new QHBoxLayout(pc_IdBox);
   this->mpc_EditCanId = new QLineEdit("0x7E0");
   this->mpc_EditCanId->setMaximumWidth(120);
   pc_IdLayout->addWidget(new QLabel("CAN ID (hex):"));
   pc_IdLayout->addWidget(this->mpc_EditCanId);
   pc_IdLayout->addStretch();
   pc_Layout->addWidget(pc_IdBox);

   // ---- Preview ----
   QGroupBox * const pc_PrevBox = new QGroupBox("4. Generated Request");
   QVBoxLayout * const pc_PrevLayout = new QVBoxLayout(pc_PrevBox);
   this->mpc_LabelPreview = new QLabel("Select a service and configure parameters, then click Generate.");
   this->mpc_LabelPreview->setWordWrap(true);
   this->mpc_LabelPreview->setTextInteractionFlags(Qt::TextSelectableByMouse);
   this->mpc_LabelPreview->setMinimumHeight(60);
   pc_PrevLayout->addWidget(this->mpc_LabelPreview);
   pc_Layout->addWidget(pc_PrevBox);

   // ---- Buttons ----
   QHBoxLayout * const pc_BtnLayout = new QHBoxLayout();
   this->mpc_BtnGenerate = new QPushButton("Generate");
   this->mpc_BtnSend = new QPushButton("Add to Tx Table");
   this->mpc_BtnSend->setEnabled(false);
   this->mpc_BtnCancel = new QPushButton("Cancel");
   pc_BtnLayout->addWidget(this->mpc_BtnGenerate);
   pc_BtnLayout->addWidget(this->mpc_BtnSend);
   pc_BtnLayout->addStretch();
   pc_BtnLayout->addWidget(this->mpc_BtnCancel);
   pc_Layout->addLayout(pc_BtnLayout);

   // ---- Connections ----
   connect(this->mpc_ComboBoxService, QOverload<int32_t>::of(&QComboBox::currentIndexChanged),
           this, &C_CamUdsRequestBuilderDialog::m_OnServiceChanged);
   connect(this->mpc_BtnGenerate, &QPushButton::clicked,
           this, &C_CamUdsRequestBuilderDialog::m_GenerateRequest);
   connect(this->mpc_BtnSend, &QPushButton::clicked,
           this, &QDialog::accept);
   connect(this->mpc_BtnCancel, &QPushButton::clicked,
           this, &QDialog::reject);

   this->m_OnServiceChanged(0);
}

void C_CamUdsRequestBuilderDialog::m_OnServiceChanged(const int32_t os32_Index)
{
   if ((os32_Index < 0) || (os32_Index >= mhs32_NUM_SERVICES))
   {
      return;
   }

   const UdsServiceEntry & rc_Svc = mha_Services[os32_Index];
   this->mpc_StackFields->setCurrentIndex(static_cast<int32_t>(rc_Svc.e_Page));

   switch (rc_Svc.u8_Sid)
   {
   case SID_DSC:
      this->mpc_LabelSubFunc->setText("Session (1=default, 2=prog, 3=extended, 0x60=safe):");
      break;
   case SID_ER:
      this->mpc_LabelSubFunc->setText("Reset type (1=hard, 2=keyOffOn, 3=soft):");
      break;
   case SID_SA:
      this->mpc_LabelSubFuncData->setText("Access level (odd=reqSeed, even=sendKey):");
      this->mpc_LabelSubFuncDataBytes->setText("Seed/Key data (hex):");
      break;
   case SID_CC:
      this->mpc_LabelSubFunc->setText("Control (0=enableRxTx, 1=enableRx, 2=enableTx, 3=disable):");
      break;
   case SID_TP:
      this->mpc_LabelSubFunc->setText("Sub-function (typically 0x00):");
      break;
   case SID_ATP:
      this->mpc_LabelSubFunc->setText("Timing (0=readExt, 1=setDefault, 2=readActive, 3=setValues):");
      break;
   case SID_CDTC:
      this->mpc_LabelSubFunc->setText("DTC setting (1=on, 2=off):");
      break;
   case SID_LC:
      this->mpc_LabelSubFunc->setText("Link (1=verifyBaudrate, 2=transitionBaudrate):");
      break;
   default:
      break;
   }
}

void C_CamUdsRequestBuilderDialog::m_GenerateRequest(void)
{
   const int32_t s32_Idx = this->mpc_ComboBoxService->currentIndex();
   if ((s32_Idx < 0) || (s32_Idx >= mhs32_NUM_SERVICES))
   {
      return;
   }

   const UdsServiceEntry & rc_Svc = mha_Services[s32_Idx];
   QByteArray c_Req;
   c_Req.append(static_cast<char>(rc_Svc.u8_Sid));

   switch (rc_Svc.e_Page)
   {
   case PAGE_SUBFUNC:
      c_Req.append(static_cast<char>(this->mpc_SpinSubFunc->value()));
      break;
   case PAGE_SUBFUNC_DID:
      c_Req.append(static_cast<char>(this->mpc_SpinSubFuncDid->value()));
      {
         const uint16_t u16_Did = static_cast<uint16_t>(this->mpc_SpinDid->value());
         c_Req.append(static_cast<char>((u16_Did >> 8) & 0xFF));
         c_Req.append(static_cast<char>(u16_Did & 0xFF));
      }
      break;
   case PAGE_DID:
      {
         const uint16_t u16_Did = static_cast<uint16_t>(this->mpc_SpinDidOnly->value());
         c_Req.append(static_cast<char>((u16_Did >> 8) & 0xFF));
         c_Req.append(static_cast<char>(u16_Did & 0xFF));
      }
      break;
   case PAGE_DID_DATA:
      {
         const uint16_t u16_Did = static_cast<uint16_t>(this->mpc_SpinDidData->value());
         c_Req.append(static_cast<char>((u16_Did >> 8) & 0xFF));
         c_Req.append(static_cast<char>(u16_Did & 0xFF));
      }
      c_Req.append(mh_ParseHex(this->mpc_EditDidData->text()));
      break;
   case PAGE_SUBFUNC_DATA:
      c_Req.append(static_cast<char>(this->mpc_SpinSubFuncData->value()));
      c_Req.append(mh_ParseHex(this->mpc_EditSubFuncData->text()));
      break;
   case PAGE_ROUTINE:
      c_Req.append(static_cast<char>(this->mpc_SpinRoutineSubFunc->value()));
      {
         const uint16_t u16_Rid = static_cast<uint16_t>(this->mpc_SpinRid->value());
         c_Req.append(static_cast<char>((u16_Rid >> 8) & 0xFF));
         c_Req.append(static_cast<char>(u16_Rid & 0xFF));
      }
      c_Req.append(mh_ParseHex(this->mpc_EditRoutineData->text()));
      break;
   case PAGE_NO_PARAMS:
      break;
   case PAGE_DATA_ONLY:
      c_Req.append(mh_ParseHex(this->mpc_EditDataOnly->text()));
      break;
   }

   this->mc_GeneratedBytes = c_Req;
   this->m_UpdatePreview();
   this->mpc_BtnSend->setEnabled(!c_Req.isEmpty());
}

void C_CamUdsRequestBuilderDialog::m_UpdatePreview(void)
{
   if (this->mc_GeneratedBytes.isEmpty())
   {
      this->mpc_LabelPreview->setText("Click Generate to build the request.");
      return;
   }

   QString c_Text;
   c_Text += "Raw bytes: ";
   for (int32_t i = 0; i < this->mc_GeneratedBytes.size(); i++)
   {
      if (i > 0) { c_Text += " "; }
      c_Text += QString("%1").arg(static_cast<uint8_t>(this->mc_GeneratedBytes[i]), 2, 16, QChar('0')).toUpper();
   }
   c_Text += "\n";
   c_Text += "DLC: " + QString::number(this->mc_GeneratedBytes.size()) + "\n";

   const uint8_t u8_Sid = static_cast<uint8_t>(this->mc_GeneratedBytes[0]);
   c_Text += "Service: " + mh_ServiceName(u8_Sid);
   c_Text += "\nCAN ID: " + this->mpc_EditCanId->text();
   c_Text += "  |  Protocol: UDS (auto-routed via CAN-TP)";

   this->mpc_LabelPreview->setText(c_Text);
}

QString C_CamUdsRequestBuilderDialog::mh_ServiceName(const uint8_t ou8_Sid)
{
   for (int32_t i = 0; i < mhs32_NUM_SERVICES; i++)
   {
      if (mha_Services[i].u8_Sid == ou8_Sid)
      {
         return mha_Services[i].pc_Name;
      }
   }
   return QString("Unknown (0x%1)").arg(ou8_Sid, 2, 16, QChar('0'));
}

QByteArray C_CamUdsRequestBuilderDialog::GetGeneratedBytes(void) const
{
   return this->mc_GeneratedBytes;
}

uint32_t C_CamUdsRequestBuilderDialog::GetCanId(void) const
{
   bool q_Ok;
   const uint32_t u32_Id = this->mpc_EditCanId->text().toUInt(&q_Ok, 16);
   return q_Ok ? u32_Id : 0x7E0U;
}

bool C_CamUdsRequestBuilderDialog::GetIsExtended(void) const
{
   return (this->GetCanId() > 0x7FFU);
}

QString C_CamUdsRequestBuilderDialog::GetMessageName(void) const
{
   const int32_t s32_Idx = this->mpc_ComboBoxService->currentIndex();
   if ((s32_Idx >= 0) && (s32_Idx < mhs32_NUM_SERVICES))
   {
      return QString("UDS_%1").arg(mha_Services[s32_Idx].pc_Name);
   }
   return "UDS_Request";
}
