//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       string list class

   \class       stw::scl::C_SclStringList
   \brief       string list class

   ANSI C++ string list class.
   Aim: provide most of the functionality that Borland's TStringList does
    while only using ANSI C++.
   So cf. the documentation of the VCL TStringList for details on most API
   functions.

   The C_SclString is used as a base (-> no unicode).

   What is NOT implemented (compared to Borland VCL TStringList):
   - OnChange and OnChanging callbacks
   - implicit sorting (explicit function available)
   - Everything associated with TObject
   - stream-based functions

   \copyright   Copyright 2009 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SCLSTRINGLISTHPP
#define C_SCLSTRINGLISTHPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "stwtypes.hpp"
#include <QStringList>

/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace scl {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */
/// Container class for lists of strings
class C_SclStringList {
private:
public:
  // from VCL:
  uint32_t Add(const QString &orc_String);
  void Append(const QString &orc_String);

  void Clear(void);
  void Delete(const uint32_t ou32_Index);
  void Exchange(const uint32_t ou32_Index1, const uint32_t ou32_Index2);
  void Insert(const uint32_t ou32_Index, const QString &orc_String);
  int32_t IndexOf(const QString &orc_String);

  QString GetText(const QString orc_LineSeparator = "\r\n") const;
  uint32_t GetCount(void) const;

  void LoadFromFile(const QString &orc_FileName);
  void SaveToFile(const QString &orc_FileName);

  int32_t IndexOfName(const QString &orc_Name) const;
  QString ValueFromIndex(const uint32_t ou32_Index) const;
  QString Values(const QString &orc_Key) const;
  void AddStrings(const C_SclStringList *const opc_Strings);

  void Sort(void);

  QStringList Strings; ///< actual strings
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace scl
} // namespace stw
#endif
