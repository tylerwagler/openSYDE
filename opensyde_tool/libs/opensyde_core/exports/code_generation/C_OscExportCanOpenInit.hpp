//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Export initialization module for CANopen

   See cpp file for detailed description

   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCEXPORTCANOPENINIT_HPP
#define C_OSCEXPORTCANOPENINIT_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscNode.hpp"
/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscExportCanOpenInit
{
public:
   static QString h_GetFileName(void);
   static int32_t h_CreateSourceCode(const QString & orc_FilePath, const C_OscNode & orc_Node,
                                     const QByteArray & orc_IfWithCanOpenMan,
                                     const QString & orc_ExportToolInfo = "");

protected:
   static void mh_ComposeDefineNumTotal(QString & orc_DefineValue,
                                        const QByteArray & orc_IfWithCanOpenMan, const bool oq_IsTx,
                                        const QString & orc_Subject);
   static void mh_ComposeDefineNumDevices(QString & orc_DefineValue,
                                          const QByteArray & orc_IfWithCanOpenMan);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
