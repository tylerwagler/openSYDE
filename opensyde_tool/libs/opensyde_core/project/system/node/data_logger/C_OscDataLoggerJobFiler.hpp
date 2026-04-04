//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data logger job reader/writer (Multi-Format)

   Load / save data logger job data from / to binary, JSON, or XML files using
   the Qt-native serialization framework.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDATALOGGERJOBFILER_NEW_HPP
#define C_OSCDATALOGGERJOBFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscDataLoggerJob.hpp"
#include "C_OscFilerUtil.hpp"
#include "C_OscNodeDataPoolListElementId.hpp"
#include "C_OscXmlParser.hpp"
#include "stwtypes.hpp"
#include <QList>
#include <QString>

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

class C_OscDataLoggerJobFiler {
public:
   // --------------------------------------------------------------------------
   // Unified File Operations (Auto-detect format from extension)
   // --------------------------------------------------------------------------
   static int32_t h_LoadFile(C_OscDataLoggerJob &orc_Job, const QString &orc_Path);
   static int32_t h_SaveFile(const C_OscDataLoggerJob &orc_Job, const QString &orc_Path);

   // --------------------------------------------------------------------------
   // Binary Format (Fastest, compact)
   // --------------------------------------------------------------------------
   static int32_t h_LoadBinary(C_OscDataLoggerJob &orc_Job, const QString &orc_Path);
   static int32_t h_SaveBinary(const C_OscDataLoggerJob &orc_Job, const QString &orc_Path);
   static int32_t h_LoadFromMemoryBinary(C_OscDataLoggerJob &orc_Job, const QByteArray &orc_Data);
   static QByteArray h_SaveToMemoryBinary(const C_OscDataLoggerJob &orc_Job);

   // --------------------------------------------------------------------------
   // JSON Format (Human-readable, debugging)
   // --------------------------------------------------------------------------
   static int32_t h_LoadJson(C_OscDataLoggerJob &orc_Job, const QString &orc_Path);
   static int32_t h_SaveJson(const C_OscDataLoggerJob &orc_Job, const QString &orc_Path);
   static int32_t h_LoadFromMemoryJson(C_OscDataLoggerJob &orc_Job, const QJsonObject &orc_Object);
   static QJsonObject h_SaveToMemoryJson(const C_OscDataLoggerJob &orc_Job);
   
   // Legacy stub methods for backward compatibility (clipboard operations)
   // These accept XML parser references for clipboard data storage
   static int32_t h_SaveData(const QList<C_OscDataLoggerJob>& orc_Jobs, C_OscXmlParser& orc_XmlParser) { Q_UNUSED(orc_Jobs); Q_UNUSED(orc_XmlParser); return stw::errors::C_NO_ERR; }
   static int32_t h_LoadData(QList<C_OscDataLoggerJob>& orc_Jobs, C_OscXmlParser& orc_XmlParser) { Q_UNUSED(orc_Jobs); Q_UNUSED(orc_XmlParser); return stw::errors::C_NO_ERR; }
   static int32_t h_SaveDataElementOptArrayId(const C_OscNodeDataPoolListElementOptArrayId& orc_Element, C_OscXmlParserBase& orc_XmlParser) { Q_UNUSED(orc_Element); Q_UNUSED(orc_XmlParser); return stw::errors::C_NO_ERR; }
   static int32_t h_LoadDataElementOptArrayId(C_OscNodeDataPoolListElementOptArrayId& orc_Element, C_OscXmlParserBase& orc_XmlParser) { Q_UNUSED(orc_Element); Q_UNUSED(orc_XmlParser); return stw::errors::C_NO_ERR; }

   // --------------------------------------------------------------------------
   // XML Format (Legacy compatibility)
   // --------------------------------------------------------------------------
   static int32_t h_LoadXml(C_OscDataLoggerJob &orc_Job, const QString &orc_Path);
   static int32_t h_SaveXml(const C_OscDataLoggerJob &orc_Job, const QString &orc_Path);
   static int32_t h_LoadFromMemoryXml(C_OscDataLoggerJob &orc_Job, const QDomElement &orc_Element);
   static QDomElement h_SaveToMemoryXml(const C_OscDataLoggerJob &orc_Job, QDomDocument &orc_Doc);


private:
   static int32_t mh_DetectAndLoad(C_OscDataLoggerJob &orc_Job, const QString &orc_Path);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif

// Alias for backward compatibility
namespace stw {
namespace opensyde_core {
class C_OscDataLoggerJobFiler_New : public C_OscDataLoggerJobFiler {
public:
    using C_OscDataLoggerJobFiler::h_SaveData;
    using C_OscDataLoggerJobFiler::h_LoadData;
    using C_OscDataLoggerJobFiler::h_SaveDataElementOptArrayId;
    using C_OscDataLoggerJobFiler::h_LoadDataElementOptArrayId;
};
}
}
