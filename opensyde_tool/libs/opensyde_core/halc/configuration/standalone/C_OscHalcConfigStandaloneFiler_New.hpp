//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       HALC configuration standalone reader/writer (Multi-Format)

   Load / save HALC configuration standalone data from / to binary, JSON, or XML files
   using the Qt-native serialization framework.

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCCONFIGSTANDALONEFILER_NEW_HPP
#define C_OSCHALCCONFIGSTANDALONEFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscHalcConfigStandalone.hpp"
#include "C_OscFilerUtil.hpp"
#include "stwtypes.hpp"

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

class C_OscHalcConfigStandaloneFiler_New {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadFile(C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path);
   static int32_t h_SaveFile(const C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path);

   // --------------------------------------------------------------------------
   // Binary Format (Fastest, compact)
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path);
   static int32_t h_SaveBinary(const C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path);
   static int32_t h_LoadFromMemoryBinary(C_OscHalcConfigStandalone &orc_Config, const QByteArray &orc_Data);
   static QByteArray h_SaveToMemoryBinary(const C_OscHalcConfigStandalone &orc_Config);

   // --------------------------------------------------------------------------
   // JSON Format (Human-readable, debugging)
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path);
   static int32_t h_SaveJson(const C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path);
   static int32_t h_LoadFromMemoryJson(C_OscHalcConfigStandalone &orc_Config, const QJsonObject &orc_Object);
   static QJsonObject h_SaveToMemoryJson(const C_OscHalcConfigStandalone &orc_Config);

   // --------------------------------------------------------------------------
   // XML Format (Legacy compatibility)
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path);
   static int32_t h_SaveXml(const C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path);
   static int32_t h_LoadFromMemoryXml(C_OscHalcConfigStandalone &orc_Config, const QDomElement &orc_Element);
   static QDomElement h_SaveToMemoryXml(const C_OscHalcConfigStandalone &orc_Config, QDomDocument &orc_Doc);

private:
   static int32_t mh_DetectAndLoad(C_OscHalcConfigStandalone &orc_Config, const QString &orc_Path);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
