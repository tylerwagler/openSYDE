//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief        Class for utilities with dependencies to Pui-classes.

   This class should not contain dependencies to any widget stuff.

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QDir>

#include "constants.hpp"

#include "C_Uti.hpp"
#include "TglFile.hpp"
#include "C_SclIniFile.hpp"
#include "C_PuiUtil.hpp"
#include "C_PuiProject.hpp"
#include "C_OscUtils.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiUtil::C_PuiUtil(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Always get absolute path from path relative to openSYDE project.

   \param[in]  orc_Path    Absolute or relative path

   \return
   Absolute path
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_PuiUtil::h_GetAbsolutePathFromProject(const QString & orc_Path)
{
   QString c_Folder = C_PuiProject::h_GetInstance()->GetFolderPath(); // always absolute or empty

   if (c_Folder == "")
   {
      // default: relative to executable path
      c_Folder = C_Uti::h_GetExePath();
   }

   return C_Uti::h_ConcatPathIfNecessary(c_Folder, orc_Path);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get rid of Data Block project path.

   If the path contains C_OscUtils::hc_PATH_VARIABLE_DATABLOCK_PROJ we replace this with the project path,
   but do not resolve any other placeholder variable.
   If the path is relative it is meant as relative to the Data Block project path,
   so we concatenate these paths.

   This might result in invalid paths if the placeholder variable is not in front of string
   but an absolute path (which would be a misconfiguration).

   \param[in]  orc_DbProjectPath    path for resolving data block project variable and concatenation
   \param[in]  orc_Path             path that probably contains variables

   \return
   Path without Data Block project path dependencies (might still contain placeholder variables or be relative)
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_PuiUtil::h_MakeIndependentOfDbProjectPath(const QString & orc_DbProjectPath, const QString & orc_Path)

{
   QString c_Result = C_OscUtils::h_MakeIndependentOfDbProjectPath(
      orc_DbProjectPath.toStdString().c_str(),
      C_PuiProject::h_GetInstance()->GetFolderPath().toStdString().c_str(),
      orc_Path.toStdString().c_str()).c_str();

   // do some path beautifying
   if (c_Result.contains("%") == false)
   {
      // clean path only if there are no path variables
      // (else %{BLUB}/../folder would be cleaned to folder which is wrong)
      c_Result = QDir::cleanPath(c_Result);
   }

   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if path contains placeholder variables (e.g. %{OPENSYDE_PROJECT}) and resolve them.

   \param[in]  orc_Path             path that probably contains variables
   \param[in]  orc_DbProjectPath    path for resolving data block project variable (special case),
                                    which might contain placeholder variables itself

   \return
   Resolved path
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_PuiUtil::h_ResolvePlaceholderVariables(const QString & orc_Path, const QString & orc_DbProjectPath)

{
   QString c_Result = C_OscUtils::h_ResolvePlaceholderVariables(
      orc_Path.toStdString().c_str(),
      C_PuiProject::h_GetInstance()->GetFolderPath().toStdString().c_str(),
      orc_DbProjectPath.toStdString().c_str()).c_str();

   // do some path beautifying
   if (c_Result.contains("%") == false)
   {
      // clean path only if there are no path variables
      // (else %{BLUB}/../folder would be cleaned to folder which is wrong)
      c_Result = QDir::cleanPath(c_Result);
   }

   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Resolve variables and interpret relative paths as relative to project

   Resolve all path variables (e.g. %{OPENSYDE_PROJ}). If the resulting path is relative
   it is meant as relative to openSYDE project, so we make it absolute by concatenating
   with project path.

   \param[in]  orc_Path    path that probably contains path variables and if relative is relative to project

   \return
   Absolute and resolved path
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_PuiUtil::h_GetResolvedAbsPathFromProject(const QString & orc_Path)
{
   QString c_Return = orc_Path;

   // first resolve, then make absolute (resolving might change from relative to absolute!)
   c_Return = C_PuiUtil::h_ResolvePlaceholderVariables(c_Return);
   c_Return = C_PuiUtil::h_GetAbsolutePathFromProject(c_Return);

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Resolve variables and interpret relative paths as relative to executable

   Resolve all path variables (e.g. %{OPENSYDE_PROJ}). If the resulting path is relative
   it is meant as relative to openSYDE.exe, so we make it absolute by concatenating
   with executable path.

   \param[in]  orc_Path             path that probably contains path variables and if relative is relative to executable
   \param[in]  orc_DbProjectPath    path for resolving data block project variable (special case),
                                    which might contain placeholder variables itself

   \return
   Absolute and resolved path
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_PuiUtil::h_GetResolvedAbsPathFromExe(const QString & orc_Path, const QString & orc_DbProjectPath)
{
   QString c_Return = orc_Path;

   // first resolve, then make absolute (resolving might change from relative to absolute!)
   c_Return = C_PuiUtil::h_ResolvePlaceholderVariables(c_Return, orc_DbProjectPath);
   c_Return = C_Uti::h_GetAbsolutePathFromExe(c_Return);

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Resolve variables and interpret relative paths as relative to Data Block project

   Resolve all path variables (e.g. %{OPENSYDE_PROJ}). If the resulting path is relative
   it is meant as relative to the Data Block project path (where the project for application
   programming is located at), so we make it absolute by concatenating with this path.
   The Data Block project path might itself be relative, and this is meant as relative to
   openSYDE project, so we make it absolute by concatenating if necessary.

   For example: C:/.../openSYDE_Project/DataBlockProject/GeneratedCode

   \param[in]  orc_DbProjectPath    path of Data Block project (for replacing hc_PATH_VARIABLE_DATABLOCK_PROJ
                                    and concatenation!)
   \param[in]  orc_Path             path that probably contains path variables and if relative is relative to
                                    Data Block project

   \return
   Absolute and resolved path
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_PuiUtil::h_GetResolvedAbsPathFromDbProject(const QString & orc_DbProjectPath, const QString & orc_Path)
{
   QString c_Return;

   // first resolve Data Block path
   c_Return = C_PuiUtil::h_MakeIndependentOfDbProjectPath(orc_DbProjectPath, orc_Path);

   // if relative it is relative to openSYDE project
   c_Return = C_PuiUtil::h_GetResolvedAbsPathFromProject(c_Return);

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Change relative paths in user devices ini to devices folder
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiUtil::h_ChangeRelativePathsInUserDevicesIniToDevicesFolder()
{
   const stw::scl::C_SclString c_IniFilePath =
      C_Uti::h_GetAbsolutePathFromExe("../devices/user_devices.ini").toStdString().c_str();

   if (stw::tgl::TglFileExists(c_IniFilePath))
   {
      stw::scl::C_SclIniFile c_IniFile(c_IniFilePath);
      const int32_t s32_NumDevices = c_IniFile.ReadInteger("User Nodes", "DeviceCount", 0);
      if (s32_NumDevices > 0)
      {
         const QDir c_DirIni(C_Uti::h_GetDevicesIniPath());

         const stw::scl::C_SclString c_IniPath = stw::tgl::TglExtractFilePath(c_IniFilePath);
         const QDir c_DirExe(C_Uti::h_GetExePath());
         // Read in all devices in the list in order
         for (int32_t s32_ItDevice = 0; s32_ItDevice < s32_NumDevices; ++s32_ItDevice)
         {
            const stw::scl::C_SclString c_DevicePath =
               c_IniFile.ReadString("User Nodes", "Device" + stw::scl::C_SclString::IntToStr(s32_ItDevice + 1), "");
            const QDir c_DirNewDevice(c_DevicePath.c_str());
            if (c_DirNewDevice.isRelative())
            {
               // Check relative to devices case
               const stw::scl::C_SclString c_CompletePathFromIni = c_IniPath + c_DevicePath;
               if (stw::tgl::TglFileExists(c_CompletePathFromIni) == false)
               {
                  const stw::scl::C_SclString c_CompletePathFromExe =
                     c_DirExe.absoluteFilePath(c_DevicePath.c_str()).toStdString().c_str();
                  // Check relative to exe case
                  if (stw::tgl::TglFileExists(c_CompletePathFromExe) == true)
                  {
                     const stw::scl::C_SclString c_NewDevicePath =
                        c_DirIni.relativeFilePath(c_CompletePathFromExe.c_str()).toStdString().c_str();
                     c_IniFile.WriteString("User Nodes", "Device" + stw::scl::C_SclString::IntToStr(
                                              s32_ItDevice + 1), c_NewDevicePath);
                  }
               }
            }
         }
      }
   }
}
