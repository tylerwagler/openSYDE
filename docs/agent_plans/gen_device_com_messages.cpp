// Headless generator: build an openSYDE device definition (opensyde-device-definition v0x0003)
// with the device's CAN messages embedded (from a DBC). Loads an existing device.syd for the
// hardware card, then adds the COM datapool + CAN protocol.
#include <Vector/DBC.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

#include "C_OscDeviceDefinition.hpp"
#include "C_OscDeviceDefinitionFiler.hpp"
#include "C_OscNodeDataPool.hpp"
#include "C_OscNodeDataPoolList.hpp"
#include "C_OscNodeDataPoolListElement.hpp"
#include "C_OscNodeDataPoolContent.hpp"
#include "C_OscCanProtocol.hpp"
#include "C_OscCanMessageContainer.hpp"
#include "C_OscCanMessage.hpp"
#include "C_OscCanSignal.hpp"

using namespace stw::opensyde_core;

static C_OscNodeDataPoolContent::E_Type mh_TypeFor(uint16_t ou16_BitLen, bool oq_Signed)
{
   if (oq_Signed)
   {
      if (ou16_BitLen <= 8U)  { return C_OscNodeDataPoolContent::eSINT8; }
      if (ou16_BitLen <= 16U) { return C_OscNodeDataPoolContent::eSINT16; }
      if (ou16_BitLen <= 32U) { return C_OscNodeDataPoolContent::eSINT32; }
      return C_OscNodeDataPoolContent::eSINT64;
   }
   else
   {
      if (ou16_BitLen <= 8U)  { return C_OscNodeDataPoolContent::eUINT8; }
      if (ou16_BitLen <= 16U) { return C_OscNodeDataPoolContent::eUINT16; }
      if (ou16_BitLen <= 32U) { return C_OscNodeDataPoolContent::eUINT32; }
      return C_OscNodeDataPoolContent::eUINT64;
   }
}

static void mh_SetContent(C_OscNodeDataPoolContent & orc_Content,
                          const C_OscNodeDataPoolContent::E_Type oe_Type, const double of64_Value)
{
   orc_Content.SetType(oe_Type);
   switch (oe_Type)
   {
   case C_OscNodeDataPoolContent::eUINT8:  orc_Content.SetValueU8(static_cast<uint8_t>(of64_Value)); break;
   case C_OscNodeDataPoolContent::eUINT16: orc_Content.SetValueU16(static_cast<uint16_t>(of64_Value)); break;
   case C_OscNodeDataPoolContent::eUINT32: orc_Content.SetValueU32(static_cast<uint32_t>(of64_Value)); break;
   case C_OscNodeDataPoolContent::eUINT64: orc_Content.SetValueU64(static_cast<uint64_t>(of64_Value)); break;
   case C_OscNodeDataPoolContent::eSINT8:  orc_Content.SetValueS8(static_cast<int8_t>(of64_Value)); break;
   case C_OscNodeDataPoolContent::eSINT16: orc_Content.SetValueS16(static_cast<int16_t>(of64_Value)); break;
   case C_OscNodeDataPoolContent::eSINT32: orc_Content.SetValueS32(static_cast<int32_t>(of64_Value)); break;
   case C_OscNodeDataPoolContent::eSINT64: orc_Content.SetValueS64(static_cast<int64_t>(of64_Value)); break;
   case C_OscNodeDataPoolContent::eFLOAT32: orc_Content.SetValueF32(static_cast<float>(of64_Value)); break;
   case C_OscNodeDataPoolContent::eFLOAT64: orc_Content.SetValueF64(of64_Value); break;
   default: break;
   }
}

// Data type from the DBC signal, honouring the float/double extended value type the GUI import uses.
static C_OscNodeDataPoolContent::E_Type mh_TypeForSig(const Vector::DBC::Signal & orc_Sig)
{
   if (orc_Sig.extendedValueType == Vector::DBC::Signal::ExtendedValueType::Float)
   {
      return C_OscNodeDataPoolContent::eFLOAT32;
   }
   if (orc_Sig.extendedValueType == Vector::DBC::Signal::ExtendedValueType::Double)
   {
      return C_OscNodeDataPoolContent::eFLOAT64;
   }
   return mh_TypeFor(static_cast<uint16_t>(orc_Sig.bitSize),
                     (orc_Sig.valueType == Vector::DBC::ValueType::Signed));
}

