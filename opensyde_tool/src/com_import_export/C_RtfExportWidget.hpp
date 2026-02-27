//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Dialog for RTF file export (header)

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_RTFEXPORTWIDGET_HPP
#define C_RTFEXPORTWIDGET_HPP

#include "C_OgePopUpDialog.hpp"
#include "C_SdTopologyWidget.hpp"
#include "stwtypes.hpp"
#include <QMap>
#include <QString>
#include <QStringList>
#include <QWidget>


namespace Ui {
class C_RtfExportWidget;
}
/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_gui {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */
class C_RtfExportWidget : public QWidget {
  Q_OBJECT

public:
  explicit C_RtfExportWidget(
      stw::opensyde_gui_elements::C_OgePopUpDialog &orc_Parent);
  ~C_RtfExportWidget(void) noexcept override;

  int32_t GetRtfPath(QString &orc_RtfPath) const;
  int32_t GetCompanyName(QString &orc_CompanyName) const;
  int32_t GetCompanyLogoPath(QString &orc_CompanyLogoPath) const;

  void SetRtfPath(const QString &orc_RtfPath) const;
  void SetCompanyName(const QString &orc_CompanyName) const;
  void SetCompanyLogoPath(const QString &orc_CompanyLogoPath) const;

  int32_t ExportToRtf(const QString &orc_RtfPath,
                      const QString &orc_CompanyName,
                      const QString &orc_CompanyLogoPath,
                      C_SdTopologyWidget *const opc_Widget,
                      QStringList &orc_WarningMessages,
                      QString &orc_ErrorMessage);

  void InitStaticNames(void) const;

protected:
  void keyPressEvent(QKeyEvent *const opc_KeyEvent) override;

  class C_ExportXmlStructure {
  public:
    // Project
    QString c_Title;
    QString c_Name;
    QString c_Version;
    QString c_Created;
    QString c_Author;
    QString c_SysDefPath;
    QString c_DevicesIniPath;
    QString c_OutputPath;
    QString c_NetworkTopologyImage;
    // openSYDE
    QString c_OpenSydeVersion;
    // Company
    QString c_CompanyName;
    QString c_CompanyLogoPath;
  };

  int32_t m_CheckSettings(void) const;

private:
  Ui::C_RtfExportWidget *mpc_Ui;
  // lint -e{1725} Only problematic if copy or assignment is allowed
  stw::opensyde_gui_elements::C_OgePopUpDialog &mrc_ParentDialog;
  QMap<QString, QString> mc_NodeMapping;
  QStringList mc_Warnings;
  QString mc_Error;

  void m_OkClicked(void);
  void m_CancelClicked(void);
  void m_RtfPathClicked(void);
  void m_LogoPathClicked(void) const;

  int32_t
  m_CreateConfigXml(const QString &orc_Path,
                    const C_ExportXmlStructure &orc_ExportXmlStructure) const;

  // Avoid call
  C_RtfExportWidget(const C_RtfExportWidget &);
  C_RtfExportWidget &operator=(const C_RtfExportWidget &) &;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_gui
} // namespace stw

#endif
