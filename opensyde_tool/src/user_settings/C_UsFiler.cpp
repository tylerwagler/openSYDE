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
               "DeviceRoots",
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
         mh_SaveDeviceRoots(c_Ini, orc_UserSettings);
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
         QSettings c_Ini(orc_Path, QSettings::IniFormat);
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
         mh_LoadDeviceRoots(orc_UserSettings, c_Ini);
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
                       QString::fromStdString(c_It->first.second.second.c_str()));
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
   const QMap<uint32_t, double> & rc_UpdateDataRateHistory = orc_Node.GetUpdateDataRateHistory();

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
/*! \brief   Save device-root search paths

   \param[in,out]  orc_Ini             Open QSettings instance
   \param[in]      orc_UserSettings    User settings
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_SaveDeviceRoots(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings)
{
   const QStringList c_List = orc_UserSettings.GetDeviceRootPaths();

   orc_Ini.remove("DeviceRoots");
   orc_Ini.beginGroup("DeviceRoots");
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
/*! \brief   Apply default values for project-dependent settings when no project is active or
             when the active project has no entry in the saved Projects array.

   \param[in,out]  orc_UserSettings    User settings to populate with defaults
*/
//----------------------------------------------------------------------------------------------------------------------
namespace
{
void mh_ApplyProjectDependentDefaults(stw::opensyde_gui_logic::C_UsHandler & orc_UserSettings)
{
   orc_UserSettings.SetProjLastMode(0); // default is SD (network topology)
   orc_UserSettings.SetNaviBarSize(300);
   orc_UserSettings.SetNaviBarNodeSectionSize(200);
   orc_UserSettings.SetProjSdTopologyViewPos(QPoint(0, 0));
   orc_UserSettings.SetProjSdTopologyViewZoom(100);
   orc_UserSettings.SetProjLastScreenMode(0, 0, 0, 0, 0, 0);
   orc_UserSettings.SetProjSdTopologyLastKnownCodeExportPath("");
   orc_UserSettings.SetProjSdTopologyLastKnownImportPath("");
   orc_UserSettings.SetProjSdTopologyLastKnownCanOpenEdsPath("");
   orc_UserSettings.SetProjSdTopologyLastKnownExportPath("");
   orc_UserSettings.SetProjSdTopologyLastKnownRtfPath("");
   orc_UserSettings.SetProjSdTopologyLastKnownRtfCompanyName("");
   orc_UserSettings.SetProjSdTopologyLastKnownRtfCompanyLogoPath("");
   orc_UserSettings.SetProjLastSysDefNodeTabIndex(0);
   orc_UserSettings.SetProjLastSysDefBusTabIndex(0);
   orc_UserSettings.ClearMaps();
}
} // namespace

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load node part of user settings.

   \param[in,out]  orc_Ini             Open QSettings positioned at the per-node array slot
   \param[in]      orc_NodeName        Node name (the "Name" value from the array slot)
   \param[in,out]  orc_UserSettings    User settings to load into
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadNode(QSettings & orc_Ini, const QString & orc_NodeName, C_UsHandler & orc_UserSettings)
{
   orc_UserSettings.SetProjSdNodeSelectedDatapoolName(orc_NodeName,
                                                      orc_Ini.value("SelectedDatapoolName", "").toString());
   orc_UserSettings.SetProjSdNodeSelectedProtocol(
      orc_NodeName, static_cast<stw::opensyde_core::C_OscCanProtocol::E_Type>(
         orc_Ini.value("SelectedProtocol", 0).toInt()));
   orc_UserSettings.SetProjSdNodeSelectedInterface(orc_NodeName,
                                                   static_cast<uint32_t>(orc_Ini.value("SelectedInterface", 0).toInt()));

   {
      std::vector<int32_t> c_Columns;
      mh_LoadColumns(orc_Ini, "CANopenOverviewColumns", c_Columns);
      orc_UserSettings.SetProjSdNodeCanOpenOverviewColumnWidth(orc_NodeName, c_Columns);
   }
   {
      std::vector<int32_t> c_Columns;
      mh_LoadColumns(orc_Ini, "CANopenPdoOverviewColumns", c_Columns);
      orc_UserSettings.SetProjSdNodeCanOpenPdoOverviewColumnWidth(orc_NodeName, c_Columns);
   }

   std::map<uint8_t, bool> c_LoadInterfaces;
   {
      const int32_t s32_Size = orc_Ini.beginReadArray("CANopenManagerExpanded");
      for (int32_t s32_It = 0; s32_It < s32_Size; ++s32_It)
      {
         orc_Ini.setArrayIndex(s32_It);
         c_LoadInterfaces[static_cast<uint8_t>(orc_Ini.value("InterfaceNumber", 0).toInt())] =
            orc_Ini.value("Expanded", false).toBool();
      }
      orc_Ini.endArray();
   }
   std::map<uint8_t, bool> c_LoadDevices;
   {
      const int32_t s32_Size = orc_Ini.beginReadArray("CANopenDevicesExpanded");
      for (int32_t s32_It = 0; s32_It < s32_Size; ++s32_It)
      {
         orc_Ini.setArrayIndex(s32_It);
         c_LoadDevices[static_cast<uint8_t>(orc_Ini.value("InterfaceNumber", 0).toInt())] =
            orc_Ini.value("Expanded", false).toBool();
      }
      orc_Ini.endArray();
   }
   std::map<std::pair<uint8_t, std::pair<uint8_t, std::string> >, bool> c_LoadDevice;
   {
      const int32_t s32_Size = orc_Ini.beginReadArray("CANopenDeviceExpanded");
      for (int32_t s32_It = 0; s32_It < s32_Size; ++s32_It)
      {
         orc_Ini.setArrayIndex(s32_It);
         const std::pair<uint8_t, std::string> c_PairInterfaceId(
            static_cast<uint8_t>(orc_Ini.value("DeviceInterfaceNumber", 0).toInt()),
            std::string(orc_Ini.value("DeviceNodeName", "").toString().toStdString().c_str()));
         const std::pair<uint8_t, std::pair<uint8_t, std::string> > c_Pair(
            static_cast<uint8_t>(orc_Ini.value("InterfaceNumber", 0).toInt()), c_PairInterfaceId);
         c_LoadDevice[c_Pair] = orc_Ini.value("Expanded", false).toBool();
      }
      orc_Ini.endArray();
   }
   orc_UserSettings.SetProjSdNodeExpandedCanOpenTree(orc_NodeName, c_LoadInterfaces, c_LoadDevices, c_LoadDevice);

   orc_UserSettings.SetProjSdNodeSelectedCanOpenManager(
      orc_NodeName, static_cast<uint8_t>(orc_Ini.value("SelectedCanOpenManager", 0).toInt()));
   orc_UserSettings.SetProjSdNodeSelectedCanOpenDevice(
      orc_NodeName,
      static_cast<uint8_t>(orc_Ini.value("SelectedCanOpenDeviceInterfaceNumber", 0).toInt()),
      orc_Ini.value("SelectedCanOpenDeviceNodeName", "").toString());
   orc_UserSettings.SetProjSdNodeSelectedCanOpenDeviceUseCaseIndex(
      orc_NodeName, static_cast<uint32_t>(orc_Ini.value("SelectedCanOpenDeviceUseCase", 0).toInt()));
   orc_UserSettings.SetProjSdNodeCanOpenSelectedUseCaseOrInterface(
      orc_NodeName, orc_Ini.value("CanOpenSelectedUseCaseOrInterface", false).toBool());

   orc_UserSettings.SetProjSdNodeSelectedHalcDomain(orc_NodeName,
                                                    orc_Ini.value("SelectedHalcDomain", "").toString());
   orc_UserSettings.SetProjSdNodeSelectedHalcChannel(orc_NodeName,
                                                     orc_Ini.value("SelectedHalcChannel", "").toString());

   {
      std::vector<int32_t> c_Columns;
      mh_LoadColumns(orc_Ini, "HalcOverviewColumns", c_Columns);
      orc_UserSettings.SetProjSdNodeHalcOverviewColumnWidth(orc_NodeName, c_Columns);
   }
   {
      std::vector<int32_t> c_Columns;
      mh_LoadColumns(orc_Ini, "HalcParamConfigColumns", c_Columns);
      orc_UserSettings.SetProjSdNodeHalcConfigColumnWidth(orc_NodeName, c_Columns);
   }

   const int32_t s32_DatapoolCount = orc_Ini.beginReadArray("Datapools");
   for (int32_t s32_It = 0; s32_It < s32_DatapoolCount; ++s32_It)
   {
      orc_Ini.setArrayIndex(s32_It);
      const QString c_DatapoolName = orc_Ini.value("Name", "").toString();
      if (c_DatapoolName.isEmpty() == false)
      {
         mh_LoadDatapool(orc_Ini, c_DatapoolName, orc_NodeName, orc_UserSettings);
      }
   }
   orc_Ini.endArray();

   orc_UserSettings.SetProjSdNodeSelectedDataLoggerLogJobIndex(
      orc_NodeName, orc_Ini.value("SelectedDataLoggerLogJobIndex", 0).toInt());
   orc_UserSettings.SetProjSdNodeIsOverviewWidgetSelected(
      orc_NodeName, orc_Ini.value("DataLoggerLogJobOverviewSelected", false).toBool());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load bus part of user settings.

   Used both for top-level project buses (oq_IsBus=true) and for per-datapool interface entries
   (oq_IsBus=false). The two cases call different setters on C_UsHandler.

   \param[in,out]  orc_Ini             Open QSettings positioned at the per-bus array slot
   \param[in]      orc_BusName         Bus name
   \param[in,out]  orc_UserSettings    User settings to load into
   \param[in]      oq_IsBus            Indicator if used on a project bus
   \param[in]      orc_NodeName        Node name (only used when oq_IsBus is false)
   \param[in]      orc_DataPoolName    Datapool name (only used when oq_IsBus is false)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadBus(QSettings & orc_Ini, const QString & orc_BusName, C_UsHandler & orc_UserSettings,
                           const bool oq_IsBus, const QString & orc_NodeName, const QString & orc_DataPoolName)
{
   std::vector<int32_t> c_MessageColumns;
   std::vector<int32_t> c_SignalColumns;

   mh_LoadColumns(orc_Ini, "MessageOverviewColumns", c_MessageColumns);
   mh_LoadColumns(orc_Ini, "SignalOverviewColumns", c_SignalColumns);

   const stw::opensyde_core::C_OscCanProtocol::E_Type e_SelectedProtocol =
      static_cast<stw::opensyde_core::C_OscCanProtocol::E_Type>(orc_Ini.value("SelectedComProtocol", 0).toInt());
   const bool q_MessageSelected = orc_Ini.value("MessageSelected", false).toBool();
   const QString c_MessageName = orc_Ini.value("SelectedMessageName", "").toString();
   const bool q_SignalSelected = orc_Ini.value("SignalSelected", false).toBool();
   const QString c_SignalName = orc_Ini.value("SelectedSignalName", "").toString();

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
/*! \brief   Load node datapool part of user settings.

   \param[in,out]  orc_Ini             Open QSettings positioned at the per-datapool array slot
   \param[in]      orc_DatapoolName    Datapool name
   \param[in]      orc_NodeName        Node name (parent)
   \param[in,out]  orc_UserSettings    User settings to load into
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadDatapool(QSettings & orc_Ini, const QString & orc_DatapoolName, const QString & orc_NodeName,
                                C_UsHandler & orc_UserSettings)
{
   std::vector<QString> c_ExpandedListNames;
   {
      const int32_t s32_Size = orc_Ini.beginReadArray("ExpandedListNames");
      c_ExpandedListNames.reserve(s32_Size);
      for (int32_t s32_It = 0; s32_It < s32_Size; ++s32_It)
      {
         orc_Ini.setArrayIndex(s32_It);
         const QString c_Name = orc_Ini.value("Name", "").toString();
         if (c_Name.isEmpty() == false)
         {
            c_ExpandedListNames.push_back(c_Name);
         }
      }
      orc_Ini.endArray();
   }
   orc_UserSettings.SetProjSdNodeDatapoolOpenListNames(orc_NodeName, orc_DatapoolName, c_ExpandedListNames);

   std::vector<QString> c_SelectedListNames;
   {
      const int32_t s32_Size = orc_Ini.beginReadArray("SelectedListNames");
      c_SelectedListNames.reserve(s32_Size);
      for (int32_t s32_It = 0; s32_It < s32_Size; ++s32_It)
      {
         orc_Ini.setArrayIndex(s32_It);
         const QString c_Name = orc_Ini.value("Name", "").toString();
         if (c_Name.isEmpty() == false)
         {
            c_SelectedListNames.push_back(c_Name);
         }
      }
      orc_Ini.endArray();
   }
   orc_UserSettings.SetProjSdNodeDatapoolSelectedListNames(orc_NodeName, orc_DatapoolName, c_SelectedListNames);

   std::vector<QString> c_SelectedVariableNames;
   {
      const int32_t s32_Size = orc_Ini.beginReadArray("SelectedVariableNames");
      c_SelectedVariableNames.reserve(s32_Size);
      for (int32_t s32_It = 0; s32_It < s32_Size; ++s32_It)
      {
         orc_Ini.setArrayIndex(s32_It);
         const QString c_Name = orc_Ini.value("Name", "").toString();
         if (c_Name.isEmpty() == false)
         {
            c_SelectedVariableNames.push_back(c_Name);
         }
      }
      orc_Ini.endArray();
   }
   orc_UserSettings.SetProjSdNodeDatapoolSelectedVariableNames(orc_NodeName, orc_DatapoolName, c_SelectedVariableNames);

   const int32_t s32_InterfaceCount = orc_Ini.beginReadArray("Interfaces");
   for (int32_t s32_It = 0; s32_It < s32_InterfaceCount; ++s32_It)
   {
      orc_Ini.setArrayIndex(s32_It);
      const QString c_BusName = orc_Ini.value("Name", "").toString();
      if (c_BusName.isEmpty() == false)
      {
         mh_LoadBus(orc_Ini, c_BusName, orc_UserSettings, false, orc_NodeName, orc_DatapoolName);
      }
   }
   orc_Ini.endArray();

   const int32_t s32_ListCount = orc_Ini.beginReadArray("Lists");
   for (int32_t s32_It = 0; s32_It < s32_ListCount; ++s32_It)
   {
      orc_Ini.setArrayIndex(s32_It);
      const QString c_ListName = orc_Ini.value("Name", "").toString();
      if (c_ListName.isEmpty() == false)
      {
         mh_LoadList(orc_Ini, c_ListName, orc_NodeName, orc_DatapoolName, orc_UserSettings);
      }
   }
   orc_Ini.endArray();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load node datapool list part of user settings.

   \param[in,out]  orc_Ini             Open QSettings positioned at the per-list array slot
   \param[in]      orc_ListName        List name
   \param[in]      orc_NodeName        Node name (grandparent)
   \param[in]      orc_DataPoolName    Datapool name (parent)
   \param[in,out]  orc_UserSettings    User settings to load into
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadList(QSettings & orc_Ini, const QString & orc_ListName, const QString & orc_NodeName,
                            const QString & orc_DataPoolName, C_UsHandler & orc_UserSettings)
{
   std::vector<int32_t> c_ColumnWidths;

   mh_LoadColumns(orc_Ini, "Columns", c_ColumnWidths);
   orc_UserSettings.SetProjSdNodeDatapoolListColumnSizes(orc_NodeName, orc_DataPoolName, orc_ListName, c_ColumnWidths);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load view part of user settings.

   \param[in,out]  orc_Ini             Open QSettings positioned at the per-view array slot
   \param[in]      orc_ViewName        View name
   \param[in,out]  orc_UserSettings    User settings to load into
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadView(QSettings & orc_Ini, const QString & orc_ViewName, C_UsHandler & orc_UserSettings)
{
   orc_UserSettings.SetProjSvNavigationExpandedStatus(orc_ViewName,
                                                      orc_Ini.value("NavigationExpandedStatus", false).toBool());

   orc_UserSettings.SetProjSvSetupViewZoom(orc_ViewName, orc_Ini.value("SetupZoom", 100).toInt());
   orc_UserSettings.SetProjSvSetupViewPos(orc_ViewName, orc_Ini.value("SetupPos", QPoint(0, 0)).toPoint());

   orc_UserSettings.SetProjSvUpdateViewZoom(orc_ViewName, orc_Ini.value("UpdateZoom", 100).toInt());
   orc_UserSettings.SetProjSvUpdateViewPos(orc_ViewName, orc_Ini.value("UpdatePos", QPoint(0, 0)).toPoint());

   orc_UserSettings.SetProjSvParamExport(orc_ViewName, orc_Ini.value("ParamExportPath", "").toString());
   orc_UserSettings.SetProjSvParamImport(orc_ViewName, orc_Ini.value("ParamImportPath", "").toString());
   orc_UserSettings.SetProjSvParamRecord(orc_ViewName, orc_Ini.value("ParamRecordPath", "").toString(),
                                         orc_Ini.value("ParamRecordFileName", "").toString());

   orc_UserSettings.SetProjSvUpdateSplitterHorizontal(orc_ViewName, orc_Ini.value("UpdateSplitterX", -1).toInt());
   orc_UserSettings.SetProjSvUpdateHorizontalSplitterVertical(orc_ViewName,
                                                              orc_Ini.value("UpdateHorizontalSplitterY", -1).toInt());

   orc_UserSettings.SetProjSvUpdateProgressLog(
      orc_ViewName,
      orc_Ini.value("UpdateProgressLogPos", QPoint(-1, -1)).toPoint(),
      orc_Ini.value("UpdateProgressLogSize", QSize(600, 400)).toSize(),
      orc_Ini.value("UpdateProgressLogIsMaximized", false).toBool());

   orc_UserSettings.SetProjSvUpdateSummaryBig(orc_ViewName, orc_Ini.value("UpdateSummaryBig", true).toBool());
   orc_UserSettings.SetProjSvUpdateEmptyOptionalSectionsVisible(
      orc_ViewName, orc_Ini.value("UpdateEmptyOptionalSectionsVisible", true).toBool());

   const int32_t s32_NodeCount = orc_Ini.beginReadArray("ViewNodes");
   for (int32_t s32_It = 0; s32_It < s32_NodeCount; ++s32_It)
   {
      orc_Ini.setArrayIndex(s32_It);
      const QString c_NodeName = orc_Ini.value("Name", "").toString();
      if (c_NodeName.isEmpty() == false)
      {
         mh_LoadViewNode(orc_Ini, c_NodeName, orc_ViewName, orc_UserSettings);
      }
   }
   orc_Ini.endArray();

   orc_UserSettings.SetProjSvDashboardToolbox(orc_ViewName,
                                              orc_Ini.value("DashboardToolboxPos", QPoint(-1, -1)).toPoint(),
                                              orc_Ini.value("DashboardToolboxSize", QSize(600, 400)).toSize(),
                                              orc_Ini.value("DashboardToolboxIsMaximized", false).toBool());

   orc_UserSettings.SetProjSvDashboardSelectedTabIndex(orc_ViewName,
                                                       orc_Ini.value("DashboardSelectedTabIndex", -1).toInt());

   const int32_t s32_DashboardCount = orc_Ini.beginReadArray("Dashboards");
   for (int32_t s32_It = 0; s32_It < s32_DashboardCount; ++s32_It)
   {
      orc_Ini.setArrayIndex(s32_It);
      const QString c_DashboardName = orc_Ini.value("Name", "").toString();
      if (c_DashboardName.isEmpty() == false)
      {
         mh_LoadDashboard(orc_Ini, c_DashboardName, orc_ViewName, orc_UserSettings);
      }
   }
   orc_Ini.endArray();

   std::array<bool, 3> c_ViewConfigs;
   c_ViewConfigs[0] = orc_Ini.value("SetupPermission", false).toBool();
   c_ViewConfigs[1] = orc_Ini.value("UpdatePermission", false).toBool();
   c_ViewConfigs[2] = orc_Ini.value("DashboardPermission", false).toBool();
   orc_UserSettings.SetViewPermission(orc_ViewName, c_ViewConfigs);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load view update data rates per node part of user settings.

   \param[in,out]  orc_Ini             Open QSettings positioned at the per-view-node array slot
   \param[in]      orc_ViewName        View name (parent)
   \param[in]      orc_NodeName        View node name
   \param[in,out]  orc_UserSettings    User settings to load into
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadDataRatesPerNode(QSettings & orc_Ini, const QString & orc_ViewName,
                                        const QString & orc_NodeName, C_UsHandler & orc_UserSettings)
{
   const int32_t s32_Size = orc_Ini.beginReadArray("UpdateDataRates");

   for (int32_t s32_It = 0; s32_It < s32_Size; ++s32_It)
   {
      orc_Ini.setArrayIndex(s32_It);
      const QString c_Checksum = orc_Ini.value("Checksum", "").toString();
      const double f64_Value = orc_Ini.value("Value", 0.0).toDouble();
      if (c_Checksum.isEmpty() == false)
      {
         bool q_Ok;
         const uint32_t u32_Checksum = c_Checksum.toULong(&q_Ok);
         if (q_Ok == true)
         {
            orc_UserSettings.AddProjSvNodeUpdateDataRate(orc_ViewName, orc_NodeName, u32_Checksum, f64_Value);
         }
      }
   }
   orc_Ini.endArray();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load view node part of user settings.

   \param[in,out]  orc_Ini             Open QSettings positioned at the per-view-node array slot
   \param[in]      orc_NodeName        View node name (the "Name" value from the array slot)
   \param[in]      orc_ViewName        View name (parent, not a copy-paste error)
   \param[in,out]  orc_UserSettings    User settings to load into
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadViewNode(QSettings & orc_Ini, const QString & orc_NodeName, const QString & orc_ViewName,
                                C_UsHandler & orc_UserSettings)
{
   QVector<bool> c_ExpandedFlags;
   const int32_t s32_SectionCount = orc_Ini.beginReadArray("SectionsExpanded");

   if (s32_SectionCount > 0)
   {
      c_ExpandedFlags.resize(s32_SectionCount);
      for (int32_t s32_It = 0; s32_It < s32_SectionCount; ++s32_It)
      {
         orc_Ini.setArrayIndex(s32_It);
         c_ExpandedFlags[s32_It] = orc_Ini.value("Expanded", true).toBool();
      }
   }
   else
   {
      // Match legacy behavior: missing array defaults to one expanded section.
      c_ExpandedFlags.resize(1);
      c_ExpandedFlags[0] = true;
   }
   orc_Ini.endArray();
   orc_UserSettings.SetProjSvUpdateSectionsExpandedFlags(orc_ViewName, orc_NodeName, c_ExpandedFlags);

   mh_LoadDataRatesPerNode(orc_Ini, orc_ViewName, orc_NodeName, orc_UserSettings);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load view dashboard part of user settings.

   \param[in,out]  orc_Ini             Open QSettings positioned at the per-dashboard array slot
   \param[in]      orc_DashboardName   Dashboard name (the "Name" value from the array slot)
   \param[in]      orc_ViewName        View name (parent, not a copy-paste error)
   \param[in,out]  orc_UserSettings    User settings to load into
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadDashboard(QSettings & orc_Ini, const QString & orc_DashboardName, const QString & orc_ViewName,
                                 C_UsHandler & orc_UserSettings)
{
   if (orc_Ini.value("TornOffFlag", false).toBool() == true)
   {
      orc_UserSettings.SetProjSvDashboardTearOffPosition(
         orc_ViewName, orc_DashboardName,
         orc_Ini.value("WindowPos", QPoint(0, 0)).toPoint(),
         orc_Ini.value("Size", QSize(0, 0)).toSize(),
         orc_Ini.value("MinFlag", false).toBool(),
         orc_Ini.value("MaxFlag", false).toBool());
   }
   else
   {
      orc_UserSettings.SetProjSvDashboardMainTab(orc_ViewName, orc_DashboardName);
   }

   orc_UserSettings.SetProjSvDashboardScenePositionAndZoom(orc_ViewName, orc_DashboardName,
                                                           orc_Ini.value("ScenePos", QPoint(0, 0)).toPoint(),
                                                           orc_Ini.value("SceneZoom", 100).toInt());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load common section of user settings.

   \param[in,out]  orc_UserSettings    User settings to load into
   \param[in,out]  orc_Ini             Open QSettings instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadCommon(C_UsHandler & orc_UserSettings, QSettings & orc_Ini)
{
   orc_Ini.beginGroup("Common");

   QString c_Lang = orc_Ini.value("Language", "American english").toString();
   if (C_UsHandler::h_CheckLanguageExists(c_Lang) != 0)
   {
      c_Lang = "American english";
   }
   orc_UserSettings.SetLanguage(c_Lang);

   orc_UserSettings.SetCurrentSaveAsPath(orc_Ini.value("SaveAsLocation", "").toString());
   orc_UserSettings.SetPerformanceActive(orc_Ini.value("PerformanceMeasurementActive", false).toBool());

   orc_Ini.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load environment section of user settings (options of GeneralSettings dialog).

   \param[in,out]  orc_UserSettings    User settings to load into
   \param[in,out]  orc_Ini             Open QSettings instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadEnvironment(C_UsHandler & orc_UserSettings, QSettings & orc_Ini)
{
   orc_Ini.beginGroup("Environment");
   orc_UserSettings.SetPathHandlingSelection(orc_Ini.value("PathHandlingSelection", "").toString());
   orc_UserSettings.SetSkipTspSelection(orc_Ini.value("SkipTspImportSelection", "").toString());
   orc_Ini.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load recent colors section of user settings.

   Loads up to 6 colors from the new-format Colors array. Missing entries are padded with white
   to preserve the legacy invariant of always exposing a 6-color palette.

   \param[in,out]  orc_UserSettings    User settings to load into
   \param[in,out]  orc_Ini             Open QSettings instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadColors(C_UsHandler & orc_UserSettings, QSettings & orc_Ini)
{
   QVector<QColor> c_RecentColors;

   orc_Ini.beginGroup("RecentColors");
   const int32_t s32_Size = orc_Ini.beginReadArray("Colors");
   for (int32_t s32_It = 0; s32_It < s32_Size; ++s32_It)
   {
      orc_Ini.setArrayIndex(s32_It);
      QColor c_Color;
      c_Color.setRed(orc_Ini.value("Red", 255).toInt());
      c_Color.setGreen(orc_Ini.value("Green", 255).toInt());
      c_Color.setBlue(orc_Ini.value("Blue", 255).toInt());
      c_Color.setAlpha(orc_Ini.value("Alpha", 255).toInt());
      c_RecentColors.push_back(c_Color);
   }
   orc_Ini.endArray();
   orc_Ini.endGroup();

   while (c_RecentColors.size() < 6)
   {
      c_RecentColors.push_back(QColor(255, 255, 255, 255));
   }

   orc_UserSettings.SetRecentColors(c_RecentColors);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load next recent color button number.

   \param[in,out]  orc_UserSettings    User settings to load into
   \param[in,out]  orc_Ini             Open QSettings instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadNextRecentColorButtonNumber(C_UsHandler & orc_UserSettings, QSettings & orc_Ini)
{
   orc_Ini.beginGroup("RecentColors");
   orc_UserSettings.SetNextRecentColorButtonNumber(orc_Ini.value("NextRecentColorButtonNumber", 1).toInt());
   orc_Ini.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load recent projects section of user settings.

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
/*! \brief   Load device-root search paths

   Falls back to the SetDefault-provided list if the section is absent or empty
   (i.e. on a fresh install).

   \param[in,out]  orc_UserSettings    User settings to load into
   \param[in,out]  orc_Ini             Open QSettings instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadDeviceRoots(C_UsHandler & orc_UserSettings, QSettings & orc_Ini)
{
   QStringList c_List;

   orc_Ini.beginGroup("DeviceRoots");
   const int32_t s32_Size = orc_Ini.beginReadArray("Items");
   for (int32_t s32_It = 0; s32_It < s32_Size; ++s32_It)
   {
      orc_Ini.setArrayIndex(s32_It);
      const QString c_Cur = orc_Ini.value("Path", "").toString();
      if (c_Cur.isEmpty() == false)
      {
         QFileInfo c_Dir;
         if (c_Cur.startsWith(".") == true)
         {
            c_Dir.setFile(C_Uti::h_GetExePath() + c_Cur);
         }
         else
         {
            c_Dir.setFile(c_Cur);
         }
         c_List.append(c_Dir.absoluteFilePath());
      }
   }
   orc_Ini.endArray();
   orc_Ini.endGroup();

   if (c_List.isEmpty() == false)
   {
      orc_UserSettings.SetDeviceRootPaths(c_List);
   }
   // else: keep the default set by SetDefault() — happens on fresh installs.
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load project-independent section of user settings.

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

   orc_Ini.beginGroup("SdTopologyToolbox");
   orc_UserSettings.SetSdTopologyToolboxPos(orc_Ini.value("Position", QPoint(-1, -1)).toPoint());
   orc_UserSettings.SetSdTopologyToolboxSize(orc_Ini.value("Size", QSize(600, 400)).toSize());
   orc_UserSettings.SetSdTopologyToolboxMaximized(orc_Ini.value("Maximized", true).toBool());
   orc_Ini.endGroup();

   orc_Ini.beginGroup("SdNodeEdit");
   orc_UserSettings.SetSdNodeEditSplitterHorizontal(orc_Ini.value("SplitterX", 1242).toInt());
   orc_UserSettings.SetSdNodeEditHalcSplitterHorizontal(orc_Ini.value("HalcSplitterX", 400).toInt());
   orc_UserSettings.SetSdNodeEditCoManagerSplitterHorizontal(orc_Ini.value("CoManagerSplitterX", 400).toInt());
   orc_UserSettings.SetSdNodeEditDataLoggerSplitterHorizontal(orc_Ini.value("DataLoggerSplitterX", 400).toInt());
   orc_Ini.endGroup();

   orc_Ini.beginGroup("SdBusEdit");
   orc_UserSettings.SetSdBusEditTreeSplitterHorizontal(orc_Ini.value("TreeSplitterX", 0).toInt());
   orc_UserSettings.SetSdBusEditTreeSplitterHorizontal2(orc_Ini.value("TreeSplitterX2", 0).toInt());
   orc_UserSettings.SetSdBusEditLayoutSplitterHorizontal(orc_Ini.value("LayoutSplitterX", 0).toInt());
   orc_Ini.endGroup();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load project-dependent section of user settings.

   Scans the Projects array for an entry whose Path matches orc_ActiveProject. If found, loads that
   slot's settings into the handler. If no project is active, or no matching entry exists, applies
   the project-dependent defaults.

   \param[in,out]  orc_UserSettings    User settings to load into
   \param[in,out]  orc_Ini             Open QSettings instance
   \param[in]      orc_ActiveProject   Active project; empty string skips project-specific load
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadProjectDependentSection(C_UsHandler & orc_UserSettings, QSettings & orc_Ini,
                                               const QString & orc_ActiveProject)
{
   if (orc_ActiveProject == "")
   {
      mh_ApplyProjectDependentDefaults(orc_UserSettings);
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
         mh_ApplyProjectDependentDefaults(orc_UserSettings);
      }
      else
      {
         orc_Ini.setArrayIndex(s32_ActiveIndex);

         orc_UserSettings.SetProjLastMode(orc_Ini.value("ProjMode", 0).toInt());
         orc_UserSettings.SetNaviBarSize(orc_Ini.value("navigation-width", 300).toInt());
         orc_UserSettings.SetNaviBarNodeSectionSize(orc_Ini.value("navigation-node-section-width", 200).toInt());

         orc_UserSettings.SetProjSdTopologyViewPos(orc_Ini.value("SdTopologyViewPos", QPoint(0, 0)).toPoint());
         orc_UserSettings.SetProjSdTopologyViewZoom(orc_Ini.value("SdTopologyViewZoom_value", 100).toInt());

         orc_UserSettings.SetProjLastScreenMode(
            orc_Ini.value("ProjSdSubMode_value", 0).toInt(),
            static_cast<uint32_t>(orc_Ini.value("ProjSdSubIndex_value", 0).toInt()),
            static_cast<uint32_t>(orc_Ini.value("ProjSdSubFlag_value", 0).toInt()),
            orc_Ini.value("ProjSvSubMode_value", 0).toInt(),
            static_cast<uint32_t>(orc_Ini.value("ProjSvSubIndex_value", 0).toInt()),
            static_cast<uint32_t>(orc_Ini.value("ProjSvSubFlag_value", 0).toInt()));

         orc_UserSettings.SetProjSdTopologyLastKnownTspPath(
            orc_Ini.value("ProjSdTopology_last_known_tsp_path", "").toString());
         orc_UserSettings.SetProjSdTopologyLastKnownCodeExportPath(
            orc_Ini.value("ProjSdTopology_last_known_code_export_path", "").toString());
         orc_UserSettings.SetProjSdTopologyLastKnownImportPath(
            orc_Ini.value("ProjSdTopology_last_known_import_path", "").toString());
         orc_UserSettings.SetProjSdTopologyLastKnownCanOpenEdsPath(
            orc_Ini.value("ProjSdTopology_last_known_CANopen_EDS_path", "").toString());
         orc_UserSettings.SetProjSdTopologyLastKnownExportPath(
            orc_Ini.value("ProjSdTopology_last_known_export_path", "").toString());
         orc_UserSettings.SetProjSdTopologyLastKnownDeviceDefPath(
            orc_Ini.value("ProjSdTopology_last_known_device_definition_path", "").toString());
         orc_UserSettings.SetProjSdTopologyLastKnownRtfPath(
            orc_Ini.value("ProjSdTopology_last_known_rtf_path", "").toString());
         orc_UserSettings.SetProjSdTopologyLastKnownRtfCompanyName(
            orc_Ini.value("ProjSdTopology_last_known_rtf_company_name", "").toString());
         orc_UserSettings.SetProjSdTopologyLastKnownRtfCompanyLogoPath(
            orc_Ini.value("ProjSdTopology_last_known_rtf_company_logo_path", "").toString());

         orc_UserSettings.SetLastKnownHalcDefPath(
            orc_Ini.value("ProjSd_last_known_halc_def_path", "").toString());
         orc_UserSettings.SetLastKnownHalcImportPath(
            orc_Ini.value("ProjSd_last_known_halc_import_path", "").toString());
         orc_UserSettings.SetLastKnownHalcExportPath(
            orc_Ini.value("ProjSd_last_known_halc_export_path", "").toString());
         orc_UserSettings.SetLastKnownServiceProjectPath(
            orc_Ini.value("ProjSd_last_known_service_project_path", "").toString());
         orc_UserSettings.SetLastKnownJ1939CatalogPath(
            orc_Ini.value("ProjSd_last_known_j1939_catalog_path", "").toString());
         orc_UserSettings.SetLastKnownCsvExportPath(
            orc_Ini.value("ProjSd_last_known_csv_export_path", "").toString());

         orc_UserSettings.SetLastKnownPublicPemFilePath(
            orc_Ini.value("ProjSd_last_known_public_pem_file_path", "").toString());
         orc_UserSettings.SetLastKnownAddPemFilePath(
            orc_Ini.value("ProjSd_last_known_pem_file_path", "").toString());
         orc_UserSettings.SetLastKnownSecureCertificatePackagePath(
            orc_Ini.value("ProjSd_last_known_secure_certificate_package_path", "").toString());
         orc_UserSettings.SetLastKnownPemFilePassword(
            orc_Ini.value("ProjSd_last_known_pem_file_password", "").toString());
         orc_UserSettings.SetLastKnownAddPemFileState(
            orc_Ini.value("ProjSd_last_known_Add_pem_file_state", false).toBool());
         orc_UserSettings.SetLastKnownSecureUpdateConfigState(
            orc_Ini.value("ProjSd_last_known_secure_update_config_state", false).toBool());

         orc_UserSettings.SetProjLastSysDefNodeTabIndex(orc_Ini.value("ProjSdNodeEditTabIndex_value", 0).toInt());
         orc_UserSettings.SetProjLastSysDefBusTabIndex(orc_Ini.value("ProjSdBusEditTabIndex_value", 0).toInt());

         const int32_t s32_NodeCount = orc_Ini.beginReadArray("SdNodes");
         for (int32_t s32_It = 0; s32_It < s32_NodeCount; ++s32_It)
         {
            orc_Ini.setArrayIndex(s32_It);
            const QString c_NodeName = orc_Ini.value("Name", "").toString();
            if (c_NodeName.isEmpty() == false)
            {
               mh_LoadNode(orc_Ini, c_NodeName, orc_UserSettings);
            }
         }
         orc_Ini.endArray();

         const int32_t s32_BusCount = orc_Ini.beginReadArray("SdBuses");
         for (int32_t s32_It = 0; s32_It < s32_BusCount; ++s32_It)
         {
            orc_Ini.setArrayIndex(s32_It);
            const QString c_BusName = orc_Ini.value("Name", "").toString();
            if (c_BusName.isEmpty() == false)
            {
               mh_LoadBus(orc_Ini, c_BusName, orc_UserSettings, true, "", "");
            }
         }
         orc_Ini.endArray();

         const int32_t s32_ViewCount = orc_Ini.beginReadArray("SvSetupViews");
         for (int32_t s32_It = 0; s32_It < s32_ViewCount; ++s32_It)
         {
            orc_Ini.setArrayIndex(s32_It);
            const QString c_ViewName = orc_Ini.value("Name", "").toString();
            if (c_ViewName.isEmpty() == false)
            {
               mh_LoadView(orc_Ini, c_ViewName, orc_UserSettings);
            }
         }
         orc_Ini.endArray();

         orc_Ini.endArray(); // ends "Projects"

         // Update widget settings live in their own top-level section, not under the project group.
         QStringList c_PemFilePaths;
         orc_Ini.beginGroup("Update");
         const int32_t s32_PemSize = orc_Ini.beginReadArray("PemFiles");
         for (int32_t s32_It = 0; s32_It < s32_PemSize; ++s32_It)
         {
            orc_Ini.setArrayIndex(s32_It);
            c_PemFilePaths.append(orc_Ini.value("Path", "").toString());
         }
         orc_Ini.endArray();
         orc_Ini.endGroup();
         orc_UserSettings.SetLastKnownUpdatePemFilePaths(c_PemFilePaths);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load column widths from a QSettings array.

   \param[in,out]  orc_Ini            Open QSettings instance (positioned at any group)
   \param[in]      orc_ArrayName      Array key name
   \param[in,out]  orc_ColumnWidths   Column widths (cleared and refilled)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadColumns(QSettings & orc_Ini, const QString & orc_ArrayName,
                               std::vector<int32_t> & orc_ColumnWidths)
{
   const int32_t s32_Size = orc_Ini.beginReadArray(orc_ArrayName);

   orc_ColumnWidths.reserve(s32_Size);
   for (int32_t s32_It = 0; s32_It < s32_Size; ++s32_It)
   {
      orc_Ini.setArrayIndex(s32_It);
      orc_ColumnWidths.push_back(orc_Ini.value("Width", 0).toInt());
   }
   orc_Ini.endArray();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load Screenshot GIF play timer from user settings.

   The corresponding save is intentionally absent — the value is load-only with a hardcoded default.

   \param[in,out]  orc_UserSettings    User settings to load into
   \param[in,out]  orc_Ini             Open QSettings instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadScreenshotGifSucessTimeout(C_UsHandler & orc_UserSettings, QSettings & orc_Ini)
{
   orc_Ini.beginGroup("Common");
   orc_UserSettings.SetScreenshotGifSucessTimeout(orc_Ini.value("ScreenshotGifSucessTimeout", 3000).toInt());
   orc_Ini.endGroup();
}
