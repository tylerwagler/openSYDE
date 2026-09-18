//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Routing calculation: reaching a node through a gateway

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <vector>

#include "C_OscErrorCategory.hpp"
#include "C_OscDeviceDefinition.hpp"
#include "C_OscNode.hpp"
#include "C_OscRoutingCalculation.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using stw::errors::Errc;

/* -- Implementation ------------------------------------------------------------------------------------------------ */
namespace
{
C_OscNodeComInterfaceSettings h_Interface(const uint8_t ou8_Number, const uint32_t ou32_Bus, const uint8_t ou8_NodeId,
                                          const bool oq_Update, const bool oq_Routing, const bool oq_Diag)
{
   C_OscNodeComInterfaceSettings c_If;

   c_If.e_InterfaceType = C_OscSystemBus::eCAN;
   c_If.u8_InterfaceNumber = ou8_Number;
   c_If.u8_NodeId = ou8_NodeId;
   c_If.q_IsUpdateEnabled = oq_Update;
   c_If.q_IsRoutingEnabled = oq_Routing;
   c_If.q_IsDiagnosisEnabled = oq_Diag;
   c_If.AddConnection(ou32_Bus);
   return c_If;
}

//the calculation reads the target's flashloader/diagnostic capabilities from its device definition
const C_OscDeviceDefinition & h_Device(void)
{
   static C_OscDeviceDefinition hc_Device;

   if (hc_Device.c_SubDevices.empty())
   {
      C_OscSubDeviceDefinition c_Sub;
      hc_Device.c_DeviceName = "RT-Device 1";
      c_Sub.c_SubDeviceName = "RT-Device 1";
      c_Sub.q_FlashloaderOpenSydeCan = true;
      c_Sub.q_DiagnosticProtocolOpenSydeCan = true;
      c_Sub.q_ProgrammingSupport = true;
      hc_Device.c_SubDevices.push_back(c_Sub);
   }
   return hc_Device;
}

//bus 0 (where the PC sits) -- Gateway -- bus 1 -- Target; Target has no interface on bus 0
std::vector<C_OscNode> h_TwoHopSystem(void)
{
   std::vector<C_OscNode> c_Nodes(2);

   for (C_OscNode & rc_Node : c_Nodes)
   {
      rc_Node.c_DeviceType = h_Device().c_DeviceName;
      rc_Node.pc_DeviceDefinition = &h_Device();
   }
   c_Nodes[0].c_Properties.c_Name = "Gateway";
   c_Nodes[0].c_Properties.c_ComInterfaces.push_back(h_Interface(0U, 0U, 10U, true, true, true));
   c_Nodes[0].c_Properties.c_ComInterfaces.push_back(h_Interface(1U, 1U, 11U, true, true, true));
   c_Nodes[1].c_Properties.c_Name = "Target";
   c_Nodes[1].c_Properties.c_ComInterfaces.push_back(h_Interface(0U, 1U, 20U, true, false, true));
   return c_Nodes;
}
}

//----------------------------------------------------------------------------------------------------------------------
TEST(RoutingCalculation, FindsTheGatewayHop)
{
   const std::vector<C_OscNode> c_Nodes = h_TwoHopSystem();
   const std::vector<uint8_t> c_Active = {1U, 1U};
   const C_OscRoutingCalculation c_Calc(c_Nodes, c_Active, 0U, 1U, C_OscRoutingCalculation::eUPDATE);

   ASSERT_FALSE(static_cast<bool>(c_Calc.GetState())) << c_Calc.GetState().message();
   const C_OscRoutingRoute * const pc_Best = c_Calc.GetBestRoute();
   ASSERT_NE(nullptr, pc_Best);
   EXPECT_EQ(1U, pc_Best->u32_TargetNodeIndex);
   //one hop: the gateway; the target itself is not a route point
   ASSERT_EQ(1U, pc_Best->c_VecRoutePoints.size());
   EXPECT_EQ(0U, pc_Best->c_VecRoutePoints[0].u32_NodeIndex) << "the gateway comes first";
   EXPECT_EQ(0U, pc_Best->c_VecRoutePoints[0].u32_InBusIndex);
   EXPECT_EQ(10U, pc_Best->c_VecRoutePoints[0].u8_InNodeId);
   EXPECT_EQ(1U, pc_Best->c_VecRoutePoints[0].u32_OutBusIndex);
   EXPECT_EQ(11U, pc_Best->c_VecRoutePoints[0].u8_OutNodeId);
}

TEST(RoutingCalculation, InactiveGatewayMeansNoRoute)
{
   const std::vector<C_OscNode> c_Nodes = h_TwoHopSystem();
   const std::vector<uint8_t> c_Active = {0U, 1U};
   const C_OscRoutingCalculation c_Calc(c_Nodes, c_Active, 0U, 1U, C_OscRoutingCalculation::eUPDATE);

   EXPECT_EQ(Errc::com, c_Calc.GetState());
   EXPECT_EQ(nullptr, c_Calc.GetBestRoute());
}

TEST(RoutingCalculation, GatewayWithoutRoutingMeansNoRoute)
{
   std::vector<C_OscNode> c_Nodes = h_TwoHopSystem();
   c_Nodes[0].c_Properties.c_ComInterfaces[1].q_IsRoutingEnabled = false;
   const std::vector<uint8_t> c_Active = {1U, 1U};
   const C_OscRoutingCalculation c_Calc(c_Nodes, c_Active, 0U, 1U, C_OscRoutingCalculation::eUPDATE);

   EXPECT_EQ(Errc::com, c_Calc.GetState());
}

TEST(RoutingCalculation, TargetWithUpdateDisabledNeedsNoRoute)
{
   std::vector<C_OscNode> c_Nodes = h_TwoHopSystem();
   c_Nodes[1].c_Properties.c_ComInterfaces[0].q_IsUpdateEnabled = false;
   const std::vector<uint8_t> c_Active = {1U, 1U};
   const C_OscRoutingCalculation c_Calc(c_Nodes, c_Active, 0U, 1U, C_OscRoutingCalculation::eUPDATE);

   EXPECT_EQ(Errc::noact, c_Calc.GetState());
}

TEST(RoutingCalculation, UnknownTargetIsRange)
{
   const std::vector<C_OscNode> c_Nodes = h_TwoHopSystem();
   const std::vector<uint8_t> c_Active = {1U, 1U};
   const C_OscRoutingCalculation c_Calc(c_Nodes, c_Active, 0U, 7U, C_OscRoutingCalculation::eUPDATE);

   EXPECT_EQ(Errc::range, c_Calc.GetState());
}
