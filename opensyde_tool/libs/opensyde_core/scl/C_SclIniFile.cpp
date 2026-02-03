//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       INI file class

   ANSI C++ INI file class using QSettings internally.
   For details cf. documentation in .h file.

   \copyright   Copyright 2009 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp" //pre-compiled headers

#include <QFile>
#include <QTextStream>

#include "C_SclIniFile.hpp"
#include "C_SclString.hpp"
#include "C_SclStringList.hpp"
#include "stwtypes.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::scl;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Helper to build section/key path for QSettings

   QSettings uses "/" as separator between section and key.

   \param[in]  orc_Section     INI section
   \param[in]  orc_Key         INI key

   \return
   Combined key path for QSettings (e.g., "Section/Key")
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_SclIniFile::mh_BuildKey(const C_SclString & orc_Section, const C_SclString & orc_Key)
{
   return orc_Section.ToQString() + "/" + orc_Key.ToQString();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Constructor

   If it can load the file, it will do so and populate
   the section list with the values from the file.

   \param[in]     orc_FileName    path to ini file
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclIniFile::C_SclIniFile(const C_SclString & orc_FileName) :
   mq_Dirty(false),
   mq_InMemoryMode(false),
   FileName(orc_FileName)
{
   if (orc_FileName == "")
   {
      // In-memory mode: use a temporary file that will be deleted when we're done
      mq_InMemoryMode = true;
      mpc_TempFile = std::make_unique<QTemporaryFile>();
      if (mpc_TempFile->open())
      {
         mpc_Settings = std::make_unique<QSettings>(mpc_TempFile->fileName(), QSettings::IniFormat);
      }
   }
   else
   {
      // Normal file mode
      mpc_Settings = std::make_unique<QSettings>(orc_FileName.ToQString(), QSettings::IniFormat);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Destructor

   Saves the file if any values have changed since the last save.

   Throws if
   - the filename is empty (and not in memory mode)
   - writing to the file fails (e.g. write protected)
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclIniFile::~C_SclIniFile() SCL_WILL_THROW
{
   if ((mq_Dirty == true) && (FileName != ""))
   {
      UpdateFile();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write data to ini file

   Attempts to save the Section list and keys to the file.
   Throws if
   - the filename is empty
   - writing to the file fails (e.g. write protected)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclIniFile::UpdateFile(void)
{
   if (FileName.Length() == 0U)
   {
      throw "C_SclIniFile::UpdateFile failed !";
   }

   if (mpc_Settings != nullptr)
   {
      // If FileName was changed after construction, we need to handle it
      if (FileName.ToQString() != mpc_Settings->fileName())
      {
         // Need to write to a different file - use GetFileAsStringList and save manually
         QStringList c_Strings;
         this->GetFileAsStringList(c_Strings);
         mh_WriteStringListToFile(c_Strings, FileName.ToQString());
      }
      else
      {
         mpc_Settings->sync();
         if (mpc_Settings->status() != QSettings::NoError)
         {
            throw "C_SclIniFile::UpdateFile: could not write to file !";
         }
      }
   }

   mq_Dirty = false;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write string value to ini file

   Writes a string value.

   If oq_ForceAppend==true a new key will be appended to the end of the section
   without checking if this key already exists. Caution: This could produce a
   corrupted INI file but makes a big performance improvement when generating a
   new file.

   Note: With QSettings, ForceAppend is ignored as QSettings handles duplicate keys.

   Throws if orc_Section or orc_Key are empty strings.

   \param[in]  orc_Section     section to write to
   \param[in]  orc_Key         key to write to
   \param[in]  orc_Value       value to write
   \param[in]  oq_ForceAppend  ignored in QSettings implementation
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclIniFile::WriteString(const C_SclString & orc_Section, const C_SclString & orc_Key,
                               const C_SclString & orc_Value, const bool oq_ForceAppend)
{
   (void)oq_ForceAppend; // Not needed with QSettings

   if ((orc_Section == "") || (orc_Key == ""))
   {
      throw "C_SclIniFile::WriteString failed !";
   }

   if (mpc_Settings != nullptr)
   {
      mpc_Settings->setValue(mh_BuildKey(orc_Section, orc_Key), orc_Value.Trim().ToQString());
      mq_Dirty = true;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write int32_t value to ini file

   Writes an int32_t value.

   If oq_ForceAppend==true a new key will be appended to the end of the section
   without checking if this key already exists. Caution: This could produce a
   corrupted INI file but makes a big performance improvement when generating a
   new file.

   Throws if orc_Section or orc_Key are empty strings.

   \param[in]  orc_Section     section to write to
   \param[in]  orc_Key         key to write to
   \param[in]  os32_Value      value to write
   \param[in]  oq_ForceAppend  ignored in QSettings implementation
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclIniFile::WriteInteger(const C_SclString & orc_Section, const C_SclString & orc_Key, const int32_t os32_Value,
                                const bool oq_ForceAppend)
{
   (void)oq_ForceAppend; // Not needed with QSettings

   if ((orc_Section == "") || (orc_Key == ""))
   {
      throw "C_SclIniFile::WriteInteger failed !";
   }

   if (mpc_Settings != nullptr)
   {
      mpc_Settings->setValue(mh_BuildKey(orc_Section, orc_Key), os32_Value);
      mq_Dirty = true;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write bool value to ini file

   Writes a bool value.

   If oq_ForceAppend==true a new key will be appended to the end of the section
   without checking if this key already exists. Caution: This could produce a
   corrupted INI file but makes a big performance improvement when generating a
   new file.

   Throws if orc_Section or orc_Key are empty strings.

   \param[in]  orc_Section     section to write to
   \param[in]  orc_Key         key to write to
   \param[in]  oq_Value        value to write
   \param[in]  oq_ForceAppend  ignored in QSettings implementation
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclIniFile::WriteBool(const C_SclString & orc_Section, const C_SclString & orc_Key, const bool oq_Value,
                             const bool oq_ForceAppend)
{
   (void)oq_ForceAppend; // Not needed with QSettings

   if ((orc_Section == "") || (orc_Key == ""))
   {
      throw "C_SclIniFile::WriteBool failed !";
   }

   if (mpc_Settings != nullptr)
   {
      // Write as integer for compatibility with existing code that reads bools as integers
      mpc_Settings->setValue(mh_BuildKey(orc_Section, orc_Key), static_cast<int32_t>(oq_Value));
      mq_Dirty = true;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write float64 value to ini file

   Writes a float64 value.

   If oq_ForceAppend==true a new key will be appended to the end of the section
   without checking if this key already exists. Caution: This could produce a
   corrupted INI file but makes a big performance improvement when generating a
   new file.

   Throws if orc_Section or orc_Key are empty strings.

   \param[in]  orc_Section     section to write to
   \param[in]  orc_Key         key to write to
   \param[in]  of64_Value      value to write
   \param[in]  oq_ForceAppend  ignored in QSettings implementation
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclIniFile::WriteFloat(const C_SclString & orc_Section, const C_SclString & orc_Key, const float64_t of64_Value,
                              const bool oq_ForceAppend)
{
   (void)oq_ForceAppend; // Not needed with QSettings

   if ((orc_Section == "") || (orc_Key == ""))
   {
      throw "C_SclIniFile::WriteFloat failed !";
   }

   if (mpc_Settings != nullptr)
   {
      mpc_Settings->setValue(mh_BuildKey(orc_Section, orc_Key), of64_Value);
      mq_Dirty = true;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   read a C_SclString value

   Read a C_SclString value from the ini file.
   In the following cases the default will be used:
   - section not present
   - key not present
   - value is empty

   \param[in]     orc_Section    ini section
   \param[in]     orc_Key        ini key
   \param[in]     orc_Default    default value

   \return
   C_SclString value from ini (or default if error)
*/
//----------------------------------------------------------------------------------------------------------------------
C_SclString C_SclIniFile::ReadString(const C_SclString & orc_Section, const C_SclString & orc_Key,
                                     const C_SclString & orc_Default)
{
   C_SclString c_Result = orc_Default;

   if (mpc_Settings != nullptr)
   {
      const QString c_Key = mh_BuildKey(orc_Section, orc_Key);
      if (mpc_Settings->contains(c_Key))
      {
         const QString c_Value = mpc_Settings->value(c_Key).toString();
         if (!c_Value.isEmpty())
         {
            c_Result = C_SclString::FromQString(c_Value);
         }
      }
   }

   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   read a uint8 value

   Read a uint8 value from the ini file.
   In the following cases the default will be used:
   - section not present
   - key not present
   - value in ini is not a number
   - value in ini is < 0 or > 0xFF

   \param[in]     orc_Section    ini section
   \param[in]     orc_Key        ini key
   \param[in]     ou8_Default    default value

   \return
   uint8 value from ini (or default if error)
*/
//----------------------------------------------------------------------------------------------------------------------
uint8_t C_SclIniFile::ReadUint8(const C_SclString & orc_Section, const C_SclString & orc_Key, const uint8_t ou8_Default)
{
   int32_t s32_Return;

   s32_Return = ReadInteger(orc_Section, orc_Key, ou8_Default);
   if ((s32_Return > 0xFF) || (s32_Return < 0))
   {
      s32_Return = ou8_Default;
   }
   return static_cast<uint8_t>(s32_Return);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   read a uint16 value

   Read a uint16 value from the ini file.
   In the following cases the default will be used:
   - section not present
   - key not present
   - value in ini is not a number
   - value in ini is < 0 or > 0xFFFF

   \param[in]     orc_Section    ini section
   \param[in]     orc_Key        ini key
   \param[in]     ou16_Default   default value

   \return
   uint16 value from ini (or default if error)
*/
//----------------------------------------------------------------------------------------------------------------------
uint16_t C_SclIniFile::ReadUint16(const C_SclString & orc_Section, const C_SclString & orc_Key,
                                  const uint16_t ou16_Default)
{
   int32_t s32_Return;

   s32_Return = ReadInteger(orc_Section, orc_Key, ou16_Default);
   if ((s32_Return < 0) || (s32_Return > 0xFFFF))
   {
      s32_Return = ou16_Default;
   }
   return static_cast<uint16_t>(s32_Return);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   read an int32_t value

   Read an int32_t value from the ini file.
   In the following cases the default will be used:
   - section not present
   - key not present
   - value in ini is not a number (or not "TRUE" or "FALSE")

   A value of "TRUE" (ignoring casing) is returned as the value 1.
   A value of "FALSE" (ignoring casing) is returned as the value 0.

   \param[in]     orc_Section    ini section
   \param[in]     orc_Key        ini key
   \param[in]     os32_Default   default value

   \return
   int32_t value from ini (or default if error)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_SclIniFile::ReadInteger(const C_SclString & orc_Section, const C_SclString & orc_Key,
                                  const int32_t os32_Default)
{
   int32_t s32_Return = os32_Default;

   if (mpc_Settings != nullptr)
   {
      const QString c_Key = mh_BuildKey(orc_Section, orc_Key);
      if (mpc_Settings->contains(c_Key))
      {
         const QVariant c_Value = mpc_Settings->value(c_Key);
         bool q_Ok = false;
         const int32_t s32_Value = c_Value.toInt(&q_Ok);

         if (q_Ok)
         {
            s32_Return = s32_Value;
         }
         else
         {
            // Check for TRUE/FALSE strings
            const QString c_StringValue = c_Value.toString().toUpper();
            if (c_StringValue == "TRUE")
            {
               s32_Return = 1;
            }
            else if (c_StringValue == "FALSE")
            {
               s32_Return = 0;
            }
            // else use default
         }
      }
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   read a bool value

   Read a bool value from the ini file.
   In the following cases the default will be used:
   - section not present
   - key not present
   - value in ini is not a number or not "TRUE" or "FALSE"

   A value of "1" or "TRUE" (ignoring casing) is returned as the value 1.
   A value of "0" or "FALSE" (ignoring casing) is returned as the value 0.

   \param[in]     orc_Section    ini section
   \param[in]     orc_Key        ini key
   \param[in]     oq_Default     default value

   \return
   bool value from ini (or default if error)
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_SclIniFile::ReadBool(const C_SclString & orc_Section, const C_SclString & orc_Key, const bool oq_Default)
{
   return ((ReadInteger(orc_Section, orc_Key, static_cast<int32_t>(oq_Default)) == 0) ? false : true);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   read a float64 value

   Read a float64 value from the ini file.
   In the following cases the default will be used:
   - section not present
   - key not present
   - value in ini is not convertible to float64

   Will accept "." or "," as decimal separator.

   \param[in]     orc_Section    ini section
   \param[in]     orc_Key        ini key
   \param[in]     of64_Default   default value

   \return
   float64 value from ini (or default if error)
*/
//----------------------------------------------------------------------------------------------------------------------
float64_t C_SclIniFile::ReadFloat(const C_SclString & orc_Section, const C_SclString & orc_Key,
                                  const float64_t of64_Default)
{
   float64_t f64_Return = of64_Default;

   if (mpc_Settings != nullptr)
   {
      const QString c_Key = mh_BuildKey(orc_Section, orc_Key);
      if (mpc_Settings->contains(c_Key))
      {
         const QVariant c_Value = mpc_Settings->value(c_Key);
         bool q_Ok = false;
         const float64_t f64_Value = c_Value.toDouble(&q_Ok);

         if (q_Ok)
         {
            f64_Return = f64_Value;
         }
         else
         {
            // Try with comma replaced by dot
            QString c_StringValue = c_Value.toString();
            c_StringValue.replace(',', '.');
            f64_Return = c_StringValue.toDouble(&q_Ok);
            if (!q_Ok)
            {
               f64_Return = of64_Default;
            }
         }
      }
   }

   return f64_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Erase whole section

   Erase one section from the ini file.
   Will throw if the section is not found.

   \param[in]     orc_Section    ini section to erase
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclIniFile::EraseSection(const C_SclString & orc_Section)
{
   if (mpc_Settings != nullptr)
   {
      const QString c_Section = orc_Section.ToQString();

      // Check if section exists
      mpc_Settings->beginGroup(c_Section);
      const QStringList c_Keys = mpc_Settings->childKeys();
      mpc_Settings->endGroup();

      if (c_Keys.isEmpty() && !mpc_Settings->childGroups().contains(c_Section))
      {
         throw "C_SclIniFile::EraseSection failed !";
      }

      mpc_Settings->remove(c_Section);
      mq_Dirty = true;
   }
   else
   {
      throw "C_SclIniFile::EraseSection failed !";
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Delete one key

   Delete a specific key and its value in a specific section.
   If the section or the key is not found the function will do nothing.

   \param[in]     orc_Section    ini section containing key to erase
   \param[in]     orc_Key        key to erase
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclIniFile::DeleteKey(const C_SclString & orc_Section, const C_SclString & orc_Key)
{
   if (mpc_Settings != nullptr)
   {
      const QString c_Key = mh_BuildKey(orc_Section, orc_Key);
      if (mpc_Settings->contains(c_Key))
      {
         mpc_Settings->remove(c_Key);
         mq_Dirty = true;
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Detect whether a section exists.

   Checks whether a specified section exists in the ini file.

   \param[in]     orc_Section    section to check

   \return
   true    section exists
   false   section does not exist
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_SclIniFile::SectionExists(const C_SclString & orc_Section)
{
   bool q_Exists = false;

   if (mpc_Settings != nullptr)
   {
      const QString c_Section = orc_Section.ToQString();

      // Check if section has any keys
      mpc_Settings->beginGroup(c_Section);
      q_Exists = !mpc_Settings->childKeys().isEmpty();
      mpc_Settings->endGroup();

      // Also check if it's in the group list (for sections with subsections but no keys)
      if (!q_Exists)
      {
         q_Exists = mpc_Settings->childGroups().contains(c_Section);
      }
   }

   return q_Exists;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Detect whether a key exists.

   Checks whether a specified key exists within a specified section in the ini file.

   \param[in]     orc_Section    section to check
   \param[in]     orc_Key        key to check

   \return
   true    key exists in section
   false   key does not exist in section
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_SclIniFile::ValueExists(const C_SclString & orc_Section, const C_SclString & orc_Key)
{
   bool q_Exists = false;

   if (mpc_Settings != nullptr)
   {
      q_Exists = mpc_Settings->contains(mh_BuildKey(orc_Section, orc_Key));
   }

   return q_Exists;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read names of keys of section

   Places the names of all keys of one section into a string list.

   \param[in]  orc_Section     section to read from
   \param[out] opc_Strings     target string list (is not checked for validity)
   \param[in]  oq_Append       true: append to string list
                               false: clear string list before adding keys
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclIniFile::ReadSection(const C_SclString & orc_Section, C_SclStringList * const opc_Strings,
                               const bool oq_Append)
{
   if (mpc_Settings != nullptr)
   {
      uint32_t u32_OldLength;

      if (oq_Append == true)
      {
         u32_OldLength = opc_Strings->GetCount();
      }
      else
      {
         u32_OldLength = 0U;
         opc_Strings->Clear();
      }

      mpc_Settings->beginGroup(orc_Section.ToQString());
      const QStringList c_Keys = mpc_Settings->childKeys();
      mpc_Settings->endGroup();

      opc_Strings->Strings.resize(static_cast<int32_t>(u32_OldLength) + c_Keys.size());
      for (int32_t s32_Loop = 0; s32_Loop < c_Keys.size(); s32_Loop++)
      {
         opc_Strings->Strings[static_cast<int32_t>(u32_OldLength) + s32_Loop] =
            c_Keys[s32_Loop];
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read keys and values of one section

   Places the keys and values of one section into a string list.
   The format for each entry is "key=value".

   \param[in]  orc_Section     section to read from
   \param[out] opc_Strings     target string list (is not checked for validity)
   \param[in]  oq_Append       true: append to string list
                               false: clear string list before adding keys
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclIniFile::ReadSectionValues(const C_SclString & orc_Section, C_SclStringList * const opc_Strings,
                                     const bool oq_Append)
{
   if (mpc_Settings != nullptr)
   {
      uint32_t u32_OldLength;

      if (oq_Append == true)
      {
         u32_OldLength = opc_Strings->GetCount();
      }
      else
      {
         u32_OldLength = 0U;
         opc_Strings->Clear();
      }

      const QString c_Section = orc_Section.ToQString();
      mpc_Settings->beginGroup(c_Section);
      const QStringList c_Keys = mpc_Settings->childKeys();
      mpc_Settings->endGroup();

      opc_Strings->Strings.resize(static_cast<int32_t>(u32_OldLength) + c_Keys.size());
      for (int32_t s32_Loop = 0; s32_Loop < c_Keys.size(); s32_Loop++)
      {
         const QString c_FullKey = c_Section + "/" + c_Keys[s32_Loop];
         const QString c_Value = mpc_Settings->value(c_FullKey).toString();
         opc_Strings->Strings[static_cast<int32_t>(u32_OldLength) + s32_Loop] =
            c_Keys[s32_Loop] + "=" + c_Value;
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read names of all sections

   Places the names of all sections into a string list.

   \param[out] opc_Strings     target string list (is not checked for validity)
   \param[in]  oq_Append       true: append to string list
                               false: clear string list before adding keys
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclIniFile::ReadSections(C_SclStringList * const opc_Strings, const bool oq_Append) const
{
   if (mpc_Settings != nullptr)
   {
      uint32_t u32_OldLength;

      if (oq_Append == true)
      {
         u32_OldLength = opc_Strings->GetCount();
      }
      else
      {
         u32_OldLength = 0U;
         opc_Strings->Clear();
      }

      const QStringList c_Groups = mpc_Settings->childGroups();

      opc_Strings->Strings.resize(static_cast<int32_t>(u32_OldLength) + c_Groups.size());
      for (int32_t s32_Loop = 0; s32_Loop < c_Groups.size(); s32_Loop++)
      {
         opc_Strings->Strings[static_cast<int32_t>(u32_OldLength) + s32_Loop] =
            c_Groups[s32_Loop];
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   place INI file content into a string list

   Will write the current ini file content to the specified string list.
   The format is identical to the one used for writing the actual INI file.

   Note: Comments are NOT included as QSettings does not preserve them.

   \param[out]     orc_Strings    content of INI file (will be cleared before adding INI file strings)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclIniFile::GetFileAsStringList(C_SclStringList & orc_Strings) const
{
   orc_Strings.Clear();

   if (mpc_Settings != nullptr)
   {
      const QStringList c_Groups = mpc_Settings->childGroups();

      for (int32_t s32_Section = 0; s32_Section < c_Groups.size(); s32_Section++)
      {
         const QString & rc_Section = c_Groups[s32_Section];

         // Add section header
         orc_Strings.Add("[" + rc_Section + "]");

         // Add keys in this section
         mpc_Settings->beginGroup(rc_Section);
         const QStringList c_Keys = mpc_Settings->childKeys();
         for (int32_t s32_Key = 0; s32_Key < c_Keys.size(); s32_Key++)
         {
            const QString & rc_Key = c_Keys[s32_Key];
            const QString c_Value = mpc_Settings->value(rc_Key).toString();
            orc_Strings.Add(rc_Key + "=" + c_Value);
         }
         mpc_Settings->endGroup();

         // Add blank line after section (except for last section)
         if (s32_Section < (c_Groups.size() - 1))
         {
            orc_Strings.Add("");
         }
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Helper to write string list to file

   Writes the content of a string list to a text file.
   Each string in the list becomes one line in the file.

   \param[in]  orc_Strings    string list to write
   \param[in]  orc_FileName   target file path
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclIniFile::mh_WriteStringListToFile(const QStringList & orc_Strings, const QString & orc_FileName)
{
   QFile c_File(orc_FileName);
   if (c_File.open(QIODevice::WriteOnly | QIODevice::Text))
   {
      QTextStream c_Stream(&c_File);
      for (const QString & rc_Line : orc_Strings)
      {
         c_Stream << rc_Line << "\n";
      }
      c_File.close();
   }
   else
   {
      throw "C_SclIniFile::mh_WriteStringListToFile failed !";
   }
}

//----------------------------------------------------------------------------------------------------------------------
