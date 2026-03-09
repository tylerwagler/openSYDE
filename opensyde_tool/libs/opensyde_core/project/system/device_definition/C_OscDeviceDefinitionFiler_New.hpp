//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for device definition files (Multi-Format)

   Load / save device definition data from / to binary, JSON, or XML
   files using the Qt-native serialization framework.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDEVICEDEFINITIONFILER_NEW_HPP
#define C_OSCDEVICEDEFINITIONFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscDeviceDefinition.hpp"
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

class C_OscDeviceDefinitionFiler_New {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadDeviceDefinitionFile(C_OscDeviceDefinition &orc_DeviceDefinition,
                                             const QString &orc_FilePath);
   static int32_t h_SaveDeviceDefinitionFile(const C_OscDeviceDefinition &orc_DeviceDefinition,
                                             const QString &orc_FilePath);

   // --------------------------------------------------------------------------
   // Binary Format (Fastest, compact)
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(C_OscDeviceDefinition &orc_DeviceDefinition,
                               const QString &orc_FilePath);
   static int32_t h_SaveBinary(const C_OscDeviceDefinition &orc_DeviceDefinition,
                               const QString &orc_FilePath);
   static int32_t h_LoadFromMemoryBinary(C_OscDeviceDefinition &orc_DeviceDefinition,
                                         const QByteArray &orc_Data);
   QByteArray h_SaveToMemoryBinary(const C_OscDeviceDefinition &orc_DeviceDefinition) const;

   // --------------------------------------------------------------------------
   // JSON Format (Human-readable, debugging)
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(C_OscDeviceDefinition &orc_DeviceDefinition,
                             const QString &orc_FilePath);
   static int32_t h_SaveJson(const C_OscDeviceDefinition &orc_DeviceDefinition,
                             const QString &orc_FilePath);
   static int32_t h_LoadFromMemoryJson(C_OscDeviceDefinition &orc_DeviceDefinition,
                                       const QJsonObject &orc_Object);
   QJsonObject h_SaveToMemoryJson(const C_OscDeviceDefinition &orc_DeviceDefinition) const;

   // --------------------------------------------------------------------------
   // XML Format (Legacy compatibility)
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(C_OscDeviceDefinition &orc_DeviceDefinition,
                            const QString &orc_FilePath);
   static int32_t h_SaveXml(const C_OscDeviceDefinition &orc_DeviceDefinition,
                            const QString &orc_FilePath);
   static int32_t h_LoadFromMemoryXml(C_OscDeviceDefinition &orc_DeviceDefinition,
                                      const QDomElement &orc_Element);
   QDomElement h_SaveToMemoryXml(const C_OscDeviceDefinition &orc_DeviceDefinition,
                                 QDomDocument &ro_Doc) const;

   // --------------------------------------------------------------------------
   // Legacy Compatibility (deprecated - for backward compatibility only)
   // --------------------------------------------------------------------------
   [[deprecated("Use h_LoadDeviceDefinitionFile/h_SaveDeviceDefinitionFile with format detection, or format-specific methods")]]
   static int32_t h_Load(C_OscDeviceDefinition &orc_DeviceDefinition,
                         const QString &orc_Path);
   [[deprecated("Use h_LoadDeviceDefinitionFile/h_SaveDeviceDefinitionFile with format detection, or format-specific methods")]]
   static int32_t h_Save(const C_OscDeviceDefinition &orc_DeviceDefinition,
                         const QString &orc_Path);

private:
   // Helper for format detection
   static int32_t mh_DetectAndLoad(C_OscDeviceDefinition &orc_DeviceDefinition,
                                   const QString &orc_FilePath);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
