//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE utility functions (implementation)

   openSYDE utility functions

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscUtils.hpp"
#include "C_SclResourceStrings.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QString>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <iterator>
#include <limits>

#define STR_TABLE_INCLUDE // we really want the symbols from the DLStrings.h
                          // header
#include "C_OscLoggingHandler.hpp"
#include "DLStrings.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
using namespace stw::opensyde_core;

using namespace stw::errors;
using namespace stw::scl;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */
const float64_t C_OscUtils::mhf64_EPSILON = 1e-5;
stw::scl::C_SCLResourceStrings C_OscUtils::mhc_ResourceStrings;

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */
const QString C_OscUtils::hc_PATH_VARIABLE_OPENSYDE_BIN = "%{OPENSYDE_BINARY}";
const QString C_OscUtils::hc_PATH_VARIABLE_OPENSYDE_PROJ =
    "%{OPENSYDE_PROJECT}";
const QString C_OscUtils::hc_PATH_VARIABLE_DATABLOCK_PROJ = "%{PROJECT_DIR}";
const QString C_OscUtils::hc_PATH_VARIABLE_USER_NAME = "%{USER_NAME}";
const QString C_OscUtils::hc_PATH_VARIABLE_COMPUTER_NAME = "%{COMPUTER_NAME}";

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
/*! \brief  CheckValidCName

   Function adapted from KEFEX to openSYDE (KFXCheckValidCName)
   Check if orc_Name follows C language naming conventions:
   -> must not be empty
   -> only alphanumeric characters + "_"
   -> should not be longer than "ou16_MaxLength" characters

   \param[in]  orc_Name                         symbol name to check
   \param[in]  oq_AutomaticCeStringAdaptation   if automatic c string adaptation
   true or false
   \param[in]  ou16_MaxLength                   permitted maximum identifier
   length

   \return
   true  -> OK
   false -> violation of rules
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscUtils::h_CheckValidCeName(const QString &orc_Name,
                                    const bool oq_AutomaticCeStringAdaptation,
                                    const uint16_t ou16_MaxLength) {
  char_t cn_Char;
  bool q_IsValid = true;

  if (orc_Name.length() == 0) {
    q_IsValid = false;
  } else {
    uint32_t u32_Index;
    // -> only alphanumeric characters + "_"
    if (oq_AutomaticCeStringAdaptation == false) {
      // no automatic c string adaptation
      for (u32_Index = 0; u32_Index < orc_Name.length(); u32_Index++) {
        // first char of name
         const QChar cn_Char = orc_Name[u32_Index];
        // is alphanumeric and no underscore true or a number true -> invalid
        // name

        // Use Qt-native character classification
        if ((!cn_Char.isLetterOrNumber() && (cn_Char != '_')) ||
            (u32_Index == 0 && !cn_Char.isDigit()))) // Qt-compliant check
        {
          q_IsValid = false;
          break;
        }
      }
    } else {
      // automatic c string adaptation
      for (u32_Index = 0; u32_Index < orc_Name.length(); u32_Index++) {
        // fist char of name
         const QChar cn_Char = orc_Name[u32_Index];

        // is alphanumeric true or no underscore and a number true -> invalid
        // name

        // Use Qt-native character classification
        if ((!cn_Char.isLetterOrNumber() && (cn_Char != '_')) ||
            (u32_Index == 0 && !cn_Char.isDigit()))) // Qt-compliant check
        {
          q_IsValid = false;
          break;
        }
      }
    }

    if (q_IsValid == true) {
      // -> should not be longer than ou16_MaxLength characters
      if (orc_Name.length() > static_cast<uint32_t>(ou16_MaxLength)) {
        q_IsValid = false;
      }
    }
  }

  return q_IsValid;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Compare to float64s for near equality

   \param[in]  orf64_Float1   Float 1 to compare
   \param[in]  orf64_Float2   Float 2 to compare

   \return
   true  Equal
   false Not equal
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscUtils::h_IsFloat64NearlyEqual(const float64_t &orf64_Float1,
                                        const float64_t &orf64_Float2) {
  // From Marshall Cline's C++ FAQ Lite document
  return std::abs(orf64_Float1 - orf64_Float2) <=
         (C_OscUtils::mhf64_EPSILON * std::abs(orf64_Float1));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Compare to float32s for near equality

   \param[in]  orf32_Float1   Float 1 to compare
   \param[in]  orf32_Float2   Float 2 to compare

   \return
   true  Equal
   false Not equal
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscUtils::h_IsFloat32NearlyEqual(const float32_t &orf32_Float1,
                                        const float32_t &orf32_Float2) {
  // From Marshall Cline's C++ FAQ Lite document
  return std::abs(orf32_Float1 - orf32_Float2) <=
         (static_cast<float32_t>(C_OscUtils::mhf64_EPSILON) *
          std::abs(orf32_Float1));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  create target folder (from bottom-up if required)

   Nice and simple logic grabbed from
    https://stackoverflow.com/questions/1530760/how-do-i-recursively-create-a-folder-in-win32

   \param[in]  orc_Folder  Path to create

   \return
   C_NO_ERR  folder created
   C_NOACT   could not create folder
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscUtils::h_CreateFolderRecursively(const QString &orc_Folder) {
  // lint -e{8080} //using type expected by the library for compatibility
  size_t x_CharIndex = 0U;
  int32_t s32_Return = C_NO_ERR;

  const QString c_Path = orc_Folder;

  do {
    x_CharIndex = c_Path.indexOf("\\/", x_CharIndex + 1);

    QString c_PartialPath = c_Path.left(x_CharIndex);
    s32_Return = QDir().mkpath(c_PartialPath) ? 0 : -1;
    if (s32_Return != 0) {
      s32_Return = C_NOACT;
    }
  } while ((x_CharIndex != -1) && (s32_Return == C_NO_ERR));
  return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Replace special characters in string

   Aims:
   * convert into strings that can be used for file system folder names or file
   names
   * prevent accidentally rendering unique names identical
     (e.g.: "ITEM!§" and "ITEM%&" should not result in the same string)

   As different file system have different valid characters this function only
   keeps commonly permitted characters.

   All characters but the following will be replaced by their ASCII codes:
   * a..z
   * A..Z
   * 0..9
   * _
   * -
   * (
   * )
   * {
   * }
   * $
   * %
   * &
   * !
   * #
   * +
   * ,
   * ;
   * =
   * @
   * [
   * ]
   * ^
   * '
   * ~
   * . (unusual but permitted)
   * <space> (unusual but permitted)

   See https://en.wikipedia.org/wiki/Filename for a list of limitations.

   Special handling will also be applied to the strings "." and "..":
   These will be replaced by "dot" resp. "doubledot".
   Furthermore empty or blank strings (e.g. "", " ", "  ") will be replaced by
   "blank".

   As a result the length of the string might change.

   \param[in]  orc_String  Original string

   \return
   Niceified string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscUtils::h_NiceifyStringForFileName(const QString &orc_String) {
  QString c_Result;
  bool q_Blank = true;

  // check fringe cases; "." and ".." have special meaning in most file systems
  // and are no valid directory names
  if (orc_String == ".") {
    c_Result = "dot";
  } else if (orc_String == "..") {
    c_Result = "doubledot";
  } else {
    for (uint32_t u32_Index = 0U; u32_Index < orc_String.length();
         u32_Index++) {
       const QChar cn_Character = orc_String[u32_Index];

       // Use Qt-native character classification
       if (!cn_Character.isLetterOrNumber() &&
           (cn_Character != '_') &&
           (cn_Character != '-') &&
           (cn_Character != '(') &&
           (cn_Character != ')') &&
           (cn_Character != '{') &&
           (cn_Character != '}') &&
           (cn_Character != '$') &&
           (cn_Character != '.') &&
           (cn_Character != ' ') &&
           (cn_Character != '%') &&
           (cn_Character != '&') &&
           (cn_Character != '!') &&
           (cn_Character != '#') &&
           (cn_Character != '+') &&
           (cn_Character != ',') &&
           (cn_Character != ';') &&
           (cn_Character != '=') &&
           (cn_Character != '@') &&
           (cn_Character != '[') &&
           (cn_Character != ']') &&
           (cn_Character != '^') &&
           (cn_Character != '\'') &&
           (cn_Character != '~')) {
         c_Result += QString::number(static_cast<int>(cn_Character.unicode()));
       } else {
         c_Result += cn_Character;

         if (cn_Character != ' ') {
           q_Blank = false;
         }
       }
    }
  }

  if (q_Blank == true) {
    c_Result = "blank";
  }

  return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Replace non-C characters in string

   Aims:
   * convert a text string so that it can be used as a comment in C code
   * also prevent star-backslash combinations in order to not accidentally
   terminate the comment

   Strategy:
   * all non-printable C characters are replaced by "_" (this also will remove
   nl,cr,tab)
   * "*" is replaced by "_" if immediately followed by "/"
   * "\" is replaced by "_" if at the end of the string

   \param[in]  orc_String  Original string

   \return
   Niceified string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscUtils::h_NiceifyStringForCeComment(const QString &orc_String) {
  QString c_Result = orc_String;

  for (int32_t s32_Index = 0; s32_Index < c_Result.length(); s32_Index++) {
    const QChar c_Character = c_Result[s32_Index];
    const char cn_Char = c_Character.toLatin1();
    const int32_t s32_NextIndex = s32_Index + 1;

    // If the value of the character is not representable as unsigned char the
    // the behavior of isprint is undefined. So be as defensive as possible:
    if ((static_cast<int8_t>(cn_Char) < 0) || (std::isprint(cn_Char) == 0) ||
        (c_Character == '@') || (c_Character == '`')) {
      c_Result[s32_Index] = '_';
    } else if ((s32_NextIndex < c_Result.length()) && (c_Character == '*') &&
               (c_Result[s32_NextIndex] == '/')) {
      // prevent adding end of C comment
      c_Result[s32_Index] = '_';
    } else if ((s32_Index == (c_Result.length() - 1)) &&
               (c_Character == '\\')) {
      // prevent continuing C++ comment
      c_Result[s32_Index] = '_';
    } else {
      // Nothing to do
    }
  }
  return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if string is valid for a file name by comparing it with result
   of h_NiceifyStringForFileName.

   See description of h_NiceifyStringForFileName. If the string has at least one
   character which would be replaced by h_NiceifyStringForFileName, the string
   is invalid and therefore the function returns false.

   For checking whole paths use h_CheckValidFileName, because here slashes and
   colons are not allowed.

   \param[in]  orc_String  File path name

   \retval   true    The string is valid
   \retval   false   The string is not valid
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscUtils::h_CheckValidFileName(const QString &orc_String) {
  bool q_Return = true;
  const QString c_Temp = h_NiceifyStringForFileName(orc_String);

  if (c_Temp != orc_String) {
    q_Return = false;
  }

  return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if string is valid for file or directory paths.

   Splits the provided string at '/' and '\'and uses h_CheckValidFileName for
   these sub-strings. If the sub-string is a double dot ".." it is skipped
   because of its special meaning (directory up) in path context If the provided
   string starts with a drive name (C:) or with a single dot "." these
   sub-strings are skipped from file name check as they are valid in their
   special context.

   Empty paths are handled invalid because an empty file path is not valid for
   file saving. Same rule applies if the path only contains (back-)slashes.
   Multiple slashes in direct sequence are handled as valid if there exists at
   least one further character.

   \param[in]  orc_String  File path string

   \retval   true    The string is valid for file or directory path
   \retval   false   The string invalid
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscUtils::h_CheckValidFilePath(const QString &orc_String) {
  bool q_Return = true;

  if (orc_String.isEmpty() == true) {
    q_Return = false;
  } else {
    bool q_AtLeastOneOtherChar = false;
    QStringList c_SplitStrings =
        orc_String.split(QRegularExpression("[\\\\/]"), Qt::SkipEmptyParts);

    for (int32_t s32_Index = 0U;
         (s32_Index < c_SplitStrings.size()) && (q_Return == true);
         s32_Index++) {
      const QString &rc_Substring = c_SplitStrings[s32_Index];

      // empty sub-strings are okay here, because they result from two
      // consecutive (back-)slashes or from trailing/leading(back-)slash
      if (rc_Substring.isEmpty() == false) {
        q_AtLeastOneOtherChar = true;

        // skip ".." as it has special meaning in paths but is not allowed for
        // names
        if (rc_Substring != "..") {
          // skip if complete string starts with drive name or "."
          if ((s32_Index > 0) ||
              (rc_Substring != ".")) /* . at beginning is allowed */
          {
            if ((rc_Substring.length() != 2U) ||
                (rc_Substring.mid(1, 1) != ':')) /* drive names e.g. C: */
            {
              q_Return = h_CheckValidFileName(rc_Substring);
            }
          }
        }
      }
    }

    // only (back-)slashes found
    if (q_AtLeastOneOtherChar == false) {
      q_Return = false;
    }
  }

  return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if scaling active

   \param[in]  of64_Factor    Factor
   \param[in]  of64_Offset    Offset

   \return
   True  Scaling active
   False Scaling inactive
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscUtils::h_IsScalingActive(const float64_t of64_Factor,
                                   const float64_t of64_Offset) {
  bool q_Return =
      (C_OscUtils::h_IsFloat64NearlyEqual(of64_Factor, 1.0) == false);

  q_Return = (q_Return ||
              (C_OscUtils::h_IsFloat64NearlyEqual(of64_Offset, 0.0) == false));

  return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Scales a value

   returns ((value * factor) + offset)

   \param[in]  of64_Value                 Original unscaled value
   \param[in]  of64_Factor                Scaling factor
   \param[in]  of64_Offset                Scaling offset
   \param[in]  oq_AllowRangeAdaptation    Allow range adaptation

   \return
   Scaled value
*/
//----------------------------------------------------------------------------------------------------------------------
float64_t C_OscUtils::h_GetValueScaled(const float64_t of64_Value,
                                       const float64_t of64_Factor,
                                       const float64_t of64_Offset,
                                       const bool oq_AllowRangeAdaptation) {
  float64_t f64_Result;

  f64_Result = of64_Value * of64_Factor;
  f64_Result += of64_Offset;

  if (oq_AllowRangeAdaptation == true) {
    h_RangeCheckFloat(f64_Result);
  }

  return f64_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates a scaled value back to the unscaled value

   returns ((value - offset) / factor)

   \param[in]  of64_Value     Scaled value
   \param[in]  of64_Factor    Scaling factor
   \param[in]  of64_Offset    Scaling offset

   \return
   Origin value
*/
//----------------------------------------------------------------------------------------------------------------------
float64_t C_OscUtils::h_GetValueUnscaled(const float64_t of64_Value,
                                         const float64_t of64_Factor,
                                         const float64_t of64_Offset) {
  float64_t f64_Result;

  f64_Result = of64_Value - of64_Offset;
  f64_Result /= of64_Factor;

  return f64_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Convert POS serial number array to string

   Converting the POS (Plain Old Serial) STW format in form of 6 BCD coded bytes
   to a string.

   Support of two serial number variants of the formats:
      1: format up to and including 2019. E.g: 05.123456.1001
      2: format from 2020. E.g: 200012345678

   \param[in]  opu8_SerialNumber    Pointer to first of six serial number array
   elements

   \return
   serial number string
*/
//----------------------------------------------------------------------------------------------------------------------
QString
C_OscUtils::h_PosSerialNumberToString(const uint8_t *const opu8_SerialNumber) {
  QString c_Result;

  if (opu8_SerialNumber != NULL) {
    if (opu8_SerialNumber[0] < static_cast<uint8_t>(0x20)) {
      // format up to and including 2019. E.g: 05.123456.1001
      c_Result = QString::asprintf("%02X.%02X%02X%02X.%02X%02X",
                                   static_cast<uint32_t>(opu8_SerialNumber[0]),
                                   static_cast<uint32_t>(opu8_SerialNumber[1]),
                                   static_cast<uint32_t>(opu8_SerialNumber[2]),
                                   static_cast<uint32_t>(opu8_SerialNumber[3]),
                                   static_cast<uint32_t>(opu8_SerialNumber[4]),
                                   static_cast<uint32_t>(opu8_SerialNumber[5]));
    } else {
      // format from 2020. E.g: 200012345678
      c_Result = QString::asprintf("%02X%02X%02X%02X%02X%02X",
                                   static_cast<uint32_t>(opu8_SerialNumber[0]),
                                   static_cast<uint32_t>(opu8_SerialNumber[1]),
                                   static_cast<uint32_t>(opu8_SerialNumber[2]),
                                   static_cast<uint32_t>(opu8_SerialNumber[3]),
                                   static_cast<uint32_t>(opu8_SerialNumber[4]),
                                   static_cast<uint32_t>(opu8_SerialNumber[5]));
    }
  }

  return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Convert FSN serial number raw string to a formatted string

   Converting the FSN (Flexible Serial Number) format in form of the defined
   ou8_ManufacturerFormat orc_RawSerialNumber must have at least one byte and
   the maximum length is 29 byte

   \param[in]  ou8_ManufacturerFormat    Manufacturer format which defines the
   type of serial number: 0:      STW POS format 1..255: reserved
   \param[in]  orc_RawSerialNumber       Unedited raw serial number

   \retval   string        formatted serial number string
   \retval   empty string  if length of orc_RawSerialNumber does not match the
   expectations
*/
//----------------------------------------------------------------------------------------------------------------------
QString
C_OscUtils::h_FsnSerialNumberToString(const uint8_t ou8_ManufacturerFormat,
                                      const QString &orc_RawSerialNumber) {
  QString c_Result;

  if ((orc_RawSerialNumber.length() > 0) &&
      (orc_RawSerialNumber.length() <= 29)) {
    if (ou8_ManufacturerFormat == 0U) {
      // Must match exactly
      if (orc_RawSerialNumber.length() == 6) {
        // STW POS format
        // lint -e{9176} //no problems as long as charn has the same size as
        // uint8; if not we'd be in deep !"=?&
        // anyway
         c_Result = C_OscUtils::h_PosSerialNumberToString(
             reinterpret_cast<const uint8_t *>(
                 orc_RawSerialNumber.toUtf8().constData()));
      }
    } else {
      // No concrete formats defined yet
      c_Result = orc_RawSerialNumber;
    }
  }

  return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Copy file to string

   \param[in]   orc_FilePath        File to copy
   \param[out]  orc_OutputString    File content
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscUtils::h_FileToString(const QString &orc_FilePath,
                                QString &orc_OutputString) {
  QFile c_File(orc_FilePath);
  if (c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
    // Read the entire file content
    orc_OutputString = c_File.readAll();
    c_File.close();
  } else {
    // File couldn't be opened, leave orc_OutputString empty
    orc_OutputString.clear();
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Range check float

   \param[out]  orf64_Value   Value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscUtils::h_RangeCheckFloat(float64_t &orf64_Value) {
  orf64_Value = std::min(orf64_Value, std::numeric_limits<float64_t>::max());
  orf64_Value = std::max(orf64_Value, -std::numeric_limits<float64_t>::max());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   load resource string

   Load resource strings from resource table.
   We use a string table using the C_SCLResourceStrings class
    and fill a singleton of it with the application strings.

   Note: this is only useful for application using an numeric-index-based
   localization approach. This is for example not compatible with
   string-index-based approaches like gettext.

   \param[in]    ou16_StringIndex     Index of string

   \return
   string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscUtils::h_LoadString(const uint16_t ou16_StringIndex) {
  static bool hq_Initialized = false;

  if (hq_Initialized == false) {
    mhc_ResourceStrings.SetStringTable(&gac_DIAG_LIB_RESOURCE_STRINGS[0],
                                       gu16_DIAGLIB_NR_RES_STRNGS);
    hq_Initialized = true;
  }

  return mhc_ResourceStrings.LoadStr(ou16_StringIndex);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Copies a file from source to target folder (internal function).

   Assumptions:
   * read permission of source folder
   * write permission of target folder

   \param[in]  orc_SourceFile            source file (full path required)
   \param[in]  orc_TargetFile            target file (        -"-       )
   \param[out] opc_ErrorPath             if != NULL and the function fails:
                                          file path (source or target) that
   caused the problem
   \param[out] opc_ErrorMessage          if != NULL and the function fails:
                                          error message that caused the problem

   \return
   C_NO_ERR    success
   C_RD_WR     read/write error (see log file for details)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscUtils::h_CopyFile(const QString &orc_SourceFile,
                                const QString &orc_TargetFile,
                                QString *const opc_ErrorPath,
                                QString *const opc_ErrorMessage) {
  int32_t s32_Return = C_NO_ERR;
  QString c_ErrorMessage = "";

  QFile c_SourceFile(orc_SourceFile);
  if (!c_SourceFile.open(QIODevice::ReadOnly)) {
    c_ErrorMessage = "Could not read \"" + orc_SourceFile + "\".";
    osc_write_log_error("Copying file",
                        c_ErrorMessage.toLocal8Bit().constData());
    s32_Return = C_RD_WR;
    if (opc_ErrorPath != NULL) {
      *opc_ErrorPath = orc_SourceFile;
    }
  } else {
    // Read all data from source file
    QByteArray c_Data = c_SourceFile.readAll();
    c_SourceFile.close();

    QFile c_TargetFile(orc_TargetFile);
    if (!c_TargetFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      c_ErrorMessage = "Could not write \"" + orc_TargetFile + "\".";
      osc_write_log_error("Copying file",
                          c_ErrorMessage.toLocal8Bit().constData());
      s32_Return = C_RD_WR;
      if (opc_ErrorPath != NULL) {
        *opc_ErrorPath = orc_TargetFile;
      }
    } else {
      // Write data to target file
      if (c_TargetFile.write(c_Data) != c_Data.size()) {
        c_ErrorMessage =
            "Could not write stream of \"" + orc_TargetFile + "\".";
        osc_write_log_error("Copying file",
                            c_ErrorMessage.toLocal8Bit().constData());
        s32_Return = C_RD_WR;
        if (opc_ErrorPath != NULL) {
          *opc_ErrorPath = orc_TargetFile;
        }
      }
      c_TargetFile.close();
    }
  }

  if ((s32_Return != C_NO_ERR) && (opc_ErrorMessage != NULL)) {
    *opc_ErrorMessage = c_ErrorMessage;
  }

  return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get command line call including arguments in one string, e.g. for
   log file entry

   \param[in]  os32_Argc   number of command line arguments
   \param[in]  oppcn_Argv  command line arguments

   \return
   Command line arguments as string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscUtils::h_GetCommandLineAsString(const int32_t os32_Argc,
                                             char_t *const *const oppcn_Argv) {
  QString c_CommandLine;

  for (int32_t s32_Argument = 0; s32_Argument < os32_Argc; s32_Argument++) {
    c_CommandLine += oppcn_Argv[s32_Argument];
    if (s32_Argument != (os32_Argc - 1)) {
      c_CommandLine += " ";
    }
  }

  return c_CommandLine;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get rid of Data Block project path.

   If the path contains mc_PATH_VARIABLE_DATABLOCK_PROJ we replace this with the
   project path, but do not resolve any other placeholder variable. If the path
   is relative it is meant as relative to the Data Block project path, so we
   concatenate these paths.

   This might result in invalid paths if the placeholder variable is not in
   front of string but an absolute path (which would be a misconfiguration).

   \param[in]  orc_DbProjectPath       path for resolving data block project
   variable and concatenation
   \param[in]  orc_OsydeProjectPath    path to opened openSYDE project without
   file name (for resolving hc_PATH_VARIABLE_OPENSYDE_PROJ references)
   \param[in]  orc_Path                path that probably contains variables

   \return
   Path without Data Block project path dependencies (might still contain
   placeholder variables or be relative)
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscUtils::h_MakeIndependentOfDbProjectPath(
    const QString &orc_DbProjectPath, const QString &orc_OsydeProjectPath,
    const QString &orc_Path) {
  QString c_Return = orc_Path;

  const uint32_t u32_Pos = c_Return.indexOf(hc_PATH_VARIABLE_DATABLOCK_PROJ);

  if (u32_Pos != 0U) {
    c_Return.replace(hc_PATH_VARIABLE_DATABLOCK_PROJ, orc_DbProjectPath);

    // remove all double slashes but the first (network paths)
    if (c_Return.indexOf("//") == 1) {
      c_Return = '/' + c_Return;
    }
    c_Return.replace("//", "/");
  } else {
    // concatenate if placeholder-resolved path would be relative
    const QString c_ResolvedPath = C_OscUtils::h_ResolvePlaceholderVariables(
        orc_Path, orc_OsydeProjectPath);

    if (QDir::isRelativePath(c_ResolvedPath) == true) {
      // relative path
      c_Return =
          C_OscUtils::h_ConcatPathIfNecessary(orc_DbProjectPath, c_Return);
    }
  }

  return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if path contains placeholder variables (e.g.
   %{OPENSYDE_PROJECT}) and resolve them.

   \param[in]  orc_Path              path that probably contains variables
   \param[in]  orc_OsydeProjectPath  path to opened openSYDE project without
   file name (for resolving hc_PATH_VARIABLE_OPENSYDE_PROJ references)
   \param[in]  orc_DbProjectPath     path for resolving data block project
   variable (special case), which might contain placeholder variables itself

   \return
   Resolved path
*/
//----------------------------------------------------------------------------------------------------------------------
QString
C_OscUtils::h_ResolvePlaceholderVariables(const QString &orc_Path,
                                          const QString &orc_OsydeProjectPath,
                                          const QString &orc_DbProjectPath) {
  QString c_Return = orc_Path;

  // first check for indicator %
  uint32_t u32_Pos = c_Return.indexOf("%");

  if (u32_Pos != 0U) {
    // replace general path variables
    c_Return =
        C_OscUtils::h_ResolveProjIndependentPlaceholderVariables(c_Return);

    // resolve project-specific variables
    c_Return.replace(hc_PATH_VARIABLE_OPENSYDE_PROJ, orc_OsydeProjectPath);

    u32_Pos = c_Return.indexOf(hc_PATH_VARIABLE_DATABLOCK_PROJ);
    if (u32_Pos != 0U) {
      const QString c_PathWithResolvedPlaceholders =
          C_OscUtils::h_ResolvePlaceholderVariables(orc_DbProjectPath, "");
      c_Return.replace(hc_PATH_VARIABLE_DATABLOCK_PROJ,
                       c_PathWithResolvedPlaceholders);
      // occurrences of orc_DbProjectPath in itself get replaced with ""
    }
  }

  return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if path contains project independent variables (e.g.
   %{OPENSYDE_BINARY}) and resolve them.

   Do not call this function for replacing every path variable!
   This functionality can be found in a utility class that knows project stuff.

   \param[in]  orc_Path    path that probably contains variables

   \return
   Resolved path
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscUtils::h_ResolveProjIndependentPlaceholderVariables(
    const QString &orc_Path) {
  QString c_Return = orc_Path;
  QString c_Replacement;

  uint32_t u32_Pos = c_Return.indexOf(hc_PATH_VARIABLE_OPENSYDE_BIN);

  if (u32_Pos != 0U) {
    const QString c_ExePath = QCoreApplication::applicationFilePath();
    const QString c_ExeDir = QFileInfo(c_ExePath).absolutePath() + "/";
    c_Replacement = c_ExeDir;
    c_Return.replace(hc_PATH_VARIABLE_OPENSYDE_BIN, c_Replacement);
  }

  u32_Pos = c_Return.indexOf(hc_PATH_VARIABLE_USER_NAME);
  if (u32_Pos != 0U) {
    Q_ASSERT(stw::opensyde_core::C_OscUtils::h_GetSystemUserName(
                 c_Replacement) == true);
    c_Return.replace(hc_PATH_VARIABLE_USER_NAME, c_Replacement);
  }

  u32_Pos = c_Return.indexOf(hc_PATH_VARIABLE_COMPUTER_NAME);
  if (u32_Pos != 0U) {
    Q_ASSERT(stw::opensyde_core::C_OscUtils::h_GetSystemMachineName(
                 c_Replacement) == true);
    c_Return.replace(hc_PATH_VARIABLE_COMPUTER_NAME, c_Replacement);
  }

  return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility function to convert relative path to absolute path if
   necessary

   Warning: assuming orc_BaseDir is not an empty string and no file.

   \param[in]  orc_BaseDir                   Base path if relative and could
   itself be relative
   \param[in]  orc_RelativeOrAbsolutePath    Path which might be relative or
   absolute (and could be empty)

   \return
   Absolute file path if input fulfills assumptions
*/
//----------------------------------------------------------------------------------------------------------------------
QString
C_OscUtils::h_ConcatPathIfNecessary(const QString &orc_BaseDir,
                                    const QString &orc_RelativeOrAbsolutePath) {
  QString c_Retval;

  const QString c_QPath = orc_RelativeOrAbsolutePath;
  const QString c_QPathDir = QFileInfo(c_QPath).absolutePath() + "/";
  const QString c_Path = c_QPathDir;

  bool q_IsRelativePath = QDir::isRelativePath(c_QPath);

  // special scenario: if the path starts with "\\" or "//" is is a UNC network
  // path for our purpose we consider it an absolute path (concatting would have
  // weird results)
  if ((orc_RelativeOrAbsolutePath.length() >= 2U) &&
      (((orc_RelativeOrAbsolutePath[1] == '/') &&
        (orc_RelativeOrAbsolutePath[2] == '/')) ||
       ((orc_RelativeOrAbsolutePath[1] == '\\') &&
        (orc_RelativeOrAbsolutePath[2] == '\\')))) {
    q_IsRelativePath = false;
  }

  if ((orc_BaseDir != "") && (q_IsRelativePath == true)) {
    c_Retval = orc_BaseDir + "/" + orc_RelativeOrAbsolutePath;

    // replace all "\" by "/":
    c_Retval.replace("\\", "/");
  } else {
    c_Retval = orc_RelativeOrAbsolutePath;
  }

  // remove all double slashes but the first (network paths)
  if (c_Retval.indexOf("//") == 1U) {
    c_Retval = '/' + c_Retval;
  }
  c_Retval.replace("//", "/");

  return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get unique item name based on proposal

   \param[in]  orc_ExistingStrings  Existing item names
   \param[in]  orc_ProposedName     Proposal for item name
   \param[in]  ou32_MaxCharLimit    Max char limit
   \param[in]  orc_SkipName         Optional name to block any adaptations for

   \return
   Unique node name
*/
//----------------------------------------------------------------------------------------------------------------------
QString
C_OscUtils::h_GetUniqueName(const QHash<QString, bool> &orc_ExistingStrings,
                          const QString &orc_ProposedName,
                          const uint32_t ou32_MaxCharLimit,
                          const QString &orc_SkipName) {
  QString c_Retval = orc_ProposedName;
  bool q_Conflict;
  int32_t s32_MaxDeviation;
  QString c_BaseStr;

  QHash<QString, bool>::const_iterator c_ItString;

  // Apply restriction
  if ((ou32_MaxCharLimit > 0UL) && (c_Retval.length() > ou32_MaxCharLimit)) {
    c_Retval = c_Retval.mid(0, ou32_MaxCharLimit);
  }

  do {
    q_Conflict = false;
    c_ItString = orc_ExistingStrings.find(c_Retval);
    if (c_ItString != orc_ExistingStrings.end()) {
      q_Conflict = true;
      mh_GetBaseNameAndCurrentConflictNumberFromString(
          c_ItString.key(), orc_SkipName, c_BaseStr, s32_MaxDeviation);
      // Do not use 0 and 1 for name adaptation
      if (s32_MaxDeviation <= 0) {
        s32_MaxDeviation = 1;
      }
      {
        const QString c_Appendix =
            '_' + QString::number(s32_MaxDeviation + static_cast<int32_t>(1));
        c_Retval = c_BaseStr + c_Appendix;
        if ((ou32_MaxCharLimit > 0UL) &&
            (c_Retval.length() > ou32_MaxCharLimit)) {
          const uint32_t u32_ReqLength = c_Appendix.length();
          if (u32_ReqLength < ou32_MaxCharLimit) {
            c_Retval =
                c_BaseStr.mid(0, ou32_MaxCharLimit - c_Appendix.length()) +
                c_Appendix;
          }
        }
      }
    }
  } while (q_Conflict == true);
  return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get number at string end

   \param[in]   orc_ProposedName    Proposal for name
   \param[out]  orc_CutString       String without number
   \param[out]  ors32_Number        Number at end (else -1)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscUtils::h_GetNumberAtStringEnd(const QString &orc_ProposedName,
                                        QString &orc_CutString,
                                        int32_t &ors32_Number) {
  QString c_Number;
  bool q_UnderscoreDetected = false;
  bool q_NumberDetected = false;
  uint32_t u32_ItStr;

  // Default return
  orc_CutString = orc_ProposedName;
  ors32_Number = -1;

  for (u32_ItStr = orc_ProposedName.length(); u32_ItStr > 0; --u32_ItStr) {
    if ((orc_ProposedName[u32_ItStr] >= '0') &&
        (orc_ProposedName[u32_ItStr] <= '9')) {
      // Continue
      q_NumberDetected = true;
    } else {
      if ('_' == orc_ProposedName[u32_ItStr]) {
        q_UnderscoreDetected = true;
      }
      // Stop
      break;
    }
  }
  if ((q_NumberDetected == true) && (q_UnderscoreDetected == true)) {
    // Cut string
    orc_CutString = orc_ProposedName.mid(0, u32_ItStr - 1); // Without
                                                            // underscore
    c_Number = orc_ProposedName.mid(
        u32_ItStr, orc_ProposedName.length()); // Without underscore
    try {
      ors32_Number = c_Number.toInt();
    } catch (...) {
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get base name and current conflict number from string

   \param[in]   orc_ConflictingValue   Conflicting value
   \param[in]   orc_SkipName           Optional name to block any adaptations
   for
   \param[out]  orc_CutString          String without number
   \param[out]  ors32_Number           Number at end (else -1)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscUtils::mh_GetBaseNameAndCurrentConflictNumberFromString(
    const QString &orc_ConflictingValue, const QString &orc_SkipName,
    QString &orc_CutString, int32_t &ors32_Number) {
  // Search for the SkipName if the skip name is a valid string
  const uint32_t u32_Pos = orc_SkipName.isEmpty()
                               ? 0UL
                               : orc_ConflictingValue.lastIndexOf(orc_SkipName);

  if (u32_Pos == 0UL) {
    // Continue examining the complete string
    h_GetNumberAtStringEnd(orc_ConflictingValue, orc_CutString, ors32_Number);
  } else {
    // Hint: mid and lastIndexOf start counting at 0
    const uint32_t u32_ZeroBasedpos = u32_Pos;
    // Extract the part of the string that may be adapted
    const QString c_StringAfterSkip = orc_ConflictingValue.mid(
        (u32_ZeroBasedpos + orc_SkipName.length()),
        (orc_ConflictingValue.length() - orc_SkipName.length()) -
            u32_ZeroBasedpos);
    const QString c_SkippedPart =
        orc_ConflictingValue.mid(0, u32_ZeroBasedpos + orc_SkipName.length());
    // Search remaining part for any number
    h_GetNumberAtStringEnd(c_StringAfterSkip, orc_CutString, ors32_Number);
    // Add skipped part to base string again
    orc_CutString = c_SkippedPart + orc_CutString;
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Include trailing delimiter
 */
//----------------------------------------------------------------------------------------------------------------------
QString C_OscUtils::h_IncludeTrailingDelimiter(const QString &orc_Path) {
  QString c_Path = orc_Path;
  if (!c_Path.endsWith("/") && !c_Path.endsWith("\\")) {
    c_Path += "/";
  }
  return c_Path;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get system user name
 */
//----------------------------------------------------------------------------------------------------------------------
bool C_OscUtils::h_GetSystemUserName(QString &orc_UserName) {
  QString c_User = qgetenv("USERNAME");
  if (c_User.isEmpty()) {
    c_User = qgetenv("USER");
  }

  bool q_Return = !c_User.isEmpty();
  if (q_Return) {
    orc_UserName = c_User;
  } else {
    orc_UserName = "????";
  }
  return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get system machine name
 */
//----------------------------------------------------------------------------------------------------------------------
bool C_OscUtils::h_GetSystemMachineName(QString &orc_MachineName) {
  QString c_Machine = qgetenv("COMPUTERNAME");
  if (c_Machine.isEmpty()) {
    c_Machine = qgetenv("HOSTNAME");
  }

  bool q_Return = !c_Machine.isEmpty();
  if (q_Return) {
    orc_MachineName = c_Machine;
  } else {
    orc_MachineName = "????";
  }
  return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle system messages (Qt process events)
 */
//----------------------------------------------------------------------------------------------------------------------
void C_OscUtils::h_HandleSystemMessages(void) {
  QCoreApplication::processEvents();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Change file extension
 */
//----------------------------------------------------------------------------------------------------------------------
QString C_OscUtils::h_ChangeFileExtension(const QString &orc_FilePath,
                                          const QString &orc_NewExtension) {
  QString c_Path = orc_FilePath;
  int s32_LastDot = c_Path.lastIndexOf('.');
  int s32_LastSep = c_Path.lastIndexOf('/');
  int s32_LastSep2 = c_Path.lastIndexOf('\\');

  if (s32_LastSep2 > s32_LastSep) {
    s32_LastSep = s32_LastSep2;
  }

  if ((s32_LastDot != -1) && (s32_LastDot > s32_LastSep)) {
    c_Path = c_Path.left(s32_LastDot);
  }

  QString c_NewExt = orc_NewExtension;
  if (!c_NewExt.startsWith(".")) {
    c_Path += ".";
  }
  c_Path += c_NewExt;

  return c_Path;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Convert integer to hexadecimal string

   Compose string from integer value data.
   Does not insert a "0x" prefix before the data.

   Example:
   "h_IntToHex(0x123, 4)" will return "0123".

   \param[in]  os64_Value    value to convert
   \param[in]  ou32_Digits   number of digits to return (zeroes will be filled in from the left)

   \return
   Resulting hexadecimal string (uppercase)
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscUtils::h_IntToHex(const int64_t os64_Value, const uint32_t ou32_Digits) {
  return QString::number(os64_Value, 16).toUpper().rightJustified(static_cast<int>(ou32_Digits), '0');
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Convert unsigned integer to hexadecimal string

   \param[in]  ou64_Value    value to convert
   \param[in]  ou32_Digits   number of digits to return (zeroes will be filled in from the left)

   \return
   Resulting hexadecimal string (uppercase)
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscUtils::h_IntToHex(const uint64_t ou64_Value, const uint32_t ou32_Digits) {
  return QString::number(ou64_Value, 16).toUpper().rightJustified(static_cast<int>(ou32_Digits), '0');
}
