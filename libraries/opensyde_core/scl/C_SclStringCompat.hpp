//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief    Compatibility helpers for migrating from C_SclString to std::string

   This header provides inline helper functions that map C_SclString methods to
   std::string equivalents. Include it in files being migrated from C_SclString
   to std::string to ease the transition.

   Once a file no longer uses any C_SclString type, this include can be removed.

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CSCLSTRINGCOMPATH
#define CSCLSTRINGCOMPATH

#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include <vector>
#include "stwtypes.hpp"

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
   std::ostringstream c_Stream;
   c_Stream << orc_Value;
   return c_Stream.str();
}

/// Replacement for C_SclString::IntToHex(value, digits).
/// Formats an integer as zero-padded uppercase hex WITHOUT "0x" prefix.
template <typename T>
inline std::string IntToHexCompat(const T orc_Value, const uint32_t ou32_Digits)
{
   std::stringstream c_Stream;
   c_Stream << std::hex << std::uppercase << std::setw(ou32_Digits) << std::setfill('0') << orc_Value;
   return c_Stream.str();
}

/// Replacement for C_SclString::FloatToStr(value).
inline std::string FloatToStrCompat(const double of64_Value)
{
   std::ostringstream c_Stream;
   c_Stream << of64_Value;
   return c_Stream.str();
}

/// Replacement for C_SclString::FloatToStr(value, digits).
inline std::string FloatToStrCompat(const double of64_Value, const int32_t os32_Digits)
{
   std::stringstream c_Stream;
   c_Stream.precision(os32_Digits);
   c_Stream << std::fixed << of64_Value;
   return c_Stream.str();
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

/// Replacement for str.ToInt().
inline int32_t ToIntCompat(const std::string & orc_Str)
{
   return static_cast<int32_t>(std::strtol(orc_Str.c_str(), NULL, 10));
}

/// Replacement for str.ToIntDef(default).
inline int32_t ToIntDefCompat(const std::string & orc_Str, const int32_t os32_Default)
{
   const char_t * opcn_Str = orc_Str.c_str();
   char_t * opcn_End = NULL;
   const int32_t s32_Val = static_cast<int32_t>(std::strtol(opcn_Str, &opcn_End, 10));
   if (opcn_End == opcn_Str)
   {
      return os32_Default;
   }
   return s32_Val;
}

/// Replacement for str.ToInt64().
inline int64_t ToInt64Compat(const std::string & orc_Str)
{
   return static_cast<int64_t>(std::strtoll(orc_Str.c_str(), NULL, 10));
}

/// Replacement for str.ToDouble().
inline double ToDoubleCompat(const std::string & orc_Str)
{
   return std::strtod(orc_Str.c_str(), NULL);
}

/// Replacement for str.Printf(format, ...) — uses vsnprintf internally.
inline std::string PrintFormattedCompat(const char_t * const opcn_Format, ...)
{
   va_list c_Args;
   va_start(c_Args, opcn_Format);
   const int s32_Len = std::vsnprintf(NULL, 0, opcn_Format, c_Args);
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
      if (LowerCaseCompat(orc_List[u32_Index]).compare(LowerCaseCompat(orc_String)) == 0)
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
   char_t * pcn_Buffer;
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
      pcn_Buffer = new char_t[x_FileSize + 1U]; //+ 1: put a terminating \0
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
      const char_t * const pcn_String = &pcn_Buffer[x_Index];

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
   const std::string c_Search = UpperCaseCompat(orc_Name);
   std::string c_Remainder;
   uint32_t u32_Pos;

   for (s32_Index = 0; s32_Index < static_cast<int32_t>(orc_List.size()); s32_Index++)
   {
      u32_Pos = PosCompat(UpperCaseCompat(orc_List[s32_Index]), c_Search);
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
