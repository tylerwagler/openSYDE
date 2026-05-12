//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle save'n load for user settings (implementation)

   QSettings-based persistence for SYDEflash's user settings. SYDEflash has only project-
   independent state (no per-project section); a single beginGroup-per-section save and load
   is enough. Helpers come from the shared C_UsFilerHelpers.hpp.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QFileInfo>
#include <QDir>
#include <QSet>
#include <QSettings>

#include "stwerrors.hpp"
#include "C_Uti.hpp"
#include "C_UsFiler.hpp"
#include "C_UsFilerHelpers.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::opensyde_gui_logic;
using namespace stw::errors;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_UsFiler::C_UsFiler(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save all user settings to default ini file.

   \param[in]  orc_UserSettings    User settings to save
   \param[in]  orc_Path            File path

   \return
   C_NO_ERR  OK
   C_RANGE   Empty path
   C_NOACT   File open failed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_UsFiler::h_Save(const C_UsHandler & orc_UserSettings, const QString & orc_Path)
{
   int32_t s32_Retval;

   if (orc_Path.compare("") != 0)
   {
      s32_Retval = C_NO_ERR;
      {
         const QFileInfo c_File(orc_Path);
         const QDir c_Dir(c_File.path());
         if (c_Dir.exists() == false)
         {
            c_Dir.mkpath(".");
         }
      }
      try
      {
         QSettings c_Ini(orc_Path, QSettings::IniFormat);

         // Drop any top-level groups that aren't part of the new schema. SYDEflash's pre-Phase-5
         // INI files don't have project-as-section issues (no project save), but the scrub
         // catches any unexpected legacy keys and keeps the file tidy after format changes.
         {
            const QSet<QString> c_KnownSections = {
               "Screen", "Layout", "Settings", "Properties", "Advanced_Properties", "Update"
            };
            const QStringList c_TopLevelGroups = c_Ini.childGroups();
            for (const QString & rc_Group : c_TopLevelGroups)
            {
               if (c_KnownSections.contains(rc_Group) == false)
               {
                  c_Ini.remove(rc_Group);
               }
            }
         }

         mh_SaveProjectIndependentSection(c_Ini, orc_UserSettings);
         c_Ini.sync();
      }
      catch (...)
      {
         s32_Retval = C_NOACT;
      }
   }
   else
   {
      s32_Retval = C_RANGE;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load user settings from file.

   \param[in,out]  orc_UserSettings    User settings to load into
   \param[in]      orc_Path            File path

   \return
   C_NO_ERR  OK
   C_RANGE   Empty path
   C_NOACT   File open failed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_UsFiler::h_Load(C_UsHandler & orc_UserSettings, const QString & orc_Path)
{
   int32_t s32_Retval;

   if (orc_Path.compare("") != 0)
   {
      try
      {
         QSettings c_Ini(orc_Path, QSettings::IniFormat);
         s32_Retval = C_NO_ERR;

         orc_UserSettings.SetDefault();
         mh_LoadProjectIndependentSection(orc_UserSettings, c_Ini);
      }
      catch (...)
      {
         s32_Retval = C_NOACT;
      }
   }
   else
   {
      s32_Retval = C_RANGE;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save project-independent (only) part of user settings.

   \param[in,out]  orc_Ini             Open QSettings instance
   \param[in]      orc_UserSettings    User settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveProjectIndependentSection(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings)
{
   const QStringList c_HexFilePaths = orc_UserSettings.GetLastKnownUpdateHexFilePaths();
   const QStringList c_HexFilePathsAsRelativeOrAbsolute = orc_UserSettings.GetHexFilePathsAsRelativeOrAbsolute();

   orc_Ini.remove("Screen");
   orc_Ini.beginGroup("Screen");
   orc_Ini.setValue("Position", orc_UserSettings.GetScreenPos());
   orc_Ini.setValue("Size", orc_UserSettings.GetAppSize());
   orc_Ini.setValue("Maximized", orc_UserSettings.GetAppMaximized());
   orc_Ini.setValue("ScreenIndex", static_cast<int32_t>(orc_UserSettings.GetAppScreenIndex()));
   orc_Ini.endGroup();

   orc_Ini.remove("Layout");
   orc_Ini.beginGroup("Layout");
   orc_Ini.setValue("SettingsSplitter_x", orc_UserSettings.GetSplitterSettingsHorizontal());
   orc_Ini.setValue("Settings_expanded", orc_UserSettings.GetSettingsAreExpanded());
   orc_Ini.endGroup();

   orc_Ini.remove("Settings");
   orc_Ini.beginGroup("Settings");
   orc_Ini.setValue("ProgressExpanded", orc_UserSettings.GetWiProgressExpanded());
   orc_Ini.setValue("AdvSettExpanded", orc_UserSettings.GetWiAdvSettExpanded());
   orc_Ini.setValue("DllExpanded", orc_UserSettings.GetWiDllConfigExpanded());
   orc_Ini.setValue("PopOpenSection", static_cast<int32_t>(orc_UserSettings.GetPopOpenSection()));
   const stw::opensyde_core::C_OscCanAdapterConfig & rc_AdapterCfg = orc_UserSettings.GetAdapterConfig();
   orc_Ini.setValue("CanAdapterBackend",
                    QString::fromStdString(::can::backendKindToString(rc_AdapterCfg.e_BackendKind)));
   orc_Ini.setValue("CanAdapterChannelId", QString::fromStdString(rc_AdapterCfg.c_ChannelId));
   orc_Ini.setValue("CanAdapterBitrateBps", rc_AdapterCfg.u32_BitrateBps);
   orc_Ini.endGroup();

   orc_Ini.remove("Properties");
   orc_Ini.beginGroup("Properties");
   orc_Ini.setValue("NodeId", orc_UserSettings.GetPropNodeId());
   orc_Ini.setValue("Bitrate", orc_UserSettings.GetPropBitrate());
   orc_Ini.setValue("InterfaceIndex", orc_UserSettings.GetNodeCfgInterfaceIndex());
   orc_Ini.endGroup();

   orc_Ini.remove("Advanced_Properties");
   orc_Ini.beginGroup("Advanced_Properties");
   orc_Ini.setValue("FlashloaderResetWaitTime", orc_UserSettings.GetFlashloaderResetWaitTime());
   orc_Ini.setValue("RequestDownloadTimeout", orc_UserSettings.GetRequestDownloadTimeout());
   orc_Ini.setValue("TransferDataTimeout", orc_UserSettings.GetTransferDataTimeout());
   orc_Ini.endGroup();

   orc_Ini.remove("Update");
   orc_Ini.beginGroup("Update");
   const int32_t s32_HexCount = static_cast<int32_t>(c_HexFilePaths.size());
   orc_Ini.beginWriteArray("HexFiles", s32_HexCount);
   for (int32_t s32_It = 0; s32_It < s32_HexCount; ++s32_It)
   {
      orc_Ini.setArrayIndex(s32_It);
      orc_Ini.setValue("Path", c_HexFilePaths[s32_It]);
      // The legacy format stored these in parallel arrays of equal length. Preserve that
      // invariant by writing the sibling value when present; default to empty otherwise.
      const QString c_RelOrAbs = (s32_It < c_HexFilePathsAsRelativeOrAbsolute.size())
                                 ? c_HexFilePathsAsRelativeOrAbsolute[s32_It]
                                 : QString("");
      orc_Ini.setValue("RelativeOrAbsolute", c_RelOrAbs);
   }
   orc_Ini.endArray();
   orc_Ini.setValue("LastKnownHexFileLocation", orc_UserSettings.GetLastKnownUpdateHexFileLocation());
   orc_Ini.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load project-independent (only) part of user settings.

   \param[in,out]  orc_UserSettings    User settings to load into
   \param[in,out]  orc_Ini             Open QSettings instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadProjectIndependentSection(C_UsHandler & orc_UserSettings, QSettings & orc_Ini)
{
   orc_Ini.beginGroup("Screen");
   orc_UserSettings.SetScreenPos(orc_Ini.value("Position", QPoint(50, 50)).toPoint());
   orc_UserSettings.SetAppSize(orc_Ini.value("Size", QSize(1000, 700)).toSize());
   orc_UserSettings.SetAppMaximized(orc_Ini.value("Maximized", true).toBool());
   orc_UserSettings.SetAppScreenIndex(static_cast<uint32_t>(orc_Ini.value("ScreenIndex", 0).toInt()));
   orc_Ini.endGroup();

   orc_Ini.beginGroup("Layout");
   orc_UserSettings.SetSplitterSettingsHorizontal(orc_Ini.value("SettingsSplitter_x", 0).toInt());
   orc_UserSettings.SetSettingsAreExpanded(orc_Ini.value("Settings_expanded", true).toBool());
   orc_Ini.endGroup();

   orc_Ini.beginGroup("Settings");
   orc_UserSettings.SetWiProgressExpanded(orc_Ini.value("ProgressExpanded", true).toBool());
   orc_UserSettings.SetWiAdvSettExpanded(orc_Ini.value("AdvSettExpanded", true).toBool());
   orc_UserSettings.SetWiDllConfigExpanded(orc_Ini.value("DllExpanded", true).toBool());
   orc_UserSettings.SetPopOpenSection(static_cast<C_UsHandler::E_SettingsSubSection>(
                                         orc_Ini.value("PopOpenSection",
                                                       static_cast<int32_t>(C_UsHandler::E_SettingsSubSection::eNONE)).
                                         toInt()));
   {
      stw::opensyde_core::C_OscCanAdapterConfig c_AdapterCfg =
         stw::opensyde_core::C_OscCanAdapterConfig::h_GetPlatformDefault();
      const QString c_BackendName = orc_Ini.value("CanAdapterBackend", "").toString();
      if (c_BackendName == "SocketCAN")      { c_AdapterCfg.e_BackendKind = ::can::BackendKind::SocketCan; }
      else if (c_BackendName == "PCANBasic") { c_AdapterCfg.e_BackendKind = ::can::BackendKind::PcanBasic; }
      else if (c_BackendName == "Kvaser")    { c_AdapterCfg.e_BackendKind = ::can::BackendKind::Kvaser; }
      else if (c_BackendName == "VectorXL")  { c_AdapterCfg.e_BackendKind = ::can::BackendKind::VectorXL; }
      else { /* keep platform default */ }
      const QString c_ChannelId = orc_Ini.value("CanAdapterChannelId", "").toString();
      if (c_ChannelId.isEmpty() == false)
      {
         c_AdapterCfg.c_ChannelId = c_ChannelId.toStdString();
      }
      const uint32_t u32_Bitrate = orc_Ini.value("CanAdapterBitrateBps", 0U).toUInt();
      if (u32_Bitrate > 0U)
      {
         c_AdapterCfg.u32_BitrateBps = u32_Bitrate;
      }
      orc_UserSettings.SetAdapterConfig(c_AdapterCfg);
   }
   orc_Ini.endGroup();

   orc_Ini.beginGroup("Properties");
   orc_UserSettings.SetPropNodeId(orc_Ini.value("NodeId", 0).toInt());
   orc_UserSettings.SetPropBitrate(orc_Ini.value("Bitrate", 0).toInt());
   orc_UserSettings.SetNodeCfgInterfaceIndex(orc_Ini.value("InterfaceIndex", 1).toInt());
   orc_Ini.endGroup();

   orc_Ini.beginGroup("Advanced_Properties");
   orc_UserSettings.SetFlashloaderResetWaitTime(orc_Ini.value("FlashloaderResetWaitTime", 1000).toInt());
   orc_UserSettings.SetRequestDownloadTimeout(orc_Ini.value("RequestDownloadTimeout", 40000).toInt());
   orc_UserSettings.SetTransferDataTimeout(orc_Ini.value("TransferDataTimeout", 1000).toInt());
   orc_Ini.endGroup();

   QStringList c_HexFilePaths;
   QStringList c_HexFilePathsAsRelativeOrAbsolute;
   orc_Ini.beginGroup("Update");
   const int32_t s32_HexCount = orc_Ini.beginReadArray("HexFiles");
   for (int32_t s32_It = 0; s32_It < s32_HexCount; ++s32_It)
   {
      orc_Ini.setArrayIndex(s32_It);
      c_HexFilePaths.append(orc_Ini.value("Path", "").toString());
      c_HexFilePathsAsRelativeOrAbsolute.append(orc_Ini.value("RelativeOrAbsolute", "").toString());
   }
   orc_Ini.endArray();
   orc_UserSettings.SetLastKnownUpdateHexFilePaths(c_HexFilePaths);
   orc_UserSettings.SetHexFilePathsAsRelativeOrAbsolute(c_HexFilePathsAsRelativeOrAbsolute);
   orc_UserSettings.SetLastKnownUpdateHexFileLocation(orc_Ini.value("LastKnownHexFileLocation", "").toString());
   orc_Ini.endGroup();
}

