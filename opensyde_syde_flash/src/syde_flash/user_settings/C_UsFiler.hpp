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
   static int32_t h_Save(const C_UsHandler & orc_UserSettings, const QString & orc_Path);
   static int32_t h_Load(C_UsHandler & orc_UserSettings, const QString & orc_Path);

private:
   C_UsFiler(void);

   static void mh_SaveProjectIndependentSection(QSettings & orc_Ini, const C_UsHandler & orc_UserSettings);
   static void mh_LoadProjectIndependentSection(C_UsHandler & orc_UserSettings, QSettings & orc_Ini);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
