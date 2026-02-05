//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Utility class to handle imports of KEFEX RAMView .def style projects

   \copyright   Copyright 2023 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDNDEDPIMPORTRAMVIE_HPP
#define C_SDNDEDPIMPORTRAMVIE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include <QStringList>

#include "stwtypes.hpp"
#include "C_OscNodeDataPool.hpp"
#include "C_PuiSdNodeDataPool.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdNdeDpImportRamView
{
public:
   static int32_t h_ImportDataPoolFromRamViewDefProject(const QString & orc_ProjectPath,
                                                        stw::opensyde_core::C_OscNodeDataPool & orc_DataPool,
                                                        C_PuiSdNodeDataPool & orc_GuiDataPool,
                                                        QStringList & orc_ImportInformation);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
