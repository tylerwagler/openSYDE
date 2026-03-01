//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Migrated Filer for XApp properties using Qt-native serialization
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXAPPPROPERTIESFILER_NEW_HPP
#define C_OSCXAPPPROPERTIESFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscXappProperties.hpp"
#include <QString>

/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_core {

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_OscXappPropertiesFiler_New {
public:
   // Primary API: Auto-detect format and load/save
   static int32_t h_LoadFile(C_OscXappProperties& orc_Config, const QString& orc_Path);
   static int32_t h_SaveFile(const C_OscXappProperties& orc_Config, const QString& orc_Path);
   
   // Format-specific API
   static int32_t h_LoadBinary(C_OscXappProperties& orc_Config, const QByteArray& orc_Data);
   static QByteArray h_SaveBinary(const C_OscXappProperties& orc_Config);
   static int32_t h_LoadJson(C_OscXappProperties& orc_Config, const QJsonObject& orc_Object);
   static QJsonObject h_SaveJson(const C_OscXappProperties& orc_Config);
   static int32_t h_LoadXml(C_OscXappProperties& orc_Config, const QDomElement& orc_Element);
   static QDomElement h_SaveXml(const C_OscXappProperties& orc_Config, QDomDocument& orc_Doc);
   
   // Legacy compatibility (deprecated)
   static int32_t h_LoadData(C_OscXappProperties& orc_Config, class C_OscXmlParserBase& orc_XmlParser);
   static void h_SaveData(const C_OscXappProperties& orc_Config, class C_OscXmlParserBase& orc_XmlParser);
   
   static const QString hc_FILE_NAME;

private:
   C_OscXappPropertiesFiler_New();
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCXAPPPROPERTIESFILER_NEW_HPP
