//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle save'n load for user settings (header)

   Handle save'n load for user settings (note: main module description should be in .cpp file)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_USFILER_HPP
#define C_USFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include <QSettings>

#include "C_UsHandler.hpp"
#include "stwtypes.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_UsFiler
{
public:
   static int32_t h_Save(const C_UsHandler & orc_UserSettings, const QString & orc_Path,
                         const QString & orc_ActiveProject);
   static int32_t h_Load(C_UsHandler & orc_UserSettings, const QString & orc_Path, const QString & orc_ActiveProject);

private:
   C_UsFiler(void);

   // Save half — QSettings-based. Each helper writes into the active QSettings group;
   // the caller is responsible for beginGroup/beginWriteArray bookkeeping.
   static void mh_SaveNode(QSettings & orc_Ini, const C_UsNode & orc_Node);
   static void mh_SaveBus(QSettings & orc_Ini, const C_UsCommunication & orc_Bus);
   static void mh_SaveDatapool(QSettings & orc_Ini, const C_UsNodeDatapool & orc_Datapool);
   static void mh_SaveList(QSettings & orc_Ini, const C_UsNodeDatapoolList & orc_List);
   static void mh_SaveView(QSettings & orc_Ini, const C_UsSystemView & orc_View);
   static void mh_SaveDataRatesPerNode(QSettings & orc_Ini, const C_UsSystemViewNode & orc_Node);
   static void mh_SaveViewNode(QSettings & orc_Ini, const C_UsSystemViewNode & orc_ViewNode);
   static void mh_SaveDashboard(QSettings & orc_Ini, const C_UsSystemViewDashboard & orc_Dashboard);
   static void mh_SaveCommon(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings);
   static void mh_SaveEnvironment(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings);
   static void mh_SaveColors(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings);
   static void mh_SaveNextRecentColorButtonNumber(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings);
   static void mh_SaveRecentProjects(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings);
   static void mh_SaveDeviceRoots(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings);
   static void mh_SaveProjectIndependentSection(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings);
   static void mh_SaveProjectDependentSection(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings,
                                              const QString & orc_ActiveProject);
   static void mh_SaveColumns(QSettings & orc_Ini, const QString & orc_ArrayName,
                              const std::vector<int32_t> & orc_ColumnWidths);

   // Load half — QSettings-based. Per-element helpers expect the QSettings to be positioned at the
   // corresponding array slot before the call (caller does setArrayIndex).
   static void mh_LoadNode(QSettings & orc_Ini, const QString & orc_NodeName, C_UsHandler & orc_UserSettings);
   static void mh_LoadBus(QSettings & orc_Ini, const QString & orc_BusName, C_UsHandler & orc_UserSettings,
                          const bool oq_IsBus, const QString & orc_NodeName, const QString & orc_DataPoolName);
   static void mh_LoadDatapool(QSettings & orc_Ini, const QString & orc_DatapoolName, const QString & orc_NodeName,
                               C_UsHandler & orc_UserSettings);
   static void mh_LoadList(QSettings & orc_Ini, const QString & orc_ListName, const QString & orc_NodeName,
                           const QString & orc_DataPoolName, C_UsHandler & orc_UserSettings);
   static void mh_LoadView(QSettings & orc_Ini, const QString & orc_ViewName, C_UsHandler & orc_UserSettings);
   static void mh_LoadDataRatesPerNode(QSettings & orc_Ini, const QString & orc_ViewName, const QString & orc_NodeName,
                                       C_UsHandler & orc_UserSettings);
   static void mh_LoadViewNode(QSettings & orc_Ini, const QString & orc_NodeName, const QString & orc_ViewName,
                               C_UsHandler & orc_UserSettings);
   static void mh_LoadDashboard(QSettings & orc_Ini, const QString & orc_DashboardName, const QString & orc_ViewName,
                                C_UsHandler & orc_UserSettings);
   static void mh_LoadCommon(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadEnvironment(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadColors(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadNextRecentColorButtonNumber(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadRecentProjects(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadDeviceRoots(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadProjectIndependentSection(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadProjectDependentSection(C_UsHandler & orc_UserSettings, QSettings & orc_Ini,
                                              const QString & orc_ActiveProject);

   static void mh_LoadColumns(QSettings & orc_Ini, const QString & orc_ArrayName,
                              std::vector<int32_t> & orc_ColumnWidths);
   static void mh_LoadScreenshotGifSucessTimeout(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
