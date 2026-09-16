//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for C_OscUtils::h_NiceifyStringForCeComment

   The comment sanitiser kept its 1-based C_SclString loop bounds. It therefore never
   examined character 0, so a comment beginning with the one sequence it exists to
   suppress passed straight through, and its final iteration wrote through
   c_Result[size()], which is undefined for the non-const operator[].

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <string>

#include <cstdint>
#include "C_OscUtils.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A comment cannot terminate the C comment it is embedded in

   Regression pin. The generated code puts this string inside a block comment, so an
   unescaped comment terminator ends the comment early and the rest of the text becomes
   code. At the very start of the string is exactly where the old bounds could not see.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(NiceifyCeComment, SuppressesCommentTerminatorAtEveryPosition)
{
   //at the start -- the position the 1-based loop skipped entirely
   EXPECT_EQ("_/rest", C_OscUtils::h_NiceifyStringForCeComment("*/rest"));
   //in the middle, which always worked
   EXPECT_EQ("a_/b", C_OscUtils::h_NiceifyStringForCeComment("a*/b"));
   //at the end
   EXPECT_EQ("ab_/", C_OscUtils::h_NiceifyStringForCeComment("ab*/"));
   //a lone star is legal inside a comment and must be left alone
   EXPECT_EQ("a*b", C_OscUtils::h_NiceifyStringForCeComment("a*b"));
   EXPECT_EQ("*", C_OscUtils::h_NiceifyStringForCeComment("*"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A trailing backslash cannot continue a C++ line comment
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(NiceifyCeComment, SuppressesTrailingBackslash)
{
   EXPECT_EQ("text_", C_OscUtils::h_NiceifyStringForCeComment("text\\"));
   //a backslash that is not last is harmless
   EXPECT_EQ("a\\b", C_OscUtils::h_NiceifyStringForCeComment("a\\b"));
   //a single backslash is both first and last
   EXPECT_EQ("_", C_OscUtils::h_NiceifyStringForCeComment("\\"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Non-printable characters are replaced wherever they appear

   Including at index 0, which the old bounds never reached.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(NiceifyCeComment, ReplacesNonPrintableCharacters)
{
   EXPECT_EQ("_bc", C_OscUtils::h_NiceifyStringForCeComment("\nbc"));
   EXPECT_EQ("a_c", C_OscUtils::h_NiceifyStringForCeComment("a\tc"));
   EXPECT_EQ("ab_", C_OscUtils::h_NiceifyStringForCeComment("ab\r"));
   EXPECT_EQ("_ackquote", C_OscUtils::h_NiceifyStringForCeComment("`ackquote"));
   EXPECT_EQ("_t", C_OscUtils::h_NiceifyStringForCeComment("@t"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   The result keeps the length of the input and ordinary text is untouched

   The old loop ran one iteration too many and wrote past the last character.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(NiceifyCeComment, PreservesLengthAndPlainText)
{
   const std::string c_Plain = "A normal comment, 123.";

   EXPECT_EQ(c_Plain, C_OscUtils::h_NiceifyStringForCeComment(c_Plain));
   EXPECT_EQ(c_Plain.size(), C_OscUtils::h_NiceifyStringForCeComment(c_Plain).size());
   EXPECT_EQ("", C_OscUtils::h_NiceifyStringForCeComment(""));
   EXPECT_EQ(1U, C_OscUtils::h_NiceifyStringForCeComment("x").size());
}
