//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Store system definition data

   (See .cpp file for full description)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSYSTEMDEFINITION_HPP
#define C_OSCSYSTEMDEFINITION_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <list>
#include <system_error>
#include <vector>
#include "C_OscNode.hpp"
#include "C_OscNodeSquad.hpp"
#include "C_OscSystemBus.hpp"
#include "C_OscCanMessageIdentificationIndices.hpp"
#include "stwtypes.hpp"
#include <string>
#include "C_OscDeviceManager.hpp"
#include "C_OscCanProtocol.hpp"
#include "C_OscSystemNameMaxCharLimitChangeReportItem.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscSystemDefinition
{
public:
   C_OscSystemDefinition(void);
   virtual ~C_OscSystemDefinition(void);

   void CalcHash(uint32_t & oru32_HashValue) const;

   void AddBus(const C_OscSystemBus & orc_Bus);
   std::error_code InsertBus(const uint32_t ou32_BusIndex, const C_OscSystemBus & orc_Bus);
   std::error_code DeleteBus(const uint32_t ou32_BusIndex);
   std::error_code AddConnection(const uint32_t ou32_NodeIndex, const uint32_t ou32_BusIndex,
                                 const uint8_t ou8_Interface);
   bool CheckInterfaceIsAvailable(const uint32_t ou32_NodeIndex, const uint32_t ou32_ComIndex,
                                  const uint8_t ou8_ComNodeId) const;
   bool CheckIpAddressIsValid(const uint32_t ou32_NodeIndex, const uint32_t ou32_ComIndex,
                              const std::vector<int32_t> & orc_Ip) const;
   bool CheckBusIdAvailable(const uint8_t ou8_BusId, const uint32_t * const opu32_BusIndexToSkip = nullptr) const;
   std::error_code GetNextFreeBusId(uint8_t & oru8_BusId) const;
   std::error_code CheckErrorNode(const uint32_t ou32_NodeIndex, bool * const opq_NameConflict,
                                  bool * const opq_NameInvalid, bool * const opq_NodeIdInvalid,
                                  bool * const opq_IpInvalid, bool * const opq_DataPoolsInvalid,
                                  bool * const opq_ApplicationsInvalid, bool * const opq_DomainsInvalid,
                                  bool * const opq_CommMinSignalCountInvalid,
                                  bool * const opq_CommMaxSignalCountInvalid, bool * const opq_CoPdoCountInvalid,
                                  bool * const opq_CoNodeIdInvalid, bool * const opq_CoHearbeatTimeInvalid,
                                  const bool & orq_AllowComDataPoolException,
                                  std::vector<uint32_t> * const opc_InvalidInterfaceIndices,
                                  std::vector<uint32_t> * const opc_InvalidDataPoolIndices,
                                  std::vector<uint32_t> * const opc_InvalidApplicationIndices,
                                  std::vector<uint32_t> * const opc_InvalidDomainIndices,
                                  std::vector<C_OscCanProtocol::E_Type> * const opc_InvalidProtocolTypes) const;
   std::error_code CheckErrorBus(const uint32_t ou32_BusIndex, bool * const opq_NameConflict,
                                 bool * const opq_NameInvalid, bool * const opq_IdInvalid,
                                 bool * const opq_DataPoolsInvalid) const;
   std::error_code CheckMessageIdBus(const uint32_t ou32_BusIndex, const C_OscCanMessageUniqueId & orc_MessageId,
                                     bool & orq_Valid,
                                     const C_OscCanMessageIdentificationIndices * const opc_SkipMessage = nullptr) const;
   std::error_code CheckMessageNameBus(const uint32_t ou32_BusIndex, const std::string & orc_MessageName,
                                       bool & orq_Valid,
                                       const C_OscCanMessageIdentificationIndices * const opc_SkipMessage = nullptr) const;
   std::error_code CheckMessageMatch(const C_OscCanMessageIdentificationIndices & orc_MessageId1,
                                     const C_OscCanMessageIdentificationIndices & orc_MessageId2, bool & orq_IsMatch,
                                     const bool oq_IgnoreMessageDirection = false) const;
   void GetNameMaxCharLimitAffectedItems(const uint32_t ou32_NameMaxCharLimit,
                                         std::list<C_OscSystemNameMaxCharLimitChangeReportItem> & orc_ChangedItems);
   void ApplyNameMaxCharLimit(const uint32_t ou32_NameMaxCharLimit);
   void GetNodeIndexesOfBus(const uint32_t ou32_BusIndex, std::vector<uint32_t> & orc_NodeIndexes,
                            std::vector<uint32_t> & orc_InterfaceIndexes) const;
   void GetNodeAndComDpIndexesOfBus(const uint32_t ou32_BusIndex, std::vector<uint32_t> & orc_NodeIndexes,
                                    std::vector<uint32_t> & orc_InterfaceIndexes,
                                    std::vector<uint32_t> & orc_DatapoolIndexes) const;
   void GetNodeAndComDpIndexesOfBus(const uint32_t ou32_BusIndex, const C_OscCanProtocol::E_Type & ore_ComProtocol,
                                    std::vector<uint32_t> & orc_NodeIndexes,
                                    std::vector<uint32_t> & orc_InterfaceIndexes,
                                    std::vector<uint32_t> & orc_DatapoolIndexes) const;

   void AddNode(C_OscNode & orc_Node, const std::string & orc_SubDeviceName = "",
                const std::string & orc_MainDeviceName = "");
   void AddNodeSquad(std::vector<C_OscNode> & orc_Nodes, const std::vector<std::string> & orc_SubDeviceNames,
                     const std::string & orc_MainDeviceName);

   std::error_code DeleteNode(const uint32_t ou32_NodeIndex);

   std::error_code SetNodeName(const uint32_t ou32_NodeIndex, const std::string & orc_NodeName);
   std::error_code GetNodeSquadIndexWithNodeIndex(const uint32_t ou32_NodeIndex, uint32_t & oru32_NodeSquadIndex) const;

   static C_OscDeviceManager hc_Devices;     ///< container of device types known in the system
   std::vector<C_OscNode> c_Nodes;           ///< all nodes that are part of this system definition
   std::vector<C_OscNodeSquad> c_NodeSquads; ///< all multi CPU based devices with sub nodes of this system definition
   std::vector<C_OscSystemBus> c_Buses;      ///< all buses that are part of this system definition
   uint32_t u32_NameMaxCharLimit;            ///< global limit for naming length checks

private:
   uint32_t m_GetDataPoolHash(const uint32_t ou32_NodeIndex, const uint32_t ou32_DataPoolIndex) const;
   uint32_t m_GetRelatedProtocolHash(const uint32_t ou32_NodeIndex, const uint32_t ou32_DataPoolIndex) const;

   void m_GetNodeAndComDpIndexesOfBus(const uint32_t ou32_BusIndex,
                                      const C_OscCanProtocol::E_Type * const ope_ComProtocol,
                                      std::vector<uint32_t> & orc_NodeIndexes,
                                      std::vector<uint32_t> & orc_InterfaceIndexes,
                                      std::vector<uint32_t> * const opc_DatapoolIndexes) const;
   void m_HandleNameMaxCharLimit(const uint32_t ou32_NameMaxCharLimit,
                                 std::list<C_OscSystemNameMaxCharLimitChangeReportItem> * const opc_ChangedItems);
   void m_HandleNameMaxCharLimitNodeName(const uint32_t ou32_NodeIndex, const uint32_t ou32_NameMaxCharLimit,
                                         std::list<C_OscSystemNameMaxCharLimitChangeReportItem> * const opc_ChangedItems);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
