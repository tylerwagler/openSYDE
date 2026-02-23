//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Utility functions for system filers (implementation)

   Utility functions for system filers

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"
#include <QDir>
#include <QFileInfo>

#include <cstdio>

#include "C_OscLoggingHandler.hpp"
#include "C_OscSystemFilerUtil.hpp"
#include "C_OscUtils.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::opensyde_core;

using namespace stw::errors;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */

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
/*! \brief   Convert bus type enum to string

   \param[in]  oe_Type  Bus type

   \return  string representation of oe_Type
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscSystemFilerUtil::h_BusTypeEnumToString(
    const C_OscSystemBus::E_Type oe_Type) {
  QString c_Retval;

  if (oe_Type == C_OscSystemBus::eETHERNET) {
    c_Retval = "ethernet";
  } else {
    c_Retval = "can";
  }
  return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Convert bus type string to enum

   \param[in]   orc_Type   type string
   \param[out]  ore_Type   Enum corresponding to type

   \return
   C_NO_ERR   no error
   C_RANGE    String unknown
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t
C_OscSystemFilerUtil::h_BusTypeStringToEnum(const QString &orc_Type,
                                            C_OscSystemBus::E_Type &ore_Type) {
  int32_t s32_Retval = C_NO_ERR;

  if (orc_Type == "ethernet") {
    ore_Type = C_OscSystemBus::eETHERNET;
  } else if (orc_Type == "can") {
    ore_Type = C_OscSystemBus::eCAN;
  } else {
    osc_write_log_error("Loading System Definition",
                        "Invalid value for bus.\"type\":" + orc_Type);
    s32_Retval = C_RANGE;
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get parser at root file node and handle all possible errors

   Warning: includes error logging

   \param[in,out]  orc_FileXmlParser   XML parser
   \param[in]      orc_Path            File path
   \param[in]      orc_RootNode        Root node name

   \return
   C_NO_ERR   data was read from file
   C_NOACT    could not read data from file
   C_RANGE    file not found
   C_CONFIG   root node not found
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemFilerUtil::h_GetParserForExistingFile(
    C_OscXmlParser &orc_FileXmlParser, const QString &orc_Path,
    const QString &orc_RootNode) {
  int32_t s32_Retval = C_NO_ERR;

  if (QFileInfo(orc_Path).exists() && QFileInfo(orc_Path).isFile()) {
    s32_Retval = orc_FileXmlParser.LoadFromFile(orc_Path);
    if (s32_Retval == C_NO_ERR) {
      if (orc_FileXmlParser.SelectRoot() != orc_RootNode) {
        s32_Retval = C_CONFIG;
        osc_write_log_error("Loading files",
                            "Unexpected content for file \"" + orc_Path + "\"");
      }
    } else {
      osc_write_log_error("Loading files",
                          "Failure parsing file \"" + orc_Path + "\"");
    }
  } else {
    osc_write_log_error("Loading files", "Missing file \"" + orc_Path + "\"");
    s32_Retval = C_RANGE;
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get parser at root file node and handle all possible errors

   Warning: includes error logging

   \param[in,out]  orc_FileXmlParser   XML parser
   \param[in]      orc_Path            File path
   \param[in]      orc_RootNode        Root node name

   \return
   C_NO_ERR   XML handle was created
   C_NOACT    existing file could not be deleted
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t
C_OscSystemFilerUtil::h_GetParserForNewFile(C_OscXmlParser &orc_FileXmlParser,
                                            const QString &orc_Path,
                                            const QString &orc_RootNode) {
  int32_t s32_Retval = C_NO_ERR;

  if (QFileInfo(orc_Path).exists() && QFileInfo(orc_Path).isFile()) {
    if (QFile::remove(orc_Path)) {
      s32_Retval = C_NO_ERR;
    } else {
      s32_Retval = C_NOACT;
      osc_write_log_error("Saving files",
                          "Could not delete file \"" + orc_Path + "\"");
    }
  }
  orc_FileXmlParser.CreateAndSelectNodeChild(orc_RootNode);
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create directory

   Warning: includes error logging

   \param[in]  orc_Path    Folder path

   \return
   C_NO_ERR   XML handle was created
   C_NOACT    folder could not be created
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemFilerUtil::h_CreateFolder(const QString &orc_Path) {
  int32_t s32_Retval;

  if (QFileInfo(orc_Path).isDir()) {
    s32_Retval = C_NO_ERR;
  } else {
    if (QDir().mkpath(orc_Path)) {
      s32_Retval = C_NO_ERR;
    } else {
      s32_Retval = C_NOACT;
      osc_write_log_error("Saving files",
                          "Could not create directory \"" + orc_Path + "\"");
    }
  }

  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Adapt item name for file name usage

   \param[in]  orc_ItemName   Item name

   \return
   Item name ready for file name usage
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscSystemFilerUtil::h_PrepareItemNameForFileName(
    const QString &orc_ItemName) {
  return C_OscUtils::h_NiceifyStringForFileName(orc_ItemName.toLower());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Combine the base path and a relative sub folder path

   \param[in]  orc_BasePathName        Base path name
   \param[in]  orc_SubFolderFileName   Sub folder directory and file name

   \return
   Full, combined path
*/
//----------------------------------------------------------------------------------------------------------------------
QString
C_OscSystemFilerUtil::h_CombinePaths(const QString &orc_BasePathName,
                                     const QString &orc_SubFolderFileName) {
  const QString c_BasePath = QFileInfo(orc_BasePathName).absolutePath() + "/";

  return c_BasePath + orc_SubFolderFileName;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save string to file

   Will overwrite the file if it already exists.

   \param[in]  orc_CompleteFileAsString   Complete file as string
   \param[in]  orc_CompleteFilePath       Complete file path
   \param[in]  orc_LogHeading             Log heading

   \return
   STW error codes

   \retval   C_NO_ERR   data saved
   \retval   C_RD_WR    could not erase pre-existing file before saving
   \retval   C_RD_WR    could not write to file (e.g. missing write permissions;
   missing folder)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemFilerUtil::h_SaveStringToFile(
    const QString &orc_CompleteFileAsString,
    const QString &orc_CompleteFilePath, const QString &orc_LogHeading) {
  int32_t s32_Retval = C_NO_ERR;

  const QString c_Folder = QFileInfo(orc_CompleteFilePath).absolutePath() + "/";

  if (!QFileInfo(c_Folder).isDir()) {
    if (!QDir().mkpath(c_Folder)) {
      osc_write_log_error(orc_LogHeading,
                          "Could not create folder \"" + c_Folder + "\".");
      s32_Retval = C_RD_WR;
    }
  }

  if (s32_Retval == C_NO_ERR) {
    // Write (erase if file exists)
    QFile c_File(orc_CompleteFilePath);
    if (c_File.open(QIODevice::WriteOnly | QIODevice::Truncate |
                    QIODevice::Text)) {
      bool q_HasFailed;

      c_File.write(orc_CompleteFileAsString.toUtf8());
      c_File.close();
      if (c_File.size() != orc_CompleteFileAsString.length()) {
        osc_write_log_error(orc_LogHeading,
                            "Could not create or overwrite file \"" +
                                orc_CompleteFilePath + "\".");
        s32_Retval = C_RD_WR;
      }
    } else {
      osc_write_log_error(orc_LogHeading, "Could not write to file \"" +
                                              orc_CompleteFilePath + "\".");
      s32_Retval = C_RD_WR;
    }
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Adapt project path to system definition file path (*.syde_sysdef)

   \param[in]   orc_ProjectPath           Project path
   \param[out]  orc_SystemDefintionPath   System defintion path
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSystemFilerUtil::h_AdaptProjectPathToSystemDefinition(
    const QString &orc_ProjectPath, QString &orc_SystemDefintionPath) {
  const QFileInfo c_FileInfo(orc_ProjectPath);
  const QString c_BaseName = c_FileInfo.completeBaseName();
  orc_SystemDefintionPath = c_FileInfo.absolutePath() + "/system_definition/" +
                            c_BaseName + ".syde_sysdef";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Adapt project path to system views file path (*.syde_sysviews)

   \param[in]   orc_ProjectPath        Project path
   \param[out]  orc_SystemViewsPath    System views path
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSystemFilerUtil::h_AdaptProjectPathToSystemViews(
    const QString &orc_ProjectPath, QString &orc_SystemViewsPath) {
  const QFileInfo c_FileInfo(orc_ProjectPath);
  const QString c_BaseName = c_FileInfo.completeBaseName();
  orc_SystemViewsPath = c_FileInfo.absolutePath() + "/system_views/" +
                        c_BaseName + ".syde_sysviews";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Transform export scaling support type to string

   \param[in]  ore_Scaling    Flash loader type

   \return
   Stringified export scaling support type
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscSystemFilerUtil::h_CodeExportScalingTypeToString(
    const C_OscNodeCodeExportSettings::E_Scaling &ore_Scaling) {
  QString c_Retval;

  switch (ore_Scaling) {
  case C_OscNodeCodeExportSettings::eFLOAT32:
    c_Retval = "float32";
    break;
  case C_OscNodeCodeExportSettings::eFLOAT64:
    c_Retval = "float64";
    break;
  case C_OscNodeCodeExportSettings::eNONE:
    c_Retval = "none";
    break;
  default:
    c_Retval = "invalid";
    break;
  }

  return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Transform string to code export scaling support type.

   \param[in]   orc_String    String to interpret
   \param[out]  ore_Scaling   Scaling support type

   \return
   C_NO_ERR   no error
   C_RANGE    String unknown
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemFilerUtil::h_StringToCodeExportScalingType(
    const QString &orc_String,
    C_OscNodeCodeExportSettings::E_Scaling &ore_Scaling) {
  int32_t s32_Retval = C_NO_ERR;

  if (orc_String == "float32") {
    ore_Scaling = C_OscNodeCodeExportSettings::eFLOAT32;
  } else if (orc_String == "float64") {
    ore_Scaling = C_OscNodeCodeExportSettings::eFLOAT64;
  } else if (orc_String == "none") {
    ore_Scaling = C_OscNodeCodeExportSettings::eNONE;
  } else {
    osc_write_log_error(
        "Loading node definition",
        "Invalid value for "
        "\"properties\".\"code-export-settings\".\"scaling-support\": " +
            orc_String);
    s32_Retval = C_RANGE;
  }

  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check version

   \param[in,out]  orc_XmlParser             Xml parser
   \param[in]      ou16_ExpectedFileVersion  Expected file version
   \param[in]      orc_TagName               Tag name
   \param[in]      orc_UseCase               Use case

   \return
   C_NO_ERR   no error
   C_CONFIG   Version error
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemFilerUtil::h_CheckVersion(
    C_OscXmlParserBase &orc_XmlParser, const uint16_t ou16_ExpectedFileVersion,
    const QString &orc_TagName, const QString &orc_UseCase) {
  int32_t s32_Retval = orc_XmlParser.SelectNodeChildError(orc_TagName);

  // File version
  if (s32_Retval == C_NO_ERR) {
    uint16_t u16_FileVersion = 0U;
    try {
      u16_FileVersion =
          static_cast<uint16_t>(orc_XmlParser.GetNodeContent().toInt());
    } catch (...) {
      orc_XmlParser.ReportErrorForNodeContentStartingWithXmlContext(
          "could not be converted to a number");
      s32_Retval = C_CONFIG;
    }

    // is the file version one we know ?
    if (s32_Retval == C_NO_ERR) {
      osc_write_log_info(orc_UseCase, "Value of \"" + orc_TagName + "\": " +
                                          QString::number(u16_FileVersion));
      // Check file version
      if (u16_FileVersion != ou16_ExpectedFileVersion) {
        orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext(
            orc_TagName, "Unsupported version defined");
        s32_Retval = C_CONFIG;
      }
    }

    // Return
    orc_XmlParser.SelectNodeParent();
  }
  return s32_Retval;
}
