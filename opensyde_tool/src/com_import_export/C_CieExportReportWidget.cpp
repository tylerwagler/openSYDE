//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Dialog for export report to DBC file (implementation)

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_SdUtil.hpp"
#include <QFileInfo>

#include "C_CieExportDbc.hpp"
#include "C_CieExportReportWidget.hpp"
#include "C_OscLoggingHandler.hpp"
#include "constants.hpp"
#include "stwerrors.hpp"
#include "ui_C_CieExportReportWidget.h"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::errors;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_core;
using namespace stw::scl;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */
const QString C_CieExportReportWidget::mhc_HTML_TABLE_HEADER_START =
    "<td align=\"left\" valign=\"top\" style=\"padding: 5px 18px 5px "
    "0px;white-space:pre;font-weight:bold;\">";
const QString C_CieExportReportWidget::mhc_HTML_TABLE_DATA_START =
    "<td align=\"left\" valign=\"top\" style=\"padding: 5px 18px 5px "
    "0px;white-space:pre;\">";

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */

/* -- Module Global Variables
 * ---------------------------------------------------------------------------------------
 */

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in,out] orc_Parent          Reference to parent
   \param[in]     orc_FilePath        Loaded file path
*/
//----------------------------------------------------------------------------------------------------------------------
C_CieExportReportWidget::C_CieExportReportWidget(
    stw::opensyde_gui_elements::C_OgePopUpDialog &orc_Parent,
    const QString &orc_FilePath)
    : QWidget(&orc_Parent), mpc_Ui(new Ui::C_CieExportReportWidget),
      mrc_ParentDialog(orc_Parent), mc_FilePath(orc_FilePath) {
  const QFileInfo c_FileInfo(orc_FilePath);

  mpc_Ui->setupUi(this);

  InitStaticNames();

  // register the widget for showing
  this->mrc_ParentDialog.SetWidget(this);

  // set main title
  this->mrc_ParentDialog.SetTitle(
      static_cast<QString>("%1 File Export")
          .arg(c_FileInfo.completeSuffix().toUpper()));
  this->mrc_ParentDialog.SetSubTitle("Report");

  // connects
  connect(this->mpc_Ui->pc_BushButtonOk, &QPushButton::clicked, this,
          &C_CieExportReportWidget::m_OkClicked);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_CieExportReportWidget::~C_CieExportReportWidget(void) { delete mpc_Ui; }

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize all displayed static names
 */
//----------------------------------------------------------------------------------------------------------------------
void C_CieExportReportWidget::InitStaticNames(void) const {
  this->mpc_Ui->pc_LabelHeadingReport->setText("Details");
  this->mpc_Ui->pc_BushButtonOk->setText("OK");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set message data for report.

   \param[in] orc_NodeMapping              key is openSYDE name, value is DBC
   symbol name
   \param[in] orc_ExportStatistic          number of messages and signals of
   network
   \param[in] orc_Warnings                 warnings of DBC file export
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CieExportReportWidget::SetMessageData(
    const QHash<QString, QString> &orc_NodeMapping,
    const C_CieExportDbc::C_ExportStatistic &orc_ExportStatistic,
    const QStringList &orc_Warnings) {
  // Copy to internal data
  this->mc_NodeMapping = orc_NodeMapping;
  this->mc_ExportStatistic = orc_ExportStatistic;
  this->mc_Warnings = orc_Warnings;

  // build up report in message dialog field
  this->m_BuildReport();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Overwritten key press event slot

   Here: Handle specific enter key cases

   \param[in,out] opc_KeyEvent Event identification and information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_CieExportReportWidget::keyPressEvent(QKeyEvent *const opc_KeyEvent) {
  bool q_CallOrg = true;

  // Handle all enter key cases manually
  if ((opc_KeyEvent->key() == static_cast<int32_t>(Qt::Key_Enter)) ||
      (opc_KeyEvent->key() == static_cast<int32_t>(Qt::Key_Return))) {
    if (((opc_KeyEvent->modifiers().testFlag(Qt::ControlModifier) == true) &&
         (opc_KeyEvent->modifiers().testFlag(Qt::AltModifier) == false)) &&
        (opc_KeyEvent->modifiers().testFlag(Qt::ShiftModifier) == false)) {
      this->mrc_ParentDialog.accept();
    } else {
      q_CallOrg = false;
    }
  }
  if (q_CallOrg == true) {
    QWidget::keyPressEvent(opc_KeyEvent);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot of OK button click
 */
//----------------------------------------------------------------------------------------------------------------------
void C_CieExportReportWidget::m_OkClicked(void) {
  this->mrc_ParentDialog.accept();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Build up report for DBC export to file.
 */
//----------------------------------------------------------------------------------------------------------------------
void C_CieExportReportWidget::m_BuildReport(void) {
  // get nodes
  QString c_Nodes = "Node(s): ";
  bool q_FirstEntry = true;

  // build up node mapping
  QHash<QString, QString>::const_iterator c_Iter;
  for (c_Iter = this->mc_NodeMapping.begin();
       c_Iter != this->mc_NodeMapping.end(); ++c_Iter) {
    if (q_FirstEntry == true) {
      q_FirstEntry = false;
    } else {
      c_Nodes += ", ";
    }
    c_Nodes += c_Iter.key() + " (DBC Name: " + c_Iter.value() + ")";
  }

  // build up warning messages
  QString c_Warnings;
  const uint32_t u32_NumOfWarnings = this->mc_Warnings.count();
  if (u32_NumOfWarnings > 0) {
    c_Warnings += "<br/>";
    c_Warnings += "Warnings: ";
    for (int32_t u32_Pos = 0; u32_Pos < this->mc_Warnings.count(); u32_Pos++) {
      c_Warnings += "<br/>";
      c_Warnings += this->mc_Warnings.at(u32_Pos);
    }
  }

  QString c_CompleteLog;
  // Start
  c_CompleteLog += "<html>";
  c_CompleteLog += "<body>";
  // File
  c_CompleteLog += "<h3>";
  c_CompleteLog += "DBC File Information";
  c_CompleteLog += "</h3>";
  c_CompleteLog += "<table><tr>";
  c_CompleteLog += C_CieExportReportWidget::mhc_HTML_TABLE_DATA_START;
  c_CompleteLog += "Path:";
  c_CompleteLog += "</td>";
  c_CompleteLog += C_CieExportReportWidget::mhc_HTML_TABLE_DATA_START;
  c_CompleteLog +=
      static_cast<QString>(
          "<a href=\"file:%1\"><span style=\"color: %2;\">%3</span></a>")
          .arg(this->mc_FilePath)
          .arg(mc_STYLESHEET_GUIDE_COLOR_LINK)
          .arg(this->mc_FilePath);
  c_CompleteLog += "</td></tr><tr>";
  // Content
  c_CompleteLog += C_CieExportReportWidget::mhc_HTML_TABLE_DATA_START;
  c_CompleteLog += "Content written:";
  c_CompleteLog += "</td>";
  c_CompleteLog += C_CieExportReportWidget::mhc_HTML_TABLE_DATA_START;
  // Add node mapping
  c_CompleteLog += c_Nodes;
  c_CompleteLog += "<br/>";
  // Translation: 1=Number of Tx messages, 2=Number of Rx messages
  c_CompleteLog += static_cast<QString>("Messages: %1;")
                       .arg(this->mc_ExportStatistic.u32_NumOfMessages);
  // Translation: 1=Number of signals
  c_CompleteLog += static_cast<QString>(" Signals: %1")
                       .arg(this->mc_ExportStatistic.u32_NumOfSignals);
  // add warnings
  c_CompleteLog += c_Warnings;
  c_CompleteLog += "<br/><br/>";
  c_CompleteLog += "For detailed information see ";
  // Update log file
  C_OscLoggingHandler::h_Flush();
  c_CompleteLog +=
      static_cast<QString>(
          "<a href=\"file:%1\"><span style=\"color: %2;\">%3</span></a>.")
          .arg(C_OscLoggingHandler::h_GetCompleteLogFileLocation())
          .arg(mc_STYLESHEET_GUIDE_COLOR_LINK)
          .arg("log file");
  c_CompleteLog += "</td></tr></table>";
  // End
  c_CompleteLog += "</body>";
  c_CompleteLog += "</html>";
  this->mpc_Ui->pc_TextBrowserReport->setHtml(c_CompleteLog);
}
