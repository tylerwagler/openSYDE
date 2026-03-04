//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for C_OscDataLoggerJob

   Filer for C_OscDataLoggerJob (legacy XML-parser-based and Qt-native multi-format serialization)

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDomDocument>
#include <QDomElement>
#include <QTextStream>

#include "C_OscDataLoggerJobFiler.hpp"
#include "C_OscDataLoggerJob.hpp"
#include "C_OscDataLoggerJobProperties.hpp"
#include "C_OscDataLoggerJobAdditionalTriggerProperties.hpp"
#include "C_OscDataLoggerDataElementReference.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscNodeDataPoolFiler.hpp"
#include "C_OscNodeDataPoolContent.hpp"
#include "C_OscNodeDataPoolListElementOptArrayId.hpp"
#include "C_OscSystemFilerUtil.hpp"
#include "C_OscXmlParserLog.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */
const uint16_t C_OscDataLoggerJobFiler::mhu16_FILE_VERSION_1 = 1;

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */

/* -- Module Global Variables
 * ---------------------------------------------------------------------------------------
 */

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscDataLoggerJobFiler::C_OscDataLoggerJobFiler() {
}

//----------------------------------------------------------------------------------------------------------------------
// ===== Legacy XML-parser-based public methods =====
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load data loggers

   \param[in,out]  orc_Config    Config
   \param[in]      orc_Path      Path

   \return
   C_NO_ERR    data read
   C_RANGE     specified data loggers file does not exist
   C_NOACT     specified file is present but structure is invalid (e.g. invalid
   XML file) C_CONFIG    data loggers content is invalid or incomplete data
   loggers could not be loaded
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t
C_OscDataLoggerJobFiler::h_LoadFile(QList<C_OscDataLoggerJob> &orc_Config,
                                    const QString &orc_Path) {
  int32_t s32_Retval = C_NO_ERR;

  if (QFileInfo(orc_Path).exists() && QFileInfo(orc_Path).isFile()) {
    C_OscXmlParserLog c_XmlParser;
    c_XmlParser.SetLogHeading("Loading data loggers data");
    s32_Retval = C_OscSystemFilerUtil::h_GetParserForExistingFile(
        c_XmlParser, orc_Path, "opensyde-data-loggers");
    if (s32_Retval == C_NO_ERR) {
      s32_Retval = h_LoadData(orc_Config, c_XmlParser);
    }
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save data loggers

   \param[in]  orc_Config  Config
   \param[in]  orc_Path    Path

   \return
   C_NO_ERR   data saved
   C_CONFIG   data invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t
C_OscDataLoggerJobFiler::h_SaveFile(const QList<C_OscDataLoggerJob> &orc_Config,
                                    const QString &orc_Path) {
  C_OscXmlParser c_XmlParser;
  int32_t s32_Retval = C_OscSystemFilerUtil::h_GetParserForNewFile(
      c_XmlParser, orc_Path, "opensyde-data-loggers");

  if (s32_Retval == C_NO_ERR) {
    // node
    C_OscDataLoggerJobFiler::h_SaveData(orc_Config, c_XmlParser);
    // Don't forget to save!
    if (c_XmlParser.SaveToFile(orc_Path) != C_NO_ERR) {
      osc_write_log_error("Saving data loggers data",
                          "Could not create file for node.");
      s32_Retval = C_CONFIG;
    }
  } else {
    // More details are in log
    s32_Retval = C_CONFIG;
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load data loggers

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser

   \return
   C_NO_ERR    data read
   C_CONFIG    Data loggers file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t
C_OscDataLoggerJobFiler::h_LoadData(QList<C_OscDataLoggerJob> &orc_Config,
                                    C_OscXmlParserBase &orc_XmlParser) {
  int32_t s32_Retval = C_OscSystemFilerUtil::h_CheckVersion(
      orc_XmlParser, mhu16_FILE_VERSION_1, "file-version",
      "Loading data loggers data");

  if (s32_Retval == C_NO_ERR) {
    s32_Retval = orc_XmlParser.SelectNodeChildError("jobs");

    orc_Config.clear();
    if (s32_Retval == C_NO_ERR) {
      uint32_t u32_ExpectedSize;
      s32_Retval =
          orc_XmlParser.GetAttributeUint32Error("length", u32_ExpectedSize);
      if (s32_Retval == C_NO_ERR) {
        QString c_NodeName = orc_XmlParser.SelectNodeChild("job");
        if (c_NodeName == "job") {
          do {
            C_OscDataLoggerJob c_Job;
            s32_Retval =
                C_OscDataLoggerJobFiler::mh_LoadJobData(c_Job, orc_XmlParser);
            if (s32_Retval == C_NO_ERR) {
              orc_Config.push_back(c_Job);
            }

            c_NodeName = orc_XmlParser.SelectNodeNext("job");
          } while ((c_NodeName == "job") && (s32_Retval == C_NO_ERR));
          Q_ASSERT(orc_XmlParser.SelectNodeParent() == "jobs");
        }
        if (u32_ExpectedSize != orc_Config.size()) {
          QString c_Tmp = QString::asprintf(
              "Unexpected data loggers count, expected: %u, got %u",
              u32_ExpectedSize, static_cast<uint32_t>(orc_Config.size()));
          orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("length",
                                                                       c_Tmp);
        }
      }
      orc_XmlParser.SelectNodeParent();
    }
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save data loggers

   \param[in]      orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDataLoggerJobFiler::h_SaveData(
    const QList<C_OscDataLoggerJob> &orc_Config,
    C_OscXmlParserBase &orc_XmlParser) {
  // File version
  Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("file-version") ==
           "file-version");
  orc_XmlParser.SetNodeContent(QString::number(mhu16_FILE_VERSION_1));
  // Return
  orc_XmlParser.SelectNodeParent();
  Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("jobs") == "jobs");
  orc_XmlParser.SetAttributeUint32("length",
                                   static_cast<uint32_t>(orc_Config.size()));
  for (QList<C_OscDataLoggerJob>::const_iterator c_It = orc_Config.begin();
       c_It != orc_Config.end(); ++c_It) {
    Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("job") == "job");
    C_OscDataLoggerJobFiler::mh_SaveJobData(*c_It, orc_XmlParser);
    Q_ASSERT(orc_XmlParser.SelectNodeParent() == "jobs");
  }
  // Return
  orc_XmlParser.SelectNodeParent();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load data element id

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser

   \return
   C_NO_ERR    data read
   C_CONFIG    Data loggers file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::h_LoadDataElementId(
    C_OscNodeDataPoolListElementId &orc_Config,
    C_OscXmlParserBase &orc_XmlParser) {
  int32_t s32_Retval = orc_XmlParser.SelectNodeChildError("index");

  if (s32_Retval == C_NO_ERR) {
    s32_Retval =
        orc_XmlParser.GetAttributeUint32Error("node", orc_Config.u32_NodeIndex);
  }
  if (s32_Retval == C_NO_ERR) {
    s32_Retval = orc_XmlParser.GetAttributeUint32Error(
        "data-pool", orc_Config.u32_DataPoolIndex);
  }
  if (s32_Retval == C_NO_ERR) {
    s32_Retval =
        orc_XmlParser.GetAttributeUint32Error("list", orc_Config.u32_ListIndex);
  }
  if (s32_Retval == C_NO_ERR) {
    s32_Retval = orc_XmlParser.GetAttributeUint32Error(
        "element", orc_Config.u32_ElementIndex);
  }
  // Return
  orc_XmlParser.SelectNodeParent();
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save data element id

   \param[in]      orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDataLoggerJobFiler::h_SaveDataElementId(
    const C_OscNodeDataPoolListElementId &orc_Config,
    C_OscXmlParserBase &orc_XmlParser) {
  orc_XmlParser.CreateAndSelectNodeChild("index");
  orc_XmlParser.SetAttributeUint32("node", orc_Config.u32_NodeIndex);
  orc_XmlParser.SetAttributeUint32("data-pool", orc_Config.u32_DataPoolIndex);
  orc_XmlParser.SetAttributeUint32("list", orc_Config.u32_ListIndex);
  orc_XmlParser.SetAttributeUint32("element", orc_Config.u32_ElementIndex);
  // Return
  orc_XmlParser.SelectNodeParent();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load data element opt array id

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser

   \return
   C_NO_ERR    data read
   C_CONFIG    Data loggers file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::h_LoadDataElementOptArrayId(
    C_OscNodeDataPoolListElementOptArrayId &orc_Config,
    C_OscXmlParserBase &orc_XmlParser) {
  bool q_UseArrayElementIndex = false;
  uint32_t u32_ArrayElementIndex = 0UL;
  C_OscNodeDataPoolListElementId c_TmpId;
  int32_t s32_Retval = orc_XmlParser.GetAttributeBoolError(
      "use-array-element-index", q_UseArrayElementIndex);

  if (s32_Retval == C_NO_ERR) {
    s32_Retval = orc_XmlParser.GetAttributeUint32Error("array-element-index",
                                                       u32_ArrayElementIndex);
  }
  if (s32_Retval == C_NO_ERR) {
    s32_Retval =
        C_OscDataLoggerJobFiler::h_LoadDataElementId(c_TmpId, orc_XmlParser);
  }
  if (s32_Retval == C_NO_ERR) {
    orc_Config = C_OscNodeDataPoolListElementOptArrayId(
        c_TmpId, q_UseArrayElementIndex, u32_ArrayElementIndex);
    s32_Retval = orc_XmlParser.SelectNodeChildError("hal-channel-name");
  }
  if (s32_Retval == C_NO_ERR) {
    s32_Retval = orc_XmlParser.SelectNodeChildError("hal-channel-name");
    if (s32_Retval == C_NO_ERR) {
      orc_Config.SetHalChannelName(orc_XmlParser.GetNodeContent());
      Q_ASSERT(orc_XmlParser.SelectNodeParent() == "hal-channel-name");
    }
  }
  // Return
  orc_XmlParser.SelectNodeParent();
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save data element opt array id

   \param[in]      orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDataLoggerJobFiler::h_SaveDataElementOptArrayId(
    const C_OscNodeDataPoolListElementOptArrayId &orc_Config,
    C_OscXmlParserBase &orc_XmlParser) {
  orc_XmlParser.SetAttributeBool("use-array-element-index",
                                 orc_Config.GetUseArrayElementIndex());
  orc_XmlParser.SetAttributeUint32("array-element-index",
                                   orc_Config.GetArrayElementIndex());
  C_OscDataLoggerJobFiler::h_SaveDataElementId(orc_Config, orc_XmlParser);
  orc_XmlParser.CreateNodeChild("hal-channel-name",
                                orc_Config.GetHalChannelName());
}

//----------------------------------------------------------------------------------------------------------------------
// ===== Legacy XML-parser-based private helpers =====
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load job data

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser

   \return
   C_NO_ERR    data read
   C_CONFIG    Data loggers file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t
C_OscDataLoggerJobFiler::mh_LoadJobData(C_OscDataLoggerJob &orc_Config,
                                        C_OscXmlParserBase &orc_XmlParser) {
  int32_t s32_Retval =
      orc_XmlParser.GetAttributeBoolError("is-enabled", orc_Config.q_IsEnabled);

  if (s32_Retval == C_NO_ERR) {
    s32_Retval = mh_LoadJobProperties(orc_Config.c_Properties, orc_XmlParser);
  }

  if (s32_Retval == C_NO_ERR) {
    s32_Retval = mh_LoadConfiguredDataElements(
        orc_Config.c_ConfiguredDataElements, orc_XmlParser);
  }

  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save job data

   \param[in]      orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDataLoggerJobFiler::mh_SaveJobData(
    const C_OscDataLoggerJob &orc_Config, C_OscXmlParserBase &orc_XmlParser) {
  orc_XmlParser.SetAttributeBool("is-enabled", orc_Config.q_IsEnabled);
  mh_SaveJobProperties(orc_Config.c_Properties, orc_XmlParser);
  mh_SaveConfiguredDataElements(orc_Config.c_ConfiguredDataElements,
                                orc_XmlParser);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load job properties

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser

   \return
   C_NO_ERR    data read
   C_CONFIG    Data loggers file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_LoadJobProperties(
    C_OscDataLoggerJobProperties &orc_Config,
    C_OscXmlParserBase &orc_XmlParser) {
  int32_t s32_Retval = orc_XmlParser.SelectNodeChildError("properties");

  if (s32_Retval == C_NO_ERR) {
    if (orc_XmlParser.AttributeExists("max-log-entries")) {
      s32_Retval = orc_XmlParser.GetAttributeUint32Error(
          "max-log-entries", orc_Config.u32_MaxLogEntries);
    }
  }

  if (s32_Retval == C_NO_ERR) {
    if (orc_XmlParser.AttributeExists("max-log-duration-sec")) {
      s32_Retval = orc_XmlParser.GetAttributeUint32Error(
          "max-log-duration-sec", orc_Config.u32_MaxLogDurationSec);
    }
  }

  if (s32_Retval == C_NO_ERR) {
    if (orc_XmlParser.AttributeExists("log-interval-ms")) {
      s32_Retval = orc_XmlParser.GetAttributeUint32Error(
          "log-interval-ms", orc_Config.u32_LogIntervalMs);
    }
  }

  if (s32_Retval == C_NO_ERR) {
    s32_Retval = orc_XmlParser.SelectNodeChildError("name");
    if (s32_Retval == C_NO_ERR) {
      orc_Config.c_Name = orc_XmlParser.GetNodeContent();
      Q_ASSERT(orc_XmlParser.SelectNodeParent() == "properties");
    }
  }

  if (s32_Retval == C_NO_ERR) {
    s32_Retval = orc_XmlParser.SelectNodeChildError("comment");
    if (s32_Retval == C_NO_ERR) {
      orc_Config.c_Comment = orc_XmlParser.GetNodeContent();
      Q_ASSERT(orc_XmlParser.SelectNodeParent() == "properties");
    }
  }

  if (s32_Retval == C_NO_ERR) {
    s32_Retval = orc_XmlParser.SelectNodeChildError("log-file-format");
    if (s32_Retval == C_NO_ERR) {
      s32_Retval = mh_StringToLogFileType(orc_XmlParser.GetNodeContent(),
                                          orc_Config.e_LogFileFormat);
      Q_ASSERT(orc_XmlParser.SelectNodeParent() == "properties");
    }
  }

  if (s32_Retval == C_NO_ERR) {
    s32_Retval = orc_XmlParser.SelectNodeChildError("local-log-trigger");
    if (s32_Retval == C_NO_ERR) {
      s32_Retval = mh_StringToLocalLogTriggerType(
          orc_XmlParser.GetNodeContent(), orc_Config.e_LocalLogTrigger);
      Q_ASSERT(orc_XmlParser.SelectNodeParent() == "properties");
    }
  }

  if (s32_Retval == C_NO_ERR) {
    if (orc_XmlParser.SelectNodeChild("use-case") == "use-case") {
      s32_Retval = mh_StringToUseCaseType(orc_XmlParser.GetNodeContent(),
                                          orc_Config.e_UseCase);
      Q_ASSERT(orc_XmlParser.SelectNodeParent() == "properties");
    }
  }

  if (s32_Retval == C_NO_ERR) {
    if (orc_XmlParser.SelectNodeChild("log-destination-directory") ==
        "log-destination-directory") {
      orc_Config.c_LogDestinationDirectory = orc_XmlParser.GetNodeContent();
      Q_ASSERT(orc_XmlParser.SelectNodeParent() == "properties");
    }
  }
  if (s32_Retval == C_NO_ERR) {
    s32_Retval = mh_LoadJobAdditionalTriggerProperties(
        orc_Config.c_AdditionalTriggerProperties, orc_XmlParser);
  }
  Q_ASSERT(orc_XmlParser.SelectNodeParent() == "job");
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save job properties

   \param[in]      orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDataLoggerJobFiler::mh_SaveJobProperties(
    const C_OscDataLoggerJobProperties &orc_Config,
    C_OscXmlParserBase &orc_XmlParser) {
  orc_XmlParser.CreateAndSelectNodeChild("properties");
  orc_XmlParser.SetAttributeUint32("max-log-entries",
                                   orc_Config.u32_MaxLogEntries);
  orc_XmlParser.SetAttributeUint32("max-log-duration-sec",
                                   orc_Config.u32_MaxLogDurationSec);
  orc_XmlParser.SetAttributeUint32("log-interval-ms",
                                   orc_Config.u32_LogIntervalMs);
  orc_XmlParser.CreateNodeChild("name", orc_Config.c_Name);
  orc_XmlParser.CreateNodeChild("comment", orc_Config.c_Comment);
  orc_XmlParser.CreateNodeChild(
      "log-file-format",
      mh_LogFileTypeTypeToString(orc_Config.e_LogFileFormat));
  orc_XmlParser.CreateNodeChild(
      "local-log-trigger",
      mh_LocalLogTriggerTypeToString(orc_Config.e_LocalLogTrigger));
  orc_XmlParser.CreateNodeChild("use-case",
                                mh_UseCaseTypeToString(orc_Config.e_UseCase));
  orc_XmlParser.CreateNodeChild("log-destination-directory",
                                orc_Config.c_LogDestinationDirectory);
  mh_SaveJobAdditionalTriggerProperties(
      orc_Config.c_AdditionalTriggerProperties, orc_XmlParser);
  Q_ASSERT(orc_XmlParser.SelectNodeParent() == "job");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load job additional trigger properties

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser

   \return
   C_NO_ERR    data read
   C_CONFIG    Data loggers file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_LoadJobAdditionalTriggerProperties(
    C_OscDataLoggerJobAdditionalTriggerProperties &orc_Config,
    C_OscXmlParserBase &orc_XmlParser) {
  int32_t s32_Retval = C_NO_ERR;

  if (orc_XmlParser.SelectNodeChild("additional-trigger-properties") ==
      "additional-trigger-properties") {
    s32_Retval =
        orc_XmlParser.GetAttributeBoolError("enable", orc_Config.q_Enable);
    if (s32_Retval == C_NO_ERR) {
      s32_Retval = orc_XmlParser.SelectNodeChildError("operation");
      if (s32_Retval == C_NO_ERR) {
        orc_Config.c_Operation = orc_XmlParser.GetNodeContent();
        Q_ASSERT(orc_XmlParser.SelectNodeParent() ==
                 "additional-trigger-properties");
      }
    }
    if (s32_Retval == C_NO_ERR) {
      s32_Retval = orc_XmlParser.SelectNodeChildError("data-pool-element");
      if (s32_Retval == C_NO_ERR) {
        s32_Retval =
            h_LoadDataElementOptArrayId(orc_Config.c_ElementId, orc_XmlParser);
      }
      // Return
      orc_XmlParser.SelectNodeParent();
    }
    if (s32_Retval == C_NO_ERR) {
      s32_Retval = orc_XmlParser.SelectNodeChildError("threshold");
      if (s32_Retval == C_NO_ERR) {
        // copy over value so we have the correct type:
        s32_Retval = C_OscNodeDataPoolFiler::h_LoadDataPoolContentV1(
            orc_Config.c_Threshold, orc_XmlParser);
        // Return
        Q_ASSERT(orc_XmlParser.SelectNodeParent() ==
                 "additional-trigger-properties");
      }
    }
    Q_ASSERT(orc_XmlParser.SelectNodeParent() == "properties");
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save job additional trigger properties

   \param[in]      orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDataLoggerJobFiler::mh_SaveJobAdditionalTriggerProperties(
    const C_OscDataLoggerJobAdditionalTriggerProperties &orc_Config,
    C_OscXmlParserBase &orc_XmlParser) {
  orc_XmlParser.CreateAndSelectNodeChild("additional-trigger-properties");
  orc_XmlParser.SetAttributeBool("enable", orc_Config.q_Enable);
  orc_XmlParser.CreateAndSelectNodeChild("data-pool-element");
  h_SaveDataElementOptArrayId(orc_Config.c_ElementId, orc_XmlParser);
  // Return
  orc_XmlParser.SelectNodeParent();
  orc_XmlParser.CreateAndSelectNodeChild("threshold");
  C_OscNodeDataPoolFiler::h_SaveDataPoolContentV1(orc_Config.c_Threshold,
                                                  orc_XmlParser);
  Q_ASSERT(orc_XmlParser.SelectNodeParent() == "additional-trigger-properties");
  orc_XmlParser.CreateNodeChild("operation", orc_Config.c_Operation);
  Q_ASSERT(orc_XmlParser.SelectNodeParent() == "properties");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load configured data elements

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser

   \return
   C_NO_ERR    data read
   C_CONFIG    Data loggers file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_LoadConfiguredDataElements(
    QList<C_OscDataLoggerDataElementReference> &orc_Config,
    C_OscXmlParserBase &orc_XmlParser) {
  int32_t s32_Retval =
      orc_XmlParser.SelectNodeChildError("configured-data-elements");

  orc_Config.clear();
  if (s32_Retval == C_NO_ERR) {
    uint32_t u32_ExpectedSize;
    s32_Retval =
        orc_XmlParser.GetAttributeUint32Error("length", u32_ExpectedSize);
    if (s32_Retval == C_NO_ERR) {
      QString c_NodeName =
          orc_XmlParser.SelectNodeChild("configured-data-element");
      if (c_NodeName == "configured-data-element") {
        do {
          C_OscDataLoggerDataElementReference c_Data;
          s32_Retval = C_OscDataLoggerJobFiler::mh_LoadConfiguredDataElement(
              c_Data, orc_XmlParser);
          if (s32_Retval == C_NO_ERR) {
            orc_Config.push_back(c_Data);
          }

          c_NodeName = orc_XmlParser.SelectNodeNext("configured-data-element");
        } while ((c_NodeName == "configured-data-element") &&
                 (s32_Retval == C_NO_ERR));
        Q_ASSERT(orc_XmlParser.SelectNodeParent() ==
                 "configured-data-elements");
      }
      if (u32_ExpectedSize != orc_Config.size()) {
        QString c_Tmp = QString::asprintf(
            "Unexpected data logger data elements count, expected: %u, got %u",
            u32_ExpectedSize, static_cast<uint32_t>(orc_Config.size()));
        orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("length",
                                                                     c_Tmp);
      }
    }
    orc_XmlParser.SelectNodeParent();
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save configured data elements

   \param[in]      orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDataLoggerJobFiler::mh_SaveConfiguredDataElements(
    const QList<C_OscDataLoggerDataElementReference> &orc_Config,
    C_OscXmlParserBase &orc_XmlParser) {
  Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild("configured-data-elements") ==
           "configured-data-elements");
  orc_XmlParser.SetAttributeUint32("length",
                                   static_cast<uint32_t>(orc_Config.size()));
  for (QList<C_OscDataLoggerDataElementReference>::const_iterator c_It =
           orc_Config.begin();
       c_It != orc_Config.end(); ++c_It) {
    Q_ASSERT(orc_XmlParser.CreateAndSelectNodeChild(
                 "configured-data-element") == "configured-data-element");
    C_OscDataLoggerJobFiler::mh_SaveConfiguredDataElement(*c_It, orc_XmlParser);
    Q_ASSERT(orc_XmlParser.SelectNodeParent() == "configured-data-elements");
  }
  // Return
  orc_XmlParser.SelectNodeParent();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load configured data element

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    Xml parser

   \return
   C_NO_ERR    data read
   C_CONFIG    Data loggers file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_LoadConfiguredDataElement(
    C_OscDataLoggerDataElementReference &orc_Config,
    C_OscXmlParserBase &orc_XmlParser) {
  int32_t s32_Retval = orc_XmlParser.GetAttributeBoolError(
      "use-custom-name", orc_Config.q_UseCustomName);

  if (s32_Retval == C_NO_ERR) {
    s32_Retval = orc_XmlParser.SelectNodeChildError("custom-name");
    if (s32_Retval == C_NO_ERR) {
      orc_Config.c_CustomName = orc_XmlParser.GetNodeContent();
      Q_ASSERT(orc_XmlParser.SelectNodeParent() == "configured-data-element");
    }
  }

  if (s32_Retval == C_NO_ERR) {
    s32_Retval = orc_XmlParser.SelectNodeChildError("data-pool-element");
    if (s32_Retval == C_NO_ERR) {
      s32_Retval = h_LoadDataElementOptArrayId(orc_Config.c_ConfiguredElementId,
                                               orc_XmlParser);
    }
    // Return
    orc_XmlParser.SelectNodeParent();
  }

  return s32_Retval;
}

void C_OscDataLoggerJobFiler::mh_SaveConfiguredDataElement(
    const C_OscDataLoggerDataElementReference &orc_Config,
    C_OscXmlParserBase &orc_XmlParser) {
  orc_XmlParser.CreateNodeChild(
      "use-custom-name", orc_Config.GetUseCustomName() ? "true" : "false");
  orc_XmlParser.CreateNodeChild("custom-name", orc_Config.GetCustomName());
  orc_XmlParser.CreateAndSelectNodeChild("data-pool-element");
  h_SaveDataElementOptArrayId(orc_Config.c_ConfiguredElementId, orc_XmlParser);
  // Return
  orc_XmlParser.SelectNodeParent();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Log file type type to string

   \param[in]  ore_Type    Type

   \return
   Stringified log file type type
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscDataLoggerJobFiler::mh_LogFileTypeTypeToString(
    const C_OscDataLoggerJobProperties::E_LogFileFormat &ore_Type) {
  QString c_Retval;

  switch (ore_Type) {
  case C_OscDataLoggerJobProperties::eLFF_CSV:
    c_Retval = "csv";
    break;
  case C_OscDataLoggerJobProperties::eLFF_PARQUET:
    c_Retval = "parquet";
    break;
  default:
    c_Retval = "invalid";
    break;
  }
  return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  String to log file type

   \param[in]   orc_String    String
   \param[out]  ore_Type      Type

   \return
   C_NO_ERR   no error
   C_RANGE    String unknown
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_StringToLogFileType(
    const QString &orc_String,
    C_OscDataLoggerJobProperties::E_LogFileFormat &ore_Type) {
  int32_t s32_Retval = C_NO_ERR;

  if (orc_String == "csv") {
    ore_Type = C_OscDataLoggerJobProperties::eLFF_CSV;
  } else if (orc_String == "parquet") {
    ore_Type = C_OscDataLoggerJobProperties::eLFF_PARQUET;
  } else {
    s32_Retval = C_RANGE;
  }

  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Local log trigger type to string

   \param[in]  ore_Type    Type

   \return
   Stringified local log trigger type
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscDataLoggerJobFiler::mh_LocalLogTriggerTypeToString(
    const C_OscDataLoggerJobProperties::E_LocalLogTrigger &ore_Type) {
  QString c_Retval;

  switch (ore_Type) {
  case C_OscDataLoggerJobProperties::eLLT_ON_CHANGE:
    c_Retval = "on-change";
    break;
  case C_OscDataLoggerJobProperties::eLLT_INTERVAL:
    c_Retval = "interval";
    break;
  default:
    c_Retval = "invalid";
    break;
  }
  return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  String to local log trigger type

   \param[in]   orc_String    String
   \param[out]  ore_Type      Type

   \return
   C_NO_ERR   no error
   C_RANGE    String unknown
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_StringToLocalLogTriggerType(
    const QString &orc_String,
    C_OscDataLoggerJobProperties::E_LocalLogTrigger &ore_Type) {
  int32_t s32_Retval = C_NO_ERR;

  if (orc_String == "on-change") {
    ore_Type = C_OscDataLoggerJobProperties::eLLT_ON_CHANGE;
  } else if (orc_String == "interval") {
    ore_Type = C_OscDataLoggerJobProperties::eLLT_INTERVAL;
  } else {
    s32_Retval = C_RANGE;
  }

  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Use case type to string

   \param[in]  ore_Type    Type

   \return
   Stringified use case type to string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscDataLoggerJobFiler::mh_UseCaseTypeToString(
    const C_OscDataLoggerJobProperties::E_UseCase &ore_Type) {
  QString c_Retval;

  switch (ore_Type) {
  case C_OscDataLoggerJobProperties::eUC_MANUAL:
    c_Retval = "manual";
    break;
  case C_OscDataLoggerJobProperties::eUC_AWS:
    c_Retval = "aws";
    break;
  case C_OscDataLoggerJobProperties::eUC_MACHINES_CLOUD:
    c_Retval = "machines-cloud";
    break;
  default:
    c_Retval = "invalid";
    break;
  }
  return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  String to use case type

   \param[in]   orc_String    String
   \param[out]  ore_Type      Type

   \return
   C_NO_ERR   no error
   C_RANGE    String unknown
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_StringToUseCaseType(
    const QString &orc_String,
    C_OscDataLoggerJobProperties::E_UseCase &ore_Type) {
  int32_t s32_Retval = C_NO_ERR;

  if (orc_String == "manual") {
    ore_Type = C_OscDataLoggerJobProperties::eUC_MANUAL;
  } else if (orc_String == "aws") {
    ore_Type = C_OscDataLoggerJobProperties::eUC_AWS;
  } else if (orc_String == "machines-cloud") {
    ore_Type = C_OscDataLoggerJobProperties::eUC_MACHINES_CLOUD;
  } else {
    s32_Retval = C_RANGE;
  }

  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
// ===== New Qt-native multi-format public methods =====
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from binary file

   \param[out] orc_Config Configuration to load into
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::h_LoadBinary(QList<C_OscDataLoggerJob> &orc_Config,
                                              const QString &orc_Path) {
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }

   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_5_15);

   // Read version
   uint16_t u16_Version = 0;
   c_Stream >> u16_Version;

   if (u16_Version != 1) {
      return C_CONFIG;
   }

   // Read count of jobs
   uint32_t u32_Count = 0;
   c_Stream >> u32_Count;

   orc_Config.clear();

   // Read jobs
   for (uint32_t i = 0; i < u32_Count; ++i) {
      C_OscDataLoggerJob c_Job;

      int32_t s32_Result = mh_LoadJobDataBinary(c_Job, c_Stream);
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }

      orc_Config.append(c_Job);
   }

   if (c_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to binary file

   \param[in] orc_Config Configuration to save
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::h_SaveBinary(const QList<C_OscDataLoggerJob> &orc_Config,
                                              const QString &orc_Path) {
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }

   QDataStream c_Stream(&c_File);
   c_Stream.setVersion(QDataStream::Qt_5_15);

   // Write version
   uint16_t u16_Version = 1;
   c_Stream << u16_Version;

   // Write count of jobs
   uint32_t u32_Count = orc_Config.size();
   c_Stream << u32_Count;

   // Write jobs
   for (const C_OscDataLoggerJob &c_Job : orc_Config) {
      int32_t s32_Result = mh_SaveJobDataBinary(c_Job, c_Stream);
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }
   }

   if (c_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from JSON file

   \param[out] orc_Config Configuration to load into
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::h_LoadJson(QList<C_OscDataLoggerJob> &orc_Config,
                                            const QString &orc_Path) {
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }

   QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll());
   if (c_Doc.isNull()) {
      return C_CONFIG;
   }

   if (!c_Doc.isObject()) {
      return C_CONFIG;
   }

   QJsonObject c_Object = c_Doc.object();

   // Look for jobs array
   if (!c_Object.contains("jobs")) {
      return C_CONFIG;
   }

   QJsonArray c_JobsArray = c_Object["jobs"].toArray();

   orc_Config.clear();

   for (const QJsonValue &c_JobValue : c_JobsArray) {
      if (!c_JobValue.isObject()) {
         return C_CONFIG;
      }

      C_OscDataLoggerJob c_Job;

      int32_t s32_Result = c_Job.FromJsonObject(c_JobValue.toObject());
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }

      orc_Config.append(c_Job);
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to JSON file

   \param[in] orc_Config Configuration to save
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::h_SaveJson(const QList<C_OscDataLoggerJob> &orc_Config,
                                            const QString &orc_Path) {
   QJsonObject c_Object;

   // Create jobs array
   QJsonArray c_JobsArray;
   for (const C_OscDataLoggerJob &c_Job : orc_Config) {
      c_JobsArray.append(c_Job.ToJsonObject());
   }

   c_Object["jobs"] = c_JobsArray;

   QJsonDocument c_Doc(c_Object);

   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }

   c_File.write(c_Doc.toJson(QJsonDocument::Compact));

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load from XML file

   \param[out] orc_Config Configuration to load into
   \param[in]  orc_Path   Path to file to load from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::h_LoadXml(QList<C_OscDataLoggerJob> &orc_Config,
                                           const QString &orc_Path) {
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly)) {
      return C_RD_WR;
   }

   QDomDocument c_Doc;
   if (!c_Doc.setContent(&c_File)) {
      return C_CONFIG;
   }

   QDomElement c_RootElement = c_Doc.documentElement();
   if (c_RootElement.tagName() != "data-logger-jobs") {
      return C_CONFIG;
   }

   orc_Config.clear();

   // Process each job element
   QDomNode c_Node = c_RootElement.firstChild();
   while (!c_Node.isNull()) {
      if (c_Node.isElement()) {
         QDomElement c_Element = c_Node.toElement();

         if (c_Element.tagName() == "job") {
            C_OscDataLoggerJob c_Job;

            int32_t s32_Result = c_Job.FromQDomElement(c_Element);
            if (s32_Result != C_NO_ERR) {
               return s32_Result;
            }

            orc_Config.append(c_Job);
         }
      }

      c_Node = c_Node.nextSibling();
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save to XML file

   \param[in] orc_Config Configuration to save
   \param[in] orc_Path   Path to file to save to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::h_SaveXml(const QList<C_OscDataLoggerJob> &orc_Config,
                                           const QString &orc_Path) {
   QDomDocument c_Doc("data-logger-jobs");
   QDomElement c_RootElement = c_Doc.createElement("data-logger-jobs");
   c_Doc.appendChild(c_RootElement);

   // Add each job
   for (const C_OscDataLoggerJob &c_Job : orc_Config) {
      QDomElement c_JobElement = c_Job.ToQDomDocument(c_Doc, "job");
      c_RootElement.appendChild(c_JobElement);
   }

   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly)) {
      return C_RD_WR;
   }

   QTextStream c_Stream(&c_File);
   c_Doc.save(c_Stream, 2);

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
// ===== New QDataStream-based private helpers (Binary suffix) =====
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load job data from QDataStream

   \param[out] orc_Config Job configuration to load into
   \param[in]  orc_Stream Stream to read from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_LoadJobDataBinary(C_OscDataLoggerJob &orc_Config,
                                                      QDataStream &orc_Stream) {
   // Load enabled flag
   orc_Stream >> orc_Config.q_IsEnabled;

   // Load properties
   int32_t s32_Result = mh_LoadJobPropertiesBinary(orc_Config.c_Properties, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }

   // Load data elements
   s32_Result = mh_LoadConfiguredDataElementsBinary(orc_Config.c_ConfiguredDataElements, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }

   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save job data to QDataStream

   \param[in] orc_Config Job configuration to save
   \param[in] orc_Stream Stream to write to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_SaveJobDataBinary(const C_OscDataLoggerJob &orc_Config,
                                                      QDataStream &orc_Stream) {
   // Save enabled flag
   orc_Stream << orc_Config.q_IsEnabled;

   // Save properties
   int32_t s32_Result = mh_SaveJobPropertiesBinary(orc_Config.c_Properties, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }

   // Save data elements
   s32_Result = mh_SaveConfiguredDataElementsBinary(orc_Config.c_ConfiguredDataElements, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }

   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load job properties from QDataStream

   \param[out] orc_Config Properties to load into
   \param[in]  orc_Stream Stream to read from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_LoadJobPropertiesBinary(C_OscDataLoggerJobProperties &orc_Config,
                                                            QDataStream &orc_Stream) {
   // Load basic properties
   orc_Stream >> orc_Config.c_Name;
   orc_Stream >> orc_Config.c_Comment;

   // Load enums as integer values
   uint32_t u32_Elem = 0;
   orc_Stream >> u32_Elem;
   orc_Config.e_UseCase = static_cast<C_OscDataLoggerJobProperties::E_UseCase>(u32_Elem);

   orc_Stream >> u32_Elem;
   orc_Config.e_LogFileFormat = static_cast<C_OscDataLoggerJobProperties::E_LogFileFormat>(u32_Elem);

   orc_Stream >> orc_Config.u32_MaxLogEntries;
   orc_Stream >> orc_Config.u32_MaxLogDurationSec;
   orc_Stream >> orc_Config.u32_LogIntervalMs;

   orc_Stream >> u32_Elem;
   orc_Config.e_LocalLogTrigger = static_cast<C_OscDataLoggerJobProperties::E_LocalLogTrigger>(u32_Elem);

   orc_Stream >> orc_Config.c_LogDestinationDirectory;

   // Load additional trigger properties
   int32_t s32_Result = mh_LoadJobAdditionalTriggerPropertiesBinary(orc_Config.c_AdditionalTriggerProperties,
                                                                    orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }

   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save job properties to QDataStream

   \param[in] orc_Config Properties to save
   \param[in] orc_Stream Stream to write to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_SaveJobPropertiesBinary(const C_OscDataLoggerJobProperties &orc_Config,
                                                            QDataStream &orc_Stream) {
   // Save basic properties
   orc_Stream << orc_Config.c_Name;
   orc_Stream << orc_Config.c_Comment;

   // Save enums as integer values
   orc_Stream << static_cast<uint32_t>(orc_Config.e_UseCase);
   orc_Stream << static_cast<uint32_t>(orc_Config.e_LogFileFormat);
   orc_Stream << orc_Config.u32_MaxLogEntries;
   orc_Stream << orc_Config.u32_MaxLogDurationSec;
   orc_Stream << orc_Config.u32_LogIntervalMs;

   orc_Stream << static_cast<uint32_t>(orc_Config.e_LocalLogTrigger);

   orc_Stream << orc_Config.c_LogDestinationDirectory;

   // Save additional trigger properties
   int32_t s32_Result = mh_SaveJobAdditionalTriggerPropertiesBinary(orc_Config.c_AdditionalTriggerProperties,
                                                                    orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }

   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load additional trigger properties from QDataStream

   \param[out] orc_Config Properties to load into
   \param[in]  orc_Stream Stream to read from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_LoadJobAdditionalTriggerPropertiesBinary(
    C_OscDataLoggerJobAdditionalTriggerProperties &orc_Config,
    QDataStream &orc_Stream) {
   // Load flag
   orc_Stream >> orc_Config.q_Enable;

   // Load element ID
   int32_t s32_Result = C_OscNodeDataPoolListElementOptArrayId::h_LoadFromStream(orc_Config.c_ElementId, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }

   // Load threshold
   s32_Result = C_OscNodeDataPoolContent::h_LoadFromStream(orc_Config.c_Threshold, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }

   // Load operation string
   orc_Stream >> orc_Config.c_Operation;

   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save additional trigger properties to QDataStream

   \param[in] orc_Config Properties to save
   \param[in] orc_Stream Stream to write to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_SaveJobAdditionalTriggerPropertiesBinary(
    const C_OscDataLoggerJobAdditionalTriggerProperties &orc_Config,
    QDataStream &orc_Stream) {
   // Save flag
   orc_Stream << orc_Config.q_Enable;

   // Save element ID
   int32_t s32_Result = C_OscNodeDataPoolListElementOptArrayId::h_SaveToStream(orc_Config.c_ElementId, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }

   // Save threshold
   s32_Result = C_OscNodeDataPoolContent::h_SaveToStream(orc_Config.c_Threshold, orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }

   // Save operation string
   orc_Stream << orc_Config.c_Operation;

   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load configured data elements from QDataStream

   \param[out] orc_Config Elements to load into
   \param[in]  orc_Stream Stream to read from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_LoadConfiguredDataElementsBinary(
    QList<C_OscDataLoggerDataElementReference> &orc_Config,
    QDataStream &orc_Stream) {
   // Load count
   uint32_t u32_Count = 0;
   orc_Stream >> u32_Count;

   orc_Config.clear();

   // Load elements
   for (uint32_t i = 0; i < u32_Count; ++i) {
      C_OscDataLoggerDataElementReference c_Element;

      int32_t s32_Result = mh_LoadConfiguredDataElementBinary(c_Element, orc_Stream);
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }

      orc_Config.append(c_Element);
   }

   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save configured data elements to QDataStream

   \param[in] orc_Config Elements to save
   \param[in] orc_Stream Stream to write to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_SaveConfiguredDataElementsBinary(
    const QList<C_OscDataLoggerDataElementReference> &orc_Config,
    QDataStream &orc_Stream) {
   // Save count
   uint32_t u32_Count = orc_Config.size();
   orc_Stream << u32_Count;

   // Save elements
   for (const C_OscDataLoggerDataElementReference &c_Element : orc_Config) {
      int32_t s32_Result = mh_SaveConfiguredDataElementBinary(c_Element, orc_Stream);
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }
   }

   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Load configured data element from QDataStream

   \param[out] orc_Config Element to load into
   \param[in]  orc_Stream Stream to read from

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_LoadConfiguredDataElementBinary(
    C_OscDataLoggerDataElementReference &orc_Config,
    QDataStream &orc_Stream) {
   // Load flags
   orc_Stream >> orc_Config.q_UseCustomName;

   // Load custom name
   orc_Stream >> orc_Config.c_CustomName;

   // Load element ID
   int32_t s32_Result = C_OscNodeDataPoolListElementOptArrayId::h_LoadFromStream(orc_Config.c_ConfiguredElementId,
                                                                                  orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }

   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Save configured data element to QDataStream

   \param[in] orc_Config Element to save
   \param[in] orc_Stream Stream to write to

   \return C_NO_ERR on success, error code otherwise
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDataLoggerJobFiler::mh_SaveConfiguredDataElementBinary(
    const C_OscDataLoggerDataElementReference &orc_Config,
    QDataStream &orc_Stream) {
   // Save flags
   orc_Stream << orc_Config.q_UseCustomName;

   // Save custom name
   orc_Stream << orc_Config.c_CustomName;

   // Save element ID
   int32_t s32_Result = C_OscNodeDataPoolListElementOptArrayId::h_SaveToStream(orc_Config.c_ConfiguredElementId,
                                                                                orc_Stream);
   if (s32_Result != C_NO_ERR) {
      return s32_Result;
   }

   if (orc_Stream.status() != QDataStream::Ok) {
      return C_RD_WR;
   }

   return C_NO_ERR;
}
