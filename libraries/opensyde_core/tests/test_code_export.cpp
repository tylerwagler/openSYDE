//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Generated C code for a node

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "C_OscErrorCategory.hpp"
#include "C_OscExportNode.hpp"
#include "C_OscNodeApplication.hpp"
#include "osy_test_models.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */
namespace
{
std::string h_ReadAll(const std::string & orc_Path)
{
   std::ifstream c_In(orc_Path, std::ios::binary);

   return std::string(std::istreambuf_iterator<char>(c_In), std::istreambuf_iterator<char>());
}
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Exporting a programmable application produces the expected files, and they are sane C

   There is no golden output to compare against, so the oracle is invariants a migration defect would break: every
   reported file exists and is non-empty, holds no stray terminators, has balanced braces and carries the names
   the model put in.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(CodeExport, ProgrammableApplicationExportsSaneFiles)
{
   const std::filesystem::path c_Dir = std::filesystem::temp_directory_path() / "osy_code_export";

   (void)std::filesystem::remove_all(c_Dir);

   C_OscNode c_Node = h_MakeNode("Exporter", 7U, 0U);
   for (C_OscNodeDataPool & rc_Dp : c_Node.c_DataPools)
   {
      rc_Dp.s32_RelatedDataBlockIndex = 1; //the programmable application owns every datapool
   }
   c_Node.c_Applications[1].u16_GenCodeVersion = C_OscNodeApplication::hu16_HIGHEST_KNOWN_CODE_VERSION;

   std::vector<std::string> c_Files;
   const std::error_code c_Res = C_OscExportNode::h_CreateSourceCode(c_Node, 1U, c_Dir.string(), c_Files,
                                                                     "osy_test", "1.0.0");
   ASSERT_FALSE(static_cast<bool>(c_Res)) << c_Res.message();
   ASSERT_FALSE(c_Files.empty());

   for (const std::string & rc_File : c_Files)
   {
      std::cout << "FILE " << rc_File << std::endl;
      ASSERT_TRUE(std::filesystem::exists(rc_File)) << rc_File;
      const std::string c_Text = h_ReadAll(rc_File);
      EXPECT_FALSE(c_Text.empty()) << rc_File;
      EXPECT_EQ(std::string::npos, c_Text.find('\0')) << rc_File << " contains a terminator";
      EXPECT_EQ(std::count(c_Text.begin(), c_Text.end(), '{'), std::count(c_Text.begin(), c_Text.end(), '}')) << rc_File;
      EXPECT_NE(std::string::npos, c_Text.find("osy_test 1.0.0")) << rc_File << " lacks the tool info";
   }

   //content pins, chosen from a reviewed export of this model: names, indices, geometry and values
   const auto h_Has = [&c_Dir](const char * const pcn_File, const char * const pcn_Needle) -> bool
   {
      return h_ReadAll((c_Dir / pcn_File).string()).find(pcn_Needle) != std::string::npos;
   };
   EXPECT_TRUE(h_Has("roundtrippool_data_pool.h", "#define ROUNDTRIPPOOL_ELEM_INDEX_LISTONE_TEMPERATURE (0U)"));
   EXPECT_TRUE(h_Has("roundtrippool_data_pool.h", "#define ROUNDTRIPPOOL_LISTONE_NUMBER_OF_ELEMENTS (3U)"));
   EXPECT_TRUE(h_Has("roundtrippool_data_pool.h", "#define ROUNDTRIPPOOL_SCALING_OFFSET_LISTONE_TEMPERATURE (-2.5F)"));
   EXPECT_TRUE(h_Has("roundtrippool_data_pool.c", "-40.25,   ///< Temperature (degrees)")) << "min value";
   EXPECT_TRUE(h_Has("roundtrippool_data_pool.c", "125.75,   ///< Temperature (degrees)")) << "max value";
   EXPECT_TRUE(h_Has("roundtrippool_data_pool.c", "OSY_DPA_ELEMENT_TYPE_FLOAT64, 1U, 8U, &gt_RoundTripPool_DataPoolValues"
                     ".t_ListOneValues.f64_Temperature"));
   EXPECT_TRUE(h_Has("nvmpool_data_pool.h", "#define NVMPOOL_DATA_POOL_INDEX (1U)"));
   EXPECT_TRUE(h_Has("comm_j1939_can1.h", "#define COMM_J1939_CAN1_TX_MSG_INDEX_ENGINESTATUS (0U)"));
   EXPECT_TRUE(h_Has("comm_j1939_can1.h", "#define COMM_J1939_CAN1_RX_MSG_INDEX_COMMAND (0U)"));
   EXPECT_TRUE(h_Has("comm_j1939_can1.h", "#define COMM_J1939_CAN1_TX_MSG_MUX_INDEX_ENGINESTATUS_VALUE_7 (0U)"));
   EXPECT_TRUE(h_Has("comm_j1939_can1.c", "{ OSY_COM_BYTE_ORDER_BIG, 12U, 16U, 0U, 1U }")) << "multiplexed signal geometry";
   EXPECT_TRUE(h_Has("comm_j1939_can1.c", "{ 0x18FEF100U, 1U, 8U, OSY_COM_COMM_METHOD_CYCLIC, 0U, 0U, 250U, 1U,"));
   EXPECT_TRUE(h_Has("comm_j1939_can1.c", "&gt_ComPool_DataPool  ///< Datapool containing signal values"))
      << "the comm stack must reference the Tx list's datapool, which needs h_ListIsComTx to work";
   EXPECT_TRUE(h_Has("osy_init.c", "const T_osy_dpd_data * osy_dpd_get_init_config(void)"));
   (void)std::filesystem::remove_all(c_Dir);
}