// The DBCs are ISO-8859-1; convert to UTF-8 so names/units/comments are valid XML.
static std::string mh_ToUtf8(const std::string & orc_Latin1)
{
   std::string c_Ret;
   c_Ret.reserve(orc_Latin1.size());
   for (const char c : orc_Latin1)
   {
      const unsigned char u8 = static_cast<unsigned char>(c);
      if (u8 < 0x80U) { c_Ret.push_back(c); }
      else
      {
         c_Ret.push_back(static_cast<char>(0xC0U | (u8 >> 6U)));
         c_Ret.push_back(static_cast<char>(0x80U | (u8 & 0x3FU)));
      }
   }
   return c_Ret;
}

// CE-name validation (C_OscUtils::h_CheckValidCeName) rejects names longer than 31 chars;
// truncate so signal/message names don't trip the error check and get flagged red.
static std::string mh_TruncateName(const std::string & orc_Name)
{
   constexpr uint32_t u32_NAME_MAX_CHAR = 31U;
   if (orc_Name.length() > u32_NAME_MAX_CHAR) { return orc_Name.substr(0, u32_NAME_MAX_CHAR); }
   return orc_Name;
}

int main(int argc, char ** argv)
{
   if (argc < 3) { std::cerr << "usage: gen_gps_device <device.syd> <file.dbc> <out.syd> [node-name] [rx-msgs]\n"; return 2; }

   C_OscDeviceDefinition c_Dev;
   std::error_code c_Rc = C_OscDeviceDefinitionFiler::h_Load(c_Dev, argv[1]);
   if (c_Rc) { std::cerr << "load device.syd failed: " << c_Rc.message() << "\n"; return 1; }

   std::ifstream c_In(argv[2]);
   if (!c_In.is_open()) { std::cerr << "open dbc fail\n"; return 1; }
   Vector::DBC::Network c_Net;
   c_In >> c_Net;
   // The device's node name in the DBC. The device.syd name often differs from the DBC
   // node (e.g. "BelFuse-12V" vs "BP_12V_1", "EMP Fan" vs "Fan"); pass it as argv[3].
   // If the device appears as a node in the DBC, split messages by transmitter
   // (device -> TX, other nodes -> RX). Otherwise treat the DBC as this device's
   // own message set (single-ECU DBC) and put everything in TX.
   const std::string c_NodeName = (argc >= 5) ? std::string(argv[4]) : c_Dev.c_DeviceName;
   const bool q_DeviceIsNode = (c_Net.nodes.count(c_NodeName) > 0U);
   // Optional comma-separated list of message names to force as RX (received). Used for
   // devices whose DBC does not encode direction (empty BU_, all transmitters Vector__XXX):
   // the IDD/manual tells us which messages are commands the device receives.
   std::vector<std::string> c_RxSet;
   if (argc >= 6)
   {
      std::stringstream c_SS(argv[5]);
      std::string c_Item;
      while (std::getline(c_SS, c_Item, ','))
      {
         if (c_Item.empty() == false) { c_RxSet.push_back(c_Item); }
      }
   }

   C_OscNodeDataPool c_Dp;
   c_Dp.e_Type = C_OscNodeDataPool::eCOM;
   c_Dp.c_Name = c_Dev.c_DeviceName;
   c_Dp.c_Comment = "CAN messages for this device";
   C_OscNodeDataPoolList c_TxList;
   c_TxList.c_Name = "CAN1_TX";
   c_TxList.c_Comment = "Transmitted messages";
   c_TxList.c_Elements.clear();
   C_OscNodeDataPoolList c_RxList;
   c_RxList.c_Name = "CAN1_RX";
   c_RxList.c_Comment = "Received messages";
   c_RxList.c_Elements.clear();

   C_OscCanProtocol c_Protocol;
   c_Protocol.e_Type = C_OscCanProtocol::eJ1939;
   c_Protocol.u32_DataPoolIndex = 0U;
   C_OscCanMessageContainer c_Container;
   c_Container.q_IsComProtocolUsedByInterface = true;
   c_Protocol.c_ComMessages.push_back(c_Container);

   uint32_t u32_TxElementIndex = 0U;
   uint32_t u32_RxElementIndex = 0U;
   for (const auto & rc_Entry : c_Net.messages)
   {
      const uint32_t u32_Id = rc_Entry.first;
      const Vector::DBC::Message & rc_Msg = rc_Entry.second;
      const bool q_IsRx = (std::find(c_RxSet.begin(), c_RxSet.end(), rc_Msg.name) != c_RxSet.end());
      const bool q_IsTx = (!q_IsRx) &&
                          ((q_DeviceIsNode == false) ||
                           (rc_Msg.transmitter == c_NodeName) ||
                           (rc_Msg.name.find("Address_Claim") != std::string::npos));

      C_OscCanMessage c_Message;
      c_Message.c_Name = mh_TruncateName(mh_ToUtf8(rc_Msg.name));
      c_Message.c_Comment = mh_ToUtf8(rc_Msg.comment);
      // The DBC stores J1939PG IDs (VFrameFormat=3) with bit 31 set (Vector's encoding); the
      // real 29-bit J1939 ID is the low 29 bits. The FIBEX confirms this (e.g. 0x9893FEFE -> 0x1893FEFE).
      // Exception: VECTOR__INDEPENDENT_SIG_MSG (id 0xC0000000, no VFrameFormat) is a Vector placeholder,
      // not a J1939PG id -- masking it would give 0, so keep it when the masked result is zero.
      const uint32_t u32_MaskedId = u32_Id & 0x1FFFFFFFU;
      c_Message.u32_CanId = (u32_MaskedId != 0U) ? u32_MaskedId : u32_Id;
      c_Message.q_IsExtended = (c_Message.u32_CanId > 0x7FFU);
      c_Message.u16_Dlc = static_cast<uint16_t>(rc_Msg.size);
      c_Message.q_IsMultipacket = (rc_Msg.size > 8U);
      c_Message.e_TxMethod = C_OscCanMessage::eTX_METHOD_CYCLIC;
      // Cycle/delay times from DBC attributes (GenMsgCycleTime / GenMsgDelayTime), else defaults.
      c_Message.u32_CycleTimeMs = 100U;
      c_Message.u16_DelayTimeMs = 10U;
      const auto c_CycleIt = rc_Msg.attributeValues.find("GenMsgCycleTime");
      if (c_CycleIt != rc_Msg.attributeValues.end()) { c_Message.u32_CycleTimeMs = static_cast<uint32_t>(c_CycleIt->second.integerValue); }
      const auto c_DelayIt = rc_Msg.attributeValues.find("GenMsgDelayTime");
      if (c_DelayIt != rc_Msg.attributeValues.end()) { c_Message.u16_DelayTimeMs = static_cast<uint16_t>(c_DelayIt->second.integerValue); }
      c_Message.u32_TimeoutMs = 0U;

      for (const auto & rc_SigEntry : rc_Msg.signals)
      {
         const std::string & rc_SigName = rc_SigEntry.first;
         const Vector::DBC::Signal & rc_Sig = rc_SigEntry.second;

         C_OscNodeDataPoolListElement c_El;
         c_El.c_Name = mh_TruncateName(mh_ToUtf8(rc_SigName));
         c_El.c_Comment = mh_ToUtf8(rc_Sig.comment);
         c_El.c_Unit = mh_ToUtf8(rc_Sig.unit);
         const C_OscNodeDataPoolContent::E_Type e_Type = mh_TypeForSig(rc_Sig);
         c_El.SetType(e_Type);
         c_El.f64_Factor = rc_Sig.factor;
         c_El.f64_Offset = rc_Sig.offset;
         // Value table (DBC VAL_ lines): raw value -> display name, as the GUI import does.
         for (const auto & rc_ValueDescr : rc_Sig.valueDescriptions)
         {
            c_El.c_ValueDescription.emplace(rc_ValueDescr.first, mh_ToUtf8(rc_ValueDescr.second));
         }
         // Min/max: physical range from the DBC, stored as raw values (raw * factor + offset = physical).
         const double f64_MinRaw = rc_Sig.physicalToRawValue(rc_Sig.minimum);
         const double f64_MaxRaw = rc_Sig.physicalToRawValue(rc_Sig.maximum);
         mh_SetContent(c_El.c_MinValue, e_Type, f64_MinRaw);
         mh_SetContent(c_El.c_MaxValue, e_Type, f64_MaxRaw);
         // Initial value: DBC GenSigStartValue attribute (raw) if present, else 0.
         double f64_InitValue = 0.0;
         const auto c_StartIt = rc_Sig.attributeValues.find("GenSigStartValue");
         if (c_StartIt != rc_Sig.attributeValues.end())
         {
            f64_InitValue = static_cast<double>(c_StartIt->second.integerValue);
         }
         mh_SetContent(c_El.c_Value, e_Type, f64_InitValue);
         // The initial value is stored as the first dataset value. Without it the UI signal
         // properties dereference c_DataSetValues[0] and crash.
         c_El.c_DataSetValues.push_back(c_El.c_Value);
         if (q_IsTx) { c_TxList.c_Elements.push_back(c_El); } else { c_RxList.c_Elements.push_back(c_El); }

         C_OscCanSignal c_Sig;
         c_Sig.u16_ComBitStart = static_cast<uint16_t>(rc_Sig.startBit);
         // Replicate the GUI DBC import's multiplexer handling so multiplexed signals
         // (which share bit ranges with different mux values) don't trip the
         // layout-conflict check and get flagged red in the message tree.
         bool q_MultiplexerSignal = false;
         switch (rc_Sig.multiplexor)
         {
         case Vector::DBC::Signal::Multiplexor::MultiplexorSwitch:
            c_Sig.e_MultiplexerType = C_OscCanSignal::eMUX_MULTIPLEXER_SIGNAL;
            c_Sig.u16_MultiplexValue = 0U;
            q_MultiplexerSignal = true;
            break;
         case Vector::DBC::Signal::Multiplexor::MultiplexedSignal:
            c_Sig.e_MultiplexerType = C_OscCanSignal::eMUX_MULTIPLEXED_SIGNAL;
            c_Sig.u16_MultiplexValue = static_cast<uint16_t>(rc_Sig.multiplexerSwitchValue);
            break;
         case Vector::DBC::Signal::Multiplexor::NoMultiplexor:
         default:
            c_Sig.e_MultiplexerType = C_OscCanSignal::eMUX_DEFAULT;
            c_Sig.u16_MultiplexValue = 0U;
            break;
         }
         // Multiplexer switch is limited to 16 bits (same as the GUI DBC import)
         if ((q_MultiplexerSignal == true) && (rc_Sig.bitSize > 16U))
         {
            c_Sig.u16_ComBitLength = 16U;
         }
         else
         {
            c_Sig.u16_ComBitLength = static_cast<uint16_t>(rc_Sig.bitSize);
         }
         c_Sig.u32_ComDataElementIndex = q_IsTx ? u32_TxElementIndex : u32_RxElementIndex;
         if ((rc_Sig.byteOrder == Vector::DBC::ByteOrder::Motorola) ||
             (rc_Sig.byteOrder == Vector::DBC::ByteOrder::BigEndian))
         {
            c_Sig.e_ComByteOrder = C_OscCanSignal::eBYTE_ORDER_MOTOROLA;
         }
         else
         {
            c_Sig.e_ComByteOrder = C_OscCanSignal::eBYTE_ORDER_INTEL;
         }
         uint32_t u32_Spn = 0U;
         const auto c_SpnIt = rc_Sig.attributeValues.find("SPN");
         if (c_SpnIt != rc_Sig.attributeValues.end())
         {
            u32_Spn = static_cast<uint32_t>(c_SpnIt->second.integerValue);
         }
         c_Sig.u32_J1939SuspectParameterNumber = u32_Spn;
         c_Message.c_Signals.push_back(c_Sig);

         if (q_IsTx) { ++u32_TxElementIndex; } else { ++u32_RxElementIndex; }
      }

      if (q_IsTx) { c_Protocol.c_ComMessages[0].c_TxMessages.push_back(c_Message); }
      else { c_Protocol.c_ComMessages[0].c_RxMessages.push_back(c_Message); }
   }

   c_Dp.c_Lists.clear();
   c_Dp.c_Lists.push_back(c_TxList);
   if (c_RxList.c_Elements.empty() == false) { c_Dp.c_Lists.push_back(c_RxList); }
   // replace, not append: re-running on a device.syd that already carries messages would otherwise
   // duplicate the COM datapool/protocol
   c_Dev.c_ComDataPools.clear();
   c_Dev.c_ComProtocols.clear();
   c_Dev.c_ComDataPools.push_back(c_Dp);
   c_Dev.c_ComProtocols.push_back(c_Protocol);

   c_Rc = C_OscDeviceDefinitionFiler::h_Save(c_Dev, argv[3]);
   std::cout << "h_Save result=" << c_Rc.value() << " msg='" << c_Rc.message() << "'\n";
   if (c_Rc) { return 1; }
   std::cout << "wrote " << argv[3] << " (signals=" << (u32_TxElementIndex + u32_RxElementIndex) << ")\n";
   return 0;
}
