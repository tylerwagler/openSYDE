//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for C_OscDataLoggerJob with Qt-native serialization
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDATALOGGERJOBFILER_NEW_HPP
#define C_OSCDATALOGGERJOBFILER_NEW_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscDataLoggerJob.hpp"
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

class C_OscDataLoggerJobFiler {
public:
   static int32_t h_LoadFile(QList<C_OscDataLoggerJob> &orc_Config,
                             const QString &orc_Path);
   static int32_t h_SaveFile(const QList<C_OscDataLoggerJob> &orc_Config,
                             const QString &orc_Path);

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

   static int32_t mh_LoadJobData(C_OscDataLoggerJob &orc_Config,
                                 QDataStream& orc_Stream);
   static int32_t mh_SaveJobData(const C_OscDataLoggerJob &orc_Config,
                                 QDataStream& orc_Stream);
   static int32_t mh_LoadJobProperties(C_OscDataLoggerJobProperties &orc_Config,
                                       QDataStream& orc_Stream);
   static int32_t mh_SaveJobProperties(const C_OscDataLoggerJobProperties &orc_Config,
                                       QDataStream& orc_Stream);
   static int32_t mh_LoadJobAdditionalTriggerProperties(
       C_OscDataLoggerJobAdditionalTriggerProperties &orc_Config,
       QDataStream& orc_Stream);
   static int32_t mh_SaveJobAdditionalTriggerProperties(
       const C_OscDataLoggerJobAdditionalTriggerProperties &orc_Config,
       QDataStream& orc_Stream);
   static int32_t mh_LoadConfiguredDataElements(
       QList<C_OscDataLoggerDataElementReference> &orc_Config,
       QDataStream& orc_Stream);
   static int32_t mh_SaveConfiguredDataElements(
       const QList<C_OscDataLoggerDataElementReference> &orc_Config,
       QDataStream& orc_Stream);
   static int32_t mh_LoadConfiguredDataElement(
       C_OscDataLoggerDataElementReference &orc_Config,
       QDataStream& orc_Stream);
   static int32_t mh_SaveConfiguredDataElement(
       const C_OscDataLoggerDataElementReference &orc_Config,
       QDataStream& orc_Stream);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif