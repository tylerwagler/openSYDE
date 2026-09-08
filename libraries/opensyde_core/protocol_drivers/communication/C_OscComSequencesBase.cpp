//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Basic sequences to communicate with flashloader protocols. (implementation)

   Initialization for C_OscComDriverFlash

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <system_error>

#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"

#include "C_OscComSequencesBase.hpp"

#include "TglUtils.hpp"
#include "C_OscRoutingRoute.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   \param[in]  oq_RoutingActive     Flag for activating routing
   \param[in]  oq_UpdateRoutingMode Flag for update specific routing or generic routing
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscComSequencesBase::C_OscComSequencesBase(const bool oq_RoutingActive, const bool oq_UpdateRoutingMode) :
   mpc_ComDriver(new C_OscComDriverFlash(oq_RoutingActive, oq_UpdateRoutingMode)),
   mpc_SystemDefinition(nullptr),
   mu32_ActiveBusIndex(0U),
   mq_OpenSydeDevicesActive(false)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   destructor

   Tear down class
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscComSequencesBase::~C_OscComSequencesBase(void)
{
   delete this->mpc_ComDriver;
   mpc_SystemDefinition = nullptr;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize class parameters to use for communication

   Set parameters required for processes:
   * whole system definition
   * Flash comm driver to use
   * flags: which of the nodes are active; this determines which nodes the client expected to be present
   * index of bus the client is connected to

   The caller is responsible to keep the referenced system definition stable while functions in this class are executed
   (this class does not create a copy)

   \param[in]  orc_SystemDefinition     Entire system definition
   \param[in]  ou32_ActiveBusIndex      Index of bus within system definition that the client tool is connected to
                                         (0 = first bus)
   \param[in]  orc_ActiveNodes          Vector of flags for each node in the system definition
                                           - set the entry to true if the device is expected to be present
                                           - set the entry to false if the device is not expected to be present
   \param[in]  opc_CanDispatcher        Pointer to concrete CAN dispatcher
   \param[in]  opc_IpDispatcher         Pointer to concrete IP dispatcher
   \param[in]  opc_SecurityPemDb        Pointer to PEM database (optional)
                                        Needed if nodes with enabled security are used in the system

   \return
   Errc::success     Configuration set
   Errc::config      Invalid system definition for parameters
                     Active bus index refers to a bus that is not part of the system definition
                     Length of active nodes vector is not identical to number of nodes in system definition
                     No STW flashloader devices and no openSYDE devices are active
                     No STW flashloader devices on active bus and no openSYDE devices are active, but STW flashloader on
                        other buses are active
   Errc::overflow    Unknown transport protocol or unknown diagnostic server for at least one node
   Errc::noact       No active nodes
   Errc::com         CAN initialization failed or no route found for at least one node
   Errc::checksum    Internal buffer overflow detected
   Errc::default_    Parameter ou32_ActiveBusIndex invalid
   Errc::range       Routing configuration failed (can all nodes marked as active be reached from the defined bus ?)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscComSequencesBase::Init(C_OscSystemDefinition & orc_SystemDefinition,
                                            const uint32_t ou32_ActiveBusIndex,
                                            const std::vector<uint8_t> & orc_ActiveNodes,
                                            stw::can::C_CanDispatcher * const opc_CanDispatcher,
                                            C_OscIpDispatcher * const opc_IpDispatcher,
                                            C_OscSecurityPemDatabase * const opc_SecurityPemDb)
{
   std::error_code c_Return = Errc::config;

   if ((orc_SystemDefinition.c_Nodes.size() == orc_ActiveNodes.size()) &&
       (ou32_ActiveBusIndex < orc_SystemDefinition.c_Buses.size()))
   {
      this->mpc_SystemDefinition = &orc_SystemDefinition;
      this->mu32_ActiveBusIndex = ou32_ActiveBusIndex;
      this->mc_ActiveNodes = orc_ActiveNodes;
      this->mc_TimeoutNodes.resize(this->mc_ActiveNodes.size(), 0);

      c_Return = this->mpc_ComDriver->Init(orc_SystemDefinition, ou32_ActiveBusIndex,
                                           orc_ActiveNodes, opc_CanDispatcher, opc_IpDispatcher, opc_SecurityPemDb);

      if (c_Return == Errc::success)
      {
         this->mq_OpenSydeDevicesActive = this->m_IsAtLeastOneOpenSydeNodeActive();

         // No openSYDE devices are active
         if (this->mq_OpenSydeDevicesActive == false)
         {
            c_Return = Errc::config;
         }
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get is initialized flag

   \return
   Is initialized flag
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscComSequencesBase::IsInitialized(void) const
{
   return this->mpc_ComDriver->IsInitialized();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Gets the node index by the server id

   \param[in]     orc_ServerId         Server id
   \param[out]    oru32_NodeIndex      Found node index

   \return
   true     Node index found
   false    Node index not found
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscComSequencesBase::GetNodeIndex(const C_OscProtocolDriverOsyNode & orc_ServerId,
                                         uint32_t & oru32_NodeIndex) const
{
   return this->mpc_ComDriver->GetNodeIndex(orc_ServerId, oru32_NodeIndex);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Gets the information if at least one openSYDE device is active

   Init function must be called first.

   \return
   true     At least one device found
   false    No device found
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscComSequencesBase::IsAtLeastOneOpenSydeNodeActive(void) const
{
   return this->mq_OpenSydeDevicesActive;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if a specific node must be capable of Ethernet to Ethernet routing

   \param[in]       ou32_RouterNodeIndex     Node to check if node must be capable of Ethernet to Ethernet routing
                                             for at least one route

   \retval   true     Ethernet to Ethernet Routing must be supported by router node
   \retval   false    Ethernet to Ethernet Routing must not be supported by router node
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscComSequencesBase::IsEthToEthRoutingNecessary(const uint32_t ou32_RouterNodeIndex) const
{
   return this->mpc_ComDriver->IsEthToEthRoutingNecessary(ou32_RouterNodeIndex);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Returns the minimum Flashloader reset wait time in ms

   Default minimum value is 500 ms independent of the type.

   \param[in]  oe_Type  Type of minimum flashloader reset wait time

   \return
   Time in ms all nodes needs at least to get from application to the Flashloader
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscComSequencesBase::GetMinimumFlashloaderResetWaitTime(
   const C_OscComDriverFlash::E_MinimumFlashloaderResetWaitTimeType oe_Type) const
{
   return this->mpc_ComDriver->GetMinimumFlashloaderResetWaitTime(oe_Type);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Returns the minimum Flashloader reset wait time in ms for a specific node

   Default minimum value is 500 ms independent of the type.

   \param[in]   oe_Type             Type of minimum flashloader reset wait time
   \param[in]   orc_ServerId        Server id to get the configured wait time
   \param[out]  oru32_TimeValue     Time in ms the node need at least to get from application to the Flashloader or
                                    from Flashloader to Flashloader

   \retval   Errc::success    Time returned
   \retval   Errc::range      Node with orc_ServerId does not exist or is not active
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscComSequencesBase::GetMinimumFlashloaderResetWaitTime(
   const C_OscComDriverFlash::E_MinimumFlashloaderResetWaitTimeType oe_Type,
   const C_OscProtocolDriverOsyNode & orc_ServerId, uint32_t & oru32_TimeValue) const
{
   return this->mpc_ComDriver->GetMinimumFlashloaderResetWaitTime(oe_Type, orc_ServerId, oru32_TimeValue);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Checks if the node is reachable on the current route

   \param[in]     ou32_NodeIndex         Index of current node

   \return
   true     Node is reachable on the current route
   false    Node is not reachable on the current route
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscComSequencesBase::m_IsNodeReachable(const uint32_t ou32_NodeIndex) const
{
   bool q_Return;

   if (this->mc_TimeoutNodes[ou32_NodeIndex] == 1U)
   {
      // Node itself had a timeout and is not reachable
      q_Return = false;
   }
   else
   {
      // Check the nodes on the route
      C_OscRoutingRoute c_Route(ou32_NodeIndex);
      uint32_t u32_PointCounter;
      q_Return = true;

      this->mpc_ComDriver->GetRouteOfNode(ou32_NodeIndex, c_Route);

      for (u32_PointCounter = 0U; u32_PointCounter < c_Route.c_VecRoutePoints.size(); ++u32_PointCounter)
      {
         if (this->mc_TimeoutNodes[c_Route.c_VecRoutePoints[u32_PointCounter].u32_NodeIndex] == 1U)
         {
            // A node on the route had a timeout
            q_Return = false;
            break;
         }
      }
   }

   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Gets the information if at least one openSYDE device is active

   \return
   true     At least one device found
   false    No device found
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscComSequencesBase::m_IsAtLeastOneOpenSydeNodeActive(void) const
{
   bool q_Return = false;

   if (this->mpc_SystemDefinition != nullptr)
   {
      for (uint32_t u32_Counter = 0U; u32_Counter < this->mc_ActiveNodes.size(); ++u32_Counter)
      {
         if ((u32_Counter < this->mpc_SystemDefinition->c_Nodes.size()) && (this->mc_ActiveNodes[u32_Counter] == 1U))
         {
            const C_OscNode & rc_Node = this->mpc_SystemDefinition->c_Nodes[u32_Counter];

            if (rc_Node.c_Properties.e_FlashLoader == C_OscNodeProperties::eFL_OPEN_SYDE)
            {
               q_Return = true;
               break;
            }
         }
      }
   }

   return q_Return;
}

