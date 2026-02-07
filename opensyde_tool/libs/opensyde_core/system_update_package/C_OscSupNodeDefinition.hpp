//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node update package definition
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSUPNODEDEFINITION_HPP
#define C_OSCSUPNODEDEFINITION_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include <QStringList>

#include "stwtypes.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscSupNodeDefinition
{
public:
   C_OscSupNodeDefinition(void);

   //lint -sem(stw::opensyde_core::C_OscSupNodeDefinition::Initialize,initializer)
   void Initialize(void);

   uint8_t u8_Active;
   uint32_t u32_Position;
   QStringList c_ApplicationFileNames; // with relative path
   QStringList c_NvmFileNames;         // with relative path
   QString c_PemFile;
   uint8_t u8_SignaturePresent;
   QString c_SignatureFile;

   /// Node configuration flags for security state
   bool q_SendSecurityEnabledState;
   bool q_SecurityEnabled;

   /// Node configuration flags for debugger state
   bool q_SendDebuggerEnabledState;
   bool q_DebuggerEnabled;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
