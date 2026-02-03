//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       INI file class

   \class       stw::scl::C_SclIniFile
   \brief       ANSI C++ INI file class.

   Aim: provide most of the functionality that Borland's TIniFile does
    while only using ANSI C++ (with STL in this case).
   So cf. the documentation of the VCL TIniFile for details on most API functions.

   Instead of AnsiString/UnicodeString the C_SclString class is used (which is not Unicode-capable).

   This implementation uses QSettings internally for INI file handling.

   Additionally the following is supported:
   - INI files > 64kB
   - using file path "" in the constructor can be used to create a INI file structure in memory only
   - GetFileAsStringList can be used to save INI file content to a string list in the same format used for the file

   Note: Comments in INI files are NOT preserved when using this QSettings-based implementation.

   What is NOT implemented (compared to Borland VCL TIniFile):
   - ReadDate
   - ReadDateTime
   - ReadTime
   - WriteDate
   - WriteDateTime
   - WriteTime
   - stream-based functions

   \copyright   Copyright 2009 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SCLINIFILEHPP
#define C_SCLINIFILEHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include <memory>
#include <QSettings>
#include <QTemporaryFile>
#include "stwtypes.hpp"
#include "C_SclString.hpp"
// QString is used for all string operations

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace scl
{
/* -- Defines ------------------------------------------------------------------------------------------------------- */

//unfortunately we need to mark throwing constructors and destructors specially in C++11
#ifndef SCL_WILL_THROW
#if __cplusplus >= 201103L //C++11 ?
#define SCL_WILL_THROW  noexcept (false)
#else
//not required (will be expected to throw by default)
#define SCL_WILL_THROW
#endif
#endif

/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

//----------------------------------------------------------------------------------------------------------------------
///INI file handling
class C_SclIniFile
{
protected:
   std::unique_ptr<QSettings> mpc_Settings;    ///< QSettings instance for INI handling
   std::unique_ptr<QTemporaryFile> mpc_TempFile; ///< Temporary file for in-memory mode
   bool mq_Dirty;                              ///< Tracks whether or not data has changed.
   bool mq_InMemoryMode;                       ///< True if operating in memory-only mode

   // Helper to build section/key path for QSettings
   static QString mh_BuildKey(const C_SclString & orc_Section, const C_SclString & orc_Key);

   // Helper to write string list to file
   static void mh_WriteStringListToFile(const QStringList & orc_Strings, const QString & orc_FileName);

public:
   // Constructors & Destructors
   C_SclIniFile(const C_SclString & orc_FileName);
   virtual ~C_SclIniFile() SCL_WILL_THROW;

   C_SclString ReadString (const C_SclString & orc_Section, const C_SclString & orc_Key,
                           const C_SclString & orc_Default);
   int32_t   ReadInteger(const C_SclString & orc_Section, const C_SclString & orc_Key, const int32_t os32_Default);
   bool      ReadBool   (const C_SclString & orc_Section, const C_SclString & orc_Key, const bool oq_Default);
   float64_t ReadFloat  (const C_SclString & orc_Section, const C_SclString & orc_Key, const float64_t of64_Default);

   //convenience shortcuts for commonly used stw_types:
   uint8_t   ReadUint8 (const C_SclString & orc_Section, const C_SclString & orc_Key, const uint8_t ou8_Default);
   uint16_t  ReadUint16(const C_SclString & orc_Section, const C_SclString & orc_Key, const uint16_t ou16_Default);

   void WriteString (const C_SclString & orc_Section, const C_SclString & orc_Key, const C_SclString & orc_Value,
                     const bool oq_ForceAppend = false);
   void WriteInteger(const C_SclString & orc_Section, const C_SclString & orc_Key, const int32_t os32_Value,
                     const bool oq_ForceAppend = false);
   void WriteBool   (const C_SclString & orc_Section, const C_SclString & orc_Key, const bool oq_Value,
                     const bool oq_ForceAppend = false);
   void WriteFloat  (const C_SclString & orc_Section, const C_SclString & orc_Key, const float64_t of64_Value,
                     const bool oq_ForceAppend = false);

   void EraseSection(const C_SclString & orc_Section);
   void DeleteKey(const C_SclString & orc_Section, const C_SclString & orc_Key);

   void UpdateFile(void);

   bool SectionExists(const C_SclString & orc_Section);
   bool ValueExists(const C_SclString & orc_Section, const C_SclString & orc_Key);

   void ReadSection(const C_SclString & orc_Section, QStringList * const opc_Strings, const bool oq_Append = false);
   void ReadSectionValues(const C_SclString & orc_Section, QStringList * const opc_Strings,
                          const bool oq_Append = false);
   void ReadSections(QStringList * const opc_Strings, const bool oq_Append = false) const;

   void GetFileAsStringList(QStringList & orc_Strings) const;

   C_SclString FileName; ///< path to ini file; can be used after creation to store data in another ini file
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}
#endif
