//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: X-Certificates Package base
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXCEBASE_HPP
#define C_OSCXCEBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include <QString>
#include <QStringList>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscXceBase
{
public:
   static QString h_GetPackageExtension();

   static const QString hc_PACKAGE_EXT;
   static const QString hc_PACKAGE_EXT_TMP; // intermediate directory before creating zip archive

protected:
   static QStringList mhc_WarningMessages; // global warnings e.g. if update position of active node is
                                           // not available
   static QString mhc_ErrorMessage;        // description of error which caused the service update package
                                                         // to fail
   static const QString mhc_CERTIFICATES_FOLDER;
   static const QString mhc_UPDATE_PACKAGE_PARAMETERS_FOLDER;

   static void mh_Init(void);
   static void mh_GetWarningsAndErrors(QStringList & orc_WarningMessages,
                                       QString & orc_ErrorMessage);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
