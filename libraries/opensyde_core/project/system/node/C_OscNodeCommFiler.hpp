//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for comm description files (header)

   See cpp file for detailed description

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODECOMMFILER_HPP
#define C_OSCNODECOMMFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <system_error>
#include <cstdint>
#include "C_OscXmlParser.hpp"
#include "C_OscCanProtocol.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscNodeCommFiler
{
public:
   static std::error_code h_LoadNodeComProtocolFile(C_OscCanProtocol & orc_NodeComProtocol,
                                                    const std::string & orc_FilePath,
                                                    const std::vector<C_OscNodeDataPool> & orc_NodeDataPools);
   static std::error_code h_LoadNodeComProtocol(C_OscCanProtocol & orc_NodeComProtocol,
                                                C_OscXmlParserBase & orc_XmlParser,
                                                const std::vector<C_OscNodeDataPool> & orc_NodeDataPools);
   static std::error_code h_SaveNodeComProtocolFile(const C_OscCanProtocol & orc_NodeComProtocol,
                                                    const std::string & orc_FilePath,
                                                    const std::string & orc_DatapoolName);
   static void h_SaveNodeComProtocol(const C_OscCanProtocol & orc_NodeComProtocol, C_OscXmlParserBase & orc_XmlParser,
                                     const std::string & orc_DatapoolName);
   static std::error_code h_LoadNodeComMessageContainers(
      std::vector<C_OscCanMessageContainer> & orc_NodeComMessageContainers, C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveNodeComMessageContainers(
      const std::vector<C_OscCanMessageContainer> & orc_NodeComMessageContainers,
      C_OscXmlParserBase & orc_XmlParser, const C_OscCanProtocol::E_Type oe_ProtocolType);
   static std::error_code h_LoadNodeComMessageContainer(C_OscCanMessageContainer & orc_NodeComMessageContainer,
                                                        C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveNodeComMessageContainer(const C_OscCanMessageContainer & orc_NodeComMessageContainer,
                                             C_OscXmlParserBase & orc_XmlParser,
                                             const C_OscCanProtocol::E_Type oe_ProtocolType);
   static std::error_code h_LoadNodeComMessages(std::vector<C_OscCanMessage> & orc_NodeComMessages,
                                                C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveNodeComMessages(const std::vector<C_OscCanMessage> & orc_NodeComMessages,
                                     C_OscXmlParserBase & orc_XmlParser,
                                     const C_OscCanProtocol::E_Type oe_ProtocolType);
   static std::error_code h_LoadNodeComMessage(C_OscCanMessage & orc_NodeComMessage,
                                               C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveNodeComMessage(const C_OscCanMessage & orc_NodeComMessage, C_OscXmlParserBase & orc_XmlParser,
                                    const C_OscCanProtocol::E_Type oe_ProtocolType);
   static std::error_code h_LoadNodeComSignals(std::vector<C_OscCanSignal> & orc_NodeComSignals,
                                               C_OscXmlParserBase & orc_XmlParser);
   static void h_SaveNodeComSignals(const std::vector<C_OscCanSignal> & orc_NodeComSignals,
                                    C_OscXmlParserBase & orc_XmlParser, const C_OscCanProtocol::E_Type oe_ProtocolType);
   static std::error_code h_LoadNodeComSignal(C_OscCanSignal & orc_NodeComSignal, C_OscXmlParserBase & orc_XmlParser,
                                              const bool oq_CanOpenOnly = false);
   static void h_SaveNodeComSignal(const C_OscCanSignal & orc_NodeComSignal, C_OscXmlParserBase & orc_XmlParser,
                                   const C_OscCanProtocol::E_Type oe_ProtocolType);
   static std::string h_CommunicationProtocolToString(
      const C_OscCanProtocol::E_Type & ore_CommunicationProtocol);
   static std::error_code h_StringToCommunicationProtocol(const std::string & orc_String,
                                                          C_OscCanProtocol::E_Type & ore_Type);
   static std::string h_GetFileName(const std::string & orc_DatapoolName);
   static void h_SaveNodeOwnerIndex(const C_OscCanInterfaceId & orc_OwnerNodeIndex, C_OscXmlParserBase & orc_XmlParser);
   static std::error_code h_LoadNodeOwnerIndex(C_OscCanInterfaceId & orc_OwnerNodeIndex,
                                               const C_OscXmlParserBase & orc_XmlParser);

private:
   static void mh_SaveMessageCanOpenPart(const C_OscCanMessage & orc_NodeCommMessage,
                                         C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_LoadMessageCanOpenPart(C_OscCanMessage & orc_NodeCommMessage,
                                                    C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveNodeOwnerIndices(const C_OscCanInterfaceId & orc_OwnerNodeIndex,
                                       C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_LoadNodeOwnerIndices(C_OscCanInterfaceId & orc_OwnerNodeIndex,
                                                  C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveSignalCanOpenPart(const C_OscCanSignal & orc_NodeCommSignal, C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_LoadSignalCanOpenPart(C_OscCanSignal & orc_NodeCommSignal,
                                                   C_OscXmlParserBase & orc_XmlParser);
   static void mh_SaveSignalJ1939Part(const C_OscCanSignal & orc_NodeCommSignal, C_OscXmlParserBase & orc_XmlParser);
   static std::error_code mh_LoadSignalJ1939Part(C_OscCanSignal & orc_NodeCommSignal,
                                                 C_OscXmlParserBase & orc_XmlParser);
   static std::string mh_CommunicationByteOrderToString(
      const C_OscCanSignal::E_ByteOrderType & ore_CommunicationByteOrder);
   static std::error_code mh_StringToCommunicationByteOrder(const std::string & orc_String,
                                                            C_OscCanSignal::E_ByteOrderType & ore_Type);
   static std::string mh_CommunicationMuxTypeToString(
      const C_OscCanSignal::E_MultiplexerType & ore_CommunicationByteOrder);
   static std::error_code mh_StringToCommunicationMuxType(const std::string & orc_String,
                                                          C_OscCanSignal::E_MultiplexerType & ore_Type);
   static std::string mh_NodeComMessageTxMethodToString(
      const C_OscCanMessage::E_TxMethodType & ore_NodeComMessageTxMethod);
   static void mh_StringToNodeComMessageTxMethod(const std::string & orc_String,
                                                 C_OscCanMessage::E_TxMethodType & ore_Type);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
