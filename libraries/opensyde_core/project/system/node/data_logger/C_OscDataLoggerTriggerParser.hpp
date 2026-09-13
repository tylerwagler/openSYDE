//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Lexer and syntax checker for data logger trigger expressions (header)

   Turns a data logger trigger condition into tokens and validates the expression
   grammar. Used by the GUI to give editor-time feedback; the device validates
   trigger conditions at runtime regardless.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDATALOGGERTRIGGERPARSER_HPP
#define C_OSCDATALOGGERTRIGGERPARSER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include <string>
#include <vector>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscDataLoggerTriggerParser
{
public:
   ///< Kind of a lexed token
   enum E_TokenType
   {
      eCONSTANT_VALUE,     ///< numeric literal; c_Text holds the literal as written
      eCHANNEL,            ///< data element path; c_Components holds the dot separated parts
      eGREATER_THAN,       ///< ">"
      eGREATER_EQUAL,      ///< ">="
      eLESS_THAN,          ///< "<"
      eLESS_EQUAL,         ///< "<="
      eEQUAL,              ///< "=" or "=="
      eNOT_EQUAL,          ///< "!="
      eLOGICAL_AND,        ///< "&&"
      eLOGICAL_OR,         ///< "||"
      eLOGICAL_NOT,        ///< "!"
      eOPEN_PARENTHESIS,   ///< "("
      eCLOSED_PARENTHESIS, ///< ")"
      eCAST_TYPE           ///< "[type]"; c_Text holds the type name without the brackets
   };

   ///< One lexed token
   class C_Token
   {
   public:
      C_Token(void);
      C_Token(const E_TokenType oe_Type, const std::string & orc_Text);

      E_TokenType e_Type;                    ///< Kind of token
      std::string c_Text;                    ///< Raw text: literal, cast type name or channel path
      std::vector<std::string> c_Components; ///< Channel path split on '.' (eCHANNEL only)
   };

   static bool h_Tokenize(const std::string & orc_Expression, std::vector<C_Token> & orc_Tokens,
                          std::string * const opc_ErrorDetails);
   static bool h_CheckSyntax(const std::vector<C_Token> & orc_Tokens, std::string * const opc_ErrorDetails);

   static bool h_IsComparisonOperator(const E_TokenType oe_Type);
   static bool h_IsOperandStart(const E_TokenType oe_Type);

private:
   C_OscDataLoggerTriggerParser(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
