//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle save'n load for user settings (implementation)

   Handle save'n load for user settings

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
#include "constants.hpp"
#include "C_Uti.hpp"
#include "C_UsFiler.hpp"
#include "C_UsFilerHelpers.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::opensyde_gui_logic;
using namespace stw::opensyde_gui;
using namespace stw::scl;
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
/*! \brief   Save all user setting to default ini file

   \param[in]  orc_UserSettings     User settings to save
   \param[in]  orc_Path             File path
   \param[in]  orc_ActiveProject    Actual project to save project specific settings
                                    Empty string results in saving no informations

   \return
   C_NO_ERR: OK
   C_RANGE:  Parameter invalid
   C_NOACT:  File open failed
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
         //Helper to seperate path and file name
         const QFileInfo c_File(orc_Path);
         //Check if directory exists
         const QDir c_Dir(c_File.path());
         if (c_Dir.exists() == false)
         {
            c_Dir.mkpath(".");
         }
      }
      try
      {
         QSettings c_Ini(orc_Path, QSettings::IniFormat);

         // Drop any top-level groups that aren't part of the new schema. This scrubs
         // legacy [/path/to/project.syde] sections written by C_SclIniFile (which
         // QSettings reads as nested groups and would otherwise rewrite as a mangled
         // [home] section with backslash key paths). Anything not in our schema is
         // assumed to be pre-Phase-1 cruft and removed.
         {
            const QSet<QString> c_KnownSections = {
               "Common", "Environment", "RecentColors", "RecentProjects",
               "Screen", "SdTopologyToolbox", "SdNodeEdit", "SdBusEdit",
               "Projects", "Update"
            };
            const QStringList c_TopLevelGroups = c_Ini.childGroups();
            for (const QString & rc_Group : c_TopLevelGroups)
            {
               if (!c_KnownSections.contains(rc_Group))
               {
                  c_Ini.remove(rc_Group);
               }
            }
         }

         mh_SaveCommon(c_Ini, orc_UserSettings);
         mh_SaveEnvironment(c_Ini, orc_UserSettings);
         mh_SaveColors(c_Ini, orc_UserSettings);
         mh_SaveNextRecentColorButtonNumber(c_Ini, orc_UserSettings);
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
/*! \brief   Load all values of ini file

   If ini not existing set default values.

   \param[in,out]  orc_UserSettings    User settings to load
   \param[in]      orc_Path            File path
   \param[in]      orc_ActiveProject   Actual project to load project specific settings.
                                       Empty string results in default values

   \return
   C_NO_ERR: OK
   C_RANGE:  Parameter invalid
   C_NOACT:  File open failed
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_UsFiler::h_Load(C_UsHandler & orc_UserSettings, const QString & orc_Path, const QString & orc_ActiveProject)
{
   int32_t s32_Retval;

   if (orc_Path.compare("") != 0)
   {
      try
      {
         C_SclIniFile c_Ini(orc_Path.toStdString().c_str());
         s32_Retval = C_NO_ERR;

         orc_UserSettings.SetDefault();

         mh_LoadCommon(orc_UserSettings, c_Ini);
         mh_LoadEnvironment(orc_UserSettings, c_Ini);
         mh_LoadColors(orc_UserSettings, c_Ini);
         mh_LoadNextRecentColorButtonNumber(orc_UserSettings, c_Ini);
         mh_LoadScreenshotGifSucessTimeout(orc_UserSettings, c_Ini);
         if (orc_ActiveProject == "")
         {
            // load recent projects only if no active project is given
            // (else it was already added to RecentProjects and hence a call to LoadRecentProjects would overwrite it)
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
/*! \brief   Save node part of user settings

   Writes node fields into the active QSettings group.

   \param[in,out]  orc_Ini      Open QSettings positioned at the per-node group
   \param[in]      orc_Node     Node data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveNode(QSettings & orc_Ini, const C_UsNode & orc_Node)
{
   orc_Ini.setValue("SelectedDatapoolName", orc_Node.GetSelectedDatapoolName());
   orc_Ini.setValue("SelectedProtocol", static_cast<int32_t>(orc_Node.GetSelectedProtocol()));
   orc_Ini.setValue("SelectedInterface", static_cast<int32_t>(orc_Node.GetSelectedInterface()));

   mh_SaveColumns(orc_Ini, "CANopenOverviewColumns", orc_Node.GetCanOpenOverviewColumnWidth());
   mh_SaveColumns(orc_Ini, "CANopenPdoOverviewColumns", orc_Node.GetCanOpenPdoOverviewColumnWidth());

   h_SaveArray(orc_Ini, "CANopenManagerExpanded", orc_Node.GetExpandedCanOpenManager(),
               [&orc_Ini] (auto c_It)
   {
      orc_Ini.setValue("InterfaceNumber", static_cast<int32_t>(c_It->first));
      orc_Ini.setValue("Expanded", c_It->second);
   });

   h_SaveArray(orc_Ini, "CANopenDevicesExpanded", orc_Node.GetExpandedCanOpenDevices(),
               [&orc_Ini] (auto c_It)
   {
      orc_Ini.setValue("InterfaceNumber", static_cast<int32_t>(c_It->first));
      orc_Ini.setValue("Expanded", c_It->second);
   });

   h_SaveArray(orc_Ini, "CANopenDeviceExpanded", orc_Node.GetExpandedCanOpenDevice(),
               [&orc_Ini] (auto c_It)
   {
      orc_Ini.setValue("InterfaceNumber", static_cast<int32_t>(c_It->first.first));
      orc_Ini.setValue("DeviceInterfaceNumber", static_cast<int32_t>(c_It->first.second.first));
      orc_Ini.setValue("DeviceNodeName",
                       QString::fromStdString(c_It->first.second.second.AsStdString()->c_str()));
      orc_Ini.setValue("Expanded", c_It->second);
   });

   orc_Ini.setValue("SelectedCanOpenManager", static_cast<int32_t>(orc_Node.GetSelectedCanOpenManager()));
   orc_Ini.setValue("SelectedCanOpenDeviceInterfaceNumber",
                    static_cast<int32_t>(orc_Node.GetSelectedCanOpenDeviceInterfaceNumber()));
   orc_Ini.setValue("SelectedCanOpenDeviceNodeName", orc_Node.GetSelectedCanOpenDeviceNodeName());
   orc_Ini.setValue("SelectedCanOpenDeviceUseCase",
                    static_cast<int32_t>(orc_Node.GetSelectedCanOpenDeviceUseCaseIndex()));
   orc_Ini.setValue("CanOpenSelectedUseCaseOrInterface", orc_Node.GetCanOpenSelectedUseCaseOrInterface());

   orc_Ini.setValue("SelectedHalcDomain", orc_Node.GetSelectedHalcDomainName());
   orc_Ini.setValue("SelectedHalcChannel", orc_Node.GetSelectedHalcChannel());

   mh_SaveColumns(orc_Ini, "HalcOverviewColumns", orc_Node.GetHalcOverviewColumnWidth());
   mh_SaveColumns(orc_Ini, "HalcParamConfigColumns", orc_Node.GetHalcConfigColumnWidth());

   const QList<QString> c_DatapoolKeys = orc_Node.GetDatapoolKeysInternal();
   h_SaveArray(orc_Ini, "Datapools", c_DatapoolKeys,
               [&orc_Ini, &orc_Node] (auto c_It)
   {
      const QString & rc_Name = *c_It;
      orc_Ini.setValue("Name", rc_Name);
      mh_SaveDatapool(orc_Ini, orc_Node.GetDatapool(rc_Name));
   });

   orc_Ini.setValue("SelectedDataLoggerLogJobIndex",
                    static_cast<int32_t>(orc_Node.GetSelectedDataLoggerLogJobIndex()));
   orc_Ini.setValue("DataLoggerLogJobOverviewSelected", orc_Node.GetIsOverviewWidgetSelected());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save bus part of user settings

   Writes bus fields into the active QSettings group. Used both for top-level
   project buses and for per-datapool interface entries.

   \param[in,out]  orc_Ini   Open QSettings positioned at the per-bus group
   \param[in]      orc_Bus   Bus data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveBus(QSettings & orc_Ini, const C_UsCommunication & orc_Bus)
{
   stw::opensyde_core::C_OscCanProtocol::E_Type e_SelectedProtocol;
   bool q_MessageSelected;
   QString c_MessageName;
   bool q_SignalSelected;
   QString c_SignalName;

   orc_Bus.GetLastSelectedMessage(e_SelectedProtocol, q_MessageSelected, c_MessageName, q_SignalSelected,
                                  c_SignalName);

   mh_SaveColumns(orc_Ini, "MessageOverviewColumns", orc_Bus.GetMessageOverviewColumnWidth());
   mh_SaveColumns(orc_Ini, "SignalOverviewColumns", orc_Bus.GetSignalOverviewColumnWidth());
   orc_Ini.setValue("SelectedComProtocol", static_cast<int32_t>(e_SelectedProtocol));
   orc_Ini.setValue("MessageSelected", q_MessageSelected);
   orc_Ini.setValue("SelectedMessageName", c_MessageName);
   orc_Ini.setValue("SignalSelected", q_SignalSelected);
   orc_Ini.setValue("SelectedSignalName", c_SignalName);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save datapool part of user settings

   \param[in,out]  orc_Ini        Open QSettings positioned at the per-datapool group
   \param[in]      orc_Datapool   Datapool data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveDatapool(QSettings & orc_Ini, const C_UsNodeDatapool & orc_Datapool)
{
   const std::vector<QString> & rc_ExpandedListNames = orc_Datapool.GetExpandedListNames();
   const std::vector<QString> & rc_SelectedListNames = orc_Datapool.GetSelectedListNames();
   const std::vector<QString> & rc_SelectedVariableNames = orc_Datapool.GetSelectedVariableNames();
   const QList<QString> c_Interfaces = orc_Datapool.GetInterfaceSettingsKeysInternal();
   const QList<QString> & rc_Lists = orc_Datapool.GetListSettingsKeysInternal();

   h_SaveArray(orc_Ini, "ExpandedListNames", rc_ExpandedListNames, [&orc_Ini] (auto c_It)
   {
      orc_Ini.setValue("Name", *c_It);
   });
   h_SaveArray(orc_Ini, "SelectedListNames", rc_SelectedListNames, [&orc_Ini] (auto c_It)
   {
      orc_Ini.setValue("Name", *c_It);
   });
   h_SaveArray(orc_Ini, "SelectedVariableNames", rc_SelectedVariableNames, [&orc_Ini] (auto c_It)
   {
      orc_Ini.setValue("Name", *c_It);
   });
   h_SaveArray(orc_Ini, "Interfaces", c_Interfaces, [&orc_Ini, &orc_Datapool] (auto c_It)
   {
      const QString & rc_Name = *c_It;
      orc_Ini.setValue("Name", rc_Name);
      mh_SaveBus(orc_Ini, orc_Datapool.GetCommList(rc_Name));
   });
   h_SaveArray(orc_Ini, "Lists", rc_Lists, [&orc_Ini, &orc_Datapool] (auto c_It)
   {
      const QString & rc_Name = *c_It;
      orc_Ini.setValue("Name", rc_Name);
      mh_SaveList(orc_Ini, orc_Datapool.GetOtherList(rc_Name));
   });
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save node datapool list part of user settings

   \param[in,out]  orc_Ini    Open QSettings positioned at the per-list group
   \param[in]      orc_List   List data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveList(QSettings & orc_Ini, const C_UsNodeDatapoolList & orc_List)
{
   mh_SaveColumns(orc_Ini, "Columns", orc_List.GetColumnWidths());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save view part of user settings

   \param[in,out]  orc_Ini    Open QSettings positioned at the per-view group
   \param[in]      orc_View   View data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveView(QSettings & orc_Ini, const C_UsSystemView & orc_View)
{
   orc_Ini.setValue("NavigationExpandedStatus", orc_View.GetNavigationExpandedStatus());
   orc_Ini.setValue("SetupPos", orc_View.c_SetupViewPos);
   orc_Ini.setValue("SetupZoom", orc_View.s32_SetupViewZoom);
   orc_Ini.setValue("UpdatePos", orc_View.c_UpdateViewPos);
   orc_Ini.setValue("UpdateZoom", orc_View.s32_UpdateViewZoom);

   orc_Ini.setValue("ParamExportPath", orc_View.c_ParamExportPath);
   orc_Ini.setValue("ParamImportPath", orc_View.c_ParamImportPath);
   orc_Ini.setValue("ParamRecordPath", orc_View.c_ParamRecordPath);
   orc_Ini.setValue("ParamRecordFileName", orc_View.c_ParamRecordFileName);

   orc_Ini.setValue("UpdateSplitterX", orc_View.GetUpdateSplitterHorizontal());
   orc_Ini.setValue("UpdateHorizontalSplitterY", orc_View.GetUpdateHorizontalSplitterVertical());

   orc_Ini.setValue("UpdateProgressLogPos", orc_View.GetUpdateProgressLogPos());
   orc_Ini.setValue("UpdateProgressLogSize", orc_View.GetUpdateProgressLogSize());
   orc_Ini.setValue("UpdateProgressLogIsMaximized", orc_View.GetUpdateProgressLogMaximized());

   orc_Ini.setValue("UpdateSummaryBig", orc_View.GetUpdateSummaryBig());
   orc_Ini.setValue("UpdateEmptyOptionalSectionsVisible", orc_View.GetUpdatePackEmptyOptionalSectionsVisible());

   const QList<QString> c_NodeKeys = orc_View.GetViewNodesKeysInternal();
   h_SaveArray(orc_Ini, "ViewNodes", c_NodeKeys, [&orc_Ini, &orc_View] (auto c_It)
   {
      const QString & rc_Name = *c_It;
      orc_Ini.setValue("Name", rc_Name);
      mh_SaveViewNode(orc_Ini, orc_View.GetSvNode(rc_Name));
   });

   orc_Ini.setValue("DashboardToolboxPos", orc_View.GetDashboardToolboxPos());
   orc_Ini.setValue("DashboardToolboxSize", orc_View.GetDashboardToolboxSize());
   orc_Ini.setValue("DashboardToolboxIsMaximized", orc_View.GetDashboardToolboxMaximized());
   orc_Ini.setValue("DashboardSelectedTabIndex", orc_View.GetDashboardSelectedTabIndex());

   const QList<QString> c_DashboardKeys = orc_View.GetDashboardKeysInternal();
   h_SaveArray(orc_Ini, "Dashboards", c_DashboardKeys, [&orc_Ini, &orc_View] (auto c_It)
   {
      const QString & rc_Name = *c_It;
      orc_Ini.setValue("Name", rc_Name);
      mh_SaveDashboard(orc_Ini, orc_View.GetDashboardSettings(rc_Name));
   });

   orc_Ini.setValue("SetupPermission", orc_View.GetSetupPermission());
   orc_Ini.setValue("UpdatePermission", orc_View.GetUpdatePermission());
   orc_Ini.setValue("DashboardPermission", orc_View.GetDashboardPermission());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save view update data rates per node part of user settings

   \param[in,out]  orc_Ini    Open QSettings positioned at the per-view-node group
   \param[in]      orc_Node   View node data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveDataRatesPerNode(QSettings & orc_Ini, const C_UsSystemViewNode & orc_Node)
{
   const QMap<uint32_t, float64_t> & rc_UpdateDataRateHistory = orc_Node.GetUpdateDataRateHistory();

   h_SaveArray(orc_Ini, "UpdateDataRates", rc_UpdateDataRateHistory, [&orc_Ini] (auto c_It)
   {
      // Persist the checksum as a string to match legacy behavior (uint32 values can exceed
      // QSettings' INI integer round-trip range on some platforms).
      orc_Ini.setValue("Checksum", QString::number(c_It.key()));
      orc_Ini.setValue("Value", c_It.value());
   });
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save view node part of user settings

   \param[in,out]  orc_Ini        Open QSettings positioned at the per-view-node group
   \param[in]      orc_ViewNode   View node data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveViewNode(QSettings & orc_Ini, const C_UsSystemViewNode & orc_ViewNode)
{
   const QVector<bool> & rc_ExpandedFlags = orc_ViewNode.GetSectionsExpanded();

   h_SaveArray(orc_Ini, "SectionsExpanded", rc_ExpandedFlags, [&orc_Ini] (auto c_It)
   {
      orc_Ini.setValue("Expanded", *c_It);
   });

   mh_SaveDataRatesPerNode(orc_Ini, orc_ViewNode);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save view dashboard part of user settings

   \param[in,out]  orc_Ini         Open QSettings positioned at the per-dashboard group
   \param[in]      orc_Dashboard   Dashboard data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveDashboard(QSettings & orc_Ini, const C_UsSystemViewDashboard & orc_Dashboard)
{
   orc_Ini.setValue("TornOffFlag", orc_Dashboard.q_TornOff);
   orc_Ini.setValue("WindowPos", orc_Dashboard.c_TornOffWindowPosition);
   orc_Ini.setValue("Size", orc_Dashboard.c_TornOffWindowSize);
   orc_Ini.setValue("MinFlag", orc_Dashboard.q_TornOffWindowMinimized);
   orc_Ini.setValue("MaxFlag", orc_Dashboard.q_TornOffWindowMaximized);
   orc_Ini.setValue("ScenePos", orc_Dashboard.c_ScenePos);
   orc_Ini.setValue("SceneZoom", orc_Dashboard.s32_SceneZoom);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save common part of user settings (language, save-as path, perf-measurement flag)

   \param[in,out]  orc_Ini             Open QSettings instance
   \param[in]      orc_UserSettings    User settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveCommon(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings)
{
   orc_Ini.remove("Common");
   orc_Ini.beginGroup("Common");
   orc_Ini.setValue("Language", orc_UserSettings.GetLanguage());
   orc_Ini.setValue("SaveAsLocation", orc_UserSettings.GetCurrentSaveAsPath());
   orc_Ini.setValue("PerformanceMeasurementActive", orc_UserSettings.GetPerformanceActive());
   orc_Ini.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save environment part of user settings (options in GeneralSettings dialog in MainWindow)

   \param[in,out]  orc_Ini             Open QSettings instance
   \param[in]      orc_UserSettings    User settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveEnvironment(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings)
{
   orc_Ini.remove("Environment");
   orc_Ini.beginGroup("Environment");
   orc_Ini.setValue("PathHandlingSelection", orc_UserSettings.GetPathHandlingSelection());
   orc_Ini.setValue("SkipTspImportSelection", orc_UserSettings.GetSkipTspSelection());
   orc_Ini.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save recent colors part of user settings

   \param[in,out]  orc_Ini             Open QSettings instance
   \param[in]      orc_UserSettings    User settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveColors(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings)
{
   const QVector<QColor> c_RecentColors = orc_UserSettings.GetRecentColors();

   // Clear the section to drop any legacy ColorNr<N>_Red etc. keys before rewriting.
   // mh_SaveNextRecentColorButtonNumber runs after this and adds the remaining key.
   orc_Ini.remove("RecentColors");
   orc_Ini.beginGroup("RecentColors");
   h_SaveArray(orc_Ini, "Colors", c_RecentColors, [&orc_Ini] (auto c_It)
   {
      orc_Ini.setValue("Red", c_It->red());
      orc_Ini.setValue("Green", c_It->green());
      orc_Ini.setValue("Blue", c_It->blue());
      orc_Ini.setValue("Alpha", c_It->alpha());
   });
   orc_Ini.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save next recent color button number part of user settings

   \param[in,out]  orc_Ini             Open QSettings instance
   \param[in]      orc_UserSettings    User settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveNextRecentColorButtonNumber(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings)
{
   orc_Ini.beginGroup("RecentColors");
   orc_Ini.setValue("NextRecentColorButtonNumber", orc_UserSettings.GetNextRecentColorButtonNumber());
   orc_Ini.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save recent projects part of user settings

   Recent projects are stored as a QSettings array. The legacy code cleared the
   whole RecentProjects section first to avoid stale entries; we do the same by
   removing the group before rewriting it.

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
/*! \brief   Save project independent part of user settings

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

   orc_Ini.remove("SdTopologyToolbox");
   orc_Ini.beginGroup("SdTopologyToolbox");
   orc_Ini.setValue("Position", orc_UserSettings.GetSdTopologyToolboxPos());
   orc_Ini.setValue("Size", orc_UserSettings.GetSdTopologyToolboxSize());
   orc_Ini.setValue("Maximized", orc_UserSettings.GetSdTopologyToolboxMaximized());
   orc_Ini.endGroup();

   orc_Ini.remove("SdNodeEdit");
   orc_Ini.beginGroup("SdNodeEdit");
   orc_Ini.setValue("SplitterX", orc_UserSettings.GetSdNodeEditSplitterHorizontal());
   orc_Ini.setValue("HalcSplitterX", orc_UserSettings.GetSdNodeEditHalcSplitterHorizontal());
   orc_Ini.setValue("CoManagerSplitterX", orc_UserSettings.GetSdNodeEditCoManagerSplitterHorizontal());
   orc_Ini.setValue("DataLoggerSplitterX", orc_UserSettings.GetSdNodeEditDataLoggerSplitterHorizontal());
   orc_Ini.endGroup();

   orc_Ini.remove("SdBusEdit");
   orc_Ini.beginGroup("SdBusEdit");
   orc_Ini.setValue("TreeSplitterX", orc_UserSettings.GetSdBusEditTreeSplitterHorizontal());
   orc_Ini.setValue("TreeSplitterX2", orc_UserSettings.GetSdBusEditTreeSplitterHorizontal2());
   orc_Ini.setValue("LayoutSplitterX", orc_UserSettings.GetSdBusEditLayoutSplitterHorizontal());
   orc_Ini.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save project dependent part of user settings

   The active project's settings are persisted as one entry in the top-level
   "Projects" QSettings array, keyed by the project file path stored as
   "Path" inside the array slot. Other projects' entries (slots not matching
   the active path) are preserved unchanged. The project path is NOT used as
   a QSettings group name because QSettings interprets '/' as group nesting,
   which would mangle absolute Unix paths.

   \param[in,out]  orc_Ini             Open QSettings instance
   \param[in]      orc_UserSettings    User settings
   \param[in]      orc_ActiveProject   Active project. Empty string means no project-specific settings written.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveProjectDependentSection(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings,
                                               const QString & orc_ActiveProject)
{
   if (orc_ActiveProject != "")
   {
      const QList<QString> c_NodeKeyList = orc_UserSettings.GetProjSdNodeKeysInternal();
      const QList<QString> c_BusKeyList = orc_UserSettings.GetProjSdBusKeysInternal();
      const QList<QString> c_ViewKeyList = orc_UserSettings.GetProjSvSetupViewKeysInternal();
      const QStringList c_PemFilePaths = orc_UserSettings.GetLastKnownUpdatePemFilePaths();
      int32_t s32_SysDefSubMode;
      uint32_t u32_SysDefIndex;
      uint32_t u32_SysDefFlag;
      int32_t s32_SysViewSubMode;
      uint32_t u32_SysViewIndex;
      uint32_t u32_SysViewFlag;

      orc_UserSettings.GetProjLastScreenMode(s32_SysDefSubMode, u32_SysDefIndex, u32_SysDefFlag,
                                             s32_SysViewSubMode, u32_SysViewIndex, u32_SysViewFlag);

      // Locate active project's existing slot in the Projects array, or allocate a new one at the end.
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
      orc_Ini.setValue("ProjMode", orc_UserSettings.GetProjLastMode());

      orc_Ini.setValue("navigation-width", orc_UserSettings.GetNaviBarSize());
      orc_Ini.setValue("navigation-node-section-width", orc_UserSettings.GetNaviBarNodeSectionSize());

      orc_Ini.setValue("SdTopologyViewPos", orc_UserSettings.GetProjSdTopologyViewPos());
      orc_Ini.setValue("SdTopologyViewZoom_value", orc_UserSettings.GetProjSdTopologyViewZoom());

      orc_Ini.setValue("ProjSdSubMode_value", static_cast<int32_t>(s32_SysDefSubMode));
      orc_Ini.setValue("ProjSdSubIndex_value", static_cast<int32_t>(u32_SysDefIndex));
      orc_Ini.setValue("ProjSdSubFlag_value", static_cast<int32_t>(u32_SysDefFlag));
      orc_Ini.setValue("ProjSvSubMode_value", static_cast<int32_t>(s32_SysViewSubMode));
      orc_Ini.setValue("ProjSvSubIndex_value", static_cast<int32_t>(u32_SysViewIndex));
      orc_Ini.setValue("ProjSvSubFlag_value", static_cast<int32_t>(u32_SysViewFlag));

      orc_Ini.setValue("ProjSdTopology_last_known_tsp_path", orc_UserSettings.GetProjSdTopologyLastKnownTspPath());
      orc_Ini.setValue("ProjSdTopology_last_known_code_export_path",
                       orc_UserSettings.GetProjSdTopologyLastKnownCodeExportPath());
      orc_Ini.setValue("ProjSdTopology_last_known_import_path",
                       orc_UserSettings.GetProjSdTopologyLastKnownImportPath());
      orc_Ini.setValue("ProjSdTopology_last_known_CANopen_EDS_path",
                       orc_UserSettings.GetProjSdTopologyLastKnownCanOpenEdsPath());
      orc_Ini.setValue("ProjSdTopology_last_known_export_path",
                       orc_UserSettings.GetProjSdTopologyLastKnownExportPath());
      orc_Ini.setValue("ProjSdTopology_last_known_device_definition_path",
                       orc_UserSettings.GetProjSdTopologyLastKnownDeviceDefPath());
      orc_Ini.setValue("ProjSdTopology_last_known_rtf_path", orc_UserSettings.GetProjSdTopologyLastKnownRtfPath());
      orc_Ini.setValue("ProjSdTopology_last_known_rtf_company_name",
                       orc_UserSettings.GetProjSdTopologyLastKnownRtfCompanyName());
      orc_Ini.setValue("ProjSdTopology_last_known_rtf_company_logo_path",
                       orc_UserSettings.GetProjSdTopologyLastKnownRtfCompanyLogoPath());

      orc_Ini.setValue("ProjSd_last_known_halc_def_path", orc_UserSettings.GetLastKnownHalcDefPath());
      orc_Ini.setValue("ProjSd_last_known_halc_import_path", orc_UserSettings.GetLastKnownHalcImportPath());
      orc_Ini.setValue("ProjSd_last_known_halc_export_path", orc_UserSettings.GetLastKnownHalcExportPath());
      orc_Ini.setValue("ProjSd_last_known_service_project_path", orc_UserSettings.GetLastKnownServiceProjectPath());
      orc_Ini.setValue("ProjSd_last_known_ramview_project_path", orc_UserSettings.GetLastKnownRamViewProjectPath());
      orc_Ini.setValue("ProjSd_last_known_j1939_catalog_path", orc_UserSettings.GetLastKnownJ1939CatalogPath());
      orc_Ini.setValue("ProjSd_last_known_csv_export_path", orc_UserSettings.GetLastKnownCsvExportPath());

      orc_Ini.setValue("ProjSdNodeEditTabIndex_value", orc_UserSettings.GetProjLastSysDefNodeTabIndex());
      orc_Ini.setValue("ProjSdBusEditTabIndex_value", orc_UserSettings.GetProjLastSysDefBusTabIndex());

      h_SaveArray(orc_Ini, "SdNodes", c_NodeKeyList, [&orc_Ini, &orc_UserSettings] (auto c_It)
      {
         const QString & rc_Name = *c_It;
         orc_Ini.setValue("Name", rc_Name);
         mh_SaveNode(orc_Ini, orc_UserSettings.GetProjSdNode(rc_Name));
      });

      h_SaveArray(orc_Ini, "SdBuses", c_BusKeyList, [&orc_Ini, &orc_UserSettings] (auto c_It)
      {
         const QString & rc_Name = *c_It;
         orc_Ini.setValue("Name", rc_Name);
         mh_SaveBus(orc_Ini, orc_UserSettings.GetProjSdBus(rc_Name));
      });

      h_SaveArray(orc_Ini, "SvSetupViews", c_ViewKeyList, [&orc_Ini, &orc_UserSettings] (auto c_It)
      {
         const QString & rc_Name = *c_It;
         orc_Ini.setValue("Name", rc_Name);
         mh_SaveView(orc_Ini, orc_UserSettings.GetProjSvSetupView(rc_Name));
      });

      orc_Ini.setValue("ProjSd_last_known_public_pem_file_path", orc_UserSettings.GetLastKnownPublicPemFilePath());
      orc_Ini.setValue("ProjSd_last_known_pem_file_path", orc_UserSettings.GetLastKnownAddPemFilePath());
      orc_Ini.setValue("ProjSd_last_known_secure_certificate_package_path",
                       orc_UserSettings.GetLastKnownSecureCertificatePackagePath());
      orc_Ini.setValue("ProjSd_last_known_pem_file_password", orc_UserSettings.GetLastKnownPemFilePassword());
      orc_Ini.setValue("ProjSd_last_known_Add_pem_file_state", orc_UserSettings.GetLastKnownAddPemFileState());
      orc_Ini.setValue("ProjSd_last_known_secure_update_config_state",
                       orc_UserSettings.GetLastKnownSecureUpdateConfigState());

      orc_Ini.endArray();

      // Update widget settings live in their own top-level section, not under the project group
      orc_Ini.remove("Update");
      orc_Ini.beginGroup("Update");
      h_SaveArray(orc_Ini, "PemFiles", c_PemFilePaths, [&orc_Ini] (auto c_It)
      {
         orc_Ini.setValue("Path", *c_It);
      });
      orc_Ini.endGroup();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save column widths as a QSettings array.

   \param[in,out]  orc_Ini            Open QSettings instance (positioned at any group)
   \param[in]      orc_ArrayName      Array key name
   \param[in]      orc_ColumnWidths   Column widths
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveColumns(QSettings & orc_Ini, const QString & orc_ArrayName,
                               const std::vector<int32_t> & orc_ColumnWidths)
{
   h_SaveArray(orc_Ini, orc_ArrayName, orc_ColumnWidths, [&orc_Ini] (auto c_It)
   {
      orc_Ini.setValue("Width", *c_It);
   });
}


//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load node part of user settings

   \param[in,out]  orc_Ini             Ini handler
   \param[in]      orc_SectionName     Section name
   \param[in]      orc_NodeIdBase      Node id base name
   \param[in]      orc_NodeName        Node name
   \param[in,out]  orc_UserSettings    User settings to load
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadNode(C_SclIniFile & orc_Ini, const QString & orc_SectionName, const QString & orc_NodeIdBase,
                            const QString & orc_NodeName, C_UsHandler & orc_UserSettings)
{
   QString c_Tmp;

   stw::opensyde_core::C_OscCanProtocol::E_Type e_Tmp;
   uint32_t u32_Tmp;
   std::vector<int32_t> c_Columns;

   const QString c_CanOpenOvColumnId = static_cast<QString>("%1CANopenOverview").arg(orc_NodeIdBase);
   const QString c_CanOpenPdoOvColumnId = static_cast<QString>("%1CANopenPdoOverview").arg(orc_NodeIdBase);
   const QString c_CanOpenSelectedCanOpenManager = static_cast<QString>("%1CANopenManager").arg(orc_NodeIdBase);
   const QString c_CanOpenSelectedCanOpenDeviceInterfaceNumber =
      static_cast<QString>("%1CANopenDeviceInterfaceNumber").arg(orc_NodeIdBase);
   const QString c_CanOpenSelectedCanOpenDeviceNodeName = static_cast<QString>("%1CANopenDeviceNodeName").arg(
      orc_NodeIdBase);
   const QString c_CanOpenSelectedCanOpenDeviceUseCaseIndex = static_cast<QString>("%1CANopenDeviceUseCase").arg(
      orc_NodeIdBase);
   const QString c_CanOpenExpandedCanOpenManager =
      static_cast<QString>("%1CANopenManagerExpanded#").arg(orc_NodeIdBase);
   const QString c_CanOpenExpandedCanOpenManagerCounter =
      static_cast<QString>("%1CANopenManagerExpandedCounter").arg(orc_NodeIdBase);
   const QString c_CanOpenExpandedCanOpenDevices =
      static_cast<QString>("%1CANopenDevicesExpanded#").arg(orc_NodeIdBase);
   const QString c_CanOpenExpandedCanOpenDevicesCounter =
      static_cast<QString>("%1CANopenDevicesExpandedCounter").arg(orc_NodeIdBase);
   const QString c_CanOpenExpandedCanOpenDevice =
      static_cast<QString>("%1CANopenDeviceExpanded#").arg(orc_NodeIdBase);
   const QString c_CanOpenExpandedCanOpenDeviceCounter =
      static_cast<QString>("%1CANopenDeviceExpandedCounter").arg(orc_NodeIdBase);
   const QString c_CanOpenSelectedUseCaseOrInterface =
      static_cast<QString>("%1CANopenSelectedUseCaseOrInterface").arg(orc_NodeIdBase);
   const QString c_HalcOvColumnId = static_cast<QString>("%1HALCOverview").arg(orc_NodeIdBase);
   const QString c_HalcConfigColumnId = static_cast<QString>("%1HALCParamConfig").arg(orc_NodeIdBase);
   const QString c_NodeIdSelectedHalcDomain = static_cast<QString>("%1Selected_HALC_domain").arg(orc_NodeIdBase);
   const QString c_NodeIdSelectedHalcChannel = static_cast<QString>("%1Selected_HALC_channel").arg(orc_NodeIdBase);
   const QString c_NodeIdDatapoolCount = static_cast<QString>("%1Datapool_count").arg(orc_NodeIdBase);
   const QString c_NodeIdSelectedDatapoolName = static_cast<QString>("%1Selected_datapool_name").arg(orc_NodeIdBase);
   const QString c_NodeIdSelectedProtocol = static_cast<QString>("%1Selected_protocol").arg(orc_NodeIdBase);
   const QString c_NodeIdSelectedInterface = static_cast<QString>("%1Selected_interface").arg(orc_NodeIdBase);
   const QString c_NodeIdSelectedDataLoggerLogJobIndex = static_cast<QString>("%1Selected_DataLogger_LogJobindex").arg(
      orc_NodeIdBase);
   const QString c_DataLoggerOverviewWidgetSelected = static_cast<QString>("%1Selected_DataLogger_LogJob_Overview").arg(
      orc_NodeIdBase);

   //Selected datapool name
   c_Tmp = orc_Ini.ReadString(orc_SectionName.toStdString().c_str(),
                              c_NodeIdSelectedDatapoolName.toStdString().c_str(), "").c_str();
   orc_UserSettings.SetProjSdNodeSelectedDatapoolName(orc_NodeName, c_Tmp);
   //Selected protocol
   e_Tmp = static_cast<stw::opensyde_core::C_OscCanProtocol::E_Type>(
      orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(), c_NodeIdSelectedProtocol.toStdString().c_str(), 0));
   orc_UserSettings.SetProjSdNodeSelectedProtocol(orc_NodeName, e_Tmp);
   //Selected interface
   u32_Tmp = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                 c_NodeIdSelectedInterface.toStdString().c_str(), 0);
   orc_UserSettings.SetProjSdNodeSelectedInterface(orc_NodeName, u32_Tmp);
   //Selected HALC domain & channel
   c_Tmp = orc_Ini.ReadString(orc_SectionName.toStdString().c_str(),
                              c_NodeIdSelectedHalcDomain.toStdString().c_str(), "").c_str();
   orc_UserSettings.SetProjSdNodeSelectedHalcDomain(orc_NodeName, c_Tmp);
   c_Tmp = orc_Ini.ReadString(orc_SectionName.toStdString().c_str(),
                              c_NodeIdSelectedHalcChannel.toStdString().c_str(), "").c_str();
   orc_UserSettings.SetProjSdNodeSelectedHalcChannel(orc_NodeName, c_Tmp);
   //CANopen columns
   c_Columns.clear();
   C_UsFiler::mh_LoadColumns(orc_Ini, orc_SectionName, c_CanOpenOvColumnId, c_Columns);
   orc_UserSettings.SetProjSdNodeCanOpenOverviewColumnWidth(orc_NodeName, c_Columns);
   c_Columns.clear();
   C_UsFiler::mh_LoadColumns(orc_Ini, orc_SectionName, c_CanOpenPdoOvColumnId, c_Columns);
   orc_UserSettings.SetProjSdNodeCanOpenPdoOverviewColumnWidth(orc_NodeName, c_Columns);

   //CANopen
   std::map<uint8_t, bool> c_LoadInterfaces;
   std::map<uint8_t, bool> c_LoadDevices;
   std::map<std::pair<uint8_t, std::pair<uint8_t, stw::scl::C_SclString> >, bool> c_LoadDevice;
   u32_Tmp = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                 c_CanOpenExpandedCanOpenManagerCounter.toStdString().c_str(), 0);
   for (uint32_t u32_InterfaceCounter = 0UL; u32_InterfaceCounter < u32_Tmp; u32_InterfaceCounter++)
   {
      c_LoadInterfaces[static_cast<uint8_t>(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                                                (c_CanOpenExpandedCanOpenManager +
                                                                 QString::number(u32_InterfaceCounter) +
                                                                 "InterfaceNumber").toStdString().c_str(), 0))] =
         orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                          (c_CanOpenExpandedCanOpenManager +
                           QString::number(u32_InterfaceCounter)).toStdString().c_str(), false);
   }
   const uint32_t u32_TmpDevices = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                                       c_CanOpenExpandedCanOpenDevicesCounter.toStdString().c_str(),
                                                       0);
   for (uint32_t u32_DevicesCounter = 0UL; u32_DevicesCounter < u32_TmpDevices; u32_DevicesCounter++)
   {
      c_LoadDevices[static_cast<uint8_t>(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                                             (c_CanOpenExpandedCanOpenDevices +
                                                              QString::number(u32_DevicesCounter) +
                                                              "InterfaceNumber").toStdString().c_str(), 0))] =
         orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                          (c_CanOpenExpandedCanOpenDevices +
                           QString::number(u32_DevicesCounter)).toStdString().c_str(), false);
   }
   const uint32_t u32_TmpDevice = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                                      c_CanOpenExpandedCanOpenDeviceCounter.toStdString().c_str(),
                                                      0);
   for (uint32_t u32_DeviceCounter = 0UL; u32_DeviceCounter < u32_TmpDevice; u32_DeviceCounter++)
   {
      const std::pair<uint8_t, stw::scl::C_SclString> c_PairInterfaceId(static_cast<uint8_t>(orc_Ini.ReadInteger(
                                                                                                orc_SectionName.toStdString().c_str(),
                                                                                                (c_CanOpenExpandedCanOpenDevice + QString::number(
                                                                                                    u32_DeviceCounter) +
                                                                                                 "DeviceInterfaceNumber").toStdString().c_str(), 0)),
                                                                        orc_Ini.ReadString(
                                                                           orc_SectionName.toStdString().c_str(),
                                                                           (c_CanOpenExpandedCanOpenDevice + QString::number(
                                                                               u32_DeviceCounter) +
                                                                            "DeviceNodeName").toStdString().c_str(),
                                                                           ""));
      const std::pair<uint8_t, std::pair<uint8_t, stw::scl::C_SclString> > c_Pair(
         static_cast<uint8_t>(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                                  (c_CanOpenExpandedCanOpenManager +
                                                   QString::number(u32_DeviceCounter) +
                                                   "InterfaceNumber").toStdString().c_str(), 0)),
         c_PairInterfaceId);
      c_LoadDevice[c_Pair] = orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                                              (c_CanOpenExpandedCanOpenDevice +
                                               QString::number(u32_DeviceCounter)).toStdString().c_str(), false);
   }
   orc_UserSettings.SetProjSdNodeExpandedCanOpenTree(orc_NodeName, c_LoadInterfaces, c_LoadDevices, c_LoadDevice);
   u32_Tmp = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                 c_CanOpenSelectedCanOpenManager.toStdString().c_str(), 0);
   orc_UserSettings.SetProjSdNodeSelectedCanOpenManager(orc_NodeName, static_cast<uint8_t>(u32_Tmp));
   u32_Tmp = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                 c_CanOpenSelectedCanOpenDeviceInterfaceNumber.toStdString().c_str(), 0);
   c_Tmp = orc_Ini.ReadString(orc_SectionName.toStdString().c_str(),
                              c_CanOpenSelectedCanOpenDeviceNodeName.toStdString().c_str(), "").c_str();
   orc_UserSettings.SetProjSdNodeSelectedCanOpenDevice(orc_NodeName, static_cast<uint8_t>(u32_Tmp), c_Tmp);
   u32_Tmp = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                 c_CanOpenSelectedCanOpenDeviceUseCaseIndex.toStdString().c_str(), 0);
   orc_UserSettings.SetProjSdNodeSelectedCanOpenDeviceUseCaseIndex(orc_NodeName, u32_Tmp);

   orc_UserSettings.SetProjSdNodeCanOpenSelectedUseCaseOrInterface(
      orc_NodeName,
      orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                       c_CanOpenSelectedUseCaseOrInterface.toStdString().c_str(), false));

   //HALC columns
   c_Columns.clear();
   C_UsFiler::mh_LoadColumns(orc_Ini, orc_SectionName, c_HalcOvColumnId, c_Columns);
   orc_UserSettings.SetProjSdNodeHalcOverviewColumnWidth(orc_NodeName, c_Columns);
   c_Columns.clear();
   C_UsFiler::mh_LoadColumns(orc_Ini, orc_SectionName, c_HalcConfigColumnId, c_Columns);
   orc_UserSettings.SetProjSdNodeHalcConfigColumnWidth(orc_NodeName, c_Columns);

   //Datapool count
   const int32_t s32_DatapoolCount = orc_Ini.ReadInteger(
      orc_SectionName.toStdString().c_str(), c_NodeIdDatapoolCount.toStdString().c_str(), 0);

   //Datapool
   for (int32_t s32_ItDatapool = 0; s32_ItDatapool < s32_DatapoolCount; ++s32_ItDatapool)
   {
      const QString c_DatapoolIdBase = static_cast<QString>("%1Datapool%2").arg(orc_NodeIdBase).arg(s32_ItDatapool);
      mh_LoadDatapool(orc_Ini, orc_SectionName, c_DatapoolIdBase, orc_NodeName, orc_UserSettings);
   }
   //DataLogger LogJob Index
   u32_Tmp = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                 c_NodeIdSelectedDataLoggerLogJobIndex.toStdString().c_str(), 0);
   orc_UserSettings.SetProjSdNodeSelectedDataLoggerLogJobIndex(orc_NodeName, u32_Tmp);
   //DataLogger Overview widget selected
   const bool q_IsOverviewWidgetSelected = orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                                                            c_DataLoggerOverviewWidgetSelected.toStdString().c_str(),
                                                            false);
   orc_UserSettings.SetProjSdNodeIsOverviewWidgetSelected(orc_NodeName, q_IsOverviewWidgetSelected);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load bus part of user settings

   \param[in,out]  orc_Ini             Ini handler
   \param[in]      orc_SectionName     Section name
   \param[in]      orc_BusIdBase       Bus id base name
   \param[in]      orc_BusName         Bus name
   \param[in,out]  orc_UserSettings    User settings to load
   \param[in]      oq_IsBus            Indicator if this function is used on a bus
   \param[in]      orc_NodeName        If not used on a bus the node name is required
   \param[in]      orc_DataPoolName    If not used on a bus the node data pool name is required
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadBus(C_SclIniFile & orc_Ini, const QString & orc_SectionName, const QString & orc_BusIdBase,
                           const QString & orc_BusName, C_UsHandler & orc_UserSettings, const bool oq_IsBus,
                           const QString & orc_NodeName, const QString & orc_DataPoolName)
{
   const QString c_BusIdSelectedComProtocol =
      static_cast<QString>("%1Selected_com_protocol").arg(orc_BusIdBase);
   const QString c_BusIdMessageOverview = static_cast<QString>("%1MessageOverview").arg(orc_BusIdBase);
   const QString c_BusIdSignalOverview = static_cast<QString>("%1SignalOverview").arg(orc_BusIdBase);
   const QString c_BusIdMessageSelected = static_cast<QString>("%1Message_selected").arg(orc_BusIdBase);
   const QString c_BusIdSelectedMessageName = static_cast<QString>("%1Selected_message_name").arg(orc_BusIdBase);
   const QString c_BusIdSignalSelected = static_cast<QString>("%1Signal_selected").arg(orc_BusIdBase);
   const QString c_BusIdSelectedSignalName = static_cast<QString>("%1Selected_signal_index").arg(orc_BusIdBase);

   stw::opensyde_core::C_OscCanProtocol::E_Type e_SelectedProtocol;
   bool q_MessageSelected;
   QString c_MessageName;
   bool q_SignalSelected;
   QString c_SignalName;
   std::vector<int32_t> c_MessageColumns;
   std::vector<int32_t> c_SignalColumns;

   C_UsFiler::mh_LoadColumns(orc_Ini, orc_SectionName, c_BusIdMessageOverview, c_MessageColumns);
   C_UsFiler::mh_LoadColumns(orc_Ini, orc_SectionName, c_BusIdSignalOverview, c_SignalColumns);

   e_SelectedProtocol = static_cast<stw::opensyde_core::C_OscCanProtocol::E_Type>(orc_Ini.ReadInteger(
                                                                                     orc_SectionName.toStdString().c_str(),
                                                                                     c_BusIdSelectedComProtocol.
                                                                                     toStdString().c_str(), 0));
   q_MessageSelected = orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                                        c_BusIdMessageSelected.toStdString().c_str(), false);
   c_MessageName = orc_Ini.ReadString(orc_SectionName.toStdString().c_str(),
                                      c_BusIdSelectedMessageName.toStdString().c_str(), "").c_str();
   q_SignalSelected = orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                                       c_BusIdSignalSelected.toStdString().c_str(), false);
   c_SignalName = orc_Ini.ReadString(orc_SectionName.toStdString().c_str(),
                                     c_BusIdSelectedSignalName.toStdString().c_str(), "").c_str();

   if (oq_IsBus == true)
   {
      orc_UserSettings.SetProjSdBusCommMessageOverviewColumnWidth(orc_BusName, c_MessageColumns);
      orc_UserSettings.SetProjSdBusCommSignalOverviewColumnWidth(orc_BusName, c_SignalColumns);
      orc_UserSettings.SetProjSdBusSelectedMessage(orc_BusName, e_SelectedProtocol, q_MessageSelected, c_MessageName,
                                                   q_SignalSelected, c_SignalName);
   }
   else
   {
      orc_UserSettings.SetProjSdNodeDatapoolCommMessageOverviewColumnWidth(orc_NodeName, orc_DataPoolName, orc_BusName,
                                                                           c_MessageColumns);
      orc_UserSettings.SetProjSdNodeDatapoolCommSignalOverviewColumnWidth(orc_NodeName, orc_DataPoolName, orc_BusName,
                                                                          c_SignalColumns);
      orc_UserSettings.SetProjSdNodeDatapoolListSelectedMessage(orc_NodeName, orc_DataPoolName, orc_BusName,
                                                                e_SelectedProtocol, q_MessageSelected, c_MessageName,
                                                                q_SignalSelected, c_SignalName);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load node datapool part of user settings

   \param[in,out]  orc_Ini             Ini handler
   \param[in]      orc_SectionName     Section name
   \param[in]      orc_DatapoolIdBase  Node datapool id base name
   \param[in]      orc_NodeName        Node name
   \param[in,out]  orc_UserSettings    User settings to load
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadDatapool(C_SclIniFile & orc_Ini, const QString & orc_SectionName,
                                const QString & orc_DatapoolIdBase, const QString & orc_NodeName,
                                C_UsHandler & orc_UserSettings)
{
   const QString c_DatapoolIdName = static_cast<QString>("%1Name").arg(orc_DatapoolIdBase);
   const QString c_DatapoolName = orc_Ini.ReadString(
      orc_SectionName.toStdString().c_str(), c_DatapoolIdName.toStdString().c_str(), "").c_str();

   if (c_DatapoolName.compare("") != 0)
   {
      int32_t s32_SystemBusCount;
      int32_t s32_ListCount;
      const QString c_DatapoolIdExpandedListNameCount =
         static_cast<QString>("%1ExpandedListName_count").arg(orc_DatapoolIdBase);
      const QString c_DatapoolIdSelectedListNameCount =
         static_cast<QString>("%1SelectedListName_count").arg(orc_DatapoolIdBase);
      const QString c_DatapoolIdSelectedVariableNameCount =
         static_cast<QString>("%1SelectedVariableName_count").arg(orc_DatapoolIdBase);
      const QString c_DatapoolIdListCount =
         static_cast<QString>("%1Lists_count").arg(orc_DatapoolIdBase);
      const QString c_DatapoolIdInterfaceCount =
         static_cast<QString>("%1Interface_count").arg(orc_DatapoolIdBase);
      std::vector<QString> c_ExpandedListNames;
      std::vector<QString> c_SelectedListNames;
      std::vector<QString> c_SelectedVariableNames;
      const int32_t s32_ExpandedListNameCount = orc_Ini.ReadInteger(
         orc_SectionName.toStdString().c_str(), c_DatapoolIdExpandedListNameCount.toStdString().c_str(), 0);
      const int32_t s32_SelectedListNameCount = orc_Ini.ReadInteger(
         orc_SectionName.toStdString().c_str(), c_DatapoolIdSelectedListNameCount.toStdString().c_str(), 0);
      const int32_t s32_SelectedVariableNameCount = orc_Ini.ReadInteger(
         orc_SectionName.toStdString().c_str(), c_DatapoolIdSelectedVariableNameCount.toStdString().c_str(), 0);

      //Expanded lists
      c_ExpandedListNames.reserve(s32_ExpandedListNameCount);
      for (int32_t s32_ItExpandedListName = 0; s32_ItExpandedListName < s32_ExpandedListNameCount;
           ++s32_ItExpandedListName)
      {
         const QString c_DatapoolIdExpandedListNameBaseId =
            static_cast<QString>("%1ExpandedListName%2").arg(orc_DatapoolIdBase).arg(s32_ItExpandedListName);
         const QString c_DatapoolIdExpandedListNameId =
            static_cast<QString>("%1Name").arg(c_DatapoolIdExpandedListNameBaseId);
         const QString c_DatapoolIdExpandedListName = orc_Ini.ReadString(
            orc_SectionName.toStdString().c_str(), c_DatapoolIdExpandedListNameId.toStdString().c_str(), "").c_str();
         if (c_DatapoolIdExpandedListName.compare("") != 0)
         {
            c_ExpandedListNames.push_back(c_DatapoolIdExpandedListName);
         }
      }
      orc_UserSettings.SetProjSdNodeDatapoolOpenListNames(orc_NodeName, c_DatapoolName, c_ExpandedListNames);

      //Selected lists
      c_SelectedListNames.reserve(s32_SelectedListNameCount);
      for (int32_t s32_ItSelectedListName = 0; s32_ItSelectedListName < s32_SelectedListNameCount;
           ++s32_ItSelectedListName)
      {
         const QString c_DatapoolIdSelectedListNameBaseId =
            static_cast<QString>("%1SelectedListName%2").arg(orc_DatapoolIdBase).arg(s32_ItSelectedListName);
         const QString c_DatapoolIdSelectedListNameId =
            static_cast<QString>("%1Name").arg(c_DatapoolIdSelectedListNameBaseId);
         const QString c_DatapoolIdSelectedListName = orc_Ini.ReadString(
            orc_SectionName.toStdString().c_str(), c_DatapoolIdSelectedListNameId.toStdString().c_str(), "").c_str();
         if (c_DatapoolIdSelectedListName.compare("") != 0)
         {
            c_SelectedListNames.push_back(c_DatapoolIdSelectedListName);
         }
      }
      orc_UserSettings.SetProjSdNodeDatapoolSelectedListNames(orc_NodeName, c_DatapoolName, c_SelectedListNames);

      //Selected variables
      c_SelectedListNames.reserve(s32_SelectedVariableNameCount);
      for (int32_t s32_ItSelectedVariableName = 0; s32_ItSelectedVariableName < s32_SelectedVariableNameCount;
           ++s32_ItSelectedVariableName)
      {
         const QString c_DatapoolIdSelectedVariableNameBaseId = static_cast<QString>("%1SelectedVariableName%2").arg(
            orc_DatapoolIdBase).arg(
            s32_ItSelectedVariableName);
         const QString c_DatapoolIdSelectedVariableNameId =
            static_cast<QString>("%1Name").arg(c_DatapoolIdSelectedVariableNameBaseId);
         const QString c_DatapoolIdSelectedVariableName = orc_Ini.ReadString(
            orc_SectionName.toStdString().c_str(), c_DatapoolIdSelectedVariableNameId.toStdString().c_str(),
            "").c_str();
         if (c_DatapoolIdSelectedVariableName.compare("") != 0)
         {
            c_SelectedVariableNames.push_back(c_DatapoolIdSelectedVariableName);
         }
      }
      orc_UserSettings.SetProjSdNodeDatapoolSelectedVariableNames(orc_NodeName, c_DatapoolName,
                                                                  c_SelectedVariableNames);

      //Interfaces
      s32_SystemBusCount = orc_Ini.ReadInteger(
         orc_SectionName.toStdString().c_str(), c_DatapoolIdInterfaceCount.toStdString().c_str(), 0);
      for (int32_t s32_ItBus = 0; s32_ItBus < s32_SystemBusCount; ++s32_ItBus)
      {
         const QString c_BusIdBase = static_cast<QString>("%1Interface%2").arg(orc_DatapoolIdBase).arg(s32_ItBus);
         const QString c_BusIdName = static_cast<QString>("%1Name").arg(c_BusIdBase);
         const QString c_BusName = orc_Ini.ReadString(
            orc_SectionName.toStdString().c_str(), c_BusIdName.toStdString().c_str(), "").c_str();
         if (c_BusName.compare("") != 0)
         {
            mh_LoadBus(orc_Ini, orc_SectionName, c_BusIdBase, c_BusName, orc_UserSettings, false, orc_NodeName,
                       c_DatapoolName);
         }
      }

      //Lists
      s32_ListCount = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                          c_DatapoolIdListCount.toStdString().c_str(), 0);
      for (int32_t s32_ItList = 0; s32_ItList < s32_ListCount; ++s32_ItList)
      {
         const QString c_ListIdBase = static_cast<QString>("%1List%2").arg(orc_DatapoolIdBase).arg(s32_ItList);
         mh_LoadList(orc_Ini, orc_SectionName, c_ListIdBase, orc_NodeName, c_DatapoolName, orc_UserSettings);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load view part of user settings

   \param[in,out]  orc_Ini             Ini handler
   \param[in]      orc_SectionName     Section name
   \param[in]      orc_ListIdBase      List id base
   \param[in]      orc_NodeName        Node name
   \param[in]      orc_DataPoolName    Data pool name
   \param[in,out]  orc_UserSettings    User settings to load
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadList(C_SclIniFile & orc_Ini, const QString & orc_SectionName, const QString & orc_ListIdBase,
                            const QString & orc_NodeName, const QString & orc_DataPoolName,
                            C_UsHandler & orc_UserSettings)
{
   const QString c_ListIdName = static_cast<QString>("%1Name").arg(orc_ListIdBase);
   const QString c_ListName = orc_Ini.ReadString(
      orc_SectionName.toStdString().c_str(), c_ListIdName.toStdString().c_str(), "").c_str();

   if (c_ListName.compare("") != 0)
   {
      std::vector<int32_t> c_ColumnWidths;
      C_UsFiler::mh_LoadColumns(orc_Ini, orc_SectionName, orc_ListIdBase, c_ColumnWidths);
      orc_UserSettings.SetProjSdNodeDatapoolListColumnSizes(orc_NodeName, orc_DataPoolName, c_ListName, c_ColumnWidths);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load view part of user settings

   \param[in,out]  orc_Ini             Ini handler
   \param[in]      orc_SectionName     Section name
   \param[in]      orc_ViewIdBase      View id base name
   \param[in]      orc_ViewName        View name
   \param[in,out]  orc_UserSettings    User settings to load
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadView(C_SclIniFile & orc_Ini, const QString & orc_SectionName, const QString & orc_ViewIdBase,
                            const QString & orc_ViewName, C_UsHandler & orc_UserSettings)
{
   const QString c_ViewIdNavigationExpandedStatus =
      static_cast<QString>("%1_navigation_expanded_status").arg(orc_ViewIdBase);
   const QString c_ViewIdNodesCount = static_cast<QString>("%1Node_count").arg(orc_ViewIdBase);
   const QString c_ViewIdSetupPosHorizontal = static_cast<QString>("%1_setup_x").arg(orc_ViewIdBase);
   const QString c_ViewIdSetupPosVertical = static_cast<QString>("%1_setup_y").arg(orc_ViewIdBase);
   const QString c_ViewIdSetupZoom = static_cast<QString>("%1_setup_zoom_value").arg(orc_ViewIdBase);
   const QString c_ViewIdUpdatePosHorizontal = static_cast<QString>("%1_update_x").arg(orc_ViewIdBase);
   const QString c_ViewIdUpdatePosVertical = static_cast<QString>("%1_update_y").arg(orc_ViewIdBase);
   const QString c_ViewIdUpdateZoom = static_cast<QString>("%1_update_zoom_value").arg(orc_ViewIdBase);
   const QString c_ViewIdParamExportPath = static_cast<QString>("%1_param_export_path").arg(orc_ViewIdBase);
   const QString c_ViewIdParamImportPath = static_cast<QString>("%1_param_import_path").arg(orc_ViewIdBase);
   const QString c_ViewIdParamRecordPath = static_cast<QString>("%1_param_record_path").arg(orc_ViewIdBase);
   const QString c_ViewIdParamRecordFileName = static_cast<QString>("%1_param_record_file_name").arg(orc_ViewIdBase);
   const QString c_ViewIdUpdateSplitterHorizontal = static_cast<QString>("%1_update_splitter_x").arg(orc_ViewIdBase);
   const QString c_ViewIdUpdateHorizontalSplitterVertical =
      static_cast<QString>("%1_update_horizontal_splitter_y").arg(orc_ViewIdBase);
   const QString c_ViewIdUpdateProgressLogPositionHorizontal =
      static_cast<QString>("%1_update_progress_log_x").arg(orc_ViewIdBase);
   const QString c_ViewIdUpdateProgressLogPositionVertical =
      static_cast<QString>("%1_update_progress_log_y").arg(orc_ViewIdBase);
   const QString c_ViewIdUpdateProgressLogSizeWidth =
      static_cast<QString>("%1_update_progress_log_width").arg(orc_ViewIdBase);
   const QString c_ViewIdUpdateProgressLogSizeHeight =
      static_cast<QString>("%1_update_progress_log_height").arg(orc_ViewIdBase);
   const QString c_ViewIdUpdateProgressLogIsMaximized =
      static_cast<QString>("%1_update_progress_log_is_maximized").arg(orc_ViewIdBase);
   const QString c_ViewIdUpdateSummaryBig = static_cast<QString>("%1_update_summary_is_type_big").arg(orc_ViewIdBase);
   const QString c_ViewIdUpdateEmptyOptionalSectionsVisible =
      static_cast<QString>("%1_empty_optional_sections_visible").arg(orc_ViewIdBase);
   const QString c_ViewIdDashboardToolboxPositionHorizontal = static_cast<QString>("%1_toolbox_x").arg(orc_ViewIdBase);
   const QString c_ViewIdDashboardToolboxPositionVertical = static_cast<QString>("%1_toolbox_y").arg(orc_ViewIdBase);
   const QString c_ViewIdDashboardToolboxSizeWidth = static_cast<QString>("%1_toolbox_width").arg(orc_ViewIdBase);
   const QString c_ViewIdDashboardToolboxSizeHeight = static_cast<QString>("%1_toolbox_height").arg(orc_ViewIdBase);
   const QString c_ViewIdDashboardToolboxIsMaximized = static_cast<QString>("%1_toolbox_is_maximized").arg(
      orc_ViewIdBase);
   const QString c_ViewIdDashboardSelectedTabIndex = static_cast<QString>("%1_selected_tab_index").arg(orc_ViewIdBase);
   const QString c_ViewIdDashboardCount = static_cast<QString>("%1Dashboard_count").arg(orc_ViewIdBase);
   const QString c_ViewIdSetupPermission = static_cast<QString>("%1_setup_permission").arg(orc_ViewIdBase);
   const QString c_ViewIdUpdatePermission = static_cast<QString>("%1_update_permission").arg(orc_ViewIdBase);
   const QString c_ViewIdDashboardPermission = static_cast<QString>("%1_dashboard_permission").arg(orc_ViewIdBase);
   QPoint c_Pos;
   QSize c_Size;
   int32_t s32_Value;
   bool q_Value;

   std::array<bool, 3> c_ViewConfigs;

   //Navigation
   q_Value = orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                              c_ViewIdNavigationExpandedStatus.toStdString().c_str(), false);
   orc_UserSettings.SetProjSvNavigationExpandedStatus(orc_ViewName, q_Value);

   //Setup pos
   c_Pos.setX(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                  c_ViewIdSetupPosHorizontal.toStdString().c_str(), 0));
   c_Pos.setY(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                  c_ViewIdSetupPosVertical.toStdString().c_str(), 0));

   //Setup zoom
   s32_Value = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                   c_ViewIdSetupZoom.toStdString().c_str(), 100);

   //Setup set
   orc_UserSettings.SetProjSvSetupViewZoom(orc_ViewName, s32_Value);
   orc_UserSettings.SetProjSvSetupViewPos(orc_ViewName, c_Pos);

   //Update pos
   c_Pos.setX(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                  c_ViewIdUpdatePosHorizontal.toStdString().c_str(), 0));
   c_Pos.setY(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                  c_ViewIdUpdatePosVertical.toStdString().c_str(), 0));

   //Update zoom
   s32_Value = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                   c_ViewIdUpdateZoom.toStdString().c_str(), 100);

   //Update set
   orc_UserSettings.SetProjSvUpdateViewZoom(orc_ViewName, s32_Value);
   orc_UserSettings.SetProjSvUpdateViewPos(orc_ViewName, c_Pos);

   //Param
   orc_UserSettings.SetProjSvParamExport(orc_ViewName, orc_Ini.ReadString(orc_SectionName.toStdString().c_str(),
                                                                          c_ViewIdParamExportPath.toStdString().c_str(),
                                                                          "").c_str());
   orc_UserSettings.SetProjSvParamImport(orc_ViewName, orc_Ini.ReadString(orc_SectionName.toStdString().c_str(),
                                                                          c_ViewIdParamImportPath.toStdString().c_str(),
                                                                          "").c_str());
   orc_UserSettings.SetProjSvParamRecord(orc_ViewName, orc_Ini.ReadString(orc_SectionName.toStdString().c_str(),
                                                                          c_ViewIdParamRecordPath.toStdString().c_str(),
                                                                          "").c_str(),
                                         orc_Ini.ReadString(orc_SectionName.toStdString().c_str(),
                                                            c_ViewIdParamRecordFileName.toStdString().c_str(),
                                                            "").c_str());

   //Splitter
   orc_UserSettings.SetProjSvUpdateSplitterHorizontal(orc_ViewName,
                                                      orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                                                          c_ViewIdUpdateSplitterHorizontal.
                                                                          toStdString().
                                                                          c_str(), -1));
   orc_UserSettings.SetProjSvUpdateHorizontalSplitterVertical(
      orc_ViewName, orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                        c_ViewIdUpdateHorizontalSplitterVertical.toStdString().c_str(), -1));

   //Progress log
   c_Pos.setX(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                  c_ViewIdUpdateProgressLogPositionHorizontal.toStdString().c_str(), -1));
   c_Pos.setY(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                  c_ViewIdUpdateProgressLogPositionVertical.toStdString().c_str(), -1));
   c_Size.setWidth(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                       c_ViewIdUpdateProgressLogSizeWidth.toStdString().c_str(), 600));
   c_Size.setHeight(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                        c_ViewIdUpdateProgressLogSizeHeight.toStdString().c_str(), 400));
   q_Value = orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                              c_ViewIdUpdateProgressLogIsMaximized.toStdString().c_str(), false);
   orc_UserSettings.SetProjSvUpdateProgressLog(orc_ViewName, c_Pos, c_Size, q_Value);

   //Update summary type
   q_Value = orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                              c_ViewIdUpdateSummaryBig.toStdString().c_str(), true);
   orc_UserSettings.SetProjSvUpdateSummaryBig(orc_ViewName, q_Value);

   // Update package sections visibility of empty optional sections
   q_Value = orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                              c_ViewIdUpdateEmptyOptionalSectionsVisible.toStdString().c_str(), true);
   orc_UserSettings.SetProjSvUpdateEmptyOptionalSectionsVisible(orc_ViewName, q_Value);

   // View nodes
   s32_Value = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                   c_ViewIdNodesCount.toStdString().c_str(), 0);
   for (int32_t s32_ItNodes = 0; s32_ItNodes < s32_Value; ++s32_ItNodes)
   {
      const QString c_DashboardIdBase = static_cast<QString>("%1Node%2").arg(orc_ViewIdBase).arg(s32_ItNodes);
      mh_LoadViewNode(orc_Ini, orc_SectionName, c_DashboardIdBase, orc_ViewName, orc_UserSettings);
   }

   //Toolbox
   c_Pos.setX(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                  c_ViewIdDashboardToolboxPositionHorizontal.toStdString().c_str(), -1));
   c_Pos.setY(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                  c_ViewIdDashboardToolboxPositionVertical.toStdString().c_str(), -1));
   c_Size.setWidth(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                       c_ViewIdDashboardToolboxSizeWidth.toStdString().c_str(), 600));
   c_Size.setHeight(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                        c_ViewIdDashboardToolboxSizeHeight.toStdString().c_str(), 400));
   q_Value = orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                              c_ViewIdDashboardToolboxIsMaximized.toStdString().c_str(), false);
   orc_UserSettings.SetProjSvDashboardToolbox(orc_ViewName, c_Pos, c_Size, q_Value);

   //General
   s32_Value = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                   c_ViewIdDashboardSelectedTabIndex.toStdString().c_str(), -1);
   orc_UserSettings.SetProjSvDashboardSelectedTabIndex(orc_ViewName, s32_Value);

   //Dashboards
   s32_Value = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                   c_ViewIdDashboardCount.toStdString().c_str(), 0);
   for (int32_t s32_ItDashboard = 0; s32_ItDashboard < s32_Value; ++s32_ItDashboard)
   {
      const QString c_DashboardIdBase = static_cast<QString>("%1Dashboard%2").arg(orc_ViewIdBase).arg(s32_ItDashboard);
      mh_LoadDashboard(orc_Ini, orc_SectionName, c_DashboardIdBase, orc_ViewName, orc_UserSettings);
   }

   //Permissions
   c_ViewConfigs[0] = orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                                       c_ViewIdSetupPermission.toStdString().c_str(),
                                       false);
   c_ViewConfigs[1] = orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                                       c_ViewIdUpdatePermission.toStdString().c_str(),
                                       false);
   c_ViewConfigs[2] = orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                                       c_ViewIdDashboardPermission.toStdString().c_str(),
                                       false);
   orc_UserSettings.SetViewPermission(orc_ViewName, c_ViewConfigs);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load data rates per node part of user settings

   \param[in,out]  orc_Ini                      Ini handler
   \param[in]      orc_SectionName              Section name
   \param[in]      orc_DataRatePerNodeIdBase    View update data rate id base name
   \param[in]      orc_ViewName                 View name
   \param[in]      orc_NodeName                 Node name
   \param[in,out]  orc_UserSettings             User settings to load
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadDataRatesPerNode(C_SclIniFile & orc_Ini, const QString & orc_SectionName,
                                        const QString & orc_DataRatePerNodeIdBase, const QString & orc_ViewName,
                                        const QString & orc_NodeName, C_UsHandler & orc_UserSettings)
{
   const QString c_DataRateIdCount = static_cast<QString>("%1_count").arg(orc_DataRatePerNodeIdBase);
   const int32_t s32_ItDataRate = orc_Ini.ReadInteger(
      orc_SectionName.toStdString().c_str(), c_DataRateIdCount.toStdString().c_str(), 0);

   //Data rate count
   //Per checksum section
   for (int32_t s32_It = 0; s32_It < s32_ItDataRate; ++s32_It)
   {
      const QString c_DataRateIdBase = static_cast<QString>("%1DataRate%2").arg(orc_DataRatePerNodeIdBase).arg(s32_It);
      const QString c_DataRateIdChecksum = static_cast<QString>("%1_checksum").arg(c_DataRateIdBase);
      const QString c_DataRateIdCurrentValue = static_cast<QString>("%1_value").arg(c_DataRateIdBase);
      //Key
      const QString c_Checksum = orc_Ini.ReadString(
         orc_SectionName.toStdString().c_str(), c_DataRateIdChecksum.toStdString().c_str(), "").c_str();
      //Value count
      const float64_t f64_Value = orc_Ini.ReadFloat(
         orc_SectionName.toStdString().c_str(), c_DataRateIdCurrentValue.toStdString().c_str(), 0.0);
      //String to uint32_t
      if (c_Checksum.compare("") != 0)
      {
         bool q_Ok1;
         const uint32_t u32_Checksum = c_Checksum.toULong(&q_Ok1);
         if (q_Ok1 == true)
         {
            //Apply
            orc_UserSettings.AddProjSvNodeUpdateDataRate(orc_ViewName, orc_NodeName, u32_Checksum, f64_Value);
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load node view part of user settings

   \param[in,out]  orc_Ini             Ini handler
   \param[in]      orc_SectionName     Section name
   \param[in]      orc_ViewNodeIdBase  View node ID base name
   \param[in]      orc_ViewName        View name (not a copy paste error)
   \param[in,out]  orc_UserSettings    User settings to load
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadViewNode(C_SclIniFile & orc_Ini, const QString & orc_SectionName,
                                const QString & orc_ViewNodeIdBase, const QString & orc_ViewName,
                                C_UsHandler & orc_UserSettings)
{
   const QString c_ViewNodeIdName = static_cast<QString>("%1Name").arg(orc_ViewNodeIdBase);
   const QString c_NodeIdUpdateDataRateBaseId = static_cast<QString>("%1_update_data_rate").arg(orc_ViewNodeIdBase);
   const QString c_ViewNodeName = orc_Ini.ReadString(orc_SectionName.toStdString().c_str(),
                                                     c_ViewNodeIdName.toStdString().c_str(), "").c_str();

   if (c_ViewNodeName.compare("") != 0)
   {
      QVector<bool> c_ExpandedFlags;
      int32_t s32_SectionCounter;
      const int32_t s32_SectionCount = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                                           static_cast<QString>("%1SectionCount").
                                                           arg(orc_ViewNodeIdBase).toStdString().c_str(), 1);
      c_ExpandedFlags.resize(s32_SectionCount);

      //Section expanded flags
      for (s32_SectionCounter = 0; s32_SectionCounter < s32_SectionCount; ++s32_SectionCounter)
      {
         c_ExpandedFlags[s32_SectionCounter] =
            orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                             static_cast<QString>("%1Section%2").
                             arg(orc_ViewNodeIdBase).
                             arg(s32_SectionCounter).toStdString().c_str(), true);
      }

      // Append
      orc_UserSettings.SetProjSvUpdateSectionsExpandedFlags(orc_ViewName, c_ViewNodeName, c_ExpandedFlags);

      //Data rate
      C_UsFiler::mh_LoadDataRatesPerNode(orc_Ini, orc_SectionName, c_NodeIdUpdateDataRateBaseId, orc_ViewName,
                                         c_ViewNodeName,
                                         orc_UserSettings);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load view dashboard part of user settings

   \param[in,out]  orc_Ini                Ini handler
   \param[in]      orc_SectionName        Section name
   \param[in]      orc_DashboardIdBase    View dashboard id base name
   \param[in]      orc_ViewName           View name (not a copy paste error)
   \param[in,out]  orc_UserSettings       User settings to load
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadDashboard(C_SclIniFile & orc_Ini, const QString & orc_SectionName,
                                 const QString & orc_DashboardIdBase, const QString & orc_ViewName,
                                 C_UsHandler & orc_UserSettings)
{
   const QString c_DashboardIdName = static_cast<QString>("%1Name").arg(orc_DashboardIdBase);
   const QString c_DashboardName = orc_Ini.ReadString(
      orc_SectionName.toStdString().c_str(), c_DashboardIdName.toStdString().c_str(), "").c_str();

   if (c_DashboardName.compare("") != 0)
   {
      QPoint c_Pos;
      QSize c_Size;
      int32_t s32_Value;
      const QString c_DashboardIdTornOffFlag = static_cast<QString>("%1_torn_off_flag").arg(orc_DashboardIdBase);
      const QString c_DashboardIdScenePosHorizontal = static_cast<QString>("%1_scene_x").arg(orc_DashboardIdBase);
      const QString c_DashboardIdScenePosVertical = static_cast<QString>("%1_scene_y").arg(orc_DashboardIdBase);
      const QString c_DashboardIdSceneZoom = static_cast<QString>("%1_scene_zoom").arg(orc_DashboardIdBase);

      //Torn off flag
      if (orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                           c_DashboardIdTornOffFlag.toStdString().c_str(), false) == true)
      {
         const QString c_DashboardIdWindowPosHorizontal = static_cast<QString>("%1_window_x").arg(orc_DashboardIdBase);
         const QString c_DashboardIdWindowPosVertical = static_cast<QString>("%1_window_y").arg(orc_DashboardIdBase);
         const QString c_DashboardIdSizeWidth = static_cast<QString>("%1_width").arg(orc_DashboardIdBase);
         const QString c_DashboardIdSizeHeight = static_cast<QString>("%1_height").arg(orc_DashboardIdBase);
         const QString c_DashboardIdMinFlag = static_cast<QString>("%1_min_flag").arg(orc_DashboardIdBase);
         const QString c_DashboardIdMaxFlag = static_cast<QString>("%1_max_flag").arg(orc_DashboardIdBase);
         bool q_Min;
         bool q_Max;
         //Window pos
         c_Pos.setX(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                        c_DashboardIdWindowPosHorizontal.toStdString().c_str(), 0));
         c_Pos.setY(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                        c_DashboardIdWindowPosVertical.toStdString().c_str(), 0));
         //Size
         c_Size.setWidth(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                             c_DashboardIdSizeWidth.toStdString().c_str(), 0));
         c_Size.setHeight(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                              c_DashboardIdSizeHeight.toStdString().c_str(), 0));
         //Flags
         q_Min = orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                                  c_DashboardIdMinFlag.toStdString().c_str(), false);
         q_Max = orc_Ini.ReadBool(orc_SectionName.toStdString().c_str(),
                                  c_DashboardIdMaxFlag.toStdString().c_str(), false);

         //Apply
         orc_UserSettings.SetProjSvDashboardTearOffPosition(orc_ViewName, c_DashboardName, c_Pos, c_Size, q_Min,
                                                            q_Max);
      }
      else
      {
         orc_UserSettings.SetProjSvDashboardMainTab(orc_ViewName, c_DashboardName);
      }
      //Scene pos
      c_Pos.setX(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                     c_DashboardIdScenePosHorizontal.toStdString().c_str(), 0));
      c_Pos.setY(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                     c_DashboardIdScenePosVertical.toStdString().c_str(), 0));

      //Zoom
      s32_Value = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                      c_DashboardIdSceneZoom.toStdString().c_str(), 100);
      //Apply
      orc_UserSettings.SetProjSvDashboardScenePositionAndZoom(orc_ViewName, c_DashboardName, c_Pos, s32_Value);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load INI common section

   \param[in,out]  orc_UserSettings    User settings
   \param[in,out]  orc_Ini             Current ini
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadCommon(C_UsHandler & orc_UserSettings, C_SclIniFile & orc_Ini)
{
   QString c_Tmp;

   //Language
   c_Tmp = orc_Ini.ReadString("Common", "Language", "American english").c_str();
   if (C_UsHandler::h_CheckLanguageExists(c_Tmp) != 0)
   {
      c_Tmp = "American english";
   }
   orc_UserSettings.SetLanguage(c_Tmp);

   //Save As
   orc_UserSettings.SetCurrentSaveAsPath(orc_Ini.ReadString("Common", "SaveAsLocation", "").c_str());

   // Performance measurement
   orc_UserSettings.SetPerformanceActive(orc_Ini.ReadBool("Common", "PerformanceMeasurementActive", false));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load INI environment section (options of GeneralSettings dialog in MainWindow)

   \param[in,out]  orc_UserSettings    User settings
   \param[in,out]  orc_Ini             Current ini
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadEnvironment(C_UsHandler & orc_UserSettings, C_SclIniFile & orc_Ini)
{
   //Path handling
   orc_UserSettings.SetPathHandlingSelection(orc_Ini.ReadString("Environment", "PathHandlingSelection", "").c_str());

   //Skip TSP import
   orc_UserSettings.SetSkipTspSelection(orc_Ini.ReadString("Environment", "SkipTspImportSelection",
                                                           "").c_str());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load INI recent colors section

   \param[in,out]  orc_UserSettings    User settings
   \param[in,out]  orc_Ini             Current ini
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadColors(C_UsHandler & orc_UserSettings, C_SclIniFile & orc_Ini)
{
   QVector<QColor> c_RecentColorsVector;

   //Colors
   for (int32_t s32_Counter = 1; s32_Counter <= 6; s32_Counter++)
   {
      QColor c_Color;
      c_Color.setRed(orc_Ini.ReadInteger("RecentColors",
                                         "ColorNr" + C_SclString::IntToStr(s32_Counter) +
                                         "_Red", 255));
      c_Color.setGreen(orc_Ini.ReadInteger("RecentColors",
                                           "ColorNr" + C_SclString::IntToStr(s32_Counter) +
                                           "_Green", 255));
      c_Color.setBlue(orc_Ini.ReadInteger("RecentColors",
                                          "ColorNr" + C_SclString::IntToStr(s32_Counter) +
                                          "_Blue", 255));
      c_Color.setAlpha(orc_Ini.ReadInteger("RecentColors",
                                           "ColorNr" + C_SclString::IntToStr(s32_Counter) +
                                           "_Alpha", 255));
      c_RecentColorsVector.push_back(c_Color);
   }

   orc_UserSettings.SetRecentColors(c_RecentColorsVector);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load INI recent colors section

   \param[in,out]  orc_UserSettings    User settings
   \param[in,out]  orc_Ini             Current ini
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadNextRecentColorButtonNumber(C_UsHandler & orc_UserSettings, C_SclIniFile & orc_Ini)
{
   //Next recent color button
   orc_UserSettings.SetNextRecentColorButtonNumber(orc_Ini.ReadInteger("RecentColors",
                                                                       "NextRecentColorButtonNumber", 1));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load INI recent projects section

   \param[in,out]  orc_UserSettings    User settings
   \param[in,out]  orc_Ini             Current ini
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadRecentProjects(C_UsHandler & orc_UserSettings, C_SclIniFile & orc_Ini)
{
   QStringList c_List;
   QString c_Cur;

   //Recent projects
   c_List.clear();
   for (uint8_t u8_It = 0; u8_It < C_UsHandler::h_GetMaxRecentProjects(); ++u8_It)
   {
      c_Cur =
         orc_Ini.ReadString("RecentProjects", C_SclString::IntToStr(u8_It), "").c_str();
      if (c_Cur.compare("") != 0)
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
   orc_UserSettings.SetRecentProjects(c_List);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load INI project independent section

   \param[in,out]  orc_UserSettings    User settings
   \param[in,out]  orc_Ini             Current ini
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadProjectIndependentSection(C_UsHandler & orc_UserSettings, C_SclIniFile & orc_Ini)
{
   QPoint c_Pos;
   QSize c_Size;
   bool q_Flag;
   int32_t s32_Value;

   //Screen position
   c_Pos.setX(orc_Ini.ReadInteger("Screen", "Position_x", 50));
   c_Pos.setY(orc_Ini.ReadInteger("Screen", "Position_y", 50));
   orc_UserSettings.SetScreenPos(c_Pos);

   // Application size
   c_Size.setWidth(orc_Ini.ReadInteger("Screen", "Size_width", 1000));
   c_Size.setHeight(orc_Ini.ReadInteger("Screen", "Size_height", 700));
   orc_UserSettings.SetAppSize(c_Size);

   // Application maximizing flag
   q_Flag = orc_Ini.ReadBool("Screen", "Size_maximized", true);
   orc_UserSettings.SetAppMaximized(q_Flag);

   // Application screen index
   s32_Value = orc_Ini.ReadInteger("Screen", "Screen_index", 0);
   orc_UserSettings.SetAppScreenIndex(static_cast<uint32_t>(s32_Value));

   // Sys def topology toolbox position
   c_Pos.setX(orc_Ini.ReadInteger("SdTopologyToolbox", "Position_x", -1));
   c_Pos.setY(orc_Ini.ReadInteger("SdTopologyToolbox", "Position_y", -1));
   orc_UserSettings.SetSdTopologyToolboxPos(c_Pos);

   // Sys def topology toolbox size
   c_Size.setWidth(orc_Ini.ReadInteger("SdTopologyToolbox", "Size_width", 600));
   c_Size.setHeight(orc_Ini.ReadInteger("SdTopologyToolbox", "Size_height", 400));
   orc_UserSettings.SetSdTopologyToolboxSize(c_Size);

   // Sys def topology toolbox maximizing flag
   q_Flag = orc_Ini.ReadBool("SdTopologyToolbox", "Size_maximized", true);
   orc_UserSettings.SetSdTopologyToolboxMaximized(q_Flag);

   // Sys def node edit splitter (Default: 1242; Fits perfect when using full hd resolution)
   s32_Value = orc_Ini.ReadInteger("SdNodeEdit", "SplitterX", 1242);
   orc_UserSettings.SetSdNodeEditSplitterHorizontal(s32_Value);

   // Sys def node edit HALC splitter
   s32_Value = orc_Ini.ReadInteger("SdNodeEdit", "HalcSplitterX", 400);
   orc_UserSettings.SetSdNodeEditHalcSplitterHorizontal(s32_Value);

   // Sys def node edit CANopen Manager splitter
   s32_Value = orc_Ini.ReadInteger("SdNodeEdit", "CoManagerSplitterX", 400);
   orc_UserSettings.SetSdNodeEditCoManagerSplitterHorizontal(s32_Value);

   // Sys def node edit data logger splitter
   s32_Value = orc_Ini.ReadInteger("SdNodeEdit", "DataLoggerSplitterX", 400);
   orc_UserSettings.SetSdNodeEditDataLoggerSplitterHorizontal(s32_Value);

   // Sys def bus edit splitters
   s32_Value = orc_Ini.ReadInteger("SdBusEdit", "TreeSplitterX", 0);
   orc_UserSettings.SetSdBusEditTreeSplitterHorizontal(s32_Value);
   s32_Value = orc_Ini.ReadInteger("SdBusEdit", "TreeSplitterX2", 0);
   orc_UserSettings.SetSdBusEditTreeSplitterHorizontal2(s32_Value);
   s32_Value = orc_Ini.ReadInteger("SdBusEdit", "LayoutSplitterX", 0);
   orc_UserSettings.SetSdBusEditLayoutSplitterHorizontal(s32_Value);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load INI project independent section

   \param[in,out]  orc_UserSettings    User settings
   \param[in,out]  orc_Ini             Current ini
   \param[in]      orc_ActiveProject   Actual project to load project specific settings.
                                       Empty string results in default values
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadProjectDependentSection(C_UsHandler & orc_UserSettings, C_SclIniFile & orc_Ini,
                                               const QString & orc_ActiveProject)
{
   QPoint c_Pos;

   if (orc_ActiveProject != "")
   {
      // project specific settings
      int32_t s32_Value;
      int32_t s32_SystemBusCount;
      int32_t s32_SystemNodeCount;
      int32_t s32_SystemViewCount;
      int32_t s32_SysDefSubMode;
      uint32_t u32_SysDefIndex;
      uint32_t u32_SysDefFlag;
      int32_t s32_SysViewSubMode;
      uint32_t u32_SysViewIndex;
      uint32_t u32_SysViewFlag;
      QStringList c_PemFilePaths;

      // Mode
      s32_Value = orc_Ini.ReadInteger(orc_ActiveProject.toStdString().c_str(), "ProjMode", 0);
      orc_UserSettings.SetProjLastMode(s32_Value);

      // Navi bar
      s32_Value = orc_Ini.ReadInteger(orc_ActiveProject.toStdString().c_str(), "navigation-width", 300);
      orc_UserSettings.SetNaviBarSize(s32_Value);
      s32_Value = orc_Ini.ReadInteger(orc_ActiveProject.toStdString().c_str(), "navigation-node-section-width", 200);
      orc_UserSettings.SetNaviBarNodeSectionSize(s32_Value);

      // Sys def topology view port position
      c_Pos.setX(orc_Ini.ReadInteger(orc_ActiveProject.toStdString().c_str(), "SdTopologyView_x", 0));
      c_Pos.setY(orc_Ini.ReadInteger(orc_ActiveProject.toStdString().c_str(), "SdTopologyView_y", 0));
      orc_UserSettings.SetProjSdTopologyViewPos(c_Pos);

      // Sys def topology view zoom value
      s32_Value = orc_Ini.ReadInteger(orc_ActiveProject.toStdString().c_str(), "SdTopologyViewZoom_value", 100);
      orc_UserSettings.SetProjSdTopologyViewZoom(s32_Value);

      // Last screen mode
      s32_SysDefSubMode =
         static_cast<int32_t>(orc_Ini.ReadInteger(orc_ActiveProject.toStdString().c_str(), "ProjSdSubMode_value", 0));
      u32_SysDefIndex =
         static_cast<uint32_t>(orc_Ini.ReadInteger(orc_ActiveProject.toStdString().c_str(), "ProjSdSubIndex_value", 0));
      u32_SysDefFlag =
         static_cast<uint32_t>(orc_Ini.ReadInteger(orc_ActiveProject.toStdString().c_str(), "ProjSdSubFlag_value", 0));
      s32_SysViewSubMode =
         static_cast<int32_t>(orc_Ini.ReadInteger(orc_ActiveProject.toStdString().c_str(), "ProjSvSubMode_value", 0));
      u32_SysViewIndex =
         static_cast<uint32_t>(orc_Ini.ReadInteger(orc_ActiveProject.toStdString().c_str(), "ProjSvSubIndex_value", 0));
      u32_SysViewFlag =
         static_cast<uint32_t>(orc_Ini.ReadInteger(orc_ActiveProject.toStdString().c_str(), "ProjSvSubFlag_value", 0));

      orc_UserSettings.SetProjLastScreenMode(s32_SysDefSubMode, u32_SysDefIndex, u32_SysDefFlag,
                                             s32_SysViewSubMode, u32_SysViewIndex, u32_SysViewFlag);

      //TSP
      orc_UserSettings.SetProjSdTopologyLastKnownTspPath(
         orc_Ini.ReadString(
            orc_ActiveProject.toStdString().c_str(), "ProjSdTopology_last_known_tsp_path", "").c_str());

      //File generation
      orc_UserSettings.SetProjSdTopologyLastKnownCodeExportPath(
         orc_Ini.ReadString(
            orc_ActiveProject.toStdString().c_str(), "ProjSdTopology_last_known_code_export_path", "").c_str());

      //Import
      orc_UserSettings.SetProjSdTopologyLastKnownImportPath(
         orc_Ini.ReadString(
            orc_ActiveProject.toStdString().c_str(), "ProjSdTopology_last_known_import_path", "").c_str());

      //Import CANopen
      orc_UserSettings.SetProjSdTopologyLastKnownCanOpenEdsPath(
         orc_Ini.ReadString(
            orc_ActiveProject.toStdString().c_str(), "ProjSdTopology_last_known_CANopen_EDS_path", "").c_str());

      //Export
      orc_UserSettings.SetProjSdTopologyLastKnownExportPath(
         orc_Ini.ReadString(
            orc_ActiveProject.toStdString().c_str(), "ProjSdTopology_last_known_export_path", "").c_str());

      //Last path from where a .syde_devdef file was loaded
      orc_UserSettings.SetProjSdTopologyLastKnownDeviceDefPath(
         orc_Ini.ReadString(
            orc_ActiveProject.toStdString().c_str(), "ProjSdTopology_last_known_device_definition_path", "").c_str());

      //RTF File Export
      orc_UserSettings.SetProjSdTopologyLastKnownRtfPath(
         orc_Ini.ReadString(
            orc_ActiveProject.toStdString().c_str(), "ProjSdTopology_last_known_rtf_path", "").c_str());
      orc_UserSettings.SetProjSdTopologyLastKnownRtfCompanyName(
         orc_Ini.ReadString(
            orc_ActiveProject.toStdString().c_str(), "ProjSdTopology_last_known_rtf_company_name", "").c_str());
      orc_UserSettings.SetProjSdTopologyLastKnownRtfCompanyLogoPath(
         orc_Ini.ReadString(
            orc_ActiveProject.toStdString().c_str(), "ProjSdTopology_last_known_rtf_company_logo_path", "").c_str());

      //HALC Paths
      orc_UserSettings.SetLastKnownHalcDefPath(
         orc_Ini.ReadString(orc_ActiveProject.toStdString().c_str(), "ProjSd_last_known_halc_def_path", "").c_str());
      orc_UserSettings.SetLastKnownHalcImportPath(
         orc_Ini.ReadString(orc_ActiveProject.toStdString().c_str(), "ProjSd_last_known_halc_import_path", "").c_str());
      orc_UserSettings.SetLastKnownHalcExportPath(
         orc_Ini.ReadString(orc_ActiveProject.toStdString().c_str(), "ProjSd_last_known_halc_export_path", "").c_str());

      //Service Project Path
      orc_UserSettings.SetLastKnownServiceProjectPath(
         orc_Ini.ReadString(
            orc_ActiveProject.toStdString().c_str(), "ProjSd_last_known_service_project_path", "").c_str());

      //RAMView Project Import Path
      orc_UserSettings.SetLastKnownRamViewProjectPath(
         orc_Ini.ReadString(
            orc_ActiveProject.toStdString().c_str(), "ProjSd_last_known_ramview_project_path", "").c_str());

      // J1939 Catalog Path
      orc_UserSettings.SetLastKnownJ1939CatalogPath(orc_Ini.ReadString(
                                                       orc_ActiveProject.toStdString().c_str(),
                                                       "ProjSd_last_known_j1939_catalog_path", "").c_str());

      // Chart data as CSV Export
      orc_UserSettings.SetLastKnownCsvExportPath(orc_Ini.ReadString(
                                                    orc_ActiveProject.toStdString().c_str(),
                                                    "ProjSd_last_known_csv_export_path", "").c_str());

      // public PEM file path
      orc_UserSettings.SetLastKnownPublicPemFilePath(orc_Ini.ReadString(
                                                        orc_ActiveProject.toStdString().c_str(),
                                                        "ProjSd_last_known_public_pem_file_path", "").c_str());

      // Add PEM file path
      orc_UserSettings.SetLastKnownAddPemFilePath(orc_Ini.ReadString(
                                                     orc_ActiveProject.toStdString().c_str(),
                                                     "ProjSd_last_known_pem_file_path", "").c_str());

      // secure certificate package path
      orc_UserSettings.SetLastKnownSecureCertificatePackagePath(orc_Ini.ReadString(
                                                                   orc_ActiveProject.toStdString().c_str(),
                                                                   "ProjSd_last_known_secure_certificate_package_path",
                                                                   "").c_str());

      // PEM file password
      orc_UserSettings.SetLastKnownPemFilePassword(orc_Ini.ReadString(
                                                      orc_ActiveProject.toStdString().c_str(),
                                                      "ProjSd_last_known_pem_file_password", "").c_str());

      // Add PEM file state
      orc_UserSettings.SetLastKnownAddPemFileState(
         orc_Ini.ReadBool(orc_ActiveProject.toStdString().c_str(), "ProjSd_last_known_Add_pem_file_state", false));

      // secure update config state
      orc_UserSettings.SetLastKnownSecureUpdateConfigState(
         orc_Ini.ReadBool(
            orc_ActiveProject.toStdString().c_str(), "ProjSd_last_known_secure_update_config_state", false));

      // Last tab index in system definition
      s32_Value = orc_Ini.ReadInteger(orc_ActiveProject.toStdString().c_str(), "ProjSdNodeEditTabIndex_value", 0);
      orc_UserSettings.SetProjLastSysDefNodeTabIndex(s32_Value);
      s32_Value = orc_Ini.ReadInteger(orc_ActiveProject.toStdString().c_str(), "ProjSdBusEditTabIndex_value", 0);
      orc_UserSettings.SetProjLastSysDefBusTabIndex(s32_Value);

      //System nodes
      s32_SystemNodeCount = orc_Ini.ReadInteger(
         orc_ActiveProject.toStdString().c_str(), "ProjSdNode_count", 0);
      for (int32_t s32_ItNode = 0; s32_ItNode < s32_SystemNodeCount; ++s32_ItNode)
      {
         const QString c_NodeIdBase = static_cast<QString>("SdNode%1").arg(s32_ItNode);
         const QString c_NodeIdName = static_cast<QString>("%1Name").arg(c_NodeIdBase);
         const QString c_NodeName = orc_Ini.ReadString(
            orc_ActiveProject.toStdString().c_str(), c_NodeIdName.toStdString().c_str(), "").c_str();
         if (c_NodeName.compare("") != 0)
         {
            mh_LoadNode(orc_Ini, orc_ActiveProject, c_NodeIdBase, c_NodeName, orc_UserSettings);
         }
      }

      //System buses
      s32_SystemBusCount = orc_Ini.ReadInteger(
         orc_ActiveProject.toStdString().c_str(), "ProjSdBus_count", 0);
      for (int32_t s32_ItBus = 0; s32_ItBus < s32_SystemBusCount; ++s32_ItBus)
      {
         const QString c_BusIdBase = static_cast<QString>("SdBus%1").arg(s32_ItBus);
         const QString c_BusIdName = static_cast<QString>("%1Name").arg(c_BusIdBase);
         const QString c_BusName = orc_Ini.ReadString(
            orc_ActiveProject.toStdString().c_str(), c_BusIdName.toStdString().c_str(), "").c_str();
         if (c_BusName.compare("") != 0)
         {
            mh_LoadBus(orc_Ini, orc_ActiveProject, c_BusIdBase, c_BusName, orc_UserSettings, true, "", "");
         }
      }

      //System views
      s32_SystemViewCount = orc_Ini.ReadInteger(
         orc_ActiveProject.toStdString().c_str(), "ProjSvSetupView_count", 0);
      for (int32_t s32_ItView = 0; s32_ItView < s32_SystemViewCount; ++s32_ItView)
      {
         const QString c_ViewIdBase = static_cast<QString>("SvSetupView%1").arg(s32_ItView);
         const QString c_ViewIdName = static_cast<QString>("%1Name").arg(c_ViewIdBase);
         const QString c_ViewName = orc_Ini.ReadString(
            orc_ActiveProject.toStdString().c_str(), c_ViewIdName.toStdString().c_str(), "").c_str();
         if (c_ViewName.compare("") != 0)
         {
            mh_LoadView(orc_Ini, orc_ActiveProject, c_ViewIdBase, c_ViewName, orc_UserSettings);
         }
      }

      // Values from Update widget
      s32_Value = orc_Ini.ReadInteger("Update", "PemFileCount", 0);
      for (int32_t s32_SectionCounter = 0; s32_SectionCounter < s32_Value; ++s32_SectionCounter)
      {
         c_PemFilePaths.append(
            orc_Ini.ReadString("Update", "PemFiles_" + C_SclString::IntToStr(s32_SectionCounter), "").c_str());
      }
      orc_UserSettings.SetLastKnownUpdatePemFilePaths(c_PemFilePaths);
   }
   else
   {
      // Mode
      orc_UserSettings.SetProjLastMode(0); // default is SD (network topology)

      // Fill default values in case of new project
      orc_UserSettings.SetNaviBarSize(300);
      orc_UserSettings.SetNaviBarNodeSectionSize(200);

      // Sys def topology view port position
      c_Pos.setX(0);
      c_Pos.setY(0);
      orc_UserSettings.SetProjSdTopologyViewPos(c_Pos);

      // Sys def topology view zoom value
      orc_UserSettings.SetProjSdTopologyViewZoom(100);

      // Last screen mode
      orc_UserSettings.SetProjLastScreenMode(0, 0, 0, 0, 0, 0);

      //File generation
      orc_UserSettings.SetProjSdTopologyLastKnownCodeExportPath("");

      //Import
      orc_UserSettings.SetProjSdTopologyLastKnownImportPath("");

      //Import CANopen
      orc_UserSettings.SetProjSdTopologyLastKnownCanOpenEdsPath("");

      //Export
      orc_UserSettings.SetProjSdTopologyLastKnownExportPath("");

      //RTF File Export
      orc_UserSettings.SetProjSdTopologyLastKnownRtfPath("");
      orc_UserSettings.SetProjSdTopologyLastKnownRtfCompanyName("");
      orc_UserSettings.SetProjSdTopologyLastKnownRtfCompanyLogoPath("");

      // Last tab index in system definition
      orc_UserSettings.SetProjLastSysDefNodeTabIndex(0);
      orc_UserSettings.SetProjLastSysDefBusTabIndex(0);

      //System nodes, System buses, System views
      orc_UserSettings.ClearMaps();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load columns

   \param[in,out]  orc_Ini             Ini
   \param[in]      orc_SectionName     Section name
   \param[in]      orc_IdBase          Id base
   \param[in,out]  orc_ColumnWidths    Column widths
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadColumns(C_SclIniFile & orc_Ini, const QString & orc_SectionName, const QString & orc_IdBase,
                               std::vector<int32_t> & orc_ColumnWidths)
{
   const QString c_IdColumnCount = static_cast<QString>("%1Column_Count").arg(orc_IdBase);
   const int32_t s32_ColumnCount = orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                                       c_IdColumnCount.toStdString().c_str(), 0);

   orc_ColumnWidths.reserve(s32_ColumnCount);
   for (int32_t s32_ItCol = 0; s32_ItCol < s32_ColumnCount; ++s32_ItCol)
   {
      const QString c_IdColumn = static_cast<QString>("%1Column%2").arg(orc_IdBase).arg(s32_ItCol);
      orc_ColumnWidths.push_back(orc_Ini.ReadInteger(orc_SectionName.toStdString().c_str(),
                                                     c_IdColumn.toStdString().c_str(), 0));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load Screenshot GIF Play timer from user settings .ini

   \param[in,out]  orc_UserSettings    User settings
   \param[in,out]  orc_Ini             Current ini
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadScreenshotGifSucessTimeout(C_UsHandler & orc_UserSettings, C_SclIniFile & orc_Ini)
{
   orc_UserSettings.SetScreenshotGifSucessTimeout(orc_Ini.ReadInteger("Common", "ScreenshotGifSucessTimeout", 3000));
}
