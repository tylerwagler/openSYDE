//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handler class for logging operations (implementation)

   Handler class for logging operations

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscLoggingHandler.hpp"
#include "stwerrors.hpp"
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QString>
#include <QTextStream>

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
using namespace stw::errors;
using namespace stw::opensyde_core;

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
bool C_OscLoggingHandler::mhq_WriteToFile = false;
bool C_OscLoggingHandler::mhq_AutoFlushAllFile = false;
bool C_OscLoggingHandler::mhq_AutoFlushWarningsAndErrorsFile = false;
bool C_OscLoggingHandler::mhq_WriteToConsole = true;
bool C_OscLoggingHandler::mhq_MeasureTime = false;
bool C_OscLoggingHandler::mhq_LogInitErrorsToConsole = false;
std::map<uint16_t, qint64> C_OscLoggingHandler::mhc_StartTimes =
    std::map<uint16_t, qint64>();
QString C_OscLoggingHandler::mhc_FileName = "";
QRecursiveMutex C_OscLoggingHandler::mhc_ConsoleCriticalSection;
QRecursiveMutex C_OscLoggingHandler::mhc_FileCriticalSection;
QFile C_OscLoggingHandler::mhc_File;

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set write to file active flag

   \param[in]  oq_Active                        New write to file active flag
   \param[in]  oq_AutoFlushAll                  true: flush output file after
   each write false: do not automatically flush (= default)
   \param[in]  oq_LogInitErrorsToConsole        true: if opening log file fails
   print a log entry to console
   \param[in]  oq_AutoFlushWarningsAndErrors    true: flush output file after
   write of warnings and errors false: do not automatically flush (= default)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_SetWriteToFileActive(
    const bool oq_Active, const bool oq_AutoFlushAll,
    const bool oq_LogInitErrorsToConsole,
    const bool oq_AutoFlushWarningsAndErrors) {
  if (C_OscLoggingHandler::mhc_File.is_open() == true) {
    C_OscLoggingHandler::mhc_File.close();
  }
  C_OscLoggingHandler::mhq_WriteToFile = oq_Active;
  C_OscLoggingHandler::mhq_AutoFlushAllFile = oq_AutoFlushAll;
  C_OscLoggingHandler::mhq_AutoFlushWarningsAndErrorsFile =
      oq_AutoFlushWarningsAndErrors;
  C_OscLoggingHandler::mhq_LogInitErrorsToConsole = oq_LogInitErrorsToConsole;
  mh_OpenFile();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set write to console active flag

   \param[in] oq_Active New write to console active flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_SetWriteToConsoleActive(const bool oq_Active) {
  C_OscLoggingHandler::mhq_WriteToConsole = oq_Active;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set measure time active flag

   \param[in] oq_Active New measure time active flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_SetMeasurePerformanceActive(const bool oq_Active) {
  C_OscLoggingHandler::mhq_MeasureTime = oq_Active;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set log file location path and file name

   \param[in] orc_CompleteLogFileLocation Log file location path and file name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_SetCompleteLogFileLocation(
    const QString &orc_CompleteLogFileLocation) {
  if (C_OscLoggingHandler::mhc_File.is_open() == true) {
    C_OscLoggingHandler::mhc_File.close();
  }
  C_OscLoggingHandler::mhc_FileName = orc_CompleteLogFileLocation;
  mh_OpenFile();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get complete log file location

   \return
   Current complete log file location
*/
//----------------------------------------------------------------------------------------------------------------------
const QString &C_OscLoggingHandler::h_GetCompleteLogFileLocation(void) {
  return C_OscLoggingHandler::mhc_FileName;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write info message to log

   \param[in] orc_Activity  Current activity
   \param[in] orc_Message   Message to write (No '\n' necessary)
   \param[in] opcn_Class    Current class
   \param[in] opcn_Function Current function
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_WriteLogInfo(const QString &orc_Activity,
                                         const QString &orc_Message,
                                         const char_t *const opcn_Class,
                                         const char_t *const opcn_Function) {
  C_OscLoggingHandler::mh_WriteLog("INFO", orc_Activity, orc_Message,
                                   opcn_Class, opcn_Function);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write warning message to log

   \param[in] orc_Activity  Current activity
   \param[in] orc_Message   Message to write (No '\n' necessary)
   \param[in] opcn_Class    Current class
   \param[in] opcn_Function Current function
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_WriteLogWarning(const QString &orc_Activity,
                                            const QString &orc_Message,
                                            const char_t *const opcn_Class,
                                            const char_t *const opcn_Function) {
  C_OscLoggingHandler::mh_WriteLog("WARNING", orc_Activity, orc_Message,
                                   opcn_Class, opcn_Function);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write error message to log

   \param[in] orc_Activity  Current activity
   \param[in] orc_Message   Message to write (No '\n' necessary)
   \param[in] opcn_Class    Current class
   \param[in] opcn_Function Current function
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_WriteLogError(const QString &orc_Activity,
                                          const QString &orc_Message,
                                          const char_t *const opcn_Class,
                                          const char_t *const opcn_Function) {
  C_OscLoggingHandler::mh_WriteLog("ERROR", orc_Activity, orc_Message,
                                   opcn_Class, opcn_Function);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write time measurement message to log

   Previous call of C_OscLoggingHandler::h_StartPerformanceTimer() is mandatory
   ("tik and tok").

   \param[in] ou16_TimerId  Timer ID returned by previous call of
   C_OscLoggingHandler::h_StartPerformanceTimer()
   \param[in] orc_Message   Message to write (No '\n' necessary)
   \param[in] opcn_Class    Current class
   \param[in] opcn_Function Current function
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_WriteLogPerformance(
    const uint16_t ou16_TimerId, const QString &orc_Message,
    const char_t *const opcn_Class, const char_t *const opcn_Function) {
  if (mhq_MeasureTime == true) {
    const auto c_StartTime = mhc_StartTimes.find(ou16_TimerId);
    if (c_StartTime != mhc_StartTimes.end()) {
      C_OscLoggingHandler::mh_WriteLog(
          "INFO", "Performance measurement",
          orc_Message + " time: " +
              QString::number(static_cast<int32_t>(
                  QDateTime::currentMSecsSinceEpoch() - c_StartTime->second)) +
              " ms",
          opcn_Class, opcn_Function);

      // update log file
      C_OscLoggingHandler::h_Flush();

      // remove ID from map
      mhc_StartTimes.erase(c_StartTime);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Start performance timer

   \return
   ID for getting elapsed time with C_OscLoggingHandler::h_WriteLogPerformance
*/
//----------------------------------------------------------------------------------------------------------------------
uint16_t C_OscLoggingHandler::h_StartPerformanceTimer(void) {
  const uint16_t u16_Id = static_cast<uint16_t>(rand());

  mhc_StartTimes[u16_Id] = QDateTime::currentMSecsSinceEpoch();

  return u16_Id;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get readable stw error

   \param[in] os32_Error STW error value

   \return
   STW error string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscLoggingHandler::h_StwError(const int32_t os32_Error) {
  QString c_Retval;

  switch (os32_Error) {
  case C_NO_ERR:
    c_Retval = "C_NO_ERR";
    break;
  case C_WARN:
    c_Retval = "C_WARN";
    break;
  case C_DEFAULT:
    c_Retval = "C_DEFAULT";
    break;
  case C_BUSY:
    c_Retval = "C_BUSY";
    break;
  case C_RANGE:
    c_Retval = "C_RANGE";
    break;
  case C_OVERFLOW:
    c_Retval = "C_OVERFLOW";
    break;
  case C_NOACT:
    c_Retval = "C_NOACT";
    break;
  case C_COM:
    c_Retval = "C_COM";
    break;
  case C_RD_WR:
    c_Retval = "C_RD_WR";
    break;
  case C_CONFIG:
    c_Retval = "C_CONFIG";
    break;
  case C_CHECKSUM:
    c_Retval = "C_CHECKSUM";
    break;
  case C_TIMEOUT:
    c_Retval = "C_TIMEOUT";
    break;
  case C_UNKNOWN_ERR:
  default:
    c_Retval = "C_UNKNOWN_ERR";
    break;
  }

  return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility to flush current log entries (No manual call necessary)
 */
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_Flush(void) {
  if (C_OscLoggingHandler::mhq_WriteToConsole == true) {
    QMutexLocker c_Locker(&C_OscLoggingHandler::mhc_ConsoleCriticalSection);
    std::cout << &std::flush;
  }

  // File
  if ((C_OscLoggingHandler::mhq_WriteToFile == true) &&
      C_OscLoggingHandler::mhc_File.isOpen()) {
    QMutexLocker c_Locker(&C_OscLoggingHandler::mhc_FileCriticalSection);
    C_OscLoggingHandler::mhc_File.flush();
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get date time as formatted string

   Format:2017-08-29 07:32:19.123

   \param[in] orc_DateTime Date time structure

   \return
   Formatted string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscLoggingHandler::h_UtilConvertDateTimeToString(
    const QDateTime &orc_DateTime) {
  return orc_DateTime.toString("yyyy-MM-dd HH:mm:ss.zzz");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write message to log

   \param[in] oe_Type       Message type (maximum 7 characters)
   \param[in] orc_Activity  Current activity (maximum 26 characters)
   \param[in] orc_Message   Message to write
   \param[in] opcn_Class    Current class (combined with function: maximum 50
   characters) May be file path string which is reduced to file base name
   automatically
   \param[in] opcn_Function Current function (combined with function: maximum 50
   characters)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::mh_WriteLog(const QString &orc_Type,
                                      const QString &orc_Activity,
                                      const QString &orc_Message,
                                      const char_t *const opcn_Class,
                                      const char_t *const opcn_Function) {
  QString c_DateTimeFormatted;
  // C_TglDateTime removed
  QString c_LogEntry;
  QString c_Class;
  QString c_Function;
  QString c_CombinedClassAndFunction;
  if (opcn_Class != NULL) {
    const QString c_QClass = QString::fromStdString(opcn_Class);
    const QFileInfo c_FileInfo(c_QClass);
    c_Class = c_FileInfo.completeBaseName();
  } else {
    c_Class = "UNKNOWN_CLASS";
  }
  if (opcn_Function != NULL) {
    c_Function = opcn_Function;
  } else {
    c_Function = "UNKNOWN_FUNCTION";
  }

  c_Class += "::";

  // Special handling:
  // Older versions of MSVC do not support the __func__ macro. They support
  // __FUNCTION__ which expands to
  //  classname::functionname (in contrast to only functionname for __func__).
  // To be defensive and prevent containing the class name twice: If function
  // already contains the class name at the
  //  beginning then strip that information.
  if (c_Function.startsWith(c_Class)) {
    c_Function.remove(0, c_Class.length());
  }

  c_CombinedClassAndFunction = c_Class + c_Function;

  QDateTime c_DateTime = QDateTime::currentDateTime();
  c_DateTimeFormatted =
      C_OscLoggingHandler::h_UtilConvertDateTimeToString(c_DateTime);

  // Format:
  //[DATE/TIME] [TYPE_OF_REPORT (Info, Warning, Error)] [ACTIVITY]
  //[CLASS::FUNCTION] [MESSAGE] 2017-08-29 07:32:19.123      INFO       Startup
  // Main                            Application started.
  c_LogEntry = c_DateTimeFormatted.leftJustified(25, ' ') + " " +
               orc_Type.leftJustified(7, ' ') + "  " +
               orc_Activity.leftJustified(26, ' ') + "  " +
               c_CombinedClassAndFunction.leftJustified(52, ' ') + "  " +
               orc_Message + "\n";

  // Console
  if (C_OscLoggingHandler::mhq_WriteToConsole == true) {
    QMutexLocker c_Locker(&C_OscLoggingHandler::mhc_ConsoleCriticalSection);
    std::cout << c_LogEntry.toUtf8().constData();
  }

  // File
  if ((C_OscLoggingHandler::mhq_WriteToFile == true) &&
      C_OscLoggingHandler::mhc_File.isOpen()) {
    QMutexLocker c_Locker(&C_OscLoggingHandler::mhc_FileCriticalSection);

    C_OscLoggingHandler::mhc_File.write(c_LogEntry.toUtf8());
    if ((mhq_AutoFlushAllFile == true) ||
        ((C_OscLoggingHandler::mhq_AutoFlushWarningsAndErrorsFile == true) &&
         ((orc_Type == "WARNING") || (orc_Type == "ERROR")))) {
      C_OscLoggingHandler::mhc_File.flush();
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Open file if necessary
 */
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::mh_OpenFile(void) {
  if (((C_OscLoggingHandler::mhc_FileName != "") &&
       (C_OscLoggingHandler::mhq_WriteToFile == true)) &&
      (C_OscLoggingHandler::mhc_File.is_open() == false)) {
    const QString c_QFileName = C_OscLoggingHandler::mhc_FileName;
    const QFileInfo c_FileInfo(c_QFileName);
    const QString c_QFilePath = c_FileInfo.absolutePath() + "/";

    // Folder
    if (!QFileInfo(c_QFilePath).isDir()) {
      QDir().mkpath(c_QFilePath);
    }

    if (!C_OscLoggingHandler::mhc_File.open(C_OscLoggingHandler::mhc_FileName,
                                            QIODevice::Append |
                                                QIODevice::Text)) {
      // if opening the file fails then at least try to write an info to console
      // once (if configured so)
      if (C_OscLoggingHandler::mhq_LogInitErrorsToConsole == true) {
        const QString c_ErrorText = "Could not open log file \"" +
                                    C_OscLoggingHandler::mhc_FileName +
                                    "\" for appending.";
        osc_write_log_warning("openSYDE logging engine", c_ErrorText);
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscLoggingHandler::C_OscLoggingHandler(void) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscLoggingHandler::~C_OscLoggingHandler(void) {
  if (C_OscLoggingHandler::mhc_File.isOpen()) {
    C_OscLoggingHandler::mhc_File.close();
  }
}
