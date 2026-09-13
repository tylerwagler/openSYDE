//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for the data logger trigger expression parser

   Pins the lexical and grammatical rules of trigger conditions. The grammar was
   recovered from the previously prebuilt-only expression library it replaces, so
   these tests are the guard against drifting away from what users already typed
   into existing projects.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "C_OscDataLoggerTriggerParser.hpp"

using namespace stw::opensyde_core;

namespace
{
typedef C_OscDataLoggerTriggerParser C_Parser;

std::vector<C_Parser::C_Token> mh_Lex(const std::string & orc_Expression)
{
   std::vector<C_Parser::C_Token> c_Tokens;

   EXPECT_TRUE(C_Parser::h_Tokenize(orc_Expression, c_Tokens, NULL)) << orc_Expression;
   return c_Tokens;
}

bool mh_IsAccepted(const std::string & orc_Expression)
{
   std::vector<C_Parser::C_Token> c_Tokens;
   bool q_Retval = C_Parser::h_Tokenize(orc_Expression, c_Tokens, NULL);

   if (q_Retval)
   {
      q_Retval = C_Parser::h_CheckSyntax(c_Tokens, NULL);
   }
   return q_Retval;
}
}

/* -- Lexer ---------------------------------------------------------------------------------------------------------- */

TEST(DataLoggerTriggerParser, LexesChannelIntoComponents)
{
   const std::vector<C_Parser::C_Token> c_Tokens = mh_Lex("Node.Datapool.List.Element");

   ASSERT_EQ(1U, c_Tokens.size());
   EXPECT_EQ(C_Parser::eCHANNEL, c_Tokens[0].e_Type);
   ASSERT_EQ(4U, c_Tokens[0].c_Components.size());
   EXPECT_EQ("Node", c_Tokens[0].c_Components[0]);
   EXPECT_EQ("Datapool", c_Tokens[0].c_Components[1]);
   EXPECT_EQ("List", c_Tokens[0].c_Components[2]);
   EXPECT_EQ("Element", c_Tokens[0].c_Components[3]);
}

TEST(DataLoggerTriggerParser, LexesComparisonAndConstant)
{
   const std::vector<C_Parser::C_Token> c_Tokens = mh_Lex("Node.Dp.List.Elem >= 5");

   ASSERT_EQ(3U, c_Tokens.size());
   EXPECT_EQ(C_Parser::eCHANNEL, c_Tokens[0].e_Type);
   EXPECT_EQ(C_Parser::eGREATER_EQUAL, c_Tokens[1].e_Type);
   EXPECT_EQ(C_Parser::eCONSTANT_VALUE, c_Tokens[2].e_Type);
   //The literal is kept as written so the caller converts it locale independently
   EXPECT_EQ("5", c_Tokens[2].c_Text);
}

TEST(DataLoggerTriggerParser, LexesAllOperators)
{
   const std::vector<C_Parser::C_Token> c_Tokens = mh_Lex("> >= < <= = == != && || ! ( )");

   ASSERT_EQ(12U, c_Tokens.size());
   EXPECT_EQ(C_Parser::eGREATER_THAN, c_Tokens[0].e_Type);
   EXPECT_EQ(C_Parser::eGREATER_EQUAL, c_Tokens[1].e_Type);
   EXPECT_EQ(C_Parser::eLESS_THAN, c_Tokens[2].e_Type);
   EXPECT_EQ(C_Parser::eLESS_EQUAL, c_Tokens[3].e_Type);
   EXPECT_EQ(C_Parser::eEQUAL, c_Tokens[4].e_Type);
   EXPECT_EQ(C_Parser::eEQUAL, c_Tokens[5].e_Type);
   EXPECT_EQ(C_Parser::eNOT_EQUAL, c_Tokens[6].e_Type);
   EXPECT_EQ(C_Parser::eLOGICAL_AND, c_Tokens[7].e_Type);
   EXPECT_EQ(C_Parser::eLOGICAL_OR, c_Tokens[8].e_Type);
   EXPECT_EQ(C_Parser::eLOGICAL_NOT, c_Tokens[9].e_Type);
   EXPECT_EQ(C_Parser::eOPEN_PARENTHESIS, c_Tokens[10].e_Type);
   EXPECT_EQ(C_Parser::eCLOSED_PARENTHESIS, c_Tokens[11].e_Type);
}

