//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief    String utility helpers for std::string

   Inline helpers for operations that std::string does not provide directly:
   printf-style formatting, hex/float/parse conversions with the legacy
   conventions, 1-based position/substring semantics inherited from the retired
   C_SclString class, case/trimming, tokenizing, and string-list file I/O.
   Files that include this header must include what they use (it does NOT transitively provide <iomanip>/<locale>;
   it still needs <sstream> for its own number parsing).

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CSCLSTRINGUTILH
#define CSCLSTRINGUTILH

#include <string>
#include <charconv>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <locale.h> // strtod_l / _strtod_l for a locale-independent double fallback
#include <climits>
#include <format>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <cstdint>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

namespace stw
{
namespace scl
{
/* -- Helpers for C_SclString static methods migrated to std::string ---------- */

/// Replacement for C_SclString::IntToStr(value).
template <typename T>
inline std::string IntToStrCompat(const T orc_Value)
{
   if constexpr (std::is_enum_v<T>)
   {
      return std::to_string(static_cast<std::underlying_type_t<T>>(orc_Value));
   }
   else
   {
      return std::to_string(orc_Value);
   }
}

/// Replacement for C_SclString::IntToHex(value, digits).
/// Formats an integer as zero-padded uppercase hex WITHOUT "0x" prefix.
template <typename T>
inline std::string IntToHexCompat(const T orc_Value, const uint32_t ou32_Digits)
{
   return std::format("{:0{}X}", orc_Value, ou32_Digits);
}

/// Replacement for C_SclString::FloatToStr(value).
inline std::string FloatToStrCompat(const double of64_Value)
{
   //std::format always uses the C locale; {:g} default precision 6 matches the
   //classic-locale ostringstream default it replaces.
   return std::format("{:g}", of64_Value);
}

/// Replacement for C_SclString::FloatToStr(value, digits).
inline std::string FloatToStrCompat(const double of64_Value, const int32_t os32_Digits)
{
   return std::format("{:.{}f}", of64_Value, os32_Digits);
}

/// Replacement for C_SclString::StringOfChar(ch, count).
inline std::string StringOfCharCompat(const char ocn_Char, const uint32_t ou32_Count)
{
   return std::string(ou32_Count, ocn_Char);
}

/* -- Helpers for C_SclString instance methods migrated to std::string -------- */

/// Replacement for str.SubString(index, count) — converts from 1-based to 0-based.
/// Returns a substring starting at 1-based position ou32_Index with ou32_Count characters.
inline std::string SubStringCompat(const std::string & orc_Str, const uint32_t ou32_Index,
                                    const uint32_t ou32_Count)
{
   if ((ou32_Index == 0U) || (ou32_Index > orc_Str.length()))
   {
      return "";
   }
   return orc_Str.substr(static_cast<size_t>(ou32_Index) - 1U, ou32_Count);
}

/// Replacement for str.Pos(sub) — returns 1-based position (0 = not found).
inline uint32_t PosCompat(const std::string & orc_Str, const std::string & orc_Sub)
{
   const size_t un_Pos = orc_Str.find(orc_Sub);
   if (un_Pos == std::string::npos)
   {
      return 0U;
   }
   return static_cast<uint32_t>(un_Pos) + 1U;
}

/// Replacement for str.LastPos(sub) — returns 1-based position (0 = not found).
inline uint32_t LastPosCompat(const std::string & orc_Str, const std::string & orc_Sub)
{
   const size_t un_Pos = orc_Str.rfind(orc_Sub);
   if (un_Pos == std::string::npos)
   {
      return 0U;
   }
   return static_cast<uint32_t>(un_Pos) + 1U;
}

/// Replacement for str.Trim() — removes leading and trailing whitespace.
inline std::string TrimCompat(const std::string & orc_Str)
{
   const std::string c_Whitespace = " \t\r\n\f\v";
   const size_t un_Start = orc_Str.find_first_not_of(c_Whitespace);
   if (un_Start == std::string::npos)
   {
      return "";
   }
   const size_t un_End = orc_Str.find_last_not_of(c_Whitespace);
   return orc_Str.substr(un_Start, un_End - un_Start + 1U);
}

/// Replacement for str.TrimLeft().
inline std::string TrimLeftCompat(const std::string & orc_Str)
{
   const std::string c_Whitespace = " \t\r\n\f\v";
   const size_t un_Start = orc_Str.find_first_not_of(c_Whitespace);
   if (un_Start == std::string::npos)
   {
      return "";
   }
   return orc_Str.substr(un_Start);
}

/// Replacement for str.TrimRight().
inline std::string TrimRightCompat(const std::string & orc_Str)
{
   const std::string c_Whitespace = " \t\r\n\f\v";
   const size_t un_End = orc_Str.find_last_not_of(c_Whitespace);
   if (un_End == std::string::npos)
   {
      return "";
   }
   return orc_Str.substr(0U, un_End + 1U);
}

/// Replacement for str.UpperCase().
inline std::string UpperCaseCompat(const std::string & orc_Str)
{
   std::string c_Result = orc_Str;
   for (std::string::size_type u32_Pos = 0U; u32_Pos < c_Result.size(); ++u32_Pos)
   {
      c_Result[u32_Pos] = static_cast<char>(std::toupper(static_cast<unsigned char>(c_Result[u32_Pos])));
   }
   return c_Result;
}

/// Replacement for str.LowerCase().
inline std::string LowerCaseCompat(const std::string & orc_Str)
{
   std::string c_Result = orc_Str;
   for (std::string::size_type u32_Pos = 0U; u32_Pos < c_Result.size(); ++u32_Pos)
   {
      c_Result[u32_Pos] = static_cast<char>(std::tolower(static_cast<unsigned char>(c_Result[u32_Pos])));
   }
   return c_Result;
}

/// Case-insensitive string equality comparison (allocation-free).
inline bool EqualsCaseInsensitive(const std::string & orc_A, const std::string & orc_B)
{
   if (orc_A.size() != orc_B.size())
   {
      return false;
   }
   for (std::string::size_type u32_Pos = 0U; u32_Pos < orc_A.size(); ++u32_Pos)
   {
      if (std::toupper(static_cast<unsigned char>(orc_A[u32_Pos])) !=
          std::toupper(static_cast<unsigned char>(orc_B[u32_Pos])))
      {
         return false;
      }
   }
   return true;
}

/// Case-insensitive find; returns 1-based position (0 = not found), like PosCompat.
inline uint32_t FindCaseInsensitive(const std::string & orc_Str, const std::string & orc_Sub)
{
   if (orc_Sub.empty())
   {
      return 1U;
   }
   if (orc_Sub.size() > orc_Str.size())
   {
      return 0U;
   }
   for (std::string::size_type u32_Start = 0U; u32_Start + orc_Sub.size() <= orc_Str.size(); ++u32_Start)
   {
      bool q_Match = true;
      for (std::string::size_type u32_Offset = 0U; u32_Offset < orc_Sub.size(); ++u32_Offset)
      {
         if (std::toupper(static_cast<unsigned char>(orc_Str[u32_Start + u32_Offset])) !=
             std::toupper(static_cast<unsigned char>(orc_Sub[u32_Offset])))
         {
            q_Match = false;
            break;
         }
      }
      if (q_Match == true)
      {
         return static_cast<uint32_t>(u32_Start) + 1U;
      }
   }
   return 0U;
}

/// Replacement for str.Delete(index, count) — 1-based to 0-based conversion.
inline std::string & DeleteCompat(std::string & orc_Str, const uint32_t ou32_Index, const uint32_t ou32_Count)
{
   if ((ou32_Index > 0U) && (ou32_Index <= orc_Str.length()))
   {
      const size_t un_Pos = static_cast<size_t>(ou32_Index) - 1U;
      const size_t un_Count = std::min(static_cast<size_t>(ou32_Count), orc_Str.length() - un_Pos);
      orc_Str.erase(un_Pos, un_Count);
   }
   return orc_Str;
}

/// Replacement for str.Insert(source, index) — 1-based to 0-based conversion.
inline std::string & InsertCompat(std::string & orc_Str, const std::string & orc_Source,
                                   const uint32_t ou32_Index)
{
   if ((ou32_Index > 0U) && (ou32_Index <= orc_Str.length() + 1U))
   {
      orc_Str.insert(static_cast<size_t>(ou32_Index) - 1U, orc_Source);
   }
   return orc_Str;
}

/// Replacement for str.ReplaceAll(search, replacement).
inline std::string & ReplaceAllCompat(std::string & orc_Str, const std::string & orc_Search,
                                       const std::string & orc_Replacement)
{
   if (orc_Search.empty())
   {
      return orc_Str;
   }
   size_t un_Pos = orc_Str.find(orc_Search);
   while (un_Pos != std::string::npos)
   {
      orc_Str.replace(un_Pos, orc_Search.size(), orc_Replacement);
      un_Pos = orc_Str.find(orc_Search, un_Pos + orc_Replacement.size());
   }
   return orc_Str;
}

/// Replacement for str.Tokenize(delimiters, tokens).
inline void TokenizeCompat(const std::string & orc_Str, const std::string & orc_Delimiters,
                            std::vector<std::string> & orc_Tokens)
{
   orc_Tokens.clear();
   size_t un_Start = orc_Str.find_first_not_of(orc_Delimiters);
   while (un_Start != std::string::npos)
   {
      const size_t un_End = orc_Str.find_first_of(orc_Delimiters, un_Start);
      if (un_End == std::string::npos)
      {
         orc_Tokens.push_back(orc_Str.substr(un_Start));
         break;
      }
      orc_Tokens.push_back(orc_Str.substr(un_Start, un_End - un_Start));
      un_Start = orc_Str.find_first_not_of(orc_Delimiters, un_End);
   }
}

/// Replacement for str.LastDelimiter(delimiters) — returns 1-based position.
inline uint32_t LastDelimiterCompat(const std::string & orc_Str, const std::string & orc_Delimiters)
{
   const size_t un_Pos = orc_Str.find_last_of(orc_Delimiters);
   if (un_Pos == std::string::npos)
   {
      return 0U;
   }
   return static_cast<uint32_t>(un_Pos) + 1U;
}

/// Base to parse a string in, matching C_SclString::ToInt() semantics: a
/// "0x"/"0X" prefix (after an optional '-') selects hex, everything else is
/// decimal. Note base 0 is deliberately NOT used here — it would treat a
/// leading "0" as octal, which the original explicitly did not do.
inline int ScanBaseCompat(const std::string & orc_Str)
{
   std::string::size_type un_Pos = 0U;

   if ((orc_Str.size() > 0U) && (orc_Str[0] == '-'))
   {
      un_Pos = 1U;
   }

   int x_Base = 10;
   if (((orc_Str.size() > (un_Pos + 1U)) && (orc_Str[un_Pos] == '0')) &&
       ((orc_Str[un_Pos + 1U] == 'x') || (orc_Str[un_Pos + 1U] == 'X')))
   {
      x_Base = 16;
   }
   return x_Base;
}

/// Replacement for str.ToInt(). Accepts decimal and "0x"-prefixed hex, and
/// throws when the string is not an integer.
///
/// Both halves of that matter, and the migration lost one of them at every call
/// site. C_SclString::ToInt() was hex-aware *and* threw on bad input, so callers
/// wrapped it in try/catch to validate. It was replaced in two different ways:
/// std::stoi(), which throws but is hard-wired to base 10 and so silently
/// returned 0 for every "0x..." string; and std::strtol(), which honours a base
/// but never throws and so silently returned 0 for outright garbage. Neither is
/// a faithful stand-in, and each failure is quiet. This is the faithful one.
///
/// Trailing characters are rejected rather than ignored. std::stoi() accepts
/// "1000junk" as 1000, but the call sites wrapping this are asking "is this
/// field a number?", and a partial parse answers that wrongly.
///
/// \throws std::invalid_argument  string is empty or not wholly an integer
/// \throws std::out_of_range      value does not fit in int32_t
inline int32_t ToIntCompat(const std::string & orc_Str)
{
   const char * const opcn_Str = orc_Str.c_str();
   char * pcn_End = nullptr;

   errno = 0;
   const long x_Value = std::strtol(opcn_Str, &pcn_End, ScanBaseCompat(orc_Str));

   if ((pcn_End == opcn_Str) || ((*pcn_End) != '\0'))
   {
      throw std::invalid_argument("ToIntCompat: string does not contain an integer");
   }
   if ((errno == ERANGE) || (x_Value < INT32_MIN) || (x_Value > INT32_MAX))
   {
      throw std::out_of_range("ToIntCompat: integer out of range");
   }
   return static_cast<int32_t>(x_Value);
}

/// Replacement for str.ToIntDef(default). Accepts decimal and "0x"-prefixed hex.
inline int32_t ToIntDefCompat(const std::string & orc_Str, const int32_t os32_Default)
{
   const char * opcn_Str = orc_Str.c_str();
   char * opcn_End = nullptr;
   const int32_t s32_Val = static_cast<int32_t>(std::strtol(opcn_Str, &opcn_End, ScanBaseCompat(orc_Str)));
   if (opcn_End == opcn_Str)
   {
      return os32_Default;
   }
   return s32_Val;
}

/// Replacement for str.ToInt64(). Accepts decimal and "0x"-prefixed hex, and
/// throws when the string is not an integer. See ToIntCompat for why.
///
/// \throws std::invalid_argument  string is empty or not wholly an integer
/// \throws std::out_of_range      value does not fit in int64_t
inline int64_t ToInt64Compat(const std::string & orc_Str)
{
   const char * const opcn_Str = orc_Str.c_str();
   char * pcn_End = nullptr;

   errno = 0;
   const long long x_Value = std::strtoll(opcn_Str, &pcn_End, ScanBaseCompat(orc_Str));

   if ((pcn_End == opcn_Str) || ((*pcn_End) != '\0'))
   {
      throw std::invalid_argument("ToInt64Compat: string does not contain an integer");
   }
   if (errno == ERANGE)
   {
      throw std::out_of_range("ToInt64Compat: integer out of range");
   }
   return static_cast<int64_t>(x_Value);
}

/// Replacement for str.ToDouble().
///
/// C_SclString::ToDouble() did three things that std::strtod() and std::stod()
/// do not, and the migration originally lost all three:
///
///  1. It parsed in the "C" locale explicitly. strtod() and stod() use the
///     *current* C locale, and Qt sets that from the environment during
///     application startup. On a German or French desktop the decimal separator
///     becomes "," and strtod("1.5") stops at the '.' and returns 1. Every
///     value in a .syde project, a DBC file or an EDS file is written with a
///     '.', so this silently truncates real data - and openSYDE's users are
///     largely in exactly those locales.
///  2. It accepted a comma as the decimal separator, replacing the first one
///     with a '.', so a hand-entered "1,5" still parsed.
///  3. It threw when the string was not a number. ToDoubleCompat wrapped
///     strtod(), which returns 0.0 instead - a malformed value read as a
///     legitimate zero.
///
/// Built on std::from_chars, which is locale-independent by specification. The
/// previous fix used an istringstream imbued with the classic locale, and that
/// turned out not to be portable: libstdc++ parses "1.5abc" as 1.5 while libc++
/// sets failbit and produces nothing, and the same divergence applies to "1e5x"
/// and "3.x". That is the standard library choosing how far to read before
/// deciding the input is bad, and the two disagree. from_chars returns exactly
/// where it stopped and gives the same answer on GCC/libstdc++ and Apple
/// clang/libc++ for every input tried.
///
/// Trailing characters are tolerated, matching the original, which extracted
/// through a stream and stopped at the first character it could not use. This
/// is deliberately laxer than ToIntCompat, which rejects them - the goal is to
/// restore documented behaviour, not to start rejecting project files that
/// have always loaded.
///
/// The two leniencies the stream had that from_chars lacks - skipping leading
/// whitespace and accepting a leading '+' - are handled explicitly so nothing
/// that parsed on Linux before stops parsing now.
///
/// \throws std::invalid_argument  string does not begin with a number
inline double ToDoubleCompat(const std::string & orc_Str)
{
   std::string c_Work = orc_Str;

   //replace up to one "," by "." - as the original did
   const std::string::size_type un_Comma = c_Work.find(',');
   if (un_Comma != std::string::npos)
   {
      c_Work[un_Comma] = '.';
   }

   const char * pcn_Begin = c_Work.c_str();
   const char * const pcn_End = pcn_Begin + c_Work.size();

   //the stream skipped leading whitespace and accepted a leading '+'; from_chars does neither
   while ((pcn_Begin < pcn_End) && (std::isspace(static_cast<unsigned char>(*pcn_Begin)) != 0))
   {
      ++pcn_Begin;
   }
   if ((pcn_Begin < pcn_End) && (*pcn_Begin == '+'))
   {
      ++pcn_Begin;
   }

   double f64_Value = 0.0;
#if defined(__cpp_lib_to_chars)
   //Preferred path: std::from_chars is locale-independent and gives identical results
   //across libstdc++ and Apple libc++ (see the note above).
   const std::from_chars_result c_Result = std::from_chars(pcn_Begin, pcn_End, f64_Value);
   if (c_Result.ec != std::errc())
   {
      throw std::invalid_argument("ToDoubleCompat: string does not contain a double value");
   }
#else
   //Fallback for standard libraries without floating-point std::from_chars (e.g.
   //LLVM-MinGW's libc++ 17, where the overload is =deleted). strtod tolerates
   //trailing characters like from_chars; parse against an explicit "C" locale so
   //the decimal point is always '.' (matching the ',' -> '.' step above) and the
   //result stays locale-independent regardless of the process locale -- plain
   //strtod would honour LC_NUMERIC and mis-parse under a comma-decimal locale.
   char * pcn_ParseEnd = nullptr;
#ifdef _WIN32
   static const _locale_t hx_CLocale = _create_locale(LC_ALL, "C");
   f64_Value = _strtod_l(pcn_Begin, &pcn_ParseEnd, hx_CLocale);
#else
   static const locale_t hx_CLocale = newlocale(LC_ALL_MASK, "C", static_cast<locale_t>(0));
   f64_Value = strtod_l(pcn_Begin, &pcn_ParseEnd, hx_CLocale);
#endif
   if (pcn_ParseEnd == pcn_Begin)
   {
      throw std::invalid_argument("ToDoubleCompat: string does not contain a double value");
   }
   static_cast<void>(pcn_End);
#endif
   return f64_Value;
}

/// Replacement for str.Printf(format, ...) — uses vsnprintf internally.
inline std::string PrintFormattedCompat(const char * const opcn_Format, ...)
{
   va_list c_Args;
   va_start(c_Args, opcn_Format);
   const int s32_Len = std::vsnprintf(nullptr, 0, opcn_Format, c_Args);
   va_end(c_Args);
   if (s32_Len < 0)
   {
      return "";
   }
   std::string c_Result(static_cast<size_t>(s32_Len), '\0');
   va_start(c_Args, opcn_Format);
   std::vsnprintf(&c_Result[0], static_cast<size_t>(s32_Len) + 1U, opcn_Format, c_Args);
   va_end(c_Args);
   return c_Result;
}

/* -- Helpers for retiring C_SclStringList (operate on std::vector<std::string>) -- */

/// Replacement for C_SclStringList::IndexOf — case-insensitive, -1 if not found.
inline int32_t VectorIndexOf(const std::vector<std::string> & orc_List, const std::string & orc_String)
{
   for (uint32_t u32_Index = 0U; u32_Index < static_cast<uint32_t>(orc_List.size()); ++u32_Index)
   {
      if (EqualsCaseInsensitive(orc_List[u32_Index], orc_String))
      {
         return static_cast<int32_t>(u32_Index);
      }
   }
   return -1;
}

/// Replacement for C_SclStringList::GetText — joins with a separator after each line.
inline std::string ListGetText(const std::vector<std::string> & orc_List,
                               const std::string & orc_LineSeparator = "\r\n")
{
   std::string c_Text;
   for (uint32_t u32_Index = 0U; u32_Index < static_cast<uint32_t>(orc_List.size()); ++u32_Index)
   {
      c_Text += (orc_List[u32_Index] + orc_LineSeparator);
   }
   return c_Text;
}

/// Replacement for C_SclStringList::LoadFromFile — preserves CRLF/LF and empty
/// lines, strips a trailing '\r', throws const char* on error (same contract).
inline void ListLoadFromFile(std::vector<std::string> & orc_List, const std::string & orc_FileName)
{
   std::FILE * pc_File;
   long x_FileSize;
   char * pcn_Buffer;
   size_t x_SizeRead;
   long x_Index;
   uint32_t u32_NumStrings;
   int32_t s32_Len; //2GB file size limit is acceptable

   orc_List.clear();
   pc_File = std::fopen(orc_FileName.c_str(), "rb");
   if (pc_File == NULL)
   {
      throw ("C_SclStringList::LoadFromFile: file not found");
   }

   (void)std::fseek(pc_File, 0, SEEK_END);
   x_FileSize = std::ftell(pc_File);
   (void)std::fseek(pc_File, 0, SEEK_SET);

   try
   {
      pcn_Buffer = new char[x_FileSize + 1U]; //+ 1: put a terminating \0
   }
   catch (...)
   {
      throw ("C_SclStringList::LoadFromFile: could not allocate buffer for file");
   }
   pcn_Buffer[x_FileSize] = '\0';

   x_SizeRead = std::fread(pcn_Buffer, 1U, static_cast<size_t>(x_FileSize), pc_File);
   if (x_SizeRead != static_cast<size_t>(x_FileSize))
   {
      delete[] pcn_Buffer;
      throw ("C_SclStringList::LoadFromFile: could not read file");
   }

   (void)std::fclose(pc_File);

   //count lines and split the file contents (see original implementation for details)
   u32_NumStrings = 0U;
   for (x_Index = 0; x_Index < x_FileSize; x_Index++)
   {
      if (pcn_Buffer[x_Index] == '\n')
      {
         pcn_Buffer[x_Index] = '\0';
         u32_NumStrings++;
      }
   }
   //maybe the last line is not terminated?
   if ((x_Index > 0) && (pcn_Buffer[x_Index - 1U] != '\0'))
   {
      u32_NumStrings++;
   }

   try
   {
      orc_List.resize(u32_NumStrings);
   }
   catch (...)
   {
      delete[] pcn_Buffer;
      throw ("C_SclStringList::LoadFromFile: could not allocate buffer for file");
   }

   x_Index = 0;
   for (uint32_t u32_Line = 0U; u32_Line < u32_NumStrings; u32_Line++)
   {
      const char * const pcn_String = &pcn_Buffer[x_Index];

      s32_Len = static_cast<int32_t>(std::strlen(pcn_String));
      x_Index += (s32_Len + 1); //skip to next string

      //Do we need to strip a final '\r'?
      if ((s32_Len > 0) && (pcn_String[s32_Len - 1] == '\r'))
      {
         s32_Len--; //one character less in this string
      }

      orc_List[u32_Line].assign(pcn_String, static_cast<size_t>(s32_Len));
   }

   delete[] pcn_Buffer;
}

/// Replacement for C_SclStringList::SaveToFile — writes each line followed by
/// "\r\n" (also after the last line), throws const char* on error (same contract).
inline void ListSaveToFile(const std::vector<std::string> & orc_List, const std::string & orc_FileName)
{
   uint32_t u32_NumWritten;

   std::FILE * const pc_File = std::fopen(orc_FileName.c_str(), "wb");
   if (pc_File == NULL)
   {
      throw ("C_SclStringList::SaveToFile: could not create file");
   }

   for (uint32_t u32_Line = 0U; u32_Line < static_cast<uint32_t>(orc_List.size()); u32_Line++)
   {
      u32_NumWritten = std::fwrite(orc_List[u32_Line].c_str(), 1U, orc_List[u32_Line].length(), pc_File);
      if (u32_NumWritten != orc_List[u32_Line].length())
      {
         (void)std::fclose(pc_File);
         throw ("C_SclStringList::SaveToFile: could not write to file");
      }
      u32_NumWritten = std::fwrite("\r\n", 1U, 2U, pc_File);
      if (u32_NumWritten != 2U)
      {
         (void)std::fclose(pc_File);
         throw ("C_SclStringList::SaveToFile: could not write to file");
      }
   }
   (void)std::fclose(pc_File);
}

/// Replacement for C_SclStringList::IndexOfName — case-insensitive, allows
/// blanks before the '=', returns index or -1.
inline int32_t ListIndexOfName(const std::vector<std::string> & orc_List, const std::string & orc_Name)
{
   bool q_Found = false;
   int32_t s32_Index;
   std::string c_Remainder;
   uint32_t u32_Pos;

   for (s32_Index = 0; s32_Index < static_cast<int32_t>(orc_List.size()); s32_Index++)
   {
      u32_Pos = FindCaseInsensitive(orc_List[s32_Index], orc_Name);
      if (u32_Pos == 1U)
      {
         //there must be a subsequent "=" (may be preceeded by blanks)
         c_Remainder = TrimLeftCompat(SubStringCompat(orc_List[s32_Index], orc_Name.length() + 1U,
                                                      static_cast<uint32_t>(orc_List[s32_Index].length())));
         if (!c_Remainder.empty() && (c_Remainder.c_str()[0] == '='))
         {
            q_Found = true;
            break;
         }
      }
   }
   if (q_Found == false)
   {
      s32_Index = -1;
   }

   return s32_Index;
}

/// Replacement for C_SclStringList::AddStrings.
inline void ListAddStrings(std::vector<std::string> & orc_Target, const std::vector<std::string> & orc_Source)
{
   orc_Target.insert(orc_Target.end(), orc_Source.begin(), orc_Source.end());
}

} // namespace scl
} // namespace stw

#endif
