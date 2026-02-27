//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Export data to Vector DBC file (header)

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CIEEXPORTDBC_HPP
#define C_CIEEXPORTDBC_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_CieConverter.hpp"
#include "C_OscNodeDataPoolContent.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"
#include <map>
#include <string>
#include <vector>
#include <QList>
#include <QString>
#include <QStringList>


/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_gui_logic {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_CieExportDbc {
public:
  static int32_t
  h_ExportNetwork(const QString &orc_File,
                  const C_CieConverter::C_CieCommDefinition &orc_Definition,
                  QStringList &orc_WarningMessages, QString &orc_ErrorMessage);

  static int32_t h_GetNodeMapping(QHash<QString, QString> &orc_NodeMapping);

  class C_ExportStatistic {
  public:
    uint32_t u32_NumOfMessages;
    uint32_t u32_NumOfSignals;
  };

  static int32_t h_GetExportStatistic(C_ExportStatistic &orc_ExportStatistic);

private:
  static QStringList mhc_WarningMessages; // global warnings e.g. why some
                                          // messages could not be exported
  static QString
      mhc_ErrorMessage; // description of error which caused the export to fail

  static int32_t
  mh_SetNodes(const QList<C_CieConverter::C_CieNode> &orc_CieNodes,
              std::map<std::string, Vector::DBC::Node> &orc_DbcNodes);
  static int32_t
  mh_SetMessages(const QList<C_CieConverter::C_CieNode> &orc_CieNodes,
                 std::map<unsigned int, Vector::DBC::Message> &orc_DbcMessages);
  static int32_t mh_SetSignals(
      const QList<C_CieConverter::C_CieCanSignal> &orc_CieSignals,
      const QList<C_CieConverter::C_CieNode> &orc_CieNodes,
      Vector::DBC::Message &orc_DbcMessage);
  static int32_t
  mh_SetSignalValues(const C_CieConverter::C_CieDataPoolElement &orc_Element,
                     Vector::DBC::Signal &orc_DbcSignal);
  static void
  mh_SetSignalSpnValue(const C_CieConverter::C_CieCanSignal &orc_Signal,
                       Vector::DBC::Signal &orc_DbcSignal);
  static int32_t
  mh_SetTransmission(const C_CieConverter::C_CieNodeMessage &orc_Message,
                     Vector::DBC::Message &orc_DbcMessage);
  static void mh_SetNewSymbols(std::vector<std::string> &orc_NewSymbols);
  static void mh_SetAttributeDefaults(
      std::map<std::string, Vector::DBC::Attribute> &orc_AttributeDefaults);
  static void mh_SetAttributeDefinitions(
      std::map<std::string, Vector::DBC::AttributeDefinition>
          &orc_AttributeDefinitions);
  static QString mh_NiceifyStringForDbcSymbol(const QString &orc_String);
  static QString mh_EscapeCriticalSymbols(const QString &orc_String);

  static const QString mhc_SIG_INITIAL_VALUE;
  static const QString mhc_MSG_CYCLE_TIME;
  static const QString mhc_MSG_SEND_TYPE;
  static QHash<QString, QString> mhc_NodeMapping;
  static bool mhq_ValidDbcExport;
  static C_ExportStatistic mhc_ExportStatistic;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_gui_logic
} // namespace stw

#endif