TEST(DataLoggerTriggerParser, LexesCastType)
{
   const std::vector<C_Parser::C_Token> c_Tokens = mh_Lex("[uint32]Node.Dp.List.Elem");

   ASSERT_EQ(2U, c_Tokens.size());
   EXPECT_EQ(C_Parser::eCAST_TYPE, c_Tokens[0].e_Type);
   //Brackets are stripped
   EXPECT_EQ("uint32", c_Tokens[0].c_Text);
   EXPECT_EQ(C_Parser::eCHANNEL, c_Tokens[1].e_Type);
}

TEST(DataLoggerTriggerParser, LexesNumberForms)
{
   const std::vector<C_Parser::C_Token> c_Tokens = mh_Lex("1 1.5 .5 -2 +3 1e3 2.5E-2");

   ASSERT_EQ(7U, c_Tokens.size());
   EXPECT_EQ("1", c_Tokens[0].c_Text);
   EXPECT_EQ("1.5", c_Tokens[1].c_Text);
   EXPECT_EQ(".5", c_Tokens[2].c_Text);
   EXPECT_EQ("-2", c_Tokens[3].c_Text);
   EXPECT_EQ("+3", c_Tokens[4].c_Text);
   EXPECT_EQ("1e3", c_Tokens[5].c_Text);
   EXPECT_EQ("2.5E-2", c_Tokens[6].c_Text);
   for (uint32_t u32_It = 0U; u32_It < c_Tokens.size(); ++u32_It)
   {
      EXPECT_EQ(C_Parser::eCONSTANT_VALUE, c_Tokens[u32_It].e_Type);
   }
}

TEST(DataLoggerTriggerParser, RejectsUnknownCharacter)
{
   std::vector<C_Parser::C_Token> c_Tokens;
   std::string c_Error;

   EXPECT_FALSE(C_Parser::h_Tokenize("a.b.c.d $ 5", c_Tokens, &c_Error));
   EXPECT_FALSE(c_Error.empty());
}

TEST(DataLoggerTriggerParser, RejectsMalformedCastAndSingleBitwiseOperators)
{
   std::vector<C_Parser::C_Token> c_Tokens;

   EXPECT_FALSE(C_Parser::h_Tokenize("[uint32 a.b.c.d", c_Tokens, NULL));
   EXPECT_FALSE(C_Parser::h_Tokenize("[]a.b.c.d", c_Tokens, NULL));
   EXPECT_FALSE(C_Parser::h_Tokenize("[ui nt]a.b.c.d", c_Tokens, NULL));
   EXPECT_FALSE(C_Parser::h_Tokenize("a.b.c.d > 1 & b.c.d.e > 2", c_Tokens, NULL));
   EXPECT_FALSE(C_Parser::h_Tokenize("a.b.c.d > 1 | b.c.d.e > 2", c_Tokens, NULL));
}

/* -- Syntax --------------------------------------------------------------------------------------------------------- */

TEST(DataLoggerTriggerParser, AcceptsValidExpressions)
{
   EXPECT_TRUE(mh_IsAccepted("Node.Dp.List.Elem > 5"));
   EXPECT_TRUE(mh_IsAccepted("Node.Dp.List.Elem >= 5.5"));
   EXPECT_TRUE(mh_IsAccepted("5 < Node.Dp.List.Elem"));
   EXPECT_TRUE(mh_IsAccepted("[uint32]Node.Dp.List.Elem = 1"));
   EXPECT_TRUE(mh_IsAccepted("Node.Dp.List.Elem == 1"));
   EXPECT_TRUE(mh_IsAccepted("(Node.Dp.List.Elem > 5)"));
   EXPECT_TRUE(mh_IsAccepted("Node.Dp.List.Elem > 5 && Other.Dp.List.Elem < 2"));
   EXPECT_TRUE(mh_IsAccepted("Node.Dp.List.Elem > 5 || Other.Dp.List.Elem != 2"));
   EXPECT_TRUE(mh_IsAccepted("!(Node.Dp.List.Elem > 5)"));
   EXPECT_TRUE(mh_IsAccepted("(a.b.c.d > 1 && e.f.g.h < 2) || !(i.j.k.l = 3)"));
}

