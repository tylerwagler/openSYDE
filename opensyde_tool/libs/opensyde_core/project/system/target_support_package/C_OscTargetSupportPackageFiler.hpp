//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for C_OscTargetSupportPackage with Qt-native serialization
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCTARGETSUPPORTPACKAGEFILER_HPP
#define C_OSCTARGETSUPPORTPACKAGEFILER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscTargetSupportPackage.hpp"
#include <QDataStream>
#include <QJsonObject>
#include <QDomDocument>

/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_core {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_OscTargetSupportPackageFiler {
public:
   static int32_t h_LoadFile(C_OscTargetSupportPackage &orc_Config,
                             QString &orc_NodePath,
                             const QString &orc_Path);
   static int32_t h_SaveFile(const C_OscTargetSupportPackage &orc_Config,
                             const QString &orc_NodePath,
                             const QString &orc_Path);

   static int32_t h_LoadBinary(C_OscTargetSupportPackage &orc_Config,
                               QString &orc_NodePath,
                               const QString &orc_Path);
   static int32_t h_SaveBinary(const C_OscTargetSupportPackage &orc_Config,
                               const QString &orc_NodePath,
                               const QString &orc_Path);
   
   static int32_t h_LoadJson(C_OscTargetSupportPackage &orc_Config,
                             QString &orc_NodePath,
                             const QString &orc_Path);
   static int32_t h_SaveJson(const C_OscTargetSupportPackage &orc_Config,
                             const QString &orc_NodePath,
                             const QString &orc_Path);
   
   static int32_t h_LoadXml(C_OscTargetSupportPackage &orc_Config,
                            QString &orc_NodePath,
                            const QString &orc_Path);
   static int32_t h_SaveXml(const C_OscTargetSupportPackage &orc_Config,
                            const QString &orc_NodePath,
                            const QString &orc_Path);

private:
   C_OscTargetSupportPackageFiler();
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
