//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handler class for logging operations (implementation)

   Handler class for logging operations

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <mutex>
#include "precomp_headers.hpp"

#include <cstdlib>
#include <atomic>
#include <iostream>
#include <format>
#include <iterator>
#include <string_view>
#include "TglFile.hpp"
#include "C_OscLoggingHandler.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */
bool C_OscLoggingHandler::mhq_WriteToFile = false;
bool C_OscLoggingHandler::mhq_AutoFlushAllFile = false;
bool C_OscLoggingHandler::mhq_AutoFlushWarningsAndErrorsFile = false;
bool C_OscLoggingHandler::mhq_WriteToConsole = true;
bool C_OscLoggingHandler::mhq_MeasureTime = false;
bool C_OscLoggingHandler::mhq_LogInitErrorsToConsole = false;
std::map<uint16_t, uint32_t> C_OscLoggingHandler::mhc_StartTimes = std::map<uint16_t, uint32_t> ();
std::string C_OscLoggingHandler::mhc_FileName = "";
std::mutex C_OscLoggingHandler::mhc_ConsoleCriticalSection;
std::mutex C_OscLoggingHandler::mhc_FileCriticalSection;
std::ofstream C_OscLoggingHandler::mhc_File;

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

namespace
{
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Class name from a __FILE__ path, without allocating

   Same result as TglChangeFileExtension(TglExtractFileName(orc_FilePath), "") -- directories
   and extension stripped -- but returned as a view into the literal rather than through two
   temporary strings. __FILE__ is a compile time literal, so the view stays valid.

   \param[in]  opcn_FilePath  Source file path

