//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data dealer over a mocked diagnostic protocol: datapool and NVM access without a device

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <cstdint>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include "C_OscErrorCategory.hpp"
#include "C_OscDataDealerNvm.hpp"
#include "C_OscDiagProtocolBase.hpp"
#include "C_OscNode.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */
namespace
{
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A diagnostic protocol that answers from memory: a byte map for NVM, a keyed store for datapool elements

   Big-endian like an openSYDE server. Everything the data dealer does not exercise answers success and records.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_MockProtocol :
   public C_OscDiagProtocolBase
{
public:
   using T_Key = std::tuple<uint8_t, uint16_t, uint16_t>;
   std::map<uint32_t, uint8_t> c_Nvm;
   std::map<T_Key, std::vector<uint8_t> > c_Elements;
   uint32_t u32_TransactionsStarted = 0U;
   uint32_t u32_TransactionsFinalized = 0U;
   uint8_t u8_LastTransactionDataPool = 0xFFU;
   uint16_t u16_LastTransactionAccessCount = 0U;
   bool q_NotifyAcknowledge = true;
   uint8_t u8_Endianness = mhu8_ENDIANNESS_BIG;

   void PokeNvm(const uint32_t ou32_Address, const std::vector<uint8_t> & orc_Bytes)
   {
      for (uint32_t u32_It = 0U; u32_It < orc_Bytes.size(); ++u32_It)
      {
         this->c_Nvm[ou32_Address + u32_It] = orc_Bytes[u32_It];
      }
   }

   std::vector<uint8_t> PeekNvm(const uint32_t ou32_Address, const uint32_t ou32_Count) const
   {
      std::vector<uint8_t> c_Out;
      for (uint32_t u32_It = 0U; u32_It < ou32_Count; ++u32_It)
      {
         const auto c_It = this->c_Nvm.find(ou32_Address + u32_It);
         c_Out.push_back((c_It == this->c_Nvm.end()) ? 0xFFU : c_It->second);
      }
      return c_Out;
   }

   uint8_t GetEndianness(void) const override
   {
      return this->u8_Endianness;
   }
   std::error_code Cycle(void) override
   {
      return Errc::success;
   }
   std::error_code DataPoolReadNumeric(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                       const uint16_t ou16_ElementIndex, std::vector<uint8_t> & orc_ReadData,
                                       uint8_t * const opu8_NrCode) override
   {
      return this->m_Read(ou8_DataPoolIndex, ou16_ListIndex, ou16_ElementIndex, orc_ReadData, opu8_NrCode);
   }
   std::error_code DataPoolReadArray(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                     const uint16_t ou16_ElementIndex, std::vector<uint8_t> & orc_ReadData,
                                     uint8_t * const opu8_NrCode) override
   {
      return this->m_Read(ou8_DataPoolIndex, ou16_ListIndex, ou16_ElementIndex, orc_ReadData, opu8_NrCode);
   }
   std::error_code DataPoolWriteNumeric(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                        const uint16_t ou16_ElementIndex, const std::vector<uint8_t> & orc_DataToWrite,
                                        uint8_t * const opu8_NrCode) override
   {
      (void)opu8_NrCode;
      this->c_Elements[T_Key(ou8_DataPoolIndex, ou16_ListIndex, ou16_ElementIndex)] = orc_DataToWrite;
      return Errc::success;
   }
   std::error_code DataPoolWriteArray(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                      const uint16_t ou16_ElementIndex, const std::vector<uint8_t> & orc_DataToWrite,
                                      uint8_t * const opu8_NrCode) override
   {
      return this->DataPoolWriteNumeric(ou8_DataPoolIndex, ou16_ListIndex, ou16_ElementIndex, orc_DataToWrite,
                                        opu8_NrCode);
   }
   std::error_code DataPoolSetEventDataRate(const uint8_t ou8_Rail, const uint16_t ou16_IntervalMs) override
   {
      (void)ou8_Rail;
      (void)ou16_IntervalMs;
      return Errc::success;
   }
   std::error_code DataPoolReadCyclic(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                      const uint16_t ou16_ElementIndex, const uint8_t ou8_Rail,
                                      uint8_t * const opu8_NrCode) override
   {
      (void)ou8_DataPoolIndex;
      (void)ou16_ListIndex;
      (void)ou16_ElementIndex;
      (void)ou8_Rail;
      (void)opu8_NrCode;
      return Errc::success;
   }
   std::error_code DataPoolReadChangeDriven(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                            const uint16_t ou16_ElementIndex, const uint8_t ou8_Rail,
                                            const uint32_t ou32_Threshold, uint8_t * const opu8_NrCode) override
   {
      (void)ou8_DataPoolIndex;
      (void)ou16_ListIndex;
      (void)ou16_ElementIndex;
      (void)ou8_Rail;
      (void)ou32_Threshold;
      (void)opu8_NrCode;
      return Errc::success;
   }
   std::error_code DataPoolStopEventDriven(void) override
   {
      return Errc::success;
   }
   std::error_code NvmRead(const uint32_t ou32_MemoryAddress, std::vector<uint8_t> & orc_DataRecord,
                           uint8_t * const opu8_NrCode) override
   {
      (void)opu8_NrCode;
      orc_DataRecord = this->PeekNvm(ou32_MemoryAddress, static_cast<uint32_t>(orc_DataRecord.size()));
      return Errc::success;
   }
   std::error_code NvmWriteStartTransaction(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_NvmAccessCount) override
   {
      ++this->u32_TransactionsStarted;
      this->u8_LastTransactionDataPool = ou8_DataPoolIndex;
      this->u16_LastTransactionAccessCount = ou16_NvmAccessCount;
      return Errc::success;
   }
   std::error_code NvmWrite(const uint32_t ou32_MemoryAddress, const std::vector<uint8_t> & orc_DataRecord,
                            uint8_t * const opu8_NrCode) override
   {
      (void)opu8_NrCode;
      this->PokeNvm(ou32_MemoryAddress, orc_DataRecord);
      return Errc::success;
   }
   std::error_code NvmWriteFinalizeTransaction(void) override
   {
      ++this->u32_TransactionsFinalized;
      return Errc::success;
   }
   std::error_code DataPoolReadVersion(const uint8_t ou8_DataPoolIndex, uint8_t (&orau8_Version)[3],
                                       uint8_t * const opu8_NrCode) override
   {
      (void)ou8_DataPoolIndex;
      (void)opu8_NrCode;
      orau8_Version[0] = 1U;
      orau8_Version[1] = 2U;
      orau8_Version[2] = 3U;
      return Errc::success;
   }
   std::error_code DataPoolReadMetaData(const uint8_t ou8_DataPoolIndex, uint8_t (&orau8_Version)[3],
                                        std::string & orc_Name, uint8_t * const opu8_NrCode) override
   {
      orc_Name = "MockPool";
      return this->DataPoolReadVersion(ou8_DataPoolIndex, orau8_Version, opu8_NrCode);
   }
   std::error_code DataPoolVerify(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_NumberOfDataPoolElements,
                                  const uint16_t ou16_DataPoolVersion, const uint32_t ou32_DataPoolChecksum,
                                  bool & orq_Match) override
   {
      (void)ou8_DataPoolIndex;
      (void)ou16_NumberOfDataPoolElements;
      (void)ou16_DataPoolVersion;
      (void)ou32_DataPoolChecksum;
      orq_Match = true;
      return Errc::success;
   }
   std::error_code NvmNotifyOfChanges(const uint8_t ou8_DataPoolIndex, const uint8_t ou8_ListIndex,
                                      bool & orq_ApplicationAcknowledge, uint8_t * const opu8_NrCode) override
   {
      (void)ou8_DataPoolIndex;
      (void)ou8_ListIndex;
      (void)opu8_NrCode;
      orq_ApplicationAcknowledge = this->q_NotifyAcknowledge;
      return Errc::success;
   }

private:
   std::error_code m_Read(const uint8_t ou8_Dp, const uint16_t ou16_List, const uint16_t ou16_El,
                          std::vector<uint8_t> & orc_Out, uint8_t * const opu8_NrCode) const
   {
      const auto c_It = this->c_Elements.find(T_Key(ou8_Dp, ou16_List, ou16_El));
      if (c_It == this->c_Elements.end())
      {
         if (opu8_NrCode != nullptr)
         {
            *opu8_NrCode = 0x31U; //request out of range
         }
         return Errc::warn;
      }
      orc_Out = c_It->second;
      return Errc::success;
   }
};

C_OscNodeDataPoolListElement h_Element(const std::string & orc_Name, const C_OscNodeDataPoolContent::E_Type oe_Type,
                                       const uint32_t ou32_NvmAddress)
{
   C_OscNodeDataPoolListElement c_El;

   c_El.c_Name = orc_Name;
   c_El.c_MinValue.SetType(oe_Type);
   c_El.c_MaxValue.SetType(oe_Type);
   c_El.c_Value.SetType(oe_Type);
   c_El.c_NvmValue.SetType(oe_Type);
   c_El.u32_NvmStartAddress = ou32_NvmAddress;
   switch (oe_Type)
   {
   case C_OscNodeDataPoolContent::eUINT16:
      c_El.c_MaxValue.SetValueU16(0xFFFFU);
      break;
   case C_OscNodeDataPoolContent::eUINT32:
      c_El.c_MaxValue.SetValueU32(0xFFFFFFFFU);
      break;
   default:
      break;
   }
   return c_El;
}

//one DIAG pool (index 0) with a uint16 and a uint32, one NVM pool (index 1) with a CRC list at 0x2000:
//CRC at 0x2000..0x2001, "Speed" (uint16) at 0x2002, "Odometer" (uint32) at 0x2004
C_OscNode h_Node(void)
{
   C_OscNode c_Node;

   c_Node.c_Properties.c_Name = "Dealt";
   c_Node.c_DataPools.clear();
   {
      C_OscNodeDataPool c_Diag;
      c_Diag.e_Type = C_OscNodeDataPool::eDIAG;
      c_Diag.c_Name = "DiagPool";
      c_Diag.c_Lists.clear();
      C_OscNodeDataPoolList c_List;
      c_List.c_Name = "Live";
      c_List.c_Elements.clear();
      c_List.c_Elements.push_back(h_Element("Rpm", C_OscNodeDataPoolContent::eUINT16, 0U));
      c_List.c_Elements.push_back(h_Element("Ticks", C_OscNodeDataPoolContent::eUINT32, 0U));
      c_Diag.c_Lists.push_back(c_List);
      c_Node.c_DataPools.push_back(c_Diag);
   }
   {
      C_OscNodeDataPool c_Nvm;
      c_Nvm.e_Type = C_OscNodeDataPool::eNVM;
      c_Nvm.c_Name = "NvmPool";
      c_Nvm.u32_NvmStartAddress = 0x2000U;
      c_Nvm.u32_NvmSize = 64U;
      c_Nvm.c_Lists.clear();
      C_OscNodeDataPoolList c_List;
      c_List.c_Name = "Persisted";
      c_List.q_NvmCrcActive = true;
      c_List.u32_NvmStartAddress = 0x2000U;
      c_List.u32_NvmSize = 8U;
      c_List.c_Elements.clear();
      c_List.c_Elements.push_back(h_Element("Speed", C_OscNodeDataPoolContent::eUINT16, 0x2002U));
      c_List.c_Elements.push_back(h_Element("Odometer", C_OscNodeDataPoolContent::eUINT32, 0x2004U));
      c_Nvm.c_Lists.push_back(c_List);
      c_Node.c_DataPools.push_back(c_Nvm);
   }
   return c_Node;
}
}

