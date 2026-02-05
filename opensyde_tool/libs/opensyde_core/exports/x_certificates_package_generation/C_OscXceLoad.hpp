//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: X-Certificates Package loading
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXCELOAD_HPP
#define C_OSCXCELOAD_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscXceBase.hpp"
#include "C_OscXceManifest.hpp"
#include <QStringList>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscXceLoad :
   public C_OscXceBase
{
public:
   static int32_t h_ProcessPackage(const QString & orc_PackagePath,
                                   const QString & orc_TargetUnzipPath, C_OscXceManifest & orc_Manifest,
                                   QStringList & orc_WarningMessages,
                                   QString & orc_ErrorMessage);

private:
   static const QString mhc_USE_CASE;

   static int32_t mh_CheckParamsToProcessPackage(const QString & orc_PackagePath,
                                                 const QString & orc_TargetUnzipPath);
   static int32_t mh_CheckXcertFiles(const QString & orc_PackagePath);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
