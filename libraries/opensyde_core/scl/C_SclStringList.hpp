//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       string list class

   \class       stw::scl::C_SclStringList
   \brief       string list class

   ANSI C++ string list class.
   Aim: provide most of the functionality that Borland's TStringList does
    while only using ANSI C++.
   So cf. the documentation of the VCL TStringList for details on most API functions.

   The std::string is used as a base (-> no unicode).

   What is NOT implemented (compared to Borland VCL TStringList):
   - OnChange and OnChanging callbacks
   - implicit sorting (explicit function available)
   - Everything associated with TObject
   - stream-based functions

   \copyright   Copyright 2009 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SCLSTRINGLISTHPP
#define C_SCLSTRINGLISTHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>
#include <string>
#include "stwtypes.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace scl
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
///Container class for lists of strings
class C_SclStringList
{
private:
public:
   //from VCL:
   uint32_t Add(const std::string & orc_String);
   void Append(const std::string & orc_String);

   void Clear(void);
   void Delete(const uint32_t ou32_Index);
   void Exchange(const uint32_t ou32_Index1, const uint32_t ou32_Index2);
   void Insert(const uint32_t ou32_Index, const std::string & orc_String);
   int32_t IndexOf(const std::string & orc_String);

   std::string GetText(const std::string orc_LineSeparator = "\r\n") const;
   uint32_t GetCount(void) const;

   void LoadFromFile(const std::string & orc_FileName);
   void SaveToFile(const std::string & orc_FileName);

   int32_t IndexOfName(const std::string & orc_Name) const;
   std::string ValueFromIndex(const uint32_t ou32_Index) const;
   std::string Values(const std::string & orc_Key) const;
   void AddStrings(const C_SclStringList * const opc_Strings);

   void Sort(void);

   std::vector<std::string> Strings; ///< actual strings
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}
#endif
