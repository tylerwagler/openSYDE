//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       string list class

   ANSI C++ string list class.
   For details cf. documentation in .h file.

   \copyright   Copyright 2009 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp" //pre-compiled headers

#include "C_SclStringList.hpp"
#include "stwtypes.hpp"
#include <QFile>
#include <QTextStream>

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

namespace stw {
namespace scl {

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
/*! \brief   Append one string

   Append one string at the end of the list

   \param[in]  orc_String   string to append

   \return
   index of new string
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_SclStringList::Add(const QString &orc_String) {
  Strings.append(orc_String);
  return static_cast<uint32_t>(Strings.size() - 1);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Append one string

   Append one string at the end of the list.
   Functionally identical to C_SclStringList::Add but without the return value.

   \param[in]  orc_String   string to append
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclStringList::Append(const QString &orc_String) {
  Strings.append(orc_String);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Clear all strings

   Empty the string list.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclStringList::Clear(void) { Strings.clear(); }

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Delete one string

   Delete one string from the string list.
   Will throw if the index is invalid.

   \param[in]  ou32_Index   index of string to remove (0 = first string)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclStringList::Delete(const uint32_t ou32_Index) {
  Strings.removeAt(static_cast<int>(ou32_Index));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Swap the contents of two strings

   Swap the tests of two strings.
   Will throw if the index is invalid.
   If one of the indexes is out of range the function has undefined bahavior.

   \param[in]  ou32_Index1   index of first string to swap (0 = first string in
   list)
   \param[in]  ou32_Index2   index of second string to swap (0 = first string in
   list)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclStringList::Exchange(const uint32_t ou32_Index1,
                               const uint32_t ou32_Index2) {
  Strings.swapItemsAt(static_cast<int>(ou32_Index1),
                      static_cast<int>(ou32_Index2));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Insert string into list

   Insert one string into the string list at a defined position.

   \param[in]  ou32_Index   index before which to insert the new string (0 =
   first string in list)
   \param[in]  orc_String   string to insert
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclStringList::Insert(const uint32_t ou32_Index,
                             const QString &orc_String) {
  Strings.insert(static_cast<int>(ou32_Index), orc_String);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Find index of string within list

   Returns the index of a specified string within the string list.
   The comparison ignores upper/lower case.

   \param[in]  orc_String   string to find

   \return  ou32_Index   index of string (0 = first string in list; -1 = string
   not found)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_SclStringList::IndexOf(const QString &orc_String) {
  return Strings.indexOf(orc_String);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Return all strings as one concatenated string

   Adds all strings to one single string.
   "\r\n" is inserted after each string (also after the last string).
   This can be modified by setting orc_LineSeparator.

   \param[in]  orc_LineSeparator   separator between individual lines

   \return  concatenated string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_SclStringList::GetText(const QString orc_LineSeparator) const {
  if (Strings.isEmpty())
    return "";
  return Strings.join(orc_LineSeparator) + orc_LineSeparator;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get number of string in list

   Return number of strings contained in string list.

   \return  number of strings
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_SclStringList::GetCount(void) const {
  return static_cast<uint32_t>(Strings.size());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load strings from file

   Load strings from an ASCII file.
   Will replace any existing strings.

   Will throw if:
   - the file does not exist
   - the memory to load the file cannot be allocated
   - the file cannot be read from

   \param[in]     orc_FileName     path to file
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclStringList::LoadFromFile(const QString &orc_FileName) {
  QFile c_File(orc_FileName);
  if (!c_File.open(QIODevice::ReadOnly | QIODevice::Text)) {
    throw("C_SclStringList::LoadFromFile: file not found");
  }

  QTextStream c_In(&c_File);
  Strings.clear();
  while (!c_In.atEnd()) {
    Strings.append(c_In.readLine());
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save strings to file

   Save the contents of the string list to a file.
   An existing file will be overwritten.
   "\r\n" will be appended after each line (also after the last line).

   Will throw if:
   - the file cannot be created
   - the file cannot be written to

   \param[in]     orc_FileName     path to file
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclStringList::SaveToFile(const QString &orc_FileName) {
  QFile c_File(orc_FileName);
  if (!c_File.open(QIODevice::WriteOnly | QIODevice::Text)) {
    throw("C_SclStringList::SaveToFile: could not create file");
  }

  QTextStream c_Out(&c_File);
  for (const QString &c_Line : Strings) {
    c_Out << c_Line << "\r\n";
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns the value of one key=value pair

   Tries to find a specified "key=" at the beginning of a string contained in
   the list. Then returns the rest of the line after the "=". If "key=" is
   contained more than once the first occurrence will be detected. The
   comparison is done independent of upper/lower case.

   Example:
   Assuming a line contains the text "foo=bar" a call to "Values("foo");" will
   return "bar".

   \param[in]     orc_Key    key to search for

   \return
   if "key=" is found:  "value" after the "="
   else:                empty string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_SclStringList::Values(const QString &orc_Key) const {
  int32_t s32_Index = IndexOfName(orc_Key);
  if (s32_Index != -1) {
    return ValueFromIndex(static_cast<uint32_t>(s32_Index));
  }
  return "";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Finds the first line containg a name + "="

   Returns the index of the first line containing a specified name with an added
   equals character. The comparison is done independent of upper/lower case. The
   "=" may be preceeded by blanks.

   \param[in]     orc_Name    name to search foe

   \return
   if "name=" is found:  index of string (0 = first string in list)
   else:                 -1
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_SclStringList::IndexOfName(const QString &orc_Name) const {
  for (int i = 0; i < Strings.size(); ++i) {
    if (Strings[i].startsWith(orc_Name, Qt::CaseInsensitive)) {
      QString c_Remainder = Strings[i].mid(orc_Name.length()).trimmed();
      if (c_Remainder.startsWith('=')) {
        return i;
      }
    }
  }
  return -1;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns the part of a string after a "="

   Returns the part of one string after the equels ("=") chraracter.

   \param[in]     ou32_Index   index of string (0 = first string in list)

   \return
   if the string contains "=":  part of string after "="
   else:                        empty string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_SclStringList::ValueFromIndex(const uint32_t ou32_Index) const {
  if (ou32_Index >= static_cast<uint32_t>(Strings.size()))
    return "";
  int s32_Pos = Strings[ou32_Index].indexOf('=');
  if (s32_Pos != -1) {
    return Strings[ou32_Index].mid(s32_Pos + 1);
  }
  return "";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Add multiple strings to string list

   Adds then content of another string list to this string list.

   \param[in]     opc_Strings   string list to add (pointer is not checked for
   validity)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclStringList::AddStrings(const C_SclStringList *const opc_Strings) {
  Strings.append(opc_Strings->Strings);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sort strings in list

   Sorts by ASCII values of the individual characters.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SclStringList::Sort(void) { Strings.sort(Qt::CaseSensitive); }

} // namespace scl
} // namespace stw

//----------------------------------------------------------------------------------------------------------------------
