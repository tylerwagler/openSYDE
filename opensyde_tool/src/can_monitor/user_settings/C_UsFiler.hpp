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

   static void mh_SaveRecentProjects(const C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_SaveProjectIndependentSection(const C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_SaveProjectDependentSection(const C_UsHandler & orc_UserSettings, QSettings & orc_Ini,
                                              const QString & orc_ActiveProject);
   static void mh_LoadRecentProjects(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadProjectIndependentSection(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
   static void mh_LoadProjectDependentSection(C_UsHandler & orc_UserSettings, QSettings & orc_Ini,
                                              const QString & orc_ActiveProject);
   static void mh_SaveColumns(QSettings & orc_Ini, const QString & orc_SectionName,
                              const QString & orc_IdentifierBaseName, const QList<int32_t> & orc_Columns);
   static void mh_LoadColumns(QSettings & orc_Ini, const QString & orc_SectionName,
                              const QString & orc_IdentifierBaseName, QList<int32_t> & orc_Columns);

   static const QString mhc_TRACE_COL_WIDTH_IDENTIFIER;
   static const QString mhc_TRACE_COL_POSITION_IDENTIFIER;
   static const QString mhc_MESSAGE_GENERATOR_IDENTIFIER;
   static const QString mhc_MESSAGE_GENERATOR_SIGNALS_IDENTIFIER;
   static const QString mhc_MESSAGE_GENERATOR_SELECTION_IDENTIFIER;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
