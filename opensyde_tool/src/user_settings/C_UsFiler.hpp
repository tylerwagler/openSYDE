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

#include "C_UsHandler.hpp"
#include "stwtypes.hpp"
#include <QSettings>
#include <QList>

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

   static void mh_SaveNode(QSettings & orc_Ini, const QString & orc_SectionName,
                           const QString & orc_NodeIdBase, const QString & orc_NodeName, const C_UsNode & orc_Node);
   static void mh_SaveBus(QSettings & orc_Ini, const QString & orc_SectionName,
                          const QString & orc_BusIdBase, const QString & orc_BusName,
                          const C_UsCommunication & orc_Bus);
   static void mh_SaveDatapool(QSettings & orc_Ini, const QString & orc_SectionName,
                               const QString & orc_DatapoolIdBase, const QString & orc_DatapoolName,
                               const C_UsNodeDatapool & orc_Datapool);
   static void mh_SaveList(QSettings & orc_Ini, const QString & orc_SectionName,
                           const QString & orc_ListIdBase, const QString & orc_ListName,
                           const C_UsNodeDatapoolList & orc_List);
   static void mh_SaveView(QSettings & orc_Ini, const QString & orc_SectionName,
                           const QString & orc_ViewIdBase, const QString & orc_ViewName,
                           const C_UsSystemView & orc_View);
   static void mh_SaveDataRatesPerNode(QSettings & orc_Ini, const QString & orc_SectionName,
                                       const QString & orc_DataRatePerNodeIdBase, const C_UsSystemViewNode & orc_Node);
   static void mh_SaveViewNode(QSettings & orc_Ini, const QString & orc_SectionName,
                               const QString & orc_ViewNodeIdBase, const QString & orc_NodeName,
                               const C_UsSystemViewNode & orc_ViewNode);
   static void mh_SaveDashboard(QSettings & orc_Ini, const QString & orc_SectionName,
                                const QString & orc_DashboardIdBase, const QString & orc_DashboardName,
                                const C_UsSystemViewDashboard & orc_Dashboard);
   static void mh_SaveCommon(const C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_SaveEnvironment(const C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_SaveColors(const C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_SaveNextRecentColorButtonNumber(const C_UsHandler & orc_UserSettings,
                                                  QSettings & orc_Ini);
   static void mh_SaveRecentProjects(const C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_SaveProjectIndependentSection(const C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_SaveProjectDependentSection(const C_UsHandler & orc_UserSettings, QSettings & orc_Ini,
                                              const QString & orc_ActiveProject);
   static void mh_LoadNode(QSettings & orc_Ini, const QString & orc_SectionName,
                           const QString & orc_NodeIdBase, const QString & orc_NodeName,
                           C_UsHandler & orc_UserSettings);
   static void mh_LoadBus(QSettings & orc_Ini, const QString & orc_SectionName,
                          const QString & orc_BusIdBase, const QString & orc_BusName, C_UsHandler & orc_UserSettings,
                          const bool oq_IsBus, const QString & orc_NodeName, const QString & orc_DataPoolName);
   static void mh_LoadDatapool(QSettings & orc_Ini, const QString & orc_SectionName,
                               const QString & orc_DatapoolIdBase, const QString & orc_NodeName,
                               C_UsHandler & orc_UserSettings);
   static void mh_LoadList(QSettings & orc_Ini, const QString & orc_SectionName,
                           const QString & orc_ListIdBase, const QString & orc_NodeName,
                           const QString & orc_DataPoolName, C_UsHandler & orc_UserSettings);
   static void mh_LoadView(QSettings & orc_Ini, const QString & orc_SectionName,
                           const QString & orc_ViewIdBase, const QString & orc_ViewName,
                           C_UsHandler & orc_UserSettings);
   static void mh_LoadDataRatesPerNode(QSettings & orc_Ini, const QString & orc_SectionName,
                                       const QString & orc_DataRatePerNodeIdBase, const QString & orc_ViewName,
                                       const QString & orc_NodeName, C_UsHandler & orc_UserSettings);
   static void mh_LoadViewNode(QSettings & orc_Ini, const QString & orc_SectionName,
                               const QString & orc_ViewNodeIdBase, const QString & orc_ViewName,
                               C_UsHandler & orc_UserSettings);
   static void mh_LoadDashboard(QSettings & orc_Ini, const QString & orc_SectionName,
                                const QString & orc_DashboardIdBase, const QString & orc_ViewName,
                                C_UsHandler & orc_UserSettings);
   static void mh_LoadCommon(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadEnvironment(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadColors(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadNextRecentColorButtonNumber(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadRecentProjects(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadProjectIndependentSection(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadProjectDependentSection(C_UsHandler & orc_UserSettings, QSettings & orc_Ini,
                                              const QString & orc_ActiveProject);

   static void mh_SaveColumns(QSettings & orc_Ini, const QString & orc_SectionName,
                              const QString & orc_IdBase, const QList<int32_t> & orc_ColumnWidths);
   static void mh_LoadColumns(QSettings & orc_Ini, const QString & orc_SectionName,
                              const QString & orc_IdBase, QList<int32_t> & orc_ColumnWidths);
   static void mh_LoadScreenshotGifSucessTimeout(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
