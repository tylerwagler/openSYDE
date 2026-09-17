//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Model builders shared by the core tests

   Objects with every field away from its default, for the filer round-trips, the code exporters and the HALC
   magician. Each builder documents what the corresponding filer persists and why some fields stay at their
   default (constructors pre-populate, persistence is per type, some formats have no element for a field).

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef OSY_TEST_MODELS_HPP
#define OSY_TEST_MODELS_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <cstdint>
#include <string>
#include <vector>

#include "C_OscNode.hpp"
#include "C_OscNodeDataPool.hpp"
#include "C_OscCanProtocol.hpp"
#include "C_OscSystemBus.hpp"
#include "C_OscHalcDef.hpp"
#include "C_OscHalcConfig.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Implementation ------------------------------------------------------------------------------------------------ */
inline C_OscNodeDataPool h_MakeDataPool(const C_OscNodeDataPool::E_Type oe_Type)
{
   const bool q_Nvm = (oe_Type == C_OscNodeDataPool::eNVM);
   C_OscNodeDataPool c_Dp;

   //the constructors add one default list and one default element; build explicitly
   c_Dp.c_Lists.clear();

   c_Dp.e_Type = oe_Type;
   c_Dp.c_Name = "RoundTripPool";
   c_Dp.c_Comment = "pool comment";
   c_Dp.u16_DefinitionCrcVersion = 3U;
   c_Dp.s32_RelatedDataBlockIndex = 2;
   c_Dp.q_IsSafety = q_Nvm;
   c_Dp.q_ScopeIsPrivate = false;
   c_Dp.u32_NvmStartAddress = q_Nvm ? 0x1000U : 0U;
   c_Dp.u32_NvmSize = q_Nvm ? 4096U : 0U;

   C_OscNodeDataPoolList c_List;
   c_List.c_Elements.clear();
   c_List.c_Name = "ListOne";
   c_List.c_Comment = "first list";
   c_List.q_NvmCrcActive = q_Nvm;
   c_List.u32_NvmCrc = q_Nvm ? 0xABCD1234U : 0U;
   c_List.u32_NvmStartAddress = q_Nvm ? 0x1010U : 0U;
   c_List.u32_NvmSize = q_Nvm ? 256U : 0U;

   C_OscNodeDataPoolDataSet c_Set;
   c_Set.c_Name = "Defaults";
   c_Set.c_Comment = "one data set";
   c_List.c_DataSets.push_back(c_Set);

   //a float64 scalar with awkward factor/offset -- the locale-sensitive path
   C_OscNodeDataPoolListElement c_F64;
   c_F64.c_Name = "Temperature";
   c_F64.c_Comment = "degrees";
   c_F64.c_Unit = "C";
   c_F64.e_Access = C_OscNodeDataPoolListElement::eACCESS_RW;
   c_F64.q_DiagEventCall = !q_Nvm;
   c_F64.u32_NvmStartAddress = q_Nvm ? 0x1010U : 0U;
   c_F64.f64_Factor = 0.001;
   c_F64.f64_Offset = -2.5;
   c_F64.c_MinValue.SetType(C_OscNodeDataPoolContent::eFLOAT64);
   c_F64.c_MinValue.SetValueF64(-40.25);
   c_F64.c_MaxValue.SetType(C_OscNodeDataPoolContent::eFLOAT64);
   c_F64.c_MaxValue.SetValueF64(125.75);
   c_F64.c_Value.SetType(C_OscNodeDataPoolContent::eFLOAT64);
   c_F64.c_Value.SetValueF64(21.5);
   c_F64.c_NvmValue = c_F64.c_Value;
   c_F64.c_DataSetValues.push_back(c_F64.c_Value);
   c_F64.c_ValueDescription[0] = "zero";
   c_F64.c_ValueDescription[7] = "seven";
   c_List.c_Elements.push_back(c_F64);

   //a uint32 scalar
   C_OscNodeDataPoolListElement c_U32;
   c_U32.c_Name = "Counter";
   c_U32.e_Access = C_OscNodeDataPoolListElement::eACCESS_RO;
   c_U32.u32_NvmStartAddress = q_Nvm ? 0x1018U : 0U;
   c_U32.c_MinValue.SetType(C_OscNodeDataPoolContent::eUINT32);
   c_U32.c_MinValue.SetValueU32(0U);
   c_U32.c_MaxValue.SetType(C_OscNodeDataPoolContent::eUINT32);
   c_U32.c_MaxValue.SetValueU32(0xFFFFFFFFU);
   c_U32.c_Value.SetType(C_OscNodeDataPoolContent::eUINT32);
   c_U32.c_Value.SetValueU32(4000000000U);
   c_U32.c_NvmValue = c_U32.c_Value;
   c_U32.c_DataSetValues.push_back(c_U32.c_Value);
   c_List.c_Elements.push_back(c_U32);

   //a sint16 array, interpreted as a string
   C_OscNodeDataPoolListElement c_Arr;
   c_Arr.c_Name = "Label";
   c_Arr.q_InterpretAsString = true;
   c_Arr.u32_NvmStartAddress = q_Nvm ? 0x101CU : 0U;
   for (C_OscNodeDataPoolContent * const pc_Content : {&c_Arr.c_MinValue, &c_Arr.c_MaxValue, &c_Arr.c_Value})
   {
      pc_Content->SetType(C_OscNodeDataPoolContent::eSINT16);
      pc_Content->SetArray(true);
      pc_Content->SetArraySize(4U);
   }
   c_Arr.c_NvmValue = c_Arr.c_Value;
   c_Arr.c_DataSetValues.push_back(c_Arr.c_Value);
   c_List.c_Elements.push_back(c_Arr);

   c_Dp.c_Lists.push_back(c_List);

   C_OscNodeDataPoolList c_Empty;
   c_Empty.c_Elements.clear();
   c_Empty.c_Name = "ListTwo";
   c_Dp.c_Lists.push_back(c_Empty);

   return c_Dp;
}

inline C_OscCanProtocol h_MakeProtocol(const C_OscCanProtocol::E_Type oe_Type)
{
   const bool q_CanOpen = (oe_Type == C_OscCanProtocol::eCAN_OPEN);
   const bool q_J1939 = (oe_Type == C_OscCanProtocol::eJ1939);
   C_OscCanProtocol c_Proto;
   c_Proto.e_Type = oe_Type;
   c_Proto.u32_DataPoolIndex = 0U;

   C_OscCanMessageContainer c_Container;
   c_Container.q_IsComProtocolUsedByInterface = true;

   C_OscCanMessage c_Tx;
   c_Tx.c_Name = "EngineStatus";
   c_Tx.c_Comment = "cyclic tx";
   c_Tx.u32_CanId = q_J1939 ? 0x18FEF100U : 0x181U;
   c_Tx.q_IsExtended = q_J1939;
   c_Tx.u16_Dlc = 8U;
   c_Tx.e_TxMethod = q_CanOpen ? C_OscCanMessage::eTX_METHOD_CAN_OPEN_TYPE_254 : C_OscCanMessage::eTX_METHOD_CYCLIC;
   c_Tx.u32_CycleTimeMs = 250U;
   c_Tx.u16_DelayTimeMs = 15U;
   c_Tx.u32_TimeoutMs = 1250U;
   if (q_CanOpen)
   {
      c_Tx.c_CanOpenManagerOwnerNodeIndex.u32_NodeIndex = 3U;
      c_Tx.c_CanOpenManagerOwnerNodeIndex.u8_InterfaceNumber = 1U;
      c_Tx.q_CanOpenManagerCobIdIncludesNodeId = true;
      c_Tx.u32_CanOpenManagerCobIdOffset = 0x180U;
      c_Tx.q_CanOpenManagerMessageActive = true;
   }

   C_OscCanSignal c_Mux;
   c_Mux.e_ComByteOrder = C_OscCanSignal::eBYTE_ORDER_INTEL;
   c_Mux.u16_ComBitStart = 0U;
   c_Mux.u16_ComBitLength = 4U;
   c_Mux.u32_ComDataElementIndex = 0U;
   c_Mux.e_MultiplexerType = C_OscCanSignal::eMUX_MULTIPLEXER_SIGNAL;
   c_Tx.c_Signals.push_back(c_Mux);

   C_OscCanSignal c_Muxed;
   c_Muxed.e_ComByteOrder = C_OscCanSignal::eBYTE_ORDER_MOTOROLA;
   c_Muxed.u16_ComBitStart = 12U;
   c_Muxed.u16_ComBitLength = 16U;
   c_Muxed.u32_ComDataElementIndex = 1U;
   c_Muxed.e_MultiplexerType = C_OscCanSignal::eMUX_MULTIPLEXED_SIGNAL;
   c_Muxed.u16_MultiplexValue = 7U;
   if (q_CanOpen)
   {
      c_Muxed.u16_CanOpenManagerObjectDictionaryIndex = 0x6000U;
      c_Muxed.u8_CanOpenManagerObjectDictionarySubIndex = 2U;
   }
   if (q_J1939)
   {
      c_Muxed.u32_J1939SuspectParameterNumber = 190U;
   }
   c_Tx.c_Signals.push_back(c_Muxed);
   c_Container.c_TxMessages.push_back(c_Tx);

   C_OscCanMessage c_Rx;
   c_Rx.c_Name = "Command";
   c_Rx.u32_CanId = 0x123U;
   c_Rx.u16_Dlc = 3U;
   c_Rx.e_TxMethod = q_CanOpen ? C_OscCanMessage::eTX_METHOD_CAN_OPEN_TYPE_255 : C_OscCanMessage::eTX_METHOD_ON_EVENT;
   c_Container.c_RxMessages.push_back(c_Rx);

   c_Proto.c_ComMessages.push_back(c_Container);
   return c_Proto;
}

//pinpoints which part of a big model lost a field: the whole-model hash only says that something did
template <typename T>
inline void h_ExpectSameHash(const T & orc_Source, const T & orc_Target, const std::string & orc_What)
{
   uint32_t u32_A = 0xFFFFFFFFUL;
   uint32_t u32_B = 0xFFFFFFFFUL;

   orc_Source.CalcHash(u32_A);
   orc_Target.CalcHash(u32_B);
   EXPECT_EQ(u32_A, u32_B) << orc_What << " does not round-trip";
}

inline C_OscNode h_MakeNode(const std::string & orc_Name, const uint8_t ou8_NodeId, const uint32_t ou32_BusIndex)
{
   C_OscNode c_Node;

   c_Node.c_DeviceType = "RoundTrip Controller";
   c_Node.q_DatapoolAutoNvmStartAddress = false;
   c_Node.c_Properties.c_Name = orc_Name;
   c_Node.c_Properties.c_Comment = "node comment for " + orc_Name;
   c_Node.c_Properties.e_DiagnosticServer = C_OscNodeProperties::eDS_OPEN_SYDE;
   c_Node.c_Properties.e_FlashLoader = C_OscNodeProperties::eFL_OPEN_SYDE;
   c_Node.c_Properties.q_XappSupport = false;
   c_Node.c_Properties.c_OpenSydeServerSettings.u8_MaxClients = 3U;
   c_Node.c_Properties.c_OpenSydeServerSettings.u8_MaxParallelTransmissions = 12U;
   c_Node.c_Properties.c_OpenSydeServerSettings.s16_DpdDataBlockIndex = 1;
   c_Node.c_Properties.c_OpenSydeServerSettings.u16_MaxMessageBufferTx = 200U;
   c_Node.c_Properties.c_OpenSydeServerSettings.u16_MaxRoutingMessageBufferRx = 100U;
   c_Node.c_Properties.c_OpenSydeServerSettings.e_MaxServiceSizeMode = C_OscNodeOpenSydeServerSettings::eMSMT_MANUAL;
   c_Node.c_Properties.c_OpenSydeServerSettings.u16_MaxServiceSizeByte = 1024U;
   {
      C_OscNodeComInterfaceSettings c_If;
      c_If.e_InterfaceType = C_OscSystemBus::eCAN;
      c_If.u8_InterfaceNumber = 0U;
      c_If.u8_NodeId = ou8_NodeId;
      c_If.q_IsUpdateEnabled = true;
      c_If.q_IsRoutingEnabled = false;
      c_If.q_IsDiagnosisEnabled = true;
      c_If.AddConnection(ou32_BusIndex);
      c_If.SetInterfaceConnectedInDevice(true);
      //C_IpAddress seeds a default address, the saver only writes it for Ethernet and the loader zeros it for
      //everything else, yet CalcHash covers it on every interface type: a CAN interface only hashes the same
      //after a round-trip when its address is already zero. Recorded as a finding; harmless, nothing reads it.
      for (uint32_t u32_B = 0U; u32_B < 4U; ++u32_B)
      {
         c_If.c_Ip.au8_IpAddress[u32_B] = 0U;
         c_If.c_Ip.au8_NetMask[u32_B] = 0U;
         c_If.c_Ip.au8_DefaultGateway[u32_B] = 0U;
      }
      c_Node.c_Properties.c_ComInterfaces.push_back(c_If);
      C_OscNodeComInterfaceSettings c_Eth;
      c_Eth.e_InterfaceType = C_OscSystemBus::eETHERNET;
      c_Eth.u8_InterfaceNumber = 0U;
      c_Eth.u8_NodeId = static_cast<uint8_t>(ou8_NodeId + 10U);
      c_Eth.c_Ip.au8_IpAddress[0] = 192U;
      c_Eth.c_Ip.au8_IpAddress[1] = 168U;
      c_Eth.c_Ip.au8_IpAddress[2] = 7U;
      c_Eth.c_Ip.au8_IpAddress[3] = ou8_NodeId;
      c_Eth.c_Ip.au8_NetMask[0] = 255U;
      c_Eth.c_Ip.au8_NetMask[1] = 255U;
      c_Eth.c_Ip.au8_NetMask[2] = 255U;
      c_Eth.c_Ip.au8_DefaultGateway[0] = 192U;
      c_Eth.c_Ip.au8_DefaultGateway[1] = 168U;
      c_Eth.c_Ip.au8_DefaultGateway[2] = 7U;
      c_Eth.c_Ip.au8_DefaultGateway[3] = 1U;
      c_Node.c_Properties.c_ComInterfaces.push_back(c_Eth);
   }
   {
      C_OscNodeApplication c_App;
      c_App.e_Type = C_OscNodeApplication::eBINARY;
      c_App.c_Name = "Bootloader";
      c_App.c_Comment = "flashed once";
      c_App.q_Active = true;
      c_App.u8_ProcessId = 0U;
      c_App.c_ResultPaths.clear(); //ctor pre-populates one empty path, like the datapool lists
      c_App.c_ResultPaths.push_back("out/boot.hex");
      c_Node.c_Applications.push_back(c_App);
      C_OscNodeApplication c_Prog;
      c_Prog.e_Type = C_OscNodeApplication::ePROGRAMMABLE_APPLICATION;
      c_Prog.c_Name = "Application";
      c_Prog.c_Comment = "generated";
      c_Prog.q_Active = true;
      c_Prog.u8_ProcessId = 1U;
      c_Prog.c_ProjectPath = "proj/app";
      c_Prog.c_IdeCall = "make -C proj/app";
      c_Prog.c_CodeGeneratorPath = "tools/syde_coder_c";
      c_Prog.c_GeneratePath = "proj/app/gen";
      c_Prog.u16_GenCodeVersion = 6U;
      c_Prog.c_ResultPaths.clear();
      c_Prog.c_ResultPaths.push_back("out/app.hex");
      c_Prog.c_ResultPaths.push_back("out/app.syde_hex");
      c_Node.c_Applications.push_back(c_Prog);
   }
   c_Node.c_DataPools.push_back(h_MakeDataPool(C_OscNodeDataPool::eDIAG));
   c_Node.c_DataPools.push_back(h_MakeDataPool(C_OscNodeDataPool::eNVM));
   c_Node.c_DataPools[1].c_Name = "NvmPool";
   {
      //the protocol filer wants its datapool at u32_DataPoolIndex; h_MakeProtocol points at index 0, so redirect it
      C_OscNodeDataPool c_ComPool;
      c_ComPool.c_Lists.clear();
      c_ComPool.e_Type = C_OscNodeDataPool::eCOM;
      c_ComPool.c_Name = "ComPool";
      c_ComPool.c_Comment = "protocol datapool";
      //a COM datapool holds one Tx and one Rx list per interface, named "<interface>_TX" / "<interface>_RX", in
      //that order; the protocol's signals index into the Tx list by u32_ComDataElementIndex
      for (const char * const pcn_List : {"CAN1_TX", "CAN1_RX"})
      {
         C_OscNodeDataPoolList c_List;
         c_List.c_Elements.clear();
         c_List.c_Name = pcn_List;
         c_ComPool.c_Lists.push_back(c_List);
      }
      {
         C_OscNodeDataPoolListElement c_Mux;
         c_Mux.c_Name = "EngineStatusMux";
         c_Mux.c_MinValue.SetType(C_OscNodeDataPoolContent::eUINT8);
         c_Mux.c_MaxValue.SetType(C_OscNodeDataPoolContent::eUINT8);
         c_Mux.c_MaxValue.SetValueU8(15U);
         c_Mux.c_Value.SetType(C_OscNodeDataPoolContent::eUINT8);
         c_ComPool.c_Lists[0].c_Elements.push_back(c_Mux);
         C_OscNodeDataPoolListElement c_Rpm;
         c_Rpm.c_Name = "EngineSpeed";
         c_Rpm.c_Unit = "rpm";
         c_Rpm.c_MinValue.SetType(C_OscNodeDataPoolContent::eUINT16);
         c_Rpm.c_MaxValue.SetType(C_OscNodeDataPoolContent::eUINT16);
         c_Rpm.c_MaxValue.SetValueU16(8000U);
         c_Rpm.c_Value.SetType(C_OscNodeDataPoolContent::eUINT16);
         c_ComPool.c_Lists[0].c_Elements.push_back(c_Rpm);
      }
      c_Node.c_DataPools.push_back(c_ComPool);
      C_OscCanProtocol c_Proto = h_MakeProtocol(C_OscCanProtocol::eJ1939);
      c_Proto.u32_DataPoolIndex = 2U;
      c_Node.c_ComProtocols.push_back(c_Proto);
   }
   return c_Node;
}

