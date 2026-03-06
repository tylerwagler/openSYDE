//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for C_OscDataLoggerJob
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDATALOGGERJOBFILER_HPP
#define C_OSCDATALOGGERJOBFILER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscDataLoggerJob.hpp"
#include "C_OscNodeDataPoolListElementId.hpp"
#include "C_OscNodeDataPoolListElementOptArrayId.hpp"
#include "C_OscXmlParser.hpp"
#include <QList>

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
   // Legacy XML-parser-based methods (used by C_OscNodeFiler)
   static int32_t h_LoadData(QList<C_OscDataLoggerJob> &orc_Config,
                             C_OscXmlParserBase &orc_XmlParser);
   static void h_SaveData(const QList<C_OscDataLoggerJob> &orc_Config,
                          C_OscXmlParserBase &orc_XmlParser);
   static int32_t h_LoadDataElementId(C_OscNodeDataPoolListElementId &orc_Config,
                                      C_OscXmlParserBase &orc_XmlParser);
   static void h_SaveDataElementId(const C_OscNodeDataPoolListElementId &orc_Config,
                                   C_OscXmlParserBase &orc_XmlParser);
   static int32_t h_LoadDataElementOptArrayId(
       C_OscNodeDataPoolListElementOptArrayId &orc_Config,
       C_OscXmlParserBase &orc_XmlParser);
   static void h_SaveDataElementOptArrayId(
       const C_OscNodeDataPoolListElementOptArrayId &orc_Config,
       C_OscXmlParserBase &orc_XmlParser);

   // File-based methods (legacy XML + new multi-format)
   static int32_t h_LoadFile(QList<C_OscDataLoggerJob> &orc_Config,
                             const QString &orc_Path);
   static int32_t h_SaveFile(const QList<C_OscDataLoggerJob> &orc_Config,
                             const QString &orc_Path);

   // New Qt-native multi-format methods
   static int32_t h_LoadBinary(QList<C_OscDataLoggerJob> &orc_Config,
                               const QString &orc_Path);
   static int32_t h_SaveBinary(const QList<C_OscDataLoggerJob> &orc_Config,
                               const QString &orc_Path);
   static int32_t h_LoadJson(QList<C_OscDataLoggerJob> &orc_Config,
                             const QString &orc_Path);
   static int32_t h_SaveJson(const QList<C_OscDataLoggerJob> &orc_Config,
                             const QString &orc_Path);
   static int32_t h_LoadXml(QList<C_OscDataLoggerJob> &orc_Config,
                            const QString &orc_Path);
   static int32_t h_SaveXml(const QList<C_OscDataLoggerJob> &orc_Config,
                            const QString &orc_Path);

private:
   C_OscDataLoggerJobFiler();

   static const uint16_t mhu16_FILE_VERSION_1;

   // Legacy XML-parser helpers
   static int32_t mh_LoadJobData(C_OscDataLoggerJob &orc_Config,
                                 C_OscXmlParserBase &orc_XmlParser);
   static void mh_SaveJobData(const C_OscDataLoggerJob &orc_Config,
                              C_OscXmlParserBase &orc_XmlParser);
   static int32_t mh_LoadJobProperties(C_OscDataLoggerJobProperties &orc_Config,
                                       C_OscXmlParserBase &orc_XmlParser);
   static void mh_SaveJobProperties(const C_OscDataLoggerJobProperties &orc_Config,
                                    C_OscXmlParserBase &orc_XmlParser);
   static int32_t mh_LoadJobAdditionalTriggerProperties(
       C_OscDataLoggerJobAdditionalTriggerProperties &orc_Config,
       C_OscXmlParserBase &orc_XmlParser);
   static void mh_SaveJobAdditionalTriggerProperties(
       const C_OscDataLoggerJobAdditionalTriggerProperties &orc_Config,
       C_OscXmlParserBase &orc_XmlParser);
   static int32_t mh_LoadConfiguredDataElements(
       QList<C_OscDataLoggerDataElementReference> &orc_Config,
       C_OscXmlParserBase &orc_XmlParser);
   static void mh_SaveConfiguredDataElements(
       const QList<C_OscDataLoggerDataElementReference> &orc_Config,
       C_OscXmlParserBase &orc_XmlParser);
   static int32_t mh_LoadConfiguredDataElement(
       C_OscDataLoggerDataElementReference &orc_Config,
       C_OscXmlParserBase &orc_XmlParser);
   static void mh_SaveConfiguredDataElement(
       const C_OscDataLoggerDataElementReference &orc_Config,
       C_OscXmlParserBase &orc_XmlParser);
   static QString mh_LogFileTypeTypeToString(
       const C_OscDataLoggerJobProperties::E_LogFileFormat &ore_Type);
   static int32_t mh_StringToLogFileType(
       const QString &orc_String,
       C_OscDataLoggerJobProperties::E_LogFileFormat &ore_Type);
   static QString mh_LocalLogTriggerTypeToString(
       const C_OscDataLoggerJobProperties::E_LocalLogTrigger &ore_Type);
   static int32_t mh_StringToLocalLogTriggerType(
       const QString &orc_String,
       C_OscDataLoggerJobProperties::E_LocalLogTrigger &ore_Type);
   static QString mh_UseCaseTypeToString(
       const C_OscDataLoggerJobProperties::E_UseCase &ore_Type);
   static int32_t mh_StringToUseCaseType(
       const QString &orc_String,
       C_OscDataLoggerJobProperties::E_UseCase &ore_Type);

};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