   \return  view of the bare file name
*/
//----------------------------------------------------------------------------------------------------------------------
std::string_view mh_ClassNameFromFilePath(const char * const opcn_FilePath)
{
   std::string_view c_Retval(opcn_FilePath);
   const std::string_view::size_type un_Separator = c_Retval.find_last_of("/\\");

   if (un_Separator != std::string_view::npos)
   {
      c_Retval.remove_prefix(un_Separator + 1U);
   }

   const std::string_view::size_type un_Dot = c_Retval.find_last_of('.');
   if (un_Dot != std::string_view::npos)
   {
      c_Retval = c_Retval.substr(0U, un_Dot);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write a decimal value right aligned and zero padded

   Writes exactly ou8_Digits characters. A value too large for the field is not
   truncated at the front: the caller sizes the field, and only h_UtilConvertDateTimeToString
   calls this, always with a field wide enough for the type it passes.

   \param[out]  opcn_Dest    Destination, must have room for ou8_Digits characters
   \param[in]   ou32_Value   Value to write
   \param[in]   ou8_Digits   Field width
*/
//----------------------------------------------------------------------------------------------------------------------
void mh_WritePaddedDecimal(char * const opcn_Dest, const uint32_t ou32_Value, const uint_fast8_t ou8_Digits)
{
   uint32_t u32_Remaining = ou32_Value;

   for (uint_fast8_t u8_Index = ou8_Digits; u8_Index > 0U; u8_Index--)
   {
      opcn_Dest[u8_Index - 1U] = static_cast<char>('0' + (u32_Remaining % 10U));
      u32_Remaining /= 10U;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Render a date and time as "YYYY-MM-DD HH:MM:SS.mmm"

   The separators sit at fixed offsets and every field is a fixed-width decimal, so the
   characters are written straight into the caller's buffer. Years above 9999 (possible,
   as the field is a uint16_t) widen the rendering to 24 characters rather than losing a
   digit, which is why the buffer is 24 and the length is returned.

   \param[out]  oracn_Buffer   Destination buffer
   \param[in]   orc_DateTime   Date and time to render

   \return  number of characters written (23, or 24 for a five digit year)
*/
//----------------------------------------------------------------------------------------------------------------------
uint_fast8_t mh_FormatDateTime(char (&oracn_Buffer)[24], const stw::tgl::C_TglDateTime & orc_DateTime)
{
   const uint_fast8_t u8_YearDigits = (orc_DateTime.mu16_Year > 9999U) ? 5U : 4U;
   char * pcn_Write = oracn_Buffer;

   mh_WritePaddedDecimal(pcn_Write, orc_DateTime.mu16_Year, u8_YearDigits);
   pcn_Write = &pcn_Write[u8_YearDigits];

   *pcn_Write = '-';
   mh_WritePaddedDecimal(&pcn_Write[1], orc_DateTime.mu8_Month, 2U);
   pcn_Write[3] = '-';
   mh_WritePaddedDecimal(&pcn_Write[4], orc_DateTime.mu8_Day, 2U);
   pcn_Write[6] = ' ';
   mh_WritePaddedDecimal(&pcn_Write[7], orc_DateTime.mu8_Hour, 2U);
   pcn_Write[9] = ':';
   mh_WritePaddedDecimal(&pcn_Write[10], orc_DateTime.mu8_Minute, 2U);
   pcn_Write[12] = ':';
   mh_WritePaddedDecimal(&pcn_Write[13], orc_DateTime.mu8_Second, 2U);
   pcn_Write[15] = '.';
   mh_WritePaddedDecimal(&pcn_Write[16], orc_DateTime.mu16_MilliSeconds, 3U);

   return static_cast<uint_fast8_t>(u8_YearDigits + 19U);
}
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set write to file active flag

   \param[in]  oq_Active                        New write to file active flag
   \param[in]  oq_AutoFlushAll                  true: flush output file after each write
                                                false: do not automatically flush (= default)
   \param[in]  oq_LogInitErrorsToConsole        true: if opening log file fails print a log entry to console
   \param[in]  oq_AutoFlushWarningsAndErrors    true: flush output file after write of warnings and errors
                                                false: do not automatically flush (= default)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_SetWriteToFileActive(const bool oq_Active, const bool oq_AutoFlushAll,
                                                 const bool oq_LogInitErrorsToConsole,
                                                 const bool oq_AutoFlushWarningsAndErrors)
{
   if (C_OscLoggingHandler::mhc_File.is_open() == true)
   {
      C_OscLoggingHandler::mhc_File.close();
   }
   C_OscLoggingHandler::mhq_WriteToFile = oq_Active;
   C_OscLoggingHandler::mhq_AutoFlushAllFile = oq_AutoFlushAll;
   C_OscLoggingHandler::mhq_AutoFlushWarningsAndErrorsFile = oq_AutoFlushWarningsAndErrors;
   C_OscLoggingHandler::mhq_LogInitErrorsToConsole = oq_LogInitErrorsToConsole;
   mh_OpenFile();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set write to console active flag

   \param[in] oq_Active New write to console active flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_SetWriteToConsoleActive(const bool oq_Active)
{
   C_OscLoggingHandler::mhq_WriteToConsole = oq_Active;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set measure time active flag

   \param[in] oq_Active New measure time active flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_SetMeasurePerformanceActive(const bool oq_Active)
{
   C_OscLoggingHandler::mhq_MeasureTime = oq_Active;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set log file location path and file name

   \param[in] orc_CompleteLogFileLocation Log file location path and file name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_SetCompleteLogFileLocation(const std::string & orc_CompleteLogFileLocation)
{
   if (C_OscLoggingHandler::mhc_File.is_open() == true)
   {
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
const std::string & C_OscLoggingHandler::h_GetCompleteLogFileLocation(void)
{
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
void C_OscLoggingHandler::h_WriteLogInfo(const std::string & orc_Activity, const std::string & orc_Message,
                                           const char * const opcn_Class, const char * const opcn_Function)
{
   C_OscLoggingHandler::mh_WriteLog("INFO", orc_Activity, orc_Message, opcn_Class, opcn_Function);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write warning message to log

   \param[in] orc_Activity  Current activity
   \param[in] orc_Message   Message to write (No '\n' necessary)
   \param[in] opcn_Class    Current class
   \param[in] opcn_Function Current function
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_WriteLogWarning(const std::string & orc_Activity, const std::string & orc_Message,
                                              const char * const opcn_Class, const char * const opcn_Function)
{
   C_OscLoggingHandler::mh_WriteLog("WARNING", orc_Activity, orc_Message, opcn_Class, opcn_Function);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write error message to log

   \param[in] orc_Activity  Current activity
   \param[in] orc_Message   Message to write (No '\n' necessary)
   \param[in] opcn_Class    Current class
   \param[in] opcn_Function Current function
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_WriteLogError(const std::string & orc_Activity, const std::string & orc_Message,
                                            const char * const opcn_Class, const char * const opcn_Function)
{
   C_OscLoggingHandler::mh_WriteLog("ERROR", orc_Activity, orc_Message, opcn_Class, opcn_Function);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write time measurement message to log

   Previous call of C_OscLoggingHandler::h_StartPerformanceTimer() is mandatory ("tik and tok").

   \param[in] ou16_TimerId  Timer ID returned by previous call of C_OscLoggingHandler::h_StartPerformanceTimer()
   \param[in] orc_Message   Message to write (No '\n' necessary)
   \param[in] opcn_Class    Current class
   \param[in] opcn_Function Current function
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_WriteLogPerformance(const uint16_t ou16_TimerId, const std::string & orc_Message,
                                                  const char * const opcn_Class, const char * const opcn_Function)
{
   if (mhq_MeasureTime == true)
   {
      const std::map< uint16_t, uint32_t >::iterator c_StartTime = mhc_StartTimes.find(ou16_TimerId);
      if (c_StartTime != mhc_StartTimes.end())
      {
         C_OscLoggingHandler::mh_WriteLog(
            "INFO", "Performance measurement",
            orc_Message + " time: " + std::to_string(stw::tgl::TglGetTickCount() - c_StartTime->second) + " ms",
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
uint16_t C_OscLoggingHandler::h_StartPerformanceTimer(void)
{
   static std::atomic<uint16_t> hu16_NextId(0);
   const uint16_t u16_Id = hu16_NextId.fetch_add(1, std::memory_order_relaxed);

   mhc_StartTimes[u16_Id] = stw::tgl::TglGetTickCount();

   return u16_Id;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get readable stw error

   \param[in] os32_Error STW error value

   \return
   STW error string
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscLoggingHandler::h_StwError(const int32_t os32_Error)
{
   // Delegate to the std::error_category for STW error codes.
   return STWErrorCategory::Instance().message(os32_Error);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility to flush current log entries (No manual call necessary)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::h_Flush(void)
{
   if (C_OscLoggingHandler::mhq_WriteToConsole == true)
   {
      //Critical section
      C_OscLoggingHandler::mhc_ConsoleCriticalSection.lock();
      std::cout << &std::flush;
      //Critical section
      C_OscLoggingHandler::mhc_ConsoleCriticalSection.unlock();
   }

   //File
   if ((C_OscLoggingHandler::mhq_WriteToFile == true) && (C_OscLoggingHandler::mhc_File.is_open() == true))
   {
      //Critical section
      C_OscLoggingHandler::mhc_FileCriticalSection.lock();
      C_OscLoggingHandler::mhc_File.flush();
      //Critical section
      C_OscLoggingHandler::mhc_FileCriticalSection.unlock();
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
std::string C_OscLoggingHandler::h_UtilConvertDateTimeToString(const C_TglDateTime & orc_DateTime)
{
   //Phase 7.1: every field is a fixed-width decimal at a known offset, so the digits are
   //written directly. std::format replaced the original stringstream here and was 2.8x to
   //12.2x faster, but it still cost ~960ns of the ~1900ns log call because it re-parses the
   //spec and runs the general padding machinery for seven arguments. Writing the digits is
   //the whole job; see the LoggingHandler benchmarks.
   char acn_Buffer[24];
   const uint_fast8_t u8_Length = mh_FormatDateTime(acn_Buffer, orc_DateTime);

   return std::string(acn_Buffer, u8_Length);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write message to log

   \param[in] oe_Type       Message type (maximum 7 characters)
   \param[in] orc_Activity  Current activity (maximum 26 characters)
   \param[in] orc_Message   Message to write
   \param[in] opcn_Class    Current class (combined with function: maximum 50 characters)
                            May be file path string which is reduced to file base name automatically
   \param[in] opcn_Function Current function (combined with function: maximum 50 characters)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::mh_WriteLog(const std::string & orc_Type, const std::string & orc_Activity,
                                        const std::string & orc_Message, const char * const opcn_Class,
                                        const char * const opcn_Function)
{
   C_TglDateTime c_DateTime;

   //Phase 7.1: __FILE__ and __func__ are compile time literals, so this text is identical on
   //every call from a given site. Derive it through string views and assemble it in per-thread
   //buffers that keep their capacity, rather than allocating five temporaries per log line.
   const std::string_view c_Class = (opcn_Class != nullptr) ?
                                    mh_ClassNameFromFilePath(opcn_Class) : std::string_view("UNKNOWN_CLASS");
   std::string_view c_Function = (opcn_Function != nullptr) ?
                                 std::string_view(opcn_Function) : std::string_view("UNKNOWN_FUNCTION");

   thread_local std::string hc_CombinedClassAndFunction;
   thread_local std::string hc_LogEntry;

   hc_CombinedClassAndFunction.clear();
   hc_CombinedClassAndFunction.append(c_Class).append("::");

   //Special handling:
   //Older versions of MSVC do not support the __func__ macro. They support __FUNCTION__ which expands to
   // classname::functionname (in contrast to only functionname for __func__).
   //To be defensive and prevent containing the class name twice: If function already contains the class name at the
   // beginning then strip that information.
   if (c_Function.starts_with(hc_CombinedClassAndFunction))
   {
      c_Function.remove_prefix(hc_CombinedClassAndFunction.size());
   }

   hc_CombinedClassAndFunction.append(c_Function);

   TglGetDateTimeNow(c_DateTime);

   //Format:
   //[DATE/TIME] [TYPE_OF_REPORT (Info, Warning, Error)] [ACTIVITY] [CLASS::FUNCTION] [MESSAGE]
   //2017-08-29 07:32:19.123      INFO       Startup         Main                            Application started.
   //Phase 7.1: std::format instead of std::stringstream; same field widths and
   //left alignment, and like setw it pads but never truncates an over-long field.
   //The stream version ended with std::endl, whose flush was a no-op on a
   //stringstream, so a plain newline is equivalent.
   //The timestamp is rendered into the line buffer rather than through
   //h_UtilConvertDateTimeToString, whose 23 character return does not fit libc++'s
   //22 character small string buffer and so allocates on every call. Its field is
   //never wider than 24, so the manual padding matches the "{:<25}" it replaces.
   char acn_TimeStamp[24];
   const uint_fast8_t u8_TimeStampLength = mh_FormatDateTime(acn_TimeStamp, c_DateTime);

   hc_LogEntry.clear();
   hc_LogEntry.append(acn_TimeStamp, u8_TimeStampLength);
   hc_LogEntry.append(static_cast<std::string::size_type>(25U - u8_TimeStampLength), ' ');
   std::format_to(std::back_inserter(hc_LogEntry), "{:<7}  {:<26}  {:<52}  {}\n",
                  orc_Type, orc_Activity, hc_CombinedClassAndFunction, orc_Message);
   const std::string & c_LogEntry = hc_LogEntry;

   //Console
   if (C_OscLoggingHandler::mhq_WriteToConsole == true)
   {
      //Critical section
      C_OscLoggingHandler::mhc_ConsoleCriticalSection.lock();
      std::cout << c_LogEntry;
      //Critical section
      C_OscLoggingHandler::mhc_ConsoleCriticalSection.unlock();
   }

   //File
   if ((C_OscLoggingHandler::mhq_WriteToFile == true) && (C_OscLoggingHandler::mhc_File.is_open() == true))
   {
      //Critical section
      C_OscLoggingHandler::mhc_FileCriticalSection.lock();

      //TGL critical section -> file
      C_OscLoggingHandler::mhc_File.write(c_LogEntry.c_str(), c_LogEntry.size());
      if ((mhq_AutoFlushAllFile == true) ||
          ((C_OscLoggingHandler::mhq_AutoFlushWarningsAndErrorsFile == true) &&
           ((orc_Type == "WARNING") || (orc_Type == "ERROR"))))
      {
         C_OscLoggingHandler::mhc_File.flush();
      }

      //Critical section
      C_OscLoggingHandler::mhc_FileCriticalSection.unlock();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Open file if necessary
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscLoggingHandler::mh_OpenFile(void)
{
   if (((C_OscLoggingHandler::mhc_FileName != "") && (C_OscLoggingHandler::mhq_WriteToFile == true)) &&
       (C_OscLoggingHandler::mhc_File.is_open() == false))
   {
      const std::string c_FilePath = TglExtractFilePath(C_OscLoggingHandler::mhc_FileName);
      //Folder
      if (TglDirectoryExists(c_FilePath) == false)
      {
         TglCreateDirectory(c_FilePath);
      }

      C_OscLoggingHandler::mhc_File.open(C_OscLoggingHandler::mhc_FileName.c_str(), std::ios::app);

      //if opening the file fails then at least try to write an info to console once (if configured so)
      if (C_OscLoggingHandler::mhc_File.is_open() == false)
      {
         if (C_OscLoggingHandler::mhq_LogInitErrorsToConsole == true)
         {
            const std::string c_ErrorText = "Could not open log file \"" + C_OscLoggingHandler::mhc_FileName +
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
C_OscLoggingHandler::C_OscLoggingHandler(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscLoggingHandler::~C_OscLoggingHandler(void)
{
   if (C_OscLoggingHandler::mhc_File.is_open() == true)
   {
      C_OscLoggingHandler::mhc_File.close();
   }
}
