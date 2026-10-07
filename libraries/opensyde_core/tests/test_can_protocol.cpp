//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       CAN protocol helpers: COM datapool list lookup

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <cstdint>

#include "C_OscErrorCategory.hpp"
#include "C_OscCanProtocol.hpp"
#include "C_OscNodeDataPool.hpp"
#include "C_OscNodeCommFiler.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   "<interface>_TX" is a transmit list, "<interface>_RX" is not

   The check looks at the second-to-last letter. On the 1-based string class that was [length() - 1]; kept verbatim
   on std::string it read the last letter -- 'X' for both -- so no list was ever a Tx list, Rx lookups returned the
   Tx list and Tx lookups nothing. Everything that maps CAN messages to datapool elements sits on this.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(CanProtocol, ListIsComTxLooksAtTheSecondToLastLetter)
{
   C_OscNodeDataPoolList c_List;

   c_List.c_Name = "CAN1_TX";
   EXPECT_TRUE(C_OscCanProtocol::h_ListIsComTx(c_List));
   c_List.c_Name = "CAN1_RX";
   EXPECT_FALSE(C_OscCanProtocol::h_ListIsComTx(c_List));
   c_List.c_Name = "ETH2_TX";
   EXPECT_TRUE(C_OscCanProtocol::h_ListIsComTx(c_List));
   c_List.c_Name = "X";
   EXPECT_FALSE(C_OscCanProtocol::h_ListIsComTx(c_List)) << "too short to be either";
}

TEST(CanProtocol, ComListIndexFindsTxAndRxPerInterface)
{
   C_OscNodeDataPool c_Pool;

   c_Pool.c_Lists.clear();
   for (const char * const pcn_Name : {"CAN1_TX", "CAN1_RX", "CAN2_TX", "CAN2_RX"})
   {
      C_OscNodeDataPoolList c_List;
      c_List.c_Name = pcn_Name;
      c_Pool.c_Lists.push_back(c_List);
   }
   uint32_t u32_Index = 99U;
   ASSERT_FALSE(static_cast<bool>(C_OscCanProtocol::h_GetComListIndex(c_Pool, 0U, true, u32_Index)));
   EXPECT_EQ(0U, u32_Index);
   ASSERT_FALSE(static_cast<bool>(C_OscCanProtocol::h_GetComListIndex(c_Pool, 0U, false, u32_Index)));
   EXPECT_EQ(1U, u32_Index);
   ASSERT_FALSE(static_cast<bool>(C_OscCanProtocol::h_GetComListIndex(c_Pool, 1U, true, u32_Index)));
   EXPECT_EQ(2U, u32_Index);
   ASSERT_FALSE(static_cast<bool>(C_OscCanProtocol::h_GetComListIndex(c_Pool, 1U, false, u32_Index)));
   EXPECT_EQ(3U, u32_Index);
   EXPECT_EQ(Errc::range, C_OscCanProtocol::h_GetComListIndex(c_Pool, 2U, true, u32_Index)) << "no third interface";
   const C_OscNodeDataPoolList * const pc_Rx = C_OscCanProtocol::h_GetComListConst(c_Pool, 1U, false);
   ASSERT_NE(nullptr, pc_Rx);
   EXPECT_EQ("CAN2_RX", pc_Rx->c_Name);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Every protocol type survives the string round trip the node filer uses

   hc_ALL_PROTOCOLS is the list the GUI and the error checks iterate. A protocol added to the enum but not to the
   two converters would save as "invalid" and fail to load; this walks the list so that cannot slip through.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(CanProtocol, EveryProtocolTypeRoundTripsThroughTheFilerStrings)
{
   ASSERT_FALSE(C_OscCanProtocol::hc_ALL_PROTOCOLS.empty());
   for (const C_OscCanProtocol::E_Type e_Type : C_OscCanProtocol::hc_ALL_PROTOCOLS)
   {
      const std::string c_Text = C_OscNodeCommFiler::h_CommunicationProtocolToString(e_Type);
      C_OscCanProtocol::E_Type e_Loaded = C_OscCanProtocol::eLAYER2;

      EXPECT_NE("invalid", c_Text) << static_cast<int32_t>(e_Type);
      ASSERT_EQ(Errc::success, C_OscNodeCommFiler::h_StringToCommunicationProtocol(c_Text, e_Loaded)) << c_Text;
      EXPECT_EQ(e_Type, e_Loaded) << c_Text;
   }
}

TEST(CanProtocol, UnknownProtocolStringIsRejected)
{
   C_OscCanProtocol::E_Type e_Loaded = C_OscCanProtocol::eJ1939;

   EXPECT_EQ(Errc::range, C_OscNodeCommFiler::h_StringToCommunicationProtocol("not-a-protocol", e_Loaded));
   EXPECT_EQ(C_OscCanProtocol::eJ1939, e_Loaded);
}
