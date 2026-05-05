//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle save'n load for user settings (implementation)

   QSettings-based persistence for CAN Monitor's user settings. Mirrors the design used by the
   main openSYDE GUI: top-level groups for app-wide state, and a Projects array indexed by Path
   for per-project state. Helpers come from the shared C_UsFilerHelpers.hpp.

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
   \param[in]  orc_ActiveProject   Active project (empty string => skip project-specific writes)

   \return
   C_NO_ERR  OK
   C_RANGE   Empty path
   C_NOACT   File open failed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_UsFiler::h_Save(const C_UsHandler & orc_UserSettings, const QString & orc_Path,
                          const QString & orc_ActiveProject)
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

         // Drop any top-level groups that aren't part of the new schema. This scrubs legacy
         // [/path/to/project.syde_cam] sections written by the pre-Phase-4 C_SclIniFile-based
         // saver (which QSettings reads as nested groups and would otherwise round-trip as a
         // mangled [home] section with backslash key paths).
         {
            const QSet<QString> c_KnownSections = {
               "RecentProjects", "Screen", "Buttons", "Trace_Settings",
               "Protocol", "Layout", "Settings", "Projects"
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

         mh_SaveRecentProjects(c_Ini, orc_UserSettings);
         mh_SaveProjectIndependentSection(c_Ini, orc_UserSettings);
         mh_SaveProjectDependentSection(c_Ini, orc_UserSettings, orc_ActiveProject);
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
   \param[in]      orc_ActiveProject   Active project (empty string => skip project-specific reads)

   \return
   C_NO_ERR  OK
   C_RANGE   Empty path
   C_NOACT   File open failed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_UsFiler::h_Load(C_UsHandler & orc_UserSettings, const QString & orc_Path, const QString & orc_ActiveProject)
{
   int32_t s32_Retval;

   if (orc_Path.compare("") != 0)
   {
      try
      {
         QSettings c_Ini(orc_Path, QSettings::IniFormat);
         s32_Retval = C_NO_ERR;

         orc_UserSettings.SetDefault();
         if (orc_ActiveProject == "")
         {
            // Load recent projects only if no active project is given (else it was already added
            // to RecentProjects and loading would overwrite the addition).
            mh_LoadRecentProjects(orc_UserSettings, c_Ini);
         }
         mh_LoadProjectIndependentSection(orc_UserSettings, c_Ini);
         mh_LoadProjectDependentSection(orc_UserSettings, c_Ini, orc_ActiveProject);
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
/*! \brief   Save recent projects.

   \param[in,out]  orc_Ini             Open QSettings instance
   \param[in]      orc_UserSettings    User settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveRecentProjects(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings)
{
   const QStringList c_List = orc_UserSettings.GetRecentProjects();

   orc_Ini.remove("RecentProjects");
   orc_Ini.beginGroup("RecentProjects");
   h_SaveArray(orc_Ini, "Items", c_List, [&orc_Ini] (auto c_It)
   {
      orc_Ini.setValue("Path", *c_It);
   });
   orc_Ini.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save project-independent (app-wide) part of user settings.

   \param[in,out]  orc_Ini             Open QSettings instance
   \param[in]      orc_UserSettings    User settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveProjectIndependentSection(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings)
{
   orc_Ini.remove("Screen");
   orc_Ini.beginGroup("Screen");
   orc_Ini.setValue("Position", orc_UserSettings.GetScreenPos());
   orc_Ini.setValue("Size", orc_UserSettings.GetAppSize());
   orc_Ini.setValue("Maximized", orc_UserSettings.GetAppMaximized());
   orc_Ini.setValue("ScreenIndex", static_cast<int32_t>(orc_UserSettings.GetAppScreenIndex()));
   orc_Ini.endGroup();

   orc_Ini.remove("Buttons");
   orc_Ini.beginGroup("Buttons");
   orc_Ini.setValue("Hex", orc_UserSettings.GetButtonHexActive());
   orc_Ini.setValue("TimeStamp", orc_UserSettings.GetButtonRelativeTimeStampActive());
   orc_Ini.setValue("Unique", orc_UserSettings.GetButtonUniqueViewActive());
   orc_Ini.endGroup();

   orc_Ini.remove("Trace_Settings");
   orc_Ini.beginGroup("Trace_Settings");
   orc_Ini.setValue("TimeStampAbsoluteTimeOfDay", orc_UserSettings.GetTraceSettingDisplayTimestampAbsoluteTimeOfDay());
   orc_Ini.setValue("TraceBufferSize", static_cast<int32_t>(orc_UserSettings.GetTraceSettingBufferSize()));
   orc_Ini.endGroup();

   orc_Ini.remove("Protocol");
   orc_Ini.beginGroup("Protocol");
   orc_Ini.setValue("Value", orc_UserSettings.GetSelectedProtocolIndex());
   orc_Ini.endGroup();

   orc_Ini.remove("Layout");
   orc_Ini.beginGroup("Layout");
   orc_Ini.setValue("MessageGenSplitter_y", orc_UserSettings.GetSplitterMessageGenVertical());
   orc_Ini.setValue("MessageGen_expanded", orc_UserSettings.GetMessageGenIsExpanded());
   orc_Ini.setValue("SettingsSplitter_x", orc_UserSettings.GetSplitterSettingsHorizontal());
   orc_Ini.setValue("Settings_expanded", orc_UserSettings.GetSettingsAreExpanded());
   orc_Ini.setValue("MessagesSignalsSplitter_x", orc_UserSettings.GetSplitterMesSigHorizontal());
   orc_Ini.endGroup();

   orc_Ini.remove("Settings");
   orc_Ini.beginGroup("Settings");
   orc_Ini.setValue("DatabaseExpanded", orc_UserSettings.GetWiDatabaseExpanded());
   orc_Ini.setValue("DllExpanded", orc_UserSettings.GetWiDllConfigExpanded());
   orc_Ini.setValue("FilterExpanded", orc_UserSettings.GetWiFilterExpanded());
   orc_Ini.setValue("LoggingExpanded", orc_UserSettings.GetWiLoggingExpanded());
   orc_Ini.setValue("PopOpenSection", static_cast<int32_t>(orc_UserSettings.GetPopOpenSection()));
   orc_Ini.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save project-dependent part of user settings.

   The active project's settings are persisted as one entry in the top-level "Projects" array,
   keyed by the project file path stored as "Path" inside the array slot. Other projects'
   entries (slots not matching the active path) are preserved unchanged. The project path is NOT
   used as a QSettings group name because QSettings interprets '/' as group nesting, which would
   mangle absolute paths.

   \param[in,out]  orc_Ini             Open QSettings instance
   \param[in]      orc_UserSettings    User settings
   \param[in]      orc_ActiveProject   Active project; empty string skips
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveProjectDependentSection(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings,
                                               const QString & orc_ActiveProject)
{
   if (orc_ActiveProject != "")
   {
      // Locate active project's existing slot in the Projects array, or allocate a new one.
      int32_t s32_ActiveIndex = -1;
      const int32_t s32_ExistingSize = orc_Ini.beginReadArray("Projects");
      for (int32_t s32_It = 0; s32_It < s32_ExistingSize; ++s32_It)
      {
         orc_Ini.setArrayIndex(s32_It);
         if (orc_Ini.value("Path").toString() == orc_ActiveProject)
         {
            s32_ActiveIndex = s32_It;
            break;
         }
      }
      orc_Ini.endArray();

      const int32_t s32_NewSize = (s32_ActiveIndex == -1) ? (s32_ExistingSize + 1) : s32_ExistingSize;
      if (s32_ActiveIndex == -1)
      {
         s32_ActiveIndex = s32_ExistingSize;
      }

      orc_Ini.beginWriteArray("Projects", s32_NewSize);
      orc_Ini.setArrayIndex(s32_ActiveIndex);

      // Clear stale fields from a prior save of this slot, then rewrite from scratch.
      orc_Ini.remove("");

      orc_Ini.setValue("Path", orc_ActiveProject);

      mh_SaveColumns(orc_Ini, "TraceColWidths", orc_UserSettings.GetTraceColWidths());
      mh_SaveColumns(orc_Ini, "TraceColPositions", orc_UserSettings.GetTraceColPositions());
      mh_SaveColumns(orc_Ini, "MessageGeneratorColWidths", orc_UserSettings.GetMessageColWidths());
      mh_SaveColumns(orc_Ini, "MessageSignalsColWidths", orc_UserSettings.GetSignalsColWidths());
      mh_SaveColumns(orc_Ini, "MessageSelection", orc_UserSettings.GetSelectedMessages());

      orc_Ini.setValue("LastKnownDatabasePath", orc_UserSettings.GetLastKnownDatabasePath());

      orc_Ini.endArray();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save a vector of int32_t values as a QSettings array.

   \param[in,out]  orc_Ini         Open QSettings instance
   \param[in]      orc_ArrayName   Array key
   \param[in]      orc_Columns     Values
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveColumns(QSettings & orc_Ini, const QString & orc_ArrayName,
                               const std::vector<int32_t> & orc_Columns)
{
   h_SaveArray(orc_Ini, orc_ArrayName, orc_Columns, [&orc_Ini] (auto c_It)
   {
      orc_Ini.setValue("Value", *c_It);
   });
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load recent projects.

   \param[in,out]  orc_UserSettings    User settings to load into
   \param[in,out]  orc_Ini             Open QSettings instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadRecentProjects(C_UsHandler & orc_UserSettings, QSettings & orc_Ini)
{
   QStringList c_List;

   orc_Ini.beginGroup("RecentProjects");
   const int32_t s32_Size = orc_Ini.beginReadArray("Items");
   for (int32_t s32_It = 0; s32_It < s32_Size; ++s32_It)
   {
      orc_Ini.setArrayIndex(s32_It);
      const QString c_Cur = orc_Ini.value("Path", "").toString();
      if (c_Cur.isEmpty() == false)
      {
         QFileInfo c_File;
         if (c_Cur.startsWith(".") == true)
         {
            c_File.setFile(C_Uti::h_GetExePath() + c_Cur);
         }
         else
         {
            c_File.setFile(c_Cur);
         }
         if (c_File.exists() == true)
         {
            c_List.append(c_File.absoluteFilePath());
         }
      }
   }
   orc_Ini.endArray();
   orc_Ini.endGroup();

   orc_UserSettings.SetRecentProjects(c_List);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load project-independent section.

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

   orc_Ini.beginGroup("Buttons");
   orc_UserSettings.SetButtonHexActive(orc_Ini.value("Hex", true).toBool());
   orc_UserSettings.SetButtonRelativeTimeStampActive(orc_Ini.value("TimeStamp", false).toBool());
   orc_UserSettings.SetButtonUniqueViewActive(orc_Ini.value("Unique", true).toBool());
   orc_Ini.endGroup();

   orc_Ini.beginGroup("Trace_Settings");
   orc_UserSettings.SetTraceSettingDisplayTimestampAbsoluteTimeOfDay(
      orc_Ini.value("TimeStampAbsoluteTimeOfDay", false).toBool());
   orc_UserSettings.SetTraceSettingBufferSize(static_cast<uint32_t>(orc_Ini.value("TraceBufferSize", 1000).toInt()));
   orc_Ini.endGroup();

   orc_Ini.beginGroup("Protocol");
   orc_UserSettings.SetSelectedProtocolIndex(orc_Ini.value("Value", 0).toInt());
   orc_Ini.endGroup();

   orc_Ini.beginGroup("Layout");
   orc_UserSettings.SetSplitterMessageGenVertical(orc_Ini.value("MessageGenSplitter_y", 348).toInt());
   orc_UserSettings.SetMessageGenIsExpanded(orc_Ini.value("MessageGen_expanded", true).toBool());
   orc_UserSettings.SetSplitterSettingsHorizontal(orc_Ini.value("SettingsSplitter_x", 0).toInt());
   orc_UserSettings.SetSettingsAreExpanded(orc_Ini.value("Settings_expanded", true).toBool());
   orc_UserSettings.SetSplitterMesSigHorizontal(orc_Ini.value("MessagesSignalsSplitter_x", 1005).toInt());
   orc_Ini.endGroup();

   orc_Ini.beginGroup("Settings");
   orc_UserSettings.SetWiDatabaseExpanded(orc_Ini.value("DatabaseExpanded", true).toBool());
   orc_UserSettings.SetWiDllConfigExpanded(orc_Ini.value("DllExpanded", true).toBool());
   orc_UserSettings.SetWiFilterExpanded(orc_Ini.value("FilterExpanded", true).toBool());
   orc_UserSettings.SetWiLoggingExpanded(orc_Ini.value("LoggingExpanded", true).toBool());
   orc_UserSettings.SetPopOpenSection(static_cast<C_UsHandler::E_SettingsSubSection>(
                                         orc_Ini.value("PopOpenSection",
                                                       static_cast<int32_t>(C_UsHandler::E_SettingsSubSection::eNONE)).
                                         toInt()));
   orc_Ini.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load project-dependent section.

   Scans the Projects array for an entry whose Path matches orc_ActiveProject. If found, loads
   that slot's per-project values into the handler. If no project is active or no matching slot
   exists, applies the new-project default for LastKnownDatabasePath.

   \param[in,out]  orc_UserSettings    User settings to load into
   \param[in,out]  orc_Ini             Open QSettings instance
   \param[in]      orc_ActiveProject   Active project; empty string applies defaults
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadProjectDependentSection(C_UsHandler & orc_UserSettings, QSettings & orc_Ini,
                                               const QString & orc_ActiveProject)
{
   if (orc_ActiveProject == "")
   {
      orc_UserSettings.SetLastKnownDatabasePath("");
   }
   else
   {
      int32_t s32_ActiveIndex = -1;
      const int32_t s32_Size = orc_Ini.beginReadArray("Projects");
      for (int32_t s32_It = 0; s32_It < s32_Size; ++s32_It)
      {
         orc_Ini.setArrayIndex(s32_It);
         if (orc_Ini.value("Path").toString() == orc_ActiveProject)
         {
            s32_ActiveIndex = s32_It;
            break;
         }
      }

      if (s32_ActiveIndex == -1)
      {
         orc_Ini.endArray();
         orc_UserSettings.SetLastKnownDatabasePath("");
      }
      else
      {
         orc_Ini.setArrayIndex(s32_ActiveIndex);

         std::vector<int32_t> c_Columns;
         mh_LoadColumns(orc_Ini, "TraceColWidths", c_Columns);
         orc_UserSettings.SetTraceColWidths(c_Columns);
         mh_LoadColumns(orc_Ini, "TraceColPositions", c_Columns);
         orc_UserSettings.SetTraceColPositions(c_Columns);
         mh_LoadColumns(orc_Ini, "MessageGeneratorColWidths", c_Columns);
         orc_UserSettings.SetMessageColWidths(c_Columns);
         mh_LoadColumns(orc_Ini, "MessageSignalsColWidths", c_Columns);
         orc_UserSettings.SetSignalsColWidths(c_Columns);
         mh_LoadColumns(orc_Ini, "MessageSelection", c_Columns);
         orc_UserSettings.SetSelectedMessages(c_Columns);

         orc_UserSettings.SetLastKnownDatabasePath(orc_Ini.value("LastKnownDatabasePath", "").toString());

         orc_Ini.endArray();
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load a vector of int32_t values from a QSettings array.

   \param[in,out]  orc_Ini         Open QSettings instance
   \param[in]      orc_ArrayName   Array key
   \param[in,out]  orc_Columns     Values (cleared and refilled)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadColumns(QSettings & orc_Ini, const QString & orc_ArrayName,
                               std::vector<int32_t> & orc_Columns)
{
   const int32_t s32_Size = orc_Ini.beginReadArray(orc_ArrayName);

   orc_Columns.clear();
   orc_Columns.reserve(s32_Size);
   for (int32_t s32_It = 0; s32_It < s32_Size; ++s32_It)
   {
      orc_Ini.setArrayIndex(s32_It);
      orc_Columns.push_back(orc_Ini.value("Value", 50).toInt());
   }
   orc_Ini.endArray();
}
