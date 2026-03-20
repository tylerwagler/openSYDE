 //----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Load/save system definition data from/to file with multi-format support (V3)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
 //----------------------------------------------------------------------------------------------------------------------
 #ifndef C_OSCSYSTEMDEFINITIONFILER_NEW_HPP
 #define C_OSCSYSTEMDEFINITIONFILER_NEW_HPP

 /* -- Includes
  * ------------------------------------------------------------------------------------------------------
  */
#include "C_OscSystemDefinition.hpp"
#include "C_OscFilerUtil.hpp"
#include "C_OscXmlParser.hpp"
#include <QString>
#include <QStringList>

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

 class C_OscSystemDefinitionFiler_New {
 public:
     // --------------------------------------------------------------------------
     // Public Static Methods - Multi-format file operations
     // --------------------------------------------------------------------------
     static int32_t h_LoadFile(C_OscSystemDefinition &orc_SystemDefinition,
                               const QString &orc_PathSystemDefinition,
                               const QString &orc_PathDeviceDefinitions,
                               const bool oq_UseDeviceDefinitions = true,
                               uint16_t *const opu16_ReadFileVersion = NULL);
     static int32_t h_SaveFile(const C_OscSystemDefinition &orc_SystemDefinition,
                               const QString &orc_Path,
                               QStringList *const opc_CreatedFiles = NULL);

     // --------------------------------------------------------------------------
     // Format-specific loading methods
     // --------------------------------------------------------------------------
     static int32_t h_LoadBinary(C_OscSystemDefinition &orc_SystemDefinition,
                                 QDataStream &orc_Stream);
     static int32_t h_LoadJson(C_OscSystemDefinition &orc_SystemDefinition,
                               const QJsonObject &orc_Object);
     static int32_t h_LoadXml(C_OscSystemDefinition &orc_SystemDefinition,
                              C_OscXmlParserBase &orc_XmlParser);

      // --------------------------------------------------------------------------
      // Format-specific saving methods
      // --------------------------------------------------------------------------
      static int32_t h_SaveBinary(const C_OscSystemDefinition &orc_SystemDefinition,
                                  QDataStream &orc_Stream);
      static int32_t h_SaveJson(const C_OscSystemDefinition &orc_SystemDefinition,
                                QJsonObject &orc_Object);
      static int32_t h_SaveXml(const C_OscSystemDefinition &orc_SystemDefinition,
                               C_OscXmlParserBase &orc_XmlParser);

       /// Known file versions
      static const uint16_t hu16_FILE_VERSION_1 = 1U;
      static const uint16_t hu16_FILE_VERSION_2 = 2U;
      static const uint16_t hu16_FILE_VERSION_3 = 3U;
      static const uint16_t hu16_FILE_VERSION_LATEST = hu16_FILE_VERSION_3;

      // --------------------------------------------------------------------------
      // Utility Methods
      // --------------------------------------------------------------------------
      static void h_SplitDeviceType(const QString &orc_CompleteType,
                                    QString &orc_MainType,
                                    QString &orc_SubType);

      // --------------------------------------------------------------------------
      // Clipboard Support Methods (XML Parser Interface)
      // --------------------------------------------------------------------------
      static int32_t h_LoadNodes(QList<C_OscNode> &orc_Nodes,
                                 C_OscXmlParserBase &orc_XmlParser,
                                 const C_OscDeviceManager &orc_DeviceDefinitions,
                                 const QString &orc_BasePath,
                                 const bool oq_UseDeviceDefinitions = true,
                                 const bool oq_UseFileInterface = true,
                                 const QByteArray *const opc_NodesToLoad = NULL,
                                 const bool oq_SkipContent = false,
                                 const QString *const opc_ExpectedNodeName = NULL,
                                 QStringList *const opc_ErrorDetailsMissingDevices = NULL);
      static int32_t h_LoadBuses(QList<C_OscSystemBus> &orc_Buses,
                                 C_OscXmlParserBase &orc_XmlParser);
      static int32_t h_SaveNodes(const QList<C_OscNode> &orc_Nodes,
                                 C_OscXmlParserBase &orc_XmlParser,
                                 const QString &orc_BasePath,
                                 QStringList *const opc_CreatedFiles);
      static void h_SaveBuses(const QList<C_OscSystemBus> &orc_Buses,
                              C_OscXmlParserBase &orc_XmlParser);
};

 /* -- Extern Global Variables
  * ---------------------------------------------------------------------------------------
  */
 } // namespace opensyde_core
 } // namespace stw

 #endif
