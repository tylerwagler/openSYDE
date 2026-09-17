//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       HALC magician: datapools generated from a HALC configuration

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <iostream>
#include <string>
#include <vector>

#include "C_OscErrorCategory.hpp"
#include "C_OscHalcMagicianGenerator.hpp"
#include "C_OscHalcMagicianUtil.hpp"
#include "osy_test_models.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A two-level (safe + non-safe) configuration yields both HALC datapools with the expected lists
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(HalcMagician, GeneratesSafeAndNonSafeDatapools)
{
   C_OscNode c_Node;

   c_Node.c_Properties.c_Name = "MagicNode";
   h_FillHalcBase(c_Node.c_HalcConfig);
   c_Node.c_HalcConfig.AddDomain(h_MakeHalcDomain());
   ASSERT_FALSE(static_cast<bool>(c_Node.c_HalcConfig.SetDomainChannelConfig(0U, 0U, true, "Ignition", "key",
                                                                             true, 0U)));

   const C_OscHalcMagicianGenerator c_Magician(&c_Node);
   std::vector<C_OscNodeDataPool> c_Dps;
   const std::error_code c_Res = c_Magician.GenerateHalcDatapools(c_Dps);
   ASSERT_FALSE(static_cast<bool>(c_Res)) << c_Res.message();

   //first run: show what came out, so the pins below are chosen from real output
   for (const C_OscNodeDataPool & rc_Dp : c_Dps)
   {
      std::cout << "DP " << rc_Dp.c_Name << " safe=" << rc_Dp.q_IsSafety << " type=" << rc_Dp.e_Type << std::endl;
      for (const C_OscNodeDataPoolList & rc_List : rc_Dp.c_Lists)
      {
         std::cout << "  LIST " << rc_List.c_Name << " (" << rc_List.c_Elements.size() << " elements, "
                   << rc_List.c_DataSets.size() << " data sets)" << std::endl;
         for (const C_OscNodeDataPoolListElement & rc_El : rc_List.c_Elements)
         {
            std::cout << "    EL " << rc_El.c_Name << " type=" << rc_El.GetType() << " array="
                      << rc_El.GetArray() << " size=" << rc_El.GetArraySize() << std::endl;
         }
      }
   }
   //two levels (safe + non-safe) times two configuration copies
   ASSERT_EQ(4U, c_Dps.size());
   EXPECT_EQ(C_OscHalcMagicianUtil::h_GetDatapoolName(false, 0U), c_Dps[0].c_Name);
   EXPECT_EQ(C_OscHalcMagicianUtil::h_GetDatapoolName(true, 0U), c_Dps[1].c_Name);
   EXPECT_EQ(C_OscHalcMagicianUtil::h_GetDatapoolName(false, 1U), c_Dps[2].c_Name);
   EXPECT_EQ(C_OscHalcMagicianUtil::h_GetDatapoolName(true, 1U), c_Dps[3].c_Name);
   for (const C_OscNodeDataPool & rc_Dp : c_Dps)
   {
      EXPECT_EQ(C_OscNodeDataPool::eHALC_NVM, rc_Dp.e_Type) << "NVM based configuration";
      ASSERT_EQ(4U, rc_Dp.c_Lists.size());
      EXPECT_EQ(C_OscHalcMagicianUtil::h_GetListName(C_OscHalcDefDomain::eVA_PARAM), rc_Dp.c_Lists[0].c_Name);
      EXPECT_EQ(C_OscHalcMagicianUtil::h_GetListName(C_OscHalcDefDomain::eVA_INPUT), rc_Dp.c_Lists[1].c_Name);
      EXPECT_EQ(C_OscHalcMagicianUtil::h_GetListName(C_OscHalcDefDomain::eVA_OUTPUT), rc_Dp.c_Lists[2].c_Name);
      EXPECT_EQ(C_OscHalcMagicianUtil::h_GetListName(C_OscHalcDefDomain::eVA_STATUS), rc_Dp.c_Lists[3].c_Name);
   }
   EXPECT_FALSE(c_Dps[0].q_IsSafety);
   EXPECT_TRUE(c_Dps[1].q_IsSafety);
   //DI_1 is safety relevant, DI_2 and DI_3 are not: the non-safe variables are arrays of two, the safe ones scalars
   const C_OscNodeDataPoolList & rc_NonSafeParams = c_Dps[0].c_Lists[0];
   const C_OscNodeDataPoolList & rc_SafeParams = c_Dps[1].c_Lists[0];
   ASSERT_EQ(5U, rc_NonSafeParams.c_Elements.size());
   EXPECT_EQ(C_OscHalcMagicianUtil::h_GetChanNumVariableName("Digital Input"), rc_NonSafeParams.c_Elements[0].c_Name);
   EXPECT_EQ(C_OscHalcMagicianUtil::h_GetUseCaseVariableName("Digital Input"), rc_NonSafeParams.c_Elements[1].c_Name);
   EXPECT_EQ("Digital InputMode", rc_NonSafeParams.c_Elements[2].c_Name);
   EXPECT_EQ("Digital InputFlags", rc_NonSafeParams.c_Elements[3].c_Name);
   EXPECT_EQ("Digital Inputdebounce", rc_NonSafeParams.c_Elements[4].c_Name);
   for (const C_OscNodeDataPoolListElement & rc_El : rc_NonSafeParams.c_Elements)
   {
      EXPECT_TRUE(rc_El.GetArray()) << rc_El.c_Name;
      EXPECT_EQ(2U, rc_El.GetArraySize()) << rc_El.c_Name;
   }
   ASSERT_EQ(6U, rc_SafeParams.c_Elements.size()) << "five variables plus reserved NVM space";
   EXPECT_FALSE(rc_SafeParams.c_Elements[2].GetArray());
   EXPECT_EQ(1U, rc_NonSafeParams.c_DataSets.size()) << "the configuration list carries the data set";
   EXPECT_EQ(C_OscNodeDataPoolContent::eUINT16, rc_NonSafeParams.c_Elements[4].GetType()) << "debounce is uint16";
   EXPECT_EQ(C_OscNodeDataPoolContent::eUINT8, rc_NonSafeParams.c_Elements[2].GetType()) << "enum on uint8";
}
