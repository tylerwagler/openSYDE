//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Load/save system definition data from/to file (V2)

   (See .cpp file for full description)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSYSTEMDEFINITIONFILERV2_HPP
#define C_OSCSYSTEMDEFINITIONFILERV2_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QList>
#include <QStringList>
#include "C_OscNode.hpp"
#include "C_OscNodeFilerV2.hpp"
#include "C_OscSystemBus.hpp"
#include "C_OscSystemBusFilerV2.hpp"
#include "C_OscSystemDefinition.hpp"
#include "C_OscXmlParser.hpp"
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

class C_OscSystemDefinitionFilerV2 {
public:
  static int32_t h_LoadSystemDefinitionFile(
      C_OscSystemDefinition &orc_SystemDefinition,
      const QString &orc_PathSystemDefinition,
      const QString &orc_PathDeviceDefinitions,
      const bool oq_UseDeviceDefinitions = true,
      QStringList *const opc_ErrorDetailsMissingDevices = NULL);
  static int32_t
  h_SaveSystemDefinitionFile(const C_OscSystemDefinition &orc_SystemDefinition,
                             const QString &orc_Path);
  static int32_t
  h_LoadSystemDefinitionString(C_OscSystemDefinition &orc_SystemDefinition,
                               const QString &orc_Content,
                               const QString &orc_PathDeviceDefinitions,
                               const bool oq_UseDeviceDefinitions = true);
  static void h_SaveSystemDefinitionString(
      const C_OscSystemDefinition &orc_SystemDefinition, QString &orc_Content);
  static int32_t h_LoadNodes(
      const uint16_t ou16_XmlFormatVersion, QList<C_OscNode> &orc_Nodes,
      C_OscXmlParserBase &orc_XmlParser,
      const C_OscDeviceManager &orc_DeviceDefinitions,
      const bool oq_UseDeviceDefinitions = true,
      QStringList *const opc_ErrorDetailsMissingDevices = NULL);
  static int32_t h_LoadBuses(QList<C_OscSystemBus> &orc_Buses,
                             C_OscXmlParserBase &orc_XmlParser);
  static void h_SaveNodes(const QList<C_OscNode> &orc_Nodes,
                          C_OscXmlParserBase &orc_XmlParser);
  static void h_SaveBuses(const QList<C_OscSystemBus> &orc_Buses,
                          C_OscXmlParserBase &orc_XmlParser);
  static int32_t h_LoadSystemDefinition(
      C_OscSystemDefinition &orc_SystemDefinition,
      C_OscXmlParserBase &orc_XmlParser,
      const QString &orc_PathDeviceDefinitions,
      const bool oq_UseDeviceDefinitions = true,
      QStringList *const opc_ErrorDetailsMissingDevices = NULL);
  static void
  h_SaveSystemDefinition(const C_OscSystemDefinition &orc_SystemDefinition,
                         C_OscXmlParserBase &orc_XmlParser);

  /// known file versions
  static const uint16_t hu16_FILE_VERSION_1 = 1U;
  static const uint16_t hu16_FILE_VERSION_2 = 2U;
  static const uint16_t hu16_FILE_VERSION_LATEST = hu16_FILE_VERSION_2;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