//----------------------------------------------------------------------------------------------------------------------
TEST(DataDealer, UninitialisedDealerRefusesEverything)
{
   C_OscDataDealerNvm c_Dealer;
   uint8_t u8_Nr = 0U;

   EXPECT_EQ(Errc::config, c_Dealer.DataPoolRead(0U, 0U, 0U, &u8_Nr));
   EXPECT_EQ(Errc::config, c_Dealer.DataPoolWrite(0U, 0U, 0U, &u8_Nr));
   EXPECT_EQ(Errc::config, c_Dealer.NvmRead(1U, 0U, 0U, &u8_Nr));
   EXPECT_EQ(Errc::config, c_Dealer.NvmWrite(1U, 0U, 0U, &u8_Nr));
   EXPECT_EQ(Errc::config, c_Dealer.NvmReadList(1U, 0U, &u8_Nr));
}

TEST(DataDealer, DataPoolWriteAndReadGoThroughTheProtocolBigEndian)
{
   C_OscNode c_Node = h_Node();
   C_MockProtocol c_Proto;
   C_OscDataDealerNvm c_Dealer(&c_Node, 0U, &c_Proto);
   uint8_t u8_Nr = 0U;

   c_Node.c_DataPools[0].c_Lists[0].c_Elements[1].c_Value.SetValueU32(0xDEADBEEFU);
   ASSERT_FALSE(static_cast<bool>(c_Dealer.DataPoolWrite(0U, 0U, 1U, &u8_Nr)));
   EXPECT_EQ((std::vector<uint8_t>{0xDEU, 0xADU, 0xBEU, 0xEFU}), c_Proto.c_Elements[C_MockProtocol::T_Key(0U, 0U, 1U)]);

   c_Proto.c_Elements[C_MockProtocol::T_Key(0U, 0U, 0U)] = {0x12U, 0x34U};
   ASSERT_FALSE(static_cast<bool>(c_Dealer.DataPoolRead(0U, 0U, 0U, &u8_Nr)));
   EXPECT_EQ(0x1234U, c_Node.c_DataPools[0].c_Lists[0].c_Elements[0].c_Value.GetValueU16());

   //a server answer of the wrong size is rejected, not stored
   c_Proto.c_Elements[C_MockProtocol::T_Key(0U, 0U, 0U)] = {0x12U, 0x34U, 0x56U};
   EXPECT_EQ(Errc::overflow, c_Dealer.DataPoolRead(0U, 0U, 0U, &u8_Nr));
   EXPECT_EQ(0x1234U, c_Node.c_DataPools[0].c_Lists[0].c_Elements[0].c_Value.GetValueU16());

   //an element the server does not know yields the negative response code
   c_Proto.c_Elements.erase(C_MockProtocol::T_Key(0U, 0U, 1U));
   EXPECT_EQ(Errc::warn, c_Dealer.DataPoolRead(0U, 0U, 1U, &u8_Nr));
   EXPECT_EQ(0x31U, u8_Nr);
   EXPECT_EQ(Errc::range, c_Dealer.DataPoolRead(0U, 0U, 7U, &u8_Nr)) << "no such element in the node";

   //little-endian servers get little-endian bytes
   c_Proto.u8_Endianness = C_OscDiagProtocolBase::mhu8_ENDIANNESS_LITTLE;
   ASSERT_FALSE(static_cast<bool>(c_Dealer.DataPoolWrite(0U, 0U, 1U, &u8_Nr)));
   EXPECT_EQ((std::vector<uint8_t>{0xEFU, 0xBEU, 0xADU, 0xDEU}), c_Proto.c_Elements[C_MockProtocol::T_Key(0U, 0U, 1U)]);
}

