//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Lexer and syntax checker for data logger trigger expressions

   Turns a trigger condition into tokens and validates the expression grammar:

      expression  := or
      or          := and ( "||" and )*
      and          := not ( "&&" not )*
      not         := "!" not | primary
      primary     := "(" or ")" | comparison
      comparison  := operand compareOperator operand
      operand     := [ "[" type "]" ] ( channel | constant )

   A channel is a dot separated data element path (Node.Datapool.List.Element);
   a constant is a decimal literal, kept as written so the caller converts it with
   the same locale-independent helper the rest of openSYDE uses.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cctype>
#include <stdexcept>

#include "C_OscDataLoggerTriggerParser.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

namespace
{
/* -- Module Global Functions --------------------------------------------------------------------------------------- */

bool mh_IsIdentifierStart(const char ocn_Char)
{
   return (std::isalpha(static_cast<unsigned char>(ocn_Char)) != 0) || (ocn_Char == '_');
}

bool mh_IsIdentifierChar(const char ocn_Char)
{
   return (std::isalnum(static_cast<unsigned char>(ocn_Char)) != 0) || (ocn_Char == '_');
}

bool mh_IsDigit(const char ocn_Char)
{
   return std::isdigit(static_cast<unsigned char>(ocn_Char)) != 0;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Recursive descent validator over an already lexed token stream

   Reports the first problem by throwing; the public entry point turns that into
   an error string.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_SyntaxChecker
{
public:
   explicit C_SyntaxChecker(const std::vector<C_OscDataLoggerTriggerParser::C_Token> & orc_Tokens) :
      mrc_Tokens(orc_Tokens),
      mu32_Index(0UL)
   {
   }

   void Run(void)
   {
      if (this->mrc_Tokens.empty())
      {
         throw std::runtime_error("No expression to combine");
      }
      this->m_ParseOr();
      if (this->mu32_Index != static_cast<uint32_t>(this->mrc_Tokens.size()))
      {
         //Something is left over that the grammar cannot attach to anything
         if (this->m_Peek() == C_OscDataLoggerTriggerParser::eCLOSED_PARENTHESIS)
         {
            throw std::runtime_error("Missing/Unexpected opening parenthesis");
         }
         throw std::runtime_error("Unexpected token in expression");
      }
   }

private:
   const std::vector<C_OscDataLoggerTriggerParser::C_Token> & mrc_Tokens;
   uint32_t mu32_Index;

   bool m_AtEnd(void) const
   {
      return this->mu32_Index >= static_cast<uint32_t>(this->mrc_Tokens.size());
   }

   C_OscDataLoggerTriggerParser::E_TokenType m_Peek(void) const
   {
      return this->mrc_Tokens[this->mu32_Index].e_Type;
   }

   void m_ParseOr(void)
   {
      this->m_ParseAnd();
      while ((!this->m_AtEnd()) && (this->m_Peek() == C_OscDataLoggerTriggerParser::eLOGICAL_OR))
      {
         ++this->mu32_Index;
         if (this->m_AtEnd())
         {
            throw std::runtime_error("Operator without expression");
         }
         this->m_ParseAnd();
      }
   }

   void m_ParseAnd(void)
   {
      this->m_ParseNot();
      while ((!this->m_AtEnd()) && (this->m_Peek() == C_OscDataLoggerTriggerParser::eLOGICAL_AND))
      {
         ++this->mu32_Index;
         if (this->m_AtEnd())
         {
            throw std::runtime_error("Operator without expression");
         }
         this->m_ParseNot();
      }
   }

   void m_ParseNot(void)
   {
      if ((!this->m_AtEnd()) && (this->m_Peek() == C_OscDataLoggerTriggerParser::eLOGICAL_NOT))
      {
         ++this->mu32_Index;
         if (this->m_AtEnd())
         {
            throw std::runtime_error("Operator without expression");
         }
         this->m_ParseNot();
      }
      else
      {
         this->m_ParsePrimary();
      }
   }

   void m_ParsePrimary(void)
   {
      if (this->m_AtEnd())
      {
         throw std::runtime_error("Operator without expression");
      }
      if (this->m_Peek() == C_OscDataLoggerTriggerParser::eOPEN_PARENTHESIS)
      {
         ++this->mu32_Index;
         if (this->m_AtEnd())
         {
            throw std::runtime_error("Missing/Unexpected closing parenthesis");
         }
         this->m_ParseOr();
         if (this->m_AtEnd() || (this->m_Peek() != C_OscDataLoggerTriggerParser::eCLOSED_PARENTHESIS))
         {
            throw std::runtime_error("Missing/Unexpected closing parenthesis");
         }
         ++this->mu32_Index;
      }
      else if (this->m_Peek() == C_OscDataLoggerTriggerParser::eCLOSED_PARENTHESIS)
      {
         throw std::runtime_error("Missing/Unexpected opening parenthesis");
      }
      else
      {
         this->m_ParseComparison();
      }
   }

   void m_ParseComparison(void)
   {
      if (C_OscDataLoggerTriggerParser::h_IsComparisonOperator(this->m_Peek()))
      {
         throw std::runtime_error("Missing left operand for comparison operator");
      }
      this->m_ParseOperand();
      if (this->m_AtEnd() || (!C_OscDataLoggerTriggerParser::h_IsComparisonOperator(this->m_Peek())))
      {
         //An operand on its own is not a boolean expression
         throw std::runtime_error("Unexpected token in expression");
      }
      ++this->mu32_Index;
      if (this->m_AtEnd())
      {
         throw std::runtime_error("Missing right operand for comparison operator");
      }
      this->m_ParseOperand();
   }

   void m_ParseOperand(void)
   {
      if (this->m_AtEnd())
      {
         throw std::runtime_error("Expect either a Channel or a ConstantValue, but got something different!");
      }
      if (this->m_Peek() == C_OscDataLoggerTriggerParser::eCAST_TYPE)
      {
         ++this->mu32_Index;
         if (this->m_AtEnd())
         {
            throw std::runtime_error("Expect either a Channel or a ConstantValue, but got something different!");
         }
      }
      if (!C_OscDataLoggerTriggerParser::h_IsOperandStart(this->m_Peek()))
      {
         throw std::runtime_error("Expect either a Channel or a ConstantValue, but got something different!");
      }
      ++this->mu32_Index;
   }
};
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscDataLoggerTriggerParser::C_Token::C_Token(void) :
   e_Type(C_OscDataLoggerTriggerParser::eCONSTANT_VALUE)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Constructor

   \param[in]  oe_Type     Token type
   \param[in]  orc_Text    Raw token text
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscDataLoggerTriggerParser::C_Token::C_Token(const E_TokenType oe_Type, const std::string & orc_Text) :
   e_Type(oe_Type),
   c_Text(orc_Text)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscDataLoggerTriggerParser::C_OscDataLoggerTriggerParser(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if the token is a comparison operator

   \param[in]  oe_Type  Token type

   \return
   true    token compares two operands
   false   token is something else
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscDataLoggerTriggerParser::h_IsComparisonOperator(const E_TokenType oe_Type)
{
   return (oe_Type == eGREATER_THAN) || (oe_Type == eGREATER_EQUAL) || (oe_Type == eLESS_THAN) ||
          (oe_Type == eLESS_EQUAL) || (oe_Type == eEQUAL) || (oe_Type == eNOT_EQUAL);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if the token can start an operand

   \param[in]  oe_Type  Token type

   \return
   true    token is a channel or a constant
   false   token is something else
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscDataLoggerTriggerParser::h_IsOperandStart(const E_TokenType oe_Type)
{
   return (oe_Type == eCHANNEL) || (oe_Type == eCONSTANT_VALUE);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Split a trigger expression into tokens

   \param[in]      orc_Expression      Expression to lex
   \param[in,out]  orc_Tokens          Resulting tokens (cleared first)
   \param[in,out]  opc_ErrorDetails    Optional error description

   \return
   true    expression could be tokenized
   false   expression contains something the lexer does not accept
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscDataLoggerTriggerParser::h_Tokenize(const std::string & orc_Expression, std::vector<C_Token> & orc_Tokens,
                                              std::string * const opc_ErrorDetails)
{
   bool q_IsValid = true;

   orc_Tokens.clear();

   try
   {
      std::string::size_type un_Pos = 0UL;
      const std::string::size_type un_Length = orc_Expression.length();

      while (un_Pos < un_Length)
      {
         const char cn_Current = orc_Expression[un_Pos];

         if (std::isspace(static_cast<unsigned char>(cn_Current)) != 0)
         {
            ++un_Pos;
         }
         else if (cn_Current == '(')
         {
            orc_Tokens.emplace_back(eOPEN_PARENTHESIS, "(");
            ++un_Pos;
         }
         else if (cn_Current == ')')
         {
            orc_Tokens.emplace_back(eCLOSED_PARENTHESIS, ")");
            ++un_Pos;
         }
         else if (cn_Current == '&')
         {
            if (((un_Pos + 1UL) < un_Length) && (orc_Expression[un_Pos + 1UL] == '&'))
            {
               orc_Tokens.emplace_back(eLOGICAL_AND, "&&");
               un_Pos += 2UL;
            }
            else
            {
               throw std::runtime_error("Single '&' is not an operator, use '&&'");
            }
         }
         else if (cn_Current == '|')
         {
            if (((un_Pos + 1UL) < un_Length) && (orc_Expression[un_Pos + 1UL] == '|'))
            {
               orc_Tokens.emplace_back(eLOGICAL_OR, "||");
               un_Pos += 2UL;
            }
            else
            {
               throw std::runtime_error("Single '|' is not an operator, use '||'");
            }
         }
         else if (cn_Current == '!')
         {
            if (((un_Pos + 1UL) < un_Length) && (orc_Expression[un_Pos + 1UL] == '='))
            {
               orc_Tokens.emplace_back(eNOT_EQUAL, "!=");
               un_Pos += 2UL;
            }
            else
            {
               orc_Tokens.emplace_back(eLOGICAL_NOT, "!");
               ++un_Pos;
            }
         }
         else if (cn_Current == '>')
         {
            if (((un_Pos + 1UL) < un_Length) && (orc_Expression[un_Pos + 1UL] == '='))
            {
               orc_Tokens.emplace_back(eGREATER_EQUAL, ">=");
               un_Pos += 2UL;
            }
            else
            {
               orc_Tokens.emplace_back(eGREATER_THAN, ">");
               ++un_Pos;
            }
         }
         else if (cn_Current == '<')
         {
            if (((un_Pos + 1UL) < un_Length) && (orc_Expression[un_Pos + 1UL] == '='))
            {
               orc_Tokens.emplace_back(eLESS_EQUAL, "<=");
               un_Pos += 2UL;
            }
            else
            {
               orc_Tokens.emplace_back(eLESS_THAN, "<");
               ++un_Pos;
            }
         }
         else if (cn_Current == '=')
         {
            //Accept both "=" and "=="
            if (((un_Pos + 1UL) < un_Length) && (orc_Expression[un_Pos + 1UL] == '='))
            {
               orc_Tokens.emplace_back(eEQUAL, "==");
               un_Pos += 2UL;
            }
            else
            {
               orc_Tokens.emplace_back(eEQUAL, "=");
               ++un_Pos;
            }
         }
         else if (cn_Current == '[')
         {
            const std::string::size_type un_End = orc_Expression.find(']', un_Pos);
            if (un_End == std::string::npos)
            {
               throw std::runtime_error("Missing ']' for cast type");
            }
            {
               const std::string c_Type = orc_Expression.substr(un_Pos + 1UL, (un_End - un_Pos) - 1UL);
               if (c_Type.empty())
               {
                  throw std::runtime_error("Empty cast type");
               }
               for (std::string::size_type un_It = 0UL; un_It < c_Type.length(); ++un_It)
               {
                  if (std::isalnum(static_cast<unsigned char>(c_Type[un_It])) == 0)
                  {
                     throw std::runtime_error("Invalid cast type \"" + c_Type + "\"");
                  }
               }
               orc_Tokens.emplace_back(eCAST_TYPE, c_Type);
            }
            un_Pos = un_End + 1UL;
         }
         else if (mh_IsDigit(cn_Current) ||
                  (((cn_Current == '+') || (cn_Current == '-') || (cn_Current == '.')) &&
                   ((un_Pos + 1UL) < un_Length) &&
                   (mh_IsDigit(orc_Expression[un_Pos + 1UL]) || (orc_Expression[un_Pos + 1UL] == '.'))))
         {
            const std::string::size_type un_Start = un_Pos;
            if ((cn_Current == '+') || (cn_Current == '-'))
            {
               ++un_Pos;
            }
            while ((un_Pos < un_Length) && mh_IsDigit(orc_Expression[un_Pos]))
            {
               ++un_Pos;
            }
            if ((un_Pos < un_Length) && (orc_Expression[un_Pos] == '.'))
            {
               ++un_Pos;
               while ((un_Pos < un_Length) && mh_IsDigit(orc_Expression[un_Pos]))
               {
                  ++un_Pos;
               }
            }
            if ((un_Pos < un_Length) &&
                ((orc_Expression[un_Pos] == 'e') || (orc_Expression[un_Pos] == 'E')))
            {
               const std::string::size_type un_ExpStart = un_Pos;
               ++un_Pos;
               if ((un_Pos < un_Length) && ((orc_Expression[un_Pos] == '+') || (orc_Expression[un_Pos] == '-')))
               {
                  ++un_Pos;
               }
               if ((un_Pos < un_Length) && mh_IsDigit(orc_Expression[un_Pos]))
               {
                  while ((un_Pos < un_Length) && mh_IsDigit(orc_Expression[un_Pos]))
                  {
                     ++un_Pos;
                  }
               }
               else
               {
                  //Not an exponent after all
                  un_Pos = un_ExpStart;
               }
            }
            orc_Tokens.emplace_back(eCONSTANT_VALUE, orc_Expression.substr(un_Start, un_Pos - un_Start));
         }
         else if (mh_IsIdentifierStart(cn_Current))
         {
            const std::string::size_type un_Start = un_Pos;
            C_Token c_Token(eCHANNEL, "");
            std::string c_Component;
            bool q_Done = false;

            while ((un_Pos < un_Length) && (!q_Done))
            {
               if (mh_IsIdentifierChar(orc_Expression[un_Pos]))
               {
                  c_Component += orc_Expression[un_Pos];
                  ++un_Pos;
               }
               else if ((orc_Expression[un_Pos] == '.') && ((un_Pos + 1UL) < un_Length) &&
                        mh_IsIdentifierChar(orc_Expression[un_Pos + 1UL]))
               {
                  c_Token.c_Components.push_back(c_Component);
                  c_Component = "";
                  ++un_Pos;
               }
               else
               {
                  q_Done = true;
               }
            }
            c_Token.c_Components.push_back(c_Component);
            c_Token.c_Text = orc_Expression.substr(un_Start, un_Pos - un_Start);
            orc_Tokens.push_back(c_Token);
         }
         else
         {
            throw std::runtime_error("Unexpected character '" + std::string(1, cn_Current) + "' in expression");
         }
      }
   }
   catch (const std::exception & rc_Error)
   {
      q_IsValid = false;
      if (opc_ErrorDetails != NULL)
      {
         *opc_ErrorDetails = rc_Error.what();
      }
   }
   catch (...)
   {
      q_IsValid = false;
      if (opc_ErrorDetails != NULL)
      {
         *opc_ErrorDetails = "Parsing expression: Unknown error";
      }
   }
   return q_IsValid;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check the expression grammar of an already lexed token stream

   \param[in]      orc_Tokens          Tokens
   \param[in,out]  opc_ErrorDetails    Optional error description

   \return
   true    token stream forms a valid boolean expression
   false   token stream does not
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscDataLoggerTriggerParser::h_CheckSyntax(const std::vector<C_Token> & orc_Tokens,
                                                 std::string * const opc_ErrorDetails)
{
   bool q_IsValid = true;

   try
   {
      C_SyntaxChecker c_Checker(orc_Tokens);
      c_Checker.Run();
   }
   catch (const std::exception & rc_Error)
   {
      q_IsValid = false;
      if (opc_ErrorDetails != NULL)
      {
         *opc_ErrorDetails = rc_Error.what();
      }
   }
   catch (...)
   {
      q_IsValid = false;
      if (opc_ErrorDetails != NULL)
      {
         *opc_ErrorDetails = "Checking syntax: Unknown error";
      }
   }
   return q_IsValid;
}
