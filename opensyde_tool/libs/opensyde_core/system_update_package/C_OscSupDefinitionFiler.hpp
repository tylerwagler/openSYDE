 //----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       System update package definition filer with multi-format support
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
 //----------------------------------------------------------------------------------------------------------------------
 #ifndef C_OSCSUPDEFINITIONFILER_HPP
 #define C_OSCSUPDEFINITIONFILER_HPP

 /* -- Includes
  * ------------------------------------------------------------------------------------------------------
  */
 #include "C_OscSupDefinition.hpp"
 #include <QString>
 #include <QStringList>
 #include <QDataStream>
 #include <QJsonObject>
 #include <QDomDocument>
 #include <QDomElement>
#include "C_OscXmlParser.hpp"

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

 class C_OscSupDefinitionFiler {
 public:
     // --------------------------------------------------------------------------
     // Public Static Methods - Multi-format file operations
     // --------------------------------------------------------------------------
     static int32_t h_CreateUpdatePackageDefFile(
         const QString &orc_Path,
         const C_OscSupDefinition &orc_SupDefContent,
         const QStringList &orc_Files);
     static int32_t h_LoadUpdatePackageDefFile(
         const QString &orc_TargetUnzipPath,
         const bool oq_IsZip,
         const QString &orc_PackagePath,
         uint32_t &oru32_FileVersion,
         QString &orc_FilePackagePath,
         uint32_t &oru32_ActiveBusIndex,
         QByteArray &orc_ActiveNodes,
         QList<uint32_t> &orc_UpdatePosition,
         QStringList &orc_PackageFiles);

     // --------------------------------------------------------------------------
     // Format-specific loading methods
     // --------------------------------------------------------------------------
     static int32_t h_LoadBinary(const QString &orc_Path,
                                 uint32_t &oru32_FileVersion,
                                 QString &orc_FilePackagePath,
                                 uint32_t &oru32_ActiveBusIndex,
                                 QByteArray &orc_ActiveNodes,
                                 QList<uint32_t> &orc_UpdatePosition,
                                 QStringList &orc_PackageFiles);
     static int32_t h_LoadJson(const QString &orc_Path,
                               uint32_t &oru32_FileVersion,
                               QString &orc_FilePackagePath,
                               uint32_t &oru32_ActiveBusIndex,
                               QByteArray &orc_ActiveNodes,
                               QList<uint32_t> &orc_UpdatePosition,
                               QStringList &orc_PackageFiles);
     static int32_t h_LoadXml(const QString &orc_Path,
                              uint32_t &oru32_FileVersion,
                              QString &orc_FilePackagePath,
                              uint32_t &oru32_ActiveBusIndex,
                              QByteArray &orc_ActiveNodes,
                              QList<uint32_t> &orc_UpdatePosition,
                              QStringList &orc_PackageFiles);

     // --------------------------------------------------------------------------
     // Format-specific saving methods
     // --------------------------------------------------------------------------
     static int32_t h_SaveBinary(const QString &orc_Path,
                                 const C_OscSupDefinition &orc_SupDefContent,
                                 const QStringList &orc_Files);
     static int32_t h_SaveJson(const QString &orc_Path,
                               const C_OscSupDefinition &orc_SupDefContent,
                               const QStringList &orc_Files);
     static int32_t h_SaveXml(const QString &orc_Path,
                              const C_OscSupDefinition &orc_SupDefContent,
                              const QStringList &orc_Files);

     // --------------------------------------------------------------------------
     // Helper methods
     // --------------------------------------------------------------------------
     static const QString hc_PACKAGE_UPDATE_DEF;

     // --------------------------------------------------------------------------
     // Legacy compatibility methods (deprecated)
     // --------------------------------------------------------------------------
     [[deprecated("Use h_CreateUpdatePackageDefFile/h_LoadUpdatePackageDefFile with auto-detection instead")]]
     static int32_t h_CreateUpdatePackageDefFileLegacy(
         const QString &orc_Path,
         const C_OscSupDefinition &orc_SupDefContent,
         const QStringList &orc_Files);
     [[deprecated("Use h_CreateUpdatePackageDefFile/h_LoadUpdatePackageDefFile with auto-detection instead")]]
     static int32_t h_LoadUpdatePackageDefFileLegacy(
         const QString &orc_TargetUnzipPath,
         const bool oq_IsZip,
         const QString &orc_PackagePath,
         uint32_t &oru32_FileVersion,
         QString &orc_FilePackagePath,
         uint32_t &oru32_ActiveBusIndex,
         QByteArray &orc_ActiveNodes,
         QList<uint32_t> &orc_UpdatePosition,
         QStringList &orc_PackageFiles);

 private:
     static void mh_SaveNodes(C_OscXmlParserBase &orc_XmlParser,
                              const QList<C_OscSupNodeDefinition> &orc_Nodes,
                              const QStringList &orc_Files);
     static void mh_LoadNodes(C_OscXmlParserBase &orc_XmlParser,
                              QByteArray &orc_ActiveNodes,
                              QList<uint32_t> &orc_UpdatePosition,
                              QStringList &orc_PackageFiles);
 };

 /* -- Extern Global Variables
  * ---------------------------------------------------------------------------------------
  */
 } // namespace opensyde_core
 } // namespace stw

 #endif