TEST(DataDealer, NvmListReadChecksTheCrc)
{
   C_OscNode c_Node = h_Node();
   C_MockProtocol c_Proto;
   C_OscDataDealerNvm c_Dealer(&c_Node, 0U, &c_Proto);
   uint8_t u8_Nr = 0U;
   C_OscNodeDataPoolList & rc_List = c_Node.c_DataPools[1].c_Lists[0];

   //the CRC the dealer expects is its own: compute it over the values the device "has"
   rc_List.c_Elements[0].c_NvmValue.SetValueU16(0x1234U);
   rc_List.c_Elements[1].c_NvmValue.SetValueU32(0x89ABCDEFU);
   const uint16_t u16_Crc = c_Dealer.NvmCalcCrc(rc_List);
   c_Proto.PokeNvm(0x2000U, {static_cast<uint8_t>(u16_Crc >> 8U), static_cast<uint8_t>(u16_Crc & 0xFFU),
                             0x12U, 0x34U, 0x89U, 0xABU, 0xCDU, 0xEFU});
   rc_List.c_Elements[0].c_NvmValue.SetValueU16(0U);
   rc_List.c_Elements[1].c_NvmValue.SetValueU32(0U);
   rc_List.u32_NvmCrc = 0U;

   ASSERT_FALSE(static_cast<bool>(c_Dealer.NvmReadList(1U, 0U, &u8_Nr)));
   EXPECT_EQ(0x1234U, rc_List.c_Elements[0].c_NvmValue.GetValueU16());
   EXPECT_EQ(0x89ABCDEFU, rc_List.c_Elements[1].c_NvmValue.GetValueU32());
   EXPECT_EQ(u16_Crc, rc_List.u32_NvmCrc) << "the CRC read from the device is kept on the list";

   //one flipped bit in the payload: the CRC no longer matches
   c_Proto.c_Nvm[0x2003U] ^= 0x01U;
   EXPECT_EQ(Errc::checksum, c_Dealer.NvmReadList(1U, 0U, &u8_Nr));

   //the CRC depends on the server's endianness, which is what makes a wrong assumption visible
   c_Proto.u8_Endianness = C_OscDiagProtocolBase::mhu8_ENDIANNESS_LITTLE;
   EXPECT_NE(u16_Crc, c_Dealer.NvmCalcCrc(rc_List));

   EXPECT_EQ(Errc::range, c_Dealer.NvmReadList(1U, 5U, &u8_Nr)) << "no such list";
   EXPECT_EQ(Errc::range, c_Dealer.NvmReadList(9U, 0U, &u8_Nr)) << "no such datapool";
}

