//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Importing data from Vector DBC file (implementation)

   cf. .cpp file header for details

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CIEIMPORTDB_HPP
#define C_CIEIMPORTDB_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>
#include <string>
#include <vector>
#include "stwtypes.hpp"
#include "C_CieConverter.hpp"
#include "C_OscNodeDataPoolContent.hpp"

#include <Vector/DBC.h>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_CieImportDbc
{
private:
   static const std::string mhc_SEND_TYPE;
   static const std::string mhc_CYCLE_TIME;
   static Vector::DBC::AttributeDefinition mhc_AttributeSendType;
   static std::string mhc_DefaultSendTypeValue;
   static const std::string mhc_INITIAL_VALUE;
   static float32_t mhf32_DefaultInitialValue;
   static bool mhq_DefaultValueDefined;
   static stw::scl::std::vector<std::string> mhc_WarningMessages; // global warnings e.g. why some messages could not be imported
   static std::string mhc_ErrorMessage;        // description of error which caused the import to fail

   static int32_t mh_ReadFile(const std::string & orc_File, Vector::DBC::Network & orc_Network);
   static void mh_GetNode(const Vector::DBC::Node & orc_DbcNode, C_CieConverter::C_CieNode & orc_Node);
   static int32_t mh_GetMessage(const Vector::DBC::Network & orc_DbcNetwork,
                                const Vector::DBC::Message & orc_DbcMessage, C_CieConverter::C_CieNode & orc_Node);
   static int32_t mh_PrepareMessage(const Vector::DBC::Network & orc_DbcNetwork,
                                    const Vector::DBC::Message & orc_DbcMessage,
                                    C_CieConverter::C_CieNodeMessage & orc_Message);
   static int32_t mh_ConvertAndAddMessage(const Vector::DBC::Network & orc_DbcNetwork,
                                          const Vector::DBC::Message & orc_DbcMessage,
                                          std::vector<C_CieConverter::C_CieNodeMessage> & orc_Messages);
   static int32_t mh_GetSignal(const Vector::DBC::Network & orc_DbcNetwork, const Vector::DBC::Signal & orc_DbcSignal,
                               bool & orq_SignalAdapted, C_CieConverter::C_CieNodeMessage & orc_Message);
   static void mh_GetSignalSpnInfo(const Vector::DBC::Network & orc_DbcNetwork,
                                   const Vector::DBC::Signal & orc_DbcSignal,
                                   C_CieConverter::C_CieCanSignal & orc_Signal);
   static void mh_VerifySignalValueTable(C_CieConverter::C_CieCanSignal & orc_DbcSignal);
   static int32_t mh_GetSignalValues(const Vector::DBC::Network & orc_DbcNetwork,
                                     const Vector::DBC::Signal & orc_DbcSignal, const bool oq_MultiplexerSignal,
                                     bool & orq_SignalAdapted, C_CieConverter::C_CieDataPoolElement & orc_Element,
                                     stw::scl::std::vector<std::string> & orc_WarningMessages);
   static int32_t mh_GetAttributeDefinitions(const Vector::DBC::Network & orc_DbcNetwork);
   static void mh_GetTransmission(const Vector::DBC::Network & orc_DbcNetwork,
                                  const Vector::DBC::Message & orc_DbcMessage,
                                  C_CieConverter::C_CieNodeMessage & orc_Message);
   static int32_t mh_CheckRange(const float64_t of64_Value,
                                const stw::opensyde_core::C_OscNodeDataPoolContent::E_Type oe_Datatype);
   static std::string mh_ReEscapeCriticalSymbols(const std::string & orc_String);

public:
   static int32_t h_ImportNetwork(const std::string & orc_File,
                                  C_CieConverter::C_CieCommDefinition & orc_Definition,
                                  stw::scl::std::vector<std::string> & orc_WarningMessages,
                                  std::string & orc_ErrorMessage, const bool oq_AddUnmappedMessages);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