TEST(DataLoggerTriggerParser, RejectsEmptyExpression)
{
   std::vector<C_Parser::C_Token> c_Tokens;
   std::string c_Error;

   ASSERT_TRUE(C_Parser::h_Tokenize("", c_Tokens, NULL));
   EXPECT_FALSE(C_Parser::h_CheckSyntax(c_Tokens, &c_Error));
   EXPECT_EQ("No expression to combine", c_Error);
}

TEST(DataLoggerTriggerParser, RejectsMissingOperands)
{
   EXPECT_FALSE(mh_IsAccepted("> 5"));
   EXPECT_FALSE(mh_IsAccepted("Node.Dp.List.Elem >"));
   EXPECT_FALSE(mh_IsAccepted("Node.Dp.List.Elem > 5 &&"));
   EXPECT_FALSE(mh_IsAccepted("&& Node.Dp.List.Elem > 5"));
   EXPECT_FALSE(mh_IsAccepted("!"));
}

TEST(DataLoggerTriggerParser, RejectsUnbalancedParentheses)
{
   EXPECT_FALSE(mh_IsAccepted("(Node.Dp.List.Elem > 5"));
   EXPECT_FALSE(mh_IsAccepted("Node.Dp.List.Elem > 5)"));
   EXPECT_FALSE(mh_IsAccepted("(Node.Dp.List.Elem > 5))"));
}

TEST(DataLoggerTriggerParser, RejectsOperandWithoutComparison)
{
   //A bare operand is not a boolean expression
   EXPECT_FALSE(mh_IsAccepted("Node.Dp.List.Elem"));
   EXPECT_FALSE(mh_IsAccepted("5"));
   EXPECT_FALSE(mh_IsAccepted("Node.Dp.List.Elem && Other.Dp.List.Elem"));
}

TEST(DataLoggerTriggerParser, ReportsMissingLeftOperandMessage)
{
   std::vector<C_Parser::C_Token> c_Tokens;
   std::string c_Error;

   ASSERT_TRUE(C_Parser::h_Tokenize("> 5", c_Tokens, NULL));
   EXPECT_FALSE(C_Parser::h_CheckSyntax(c_Tokens, &c_Error));
   EXPECT_EQ("Missing left operand for comparison operator", c_Error);
}

TEST(DataLoggerTriggerParser, ReportsMissingRightOperandMessage)
{
   std::vector<C_Parser::C_Token> c_Tokens;
   std::string c_Error;

   ASSERT_TRUE(C_Parser::h_Tokenize("a.b.c.d >", c_Tokens, NULL));
   EXPECT_FALSE(C_Parser::h_CheckSyntax(c_Tokens, &c_Error));
   EXPECT_EQ("Missing right operand for comparison operator", c_Error);
}

TEST(DataLoggerTriggerParser, ReportsParenthesisMessages)
{
   std::vector<C_Parser::C_Token> c_Tokens;
   std::string c_Error;

   ASSERT_TRUE(C_Parser::h_Tokenize("(a.b.c.d > 5", c_Tokens, NULL));
   EXPECT_FALSE(C_Parser::h_CheckSyntax(c_Tokens, &c_Error));
   EXPECT_EQ("Missing/Unexpected closing parenthesis", c_Error);

   ASSERT_TRUE(C_Parser::h_Tokenize("a.b.c.d > 5)", c_Tokens, NULL));
   EXPECT_FALSE(C_Parser::h_CheckSyntax(c_Tokens, &c_Error));
   EXPECT_EQ("Missing/Unexpected opening parenthesis", c_Error);
}