TEST(DataDealer, NvmElementReadAndWriteUseTheElementAddress)
{
   C_OscNode c_Node = h_Node();
   C_MockProtocol c_Proto;
   C_OscDataDealerNvm c_Dealer(&c_Node, 0U, &c_Proto);
   uint8_t u8_Nr = 0U;
   C_OscNodeDataPoolList & rc_List = c_Node.c_DataPools[1].c_Lists[0];

   rc_List.c_Elements[1].c_NvmValue.SetValueU32(0x01020304U);
   ASSERT_FALSE(static_cast<bool>(c_Dealer.NvmWrite(1U, 0U, 1U, &u8_Nr)));
   EXPECT_EQ((std::vector<uint8_t>{0x01U, 0x02U, 0x03U, 0x04U}), c_Proto.PeekNvm(0x2004U, 4U));
   EXPECT_EQ(0xFFU, c_Proto.PeekNvm(0x2002U, 1U)[0]) << "the neighbouring element is untouched";

   c_Proto.PokeNvm(0x2002U, {0xBEU, 0xEFU});
   ASSERT_FALSE(static_cast<bool>(c_Dealer.NvmRead(1U, 0U, 0U, &u8_Nr)));
   EXPECT_EQ(0xBEEFU, rc_List.c_Elements[0].c_NvmValue.GetValueU16());

   bool q_Ack = false;
   ASSERT_FALSE(static_cast<bool>(c_Dealer.NvmNotifyOfChanges(1U, 0U, q_Ack, &u8_Nr)));
   EXPECT_TRUE(q_Ack);
   c_Proto.q_NotifyAcknowledge = false;
   ASSERT_FALSE(static_cast<bool>(c_Dealer.NvmNotifyOfChanges(1U, 0U, q_Ack, &u8_Nr)));
   EXPECT_FALSE(q_Ack);
}
