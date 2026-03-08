//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Bus reader/writer (Multi-Format)

   Load / save bus data from / to binary, JSON, or XML files using the
   Qt-native serialization framework.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSYSTEMBUSFILER_NEW_HPP
#define C_OSCSYSTEMBUSFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscSystemBus.hpp"
#include "C_OscFilerHelper.hpp"
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

class C_OscSystemBusFiler_New {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadFile(C_OscSystemBus &orc_Bus, const QString &orc_Path);
   static int32_t h_SaveFile(const C_OscSystemBus &orc_Bus, const QString &orc_Path);

   // --------------------------------------------------------------------------
   // Binary Format (Fastest, compact)
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(C_OscSystemBus &orc_Bus, const QString &orc_Path);
   static int32_t h_SaveBinary(const C_OscSystemBus &orc_Bus, const QString &orc_Path);
   static int32_t h_LoadFromMemoryBinary(C_OscSystemBus &orc_Bus, const QByteArray &orc_Data);
   static QByteArray h_SaveToMemoryBinary(const C_OscSystemBus &orc_Bus);

   // --------------------------------------------------------------------------
   // JSON Format (Human-readable, debugging)
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(C_OscSystemBus &orc_Bus, const QString &orc_Path);
   static int32_t h_SaveJson(const C_OscSystemBus &orc_Bus, const QString &orc_Path);
   static int32_t h_LoadFromMemoryJson(C_OscSystemBus &orc_Bus, const QJsonObject &orc_Object);
   static QJsonObject h_SaveToMemoryJson(const C_OscSystemBus &orc_Bus);

   // --------------------------------------------------------------------------
   // XML Format (Legacy compatibility)
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(C_OscSystemBus &orc_Bus, const QString &orc_Path);
   static int32_t h_SaveXml(const C_OscSystemBus &orc_Bus, const QString &orc_Path);
   static int32_t h_LoadFromMemoryXml(C_OscSystemBus &orc_Bus, const QDomElement &orc_Element);
   static QDomElement h_SaveToMemoryXml(const C_OscSystemBus &orc_Bus, QDomDocument &orc_Doc);

   // --------------------------------------------------------------------------
   // Legacy Compatibility (deprecated - for backward compatibility only)
   // --------------------------------------------------------------------------
   [[deprecated("Use h_LoadFile/h_SaveFile with format detection, or format-specific methods")]]
   static int32_t h_LoadBus(C_OscSystemBus &orc_Bus, C_OscXmlParserBase &orc_XmlParser);
   [[deprecated("Use h_LoadFile/h_SaveFile with format detection, or format-specific methods")]]
   static void h_SaveBus(const C_OscSystemBus &orc_Bus, C_OscXmlParserBase &orc_XmlParser);

private:
   // Helper for format detection
   static int32_t mh_DetectAndLoad(C_OscSystemBus &orc_Bus, const QString &orc_Path);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