inline C_OscHalcDefElement h_MakeHalcU16(const std::string & orc_Id, const uint16_t ou16_Initial, const uint16_t ou16_Min,
                                  const uint16_t ou16_Max, const std::vector<uint32_t> & orc_UseCases)
{
   C_OscHalcDefElement c_E;

   c_E.c_Id = orc_Id;
   //display names are combined with the domain's singular name into generated C identifiers and length-checked
   c_E.c_Display = orc_Id;
   c_E.c_Comment = orc_Id + " comment";
   c_E.SetType(C_OscNodeDataPoolContent::eUINT16);
   c_E.c_InitialValue.SetValueU16(ou16_Initial);
   c_E.c_MinValue.SetValueU16(ou16_Min);
   c_E.c_MaxValue.SetValueU16(ou16_Max);
   c_E.c_UseCaseAvailabilities = orc_UseCases;
   return c_E;
}

inline C_OscHalcDefDomain h_MakeHalcDomain(void)
{
   C_OscHalcDefDomain c_Dom;

   c_Dom.c_Id = "DI";
   c_Dom.c_Name = "Digital Inputs";
   c_Dom.c_SingularName = "Digital Input";
   //C_OscHalcDefDomain::c_Comment is hashed but the definition format has no element for it in either direction:
   //real device files never carry one, so it stays empty here too (recorded as a finding)
   c_Dom.e_Category = C_OscHalcDefDomain::eCA_INPUT;
   for (const char * const pcn_Name : {"DI_1", "DI_2", "DI_3"})
   {
      C_OscHalcDefChannelDef c_Ch;
      c_Ch.c_Name = pcn_Name;
      c_Dom.c_Channels.push_back(c_Ch);
   }
   {
      //every channel needs exactly one default use-case, and the default has to be within that use-case's availability
      C_OscHalcDefChannelUseCase c_Off;
      c_Off.c_Id = "off";
      c_Off.c_Display = "Off";
      c_Off.c_Comment = "channel unused";
      c_Off.c_Value.SetType(C_OscNodeDataPoolContent::eUINT8);
      c_Off.c_Value.SetValueU8(0U);
      for (uint32_t u32_It = 0U; u32_It < 3U; ++u32_It)
      {
         C_OscHalcDefChannelAvailability c_A;
         c_A.u32_ValueIndex = u32_It;
         c_Off.c_Availability.push_back(c_A);
      }
      c_Off.c_DefaultChannels = {0U, 1U};
      c_Dom.c_ChannelUseCases.push_back(c_Off);
      C_OscHalcDefChannelUseCase c_Freq;
      c_Freq.c_Id = "frequency";
      c_Freq.c_Display = "Frequency";
      c_Freq.c_Comment = "counts edges";
      c_Freq.c_Value.SetType(C_OscNodeDataPoolContent::eUINT8);
      c_Freq.c_Value.SetValueU8(1U);
      for (uint32_t u32_It = 1U; u32_It < 3U; ++u32_It)
      {
         C_OscHalcDefChannelAvailability c_A;
         c_A.u32_ValueIndex = u32_It;
         c_Freq.c_Availability.push_back(c_A);
      }
      c_Freq.c_DefaultChannels = {2U};
      c_Dom.c_ChannelUseCases.push_back(c_Freq);
   }
   //domain-level parameter: plain
   {
      C_OscHalcDefStruct c_S;
      static_cast<C_OscHalcDefElement &>(c_S) = h_MakeHalcU16("sample-rate", 100U, 1U, 1000U, {0U, 1U});
      c_Dom.c_DomainValues.c_Parameters.push_back(c_S);
   }
   //channel-level parameters: a struct of enum + bitmask, then a plain one
   {
      C_OscHalcDefStruct c_Filter;
      c_Filter.c_Id = "filter";
      c_Filter.c_Display = "Filter";
      c_Filter.c_Comment = "input conditioning";
      c_Filter.c_UseCaseAvailabilities = {0U, 1U};
      {
         C_OscHalcDefElement c_Mode;
         c_Mode.c_Id = "mode";
         c_Mode.c_Display = "Mode";
         c_Mode.c_Comment = "sampling mode";
         c_Mode.SetType(C_OscNodeDataPoolContent::eUINT8);
         c_Mode.SetComplexType(C_OscHalcDefContent::eCT_ENUM);
         C_OscNodeDataPoolContent c_V;
         c_V.SetType(C_OscNodeDataPoolContent::eUINT8);
         c_V.SetValueU8(0U);
         EXPECT_FALSE(static_cast<bool>(c_Mode.AddEnumItem("Level", c_V)));
         c_V.SetValueU8(1U);
         EXPECT_FALSE(static_cast<bool>(c_Mode.AddEnumItem("Edge", c_V)));
         c_Mode.c_InitialValue.SetValueU8(0U);
         c_Mode.c_MinValue.SetValueU8(0U);
         c_Mode.c_MaxValue.SetValueU8(1U);
         c_Mode.c_UseCaseAvailabilities = {0U, 1U};
         c_Filter.c_StructElements.push_back(c_Mode);
         C_OscHalcDefElement c_Flags;
         c_Flags.c_Id = "flags";
         c_Flags.c_Display = "Flags";
         c_Flags.c_Comment = "line options";
         c_Flags.SetType(C_OscNodeDataPoolContent::eUINT8);
         c_Flags.SetComplexType(C_OscHalcDefContent::eCT_BIT_MASK);
         for (const auto & rc_Item : std::vector<std::pair<std::string, uint64_t> >{{"Invert", 0x01ULL},
                                                                                     {"Pullup", 0x02ULL}})
         {
            C_OscHalcDefContentBitmaskItem c_B;
            c_B.c_Display = rc_Item.first;
            c_B.c_Comment = rc_Item.first + " comment";
            c_B.q_ApplyValueSetting = (rc_Item.second == 0x01ULL);
            c_B.u64_Value = rc_Item.second;
            c_Flags.AddBitmaskItem(c_B);
         }
         c_Flags.c_InitialValue.SetValueU8(0x01U);
         c_Flags.c_MinValue.SetValueU8(0U);
         c_Flags.c_MaxValue.SetValueU8(0x03U);
         c_Flags.c_UseCaseAvailabilities = {1U};
         c_Filter.c_StructElements.push_back(c_Flags);
      }
      c_Dom.c_ChannelValues.c_Parameters.push_back(c_Filter);
      C_OscHalcDefStruct c_Deb;
      static_cast<C_OscHalcDefElement &>(c_Deb) = h_MakeHalcU16("debounce", 5U, 0U, 255U, {0U, 1U});
      c_Dom.c_ChannelValues.c_Parameters.push_back(c_Deb);
   }
   {
      C_OscHalcDefStruct c_State;
      static_cast<C_OscHalcDefElement &>(c_State) = h_MakeHalcU16("state", 0U, 0U, 1U, {0U, 1U});
      c_Dom.c_ChannelValues.c_InputValues.push_back(c_State);
      C_OscHalcDefStruct c_Diag;
      static_cast<C_OscHalcDefElement &>(c_Diag) = h_MakeHalcU16("diag", 0U, 0U, 65535U, {1U});
      c_Dom.c_ChannelValues.c_StatusValues.push_back(c_Diag);
   }
   return c_Dom;
}

