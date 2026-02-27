//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class for use case implementation functionality (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_IMPUTIL_HPP
#define C_IMPUTIL_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_ImpCodeGenerationReportWidget.hpp"
#include "C_OscNodeApplication.hpp"
#include "stwtypes.hpp"
#include <QString>
#include <QWidget>
#include <QList>
#ifdef _WIN32
#include <windows.h>
#endif


/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_gui_logic {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_ImpUtil {
public:
  C_ImpUtil(void);

  static void h_ExportCodeAll(QWidget *const opc_Parent);
  static void
  h_ExportCode(const QList<uint32_t> &orc_NodeIndices,
               const QList<QList<uint32_t>> &orc_AppIndicesPerNode,
               QWidget *const opc_Parent);
  static void h_ExportCodeNodes(const QList<uint32_t> &orc_NodeIndices,
                                QWidget *const opc_Parent);
  static int32_t h_OpenIde(const QString &orc_IdeExeCall);
  static QString h_GetSydeCoderCePath(void);
  static QString h_GetSydeXgenPath(void);
  static QString h_GetAbsoluteGeneratedDir(
      const stw::opensyde_core::C_OscNodeApplication &orc_Application,
      const QString &orc_NodeName);
  static QString h_GetDefaultGeneratedDir(const QString &orc_ApplicationName,
                                          const QString &orc_NodeName);
  static bool h_CheckProjForCodeGeneration(QWidget *const opc_Parent);
  static QString
  h_AskUserToSaveRelativePath(QWidget *const opc_Parent,
                              const QString &orc_Path,
                              const QString &orc_AbsoluteReferenceDir);
  static QStringList
  h_AskUserToSaveRelativePath(QWidget *const opc_Parent,
                              const QStringList &orc_Paths,
                              const QString &orc_AbsoluteReferenceDir);
  static QString
  h_FormatSourceFileInfoForReport(const QString &orc_FilePath,
                                  const QString &orc_ReadContent);

private:
#ifdef _WIN32
  class C_HandleData {
  public:
    uint32_t u32_ProcessId;
    HWND pc_WindowHandle;
  };
#endif

  static bool mh_CheckDatapoolsAssignmentForExportCode(
      const QList<uint32_t> &orc_NodeIndices, QWidget *const opc_Parent);
  static void mh_CheckNodeDatapoolsAssignmentForExportCode(
      const uint32_t ou32_NodeIndex, bool &orq_Continue,
      QString &orc_DataPoolErrorMessage);
#ifdef _WIN32
  static void mh_GetExistingApplicationHandle(const std::wstring &orc_ExeName,
                                              QList<HWND> &orc_Windows);
#endif
  static int32_t mh_ExecuteCodeGenerator(const QString &orc_NodeName,
                                         const QString &orc_AppName,
                                         const QString &orc_ExportFolder,
                                         QStringList &orc_ExportedFiles,
                                         const QString &orc_CodeGenerator,
                                         const bool &orq_Erase);
  static int32_t mh_ExportCodeNode(
      const uint32_t ou32_NodeIndex,
      const QList<uint32_t> &orc_AppIndices,
      QList<
          stw::opensyde_gui::C_ImpCodeGenerationReportWidget::C_ReportData>
          &orc_ExportInfo,
      const bool &orq_Erase);
#ifdef _WIN32
  // lint -e{8080} //using type expected by the library for API compatibility
  static WINBOOL CALLBACK mh_EnumWindowsCallback(HWND opc_Handle,
                                                 const LPARAM ox_LoParam);
#endif
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_gui_logic
} // namespace stw

#endif
