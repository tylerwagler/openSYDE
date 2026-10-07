//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       A node's UDS (ISO 14229) configuration: addressing, timing, routines and DTCs

   The data identifiers a UDS node serves are described as messages of a C_OscCanProtocol of type eUDS (a DID is
   a "message" whose identifier is the 16 bit DID and whose signals are the record layout). Everything about the
   node that is not a DID lives here: how to address it on the bus, its timing, the sessions and security levels
   it supports, and the routines and diagnostic trouble codes it knows.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODEUDSCONFIG_HPP
#define C_OSCNODEUDSCONFIG_HPP

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
///a routine (RoutineControl, 0x31) the node offers
class C_OscUdsRoutine
{
public:
   C_OscUdsRoutine(void);

   void CalcHash(uint32_t & oru32_HashValue) const;

   uint16_t u16_Identifier;       ///< routine identifier (RID)
   std::string c_Name;
   std::string c_Comment;
   bool q_SupportsStart;          ///< sub-function startRoutine
   bool q_SupportsStop;           ///< sub-function stopRoutine
   bool q_SupportsRequestResults; ///< sub-function requestRoutineResults
};

///a diagnostic trouble code the node can report
class C_OscUdsDtc
{
public:
   C_OscUdsDtc(void);

   void CalcHash(uint32_t & oru32_HashValue) const;

   uint32_t u32_Code; ///< 24 bit DTC
   std::string c_Name;
   std::string c_Comment;
   uint8_t u8_Severity; ///< DTC severity byte (ISO 14229-1 D.3); 0 if the node does not use it
};

///a node's UDS configuration
class C_OscNodeUdsConfig
{
public:
   C_OscNodeUdsConfig(void);

   void Initialize(void);
   void CalcHash(uint32_t & oru32_HashValue) const;

   //addressing on the CAN interface the UDS protocol is used on
   uint32_t u32_RequestId;    ///< identifier for client -> node (physical)
   uint32_t u32_ResponseId;   ///< identifier for node -> client
   uint32_t u32_FunctionalId; ///< identifier for client -> all nodes (functional); 0 if none
   bool q_ExtendedId;         ///< true: 29 bit identifiers; false: 11 bit
   bool q_PadFrames;          ///< true: the node expects every frame padded to DLC 8
   uint8_t u8_PadByte;        ///< pad byte

   //timing (ISO 14229-2)
   uint32_t u32_P2Ms;       ///< time the node takes to answer
   uint32_t u32_P2StarMs;   ///< time the node takes to answer after a ResponsePending
   uint32_t u32_S3ClientMs; ///< TesterPresent interval to keep a non-default session alive

   //capabilities
   std::vector<uint8_t> c_SupportedSessions; ///< DiagnosticSessionControl sub-functions the node accepts
   std::vector<uint8_t> c_SecurityLevels;    ///< SecurityAccess requestSeed sub-functions (odd) the node offers
   std::string c_SeedKeyAlgorithm;           ///< which seed-to-key algorithm unlocks it; hc_SEED_KEY_CONSTANT by default

   std::vector<C_OscUdsRoutine> c_Routines;
   std::vector<C_OscUdsDtc> c_Dtcs;

   static const std::string hc_SEED_KEY_CONSTANT; ///< the built-in constant key
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