inline void h_FillHalcBase(C_OscHalcDefBase & orc_Base)
{
   orc_Base.u32_ContentVersion = 3U;
   orc_Base.c_DeviceName = "RT-IO";
   orc_Base.e_SafetyMode = C_OscHalcDefBase::eTWO_LEVELS_WITH_DROPPING;
   orc_Base.u8_NumConfigCopies = 2U;
   orc_Base.q_NvmBasedConfig = true;
   orc_Base.c_NvmSafeAddressOffset = {0x100U, 0x200U};
   orc_Base.c_NvmNonSafeAddressOffset = {0x300U, 0x400U};
   orc_Base.u32_NvmReservedListSizeParameters = 10U;
   orc_Base.u32_NvmReservedListSizeInputValues = 11U;
   orc_Base.u32_NvmReservedListSizeOutputValues = 12U;
   orc_Base.u32_NvmReservedListSizeStatusValues = 13U;
}

//field by field so a mismatch names the piece; the config hash alone only says that there is one
inline void h_ExpectSameChannelConfig(const C_OscHalcConfigChannel & orc_S, const C_OscHalcConfigChannel & orc_T,
                               const std::string & orc_What)
{
   h_ExpectSameHash(orc_S, orc_T, orc_What);
   EXPECT_EQ(orc_S.c_Name, orc_T.c_Name) << orc_What << " name";
   EXPECT_EQ(orc_S.c_Comment, orc_T.c_Comment) << orc_What << " comment";
   EXPECT_EQ(orc_S.q_SafetyRelevant, orc_T.q_SafetyRelevant) << orc_What << " safety";
   EXPECT_EQ(orc_S.u32_UseCaseIndex, orc_T.u32_UseCaseIndex) << orc_What << " use-case";
   ASSERT_EQ(orc_S.c_Parameters.size(), orc_T.c_Parameters.size()) << orc_What << " parameter count";
   for (uint32_t u32_P = 0U; u32_P < orc_S.c_Parameters.size(); ++u32_P)
   {
      const std::string c_P = orc_What + " parameter " + std::to_string(u32_P);
      h_ExpectSameHash(orc_S.c_Parameters[u32_P].c_Value, orc_T.c_Parameters[u32_P].c_Value, c_P + " value");
      EXPECT_EQ(orc_S.c_Parameters[u32_P].c_Comment, orc_T.c_Parameters[u32_P].c_Comment) << c_P << " comment";
      ASSERT_EQ(orc_S.c_Parameters[u32_P].c_ParameterElements.size(),
                orc_T.c_Parameters[u32_P].c_ParameterElements.size()) << c_P << " element count";
      for (uint32_t u32_E = 0U; u32_E < orc_S.c_Parameters[u32_P].c_ParameterElements.size(); ++u32_E)
      {
         const std::string c_E = c_P + " element " + std::to_string(u32_E);
         h_ExpectSameHash(orc_S.c_Parameters[u32_P].c_ParameterElements[u32_E].c_Value,
                          orc_T.c_Parameters[u32_P].c_ParameterElements[u32_E].c_Value, c_E + " value");
         EXPECT_EQ(orc_S.c_Parameters[u32_P].c_ParameterElements[u32_E].c_Comment,
                   orc_T.c_Parameters[u32_P].c_ParameterElements[u32_E].c_Comment) << c_E << " comment";
      }
   }
}

#endif
