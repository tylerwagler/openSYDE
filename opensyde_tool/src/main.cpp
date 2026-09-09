//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       main function of project

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include "precomp_headers.hpp"

#include <QString>
#include <QApplication>
#include <QIcon>
#include <QImageReader>
#include "C_Uti.hpp"
#include "C_UtiStyleSheets.hpp"
#include "C_NagMainWindow.hpp"
#include "C_PuiProject.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_PopErrorHandling.hpp"
#include <cstdint>
#include "stwerrors.hpp"
#include "C_SdUtil.hpp"
#include "version_config.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   default main

   Set up and start application.

   \param[in]   os32_Argc    Number of command line arguments
   \param[in]   opacn_Argv   Command line arguments

   \return
   0: success
   else: error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t main(int32_t os32_Argc, char * opacn_Argv[])
{
   const uint16_t u16_Timer = osc_write_log_performance_start();
   int32_t s32_Result;

   // turn on the DPI support**
   QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
   QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

   // increase allocation limit for image reading: default is 256MB but we have users with images 11700x7800 which then
   // leads to allocation size of 11700x7800x4 (RGBA) = 365MB
   QImageReader::setAllocationLimit(512);

   QApplication c_Appl(os32_Argc, opacn_Argv);
   c_Appl.setApplicationVersion(QString("%1.%2.%3")
                                .arg(PROJECT_VERSION_MAJOR)
                                .arg(PROJECT_VERSION_MINOR)
                                .arg(PROJECT_VERSION_RELEASE));
   QApplication::setWindowIcon(QIcon(":/images/LogoOpensyde_XXL.png"));
   QGuiApplication::setDesktopFileName("openSYDE");

   //If the binary is called with a current directory differing from the exe directory this can cause
   // unexpected issues with relative path resolution (config files, fonts, log location).
   // Must be invoked after instancing QApplication.
   stw::opensyde_gui_logic::C_Uti::h_SetCurrentDirectoryToExeDirectory();
   {
      const QString c_FilePath = stw::opensyde_gui_logic::C_Uti::h_GetCompleteLogFileLocation(".syde_log");
      const QString c_ExeHash = stw::opensyde_gui_logic::C_Uti::h_GetHashValueAsQtString();

      //Set up logging (FIRST)
      stw::opensyde_core::C_OscLoggingHandler::h_SetWriteToConsoleActive(false);
      stw::opensyde_core::C_OscLoggingHandler::h_SetWriteToFileActive(true, false, false, true);
      stw::opensyde_core::C_OscLoggingHandler::h_SetCompleteLogFileLocation(c_FilePath.toStdString().c_str());

      osc_write_log_info("Startup", static_cast<QString>("Starting openSYDE Version: " +
                                                         stw::opensyde_gui_logic::C_Uti::h_GetApplicationVersion() +
                                                         ", MD5-Checksum: " +
                                                         c_ExeHash).toStdString().c_str());
   }
   {
      //Set stylesheet (SECOND)
      c_Appl.setStyleSheet(stw::opensyde_gui_logic::C_UtiStyleSheets::h_GetStylesheet());
   }
   {
      //Set language (THIRD) — gettext removed, English-only
   }
   {
      //Application (FOURTH)
      stw::opensyde_gui::C_NagMainWindow c_Window(u16_Timer);

      c_Window.show();

      //lint -e{1705} Working Qt example code
      s32_Result = c_Appl.exec();
   }
   return s32_Result;
}
