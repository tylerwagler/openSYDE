//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for the X-config package creator's parameter checks

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <vector>

#include "C_OscErrorCategory.hpp"
#include "C_OscXcoCreate.hpp"
#include "C_OscXcoManifest.hpp"
#include "C_OscSystemDefinition.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A manifest naming a node the system definition does not have is refused, not read past the end

   The node lookup iterated with "<= c_Nodes.size()" and dereferenced the element, so a missing node -- the very
   case the loop exists to detect -- read one past the end before reporting. With no nodes at all that was
   c_Nodes[0] of an empty vector.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(XcoCreate, MissingNodeIsRefusedWithoutReadingPastTheEnd)
{
   const std::filesystem::path c_Package = std::filesystem::temp_directory_path() / "osy_xco_ghost_node.syde_xcfg";

   (void)std::filesystem::remove(c_Package);

   C_OscXcoManifest c_Manifest;
   c_Manifest.c_NodeName = "Ghost";
   std::vector<std::string> c_Warnings;
   std::string c_Error;

   {
      const C_OscSystemDefinition c_Empty;
      EXPECT_EQ(Errc::noact, C_OscXcoCreate::h_CreatePackage(c_Package.string(), c_Empty, c_Manifest, c_Warnings,
                                                             c_Error));
      EXPECT_NE(std::string::npos, c_Error.find("Ghost")) << c_Error;
   }
   {
      C_OscSystemDefinition c_OneNode;
      c_OneNode.c_Nodes.resize(1);
      c_OneNode.c_Nodes[0].c_Properties.c_Name = "Real";
      c_Error.clear();
      EXPECT_EQ(Errc::noact, C_OscXcoCreate::h_CreatePackage(c_Package.string(), c_OneNode, c_Manifest, c_Warnings,
                                                             c_Error));
      EXPECT_NE(std::string::npos, c_Error.find("Ghost")) << c_Error;
   }
   EXPECT_FALSE(std::filesystem::exists(c_Package)) << "a refused package must not leave a file behind";
}
