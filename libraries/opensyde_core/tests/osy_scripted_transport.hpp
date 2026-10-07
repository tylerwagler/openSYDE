//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       A transport protocol that answers from a script: every byte on the wire, without a device

   The protocol drivers encode each service request, poll the transport for the matching response and decode
   it. The transport is an abstract queue pair, so a double that answers from a script exercises the whole
   encode / poll / decode path in-process. Shared by the openSYDE driver suite and the generic UDS driver suite.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef OSY_SCRIPTED_TRANSPORT_HPP
#define OSY_SCRIPTED_TRANSPORT_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include <deque>
#include <initializer_list>
#include <vector>

#include "C_OscErrorCategory.hpp"
#include "C_OscProtocolDriverOsyTpBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace osy_scripted_transport
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A transport that answers from a script

   Cycle() is where a real transport talks to a bus. This one moves every queued request into the
   record, and for each request hands out the next scripted reply (if any) as if the server had
   answered at once, or after a number of cycles if the reply was scripted with a delay. Injected
   services are unsolicited: they land in the Rx queue on the next cycle whether or not anything was sent.
*/
//----------------------------------------------------------------------------------------------------------------------
class C_ScriptedTransport :
   public stw::opensyde_core::C_OscProtocolDriverOsyTpBase
{
public:
   using T_Bytes = std::vector<uint8_t>;

   C_ScriptedTransport(void) :
      C_OscProtocolDriverOsyTpBase(50U),
      u32_Cycles(0U)
   {
   }

   std::error_code Cycle(void) override
   {
      ++u32_Cycles;
      for (const T_Bytes & rc_Unsolicited : c_Injected)
      {
         stw::opensyde_core::C_OscProtocolDriverOsyService c_Service;
         c_Service.c_Data = rc_Unsolicited;
         (void)m_AddToRxQueue(c_Service);
      }
      c_Injected.clear();
      //replies whose delay has run out
      for (T_Pending & rc_Pending : mc_Pending)
      {
         if (rc_Pending.u32_CyclesLeft > 0U)
         {
            --rc_Pending.u32_CyclesLeft;
         }
         if ((rc_Pending.u32_CyclesLeft == 0U) && (rc_Pending.q_Delivered == false))
         {
            m_Deliver(rc_Pending.c_Services);
            rc_Pending.q_Delivered = true;
         }
      }
      stw::opensyde_core::C_OscProtocolDriverOsyService c_Request;
      while (!m_GetFromTxQueue(c_Request))
      {
         c_Requests.push_back(c_Request.c_Data);
         if (c_Replies.empty() == false)
         {
            T_Pending c_Pending = c_Replies.front();
            c_Replies.pop_front();
            if (c_Pending.u32_CyclesLeft == 0U)
            {
               m_Deliver(c_Pending.c_Services);
            }
            else
            {
               mc_Pending.push_back(c_Pending);
            }
         }
      }
      return stw::errors::Errc::success;
   }

   ///the next request gets exactly these services back, in this order
   void Reply(const std::initializer_list<T_Bytes> oc_Services)
   {
      c_Replies.push_back({std::vector<T_Bytes>(oc_Services), 0U, false});
   }

   ///the next request gets these services back, but only ou32_Cycles cycles after it was sent
   void ReplyAfterCycles(const uint32_t ou32_Cycles, const std::initializer_list<T_Bytes> oc_Services)
   {
      c_Replies.push_back({std::vector<T_Bytes>(oc_Services), ou32_Cycles, false});
   }

   ///arrives on the next cycle, unasked
   void Inject(const T_Bytes & orc_Service)
   {
      c_Injected.push_back(orc_Service);
   }

   ///arrives ou32_Cycles cycles from now, unasked: a server that answers late
   void InjectAfterCycles(const uint32_t ou32_Cycles, const T_Bytes & orc_Service)
   {
      mc_Pending.push_back({std::vector<T_Bytes>{orc_Service}, ou32_Cycles, false});
   }

   const T_Bytes & LastRequest(void) const
   {
      return c_Requests.back();
   }

   std::vector<T_Bytes> c_Requests;
   uint32_t u32_Cycles;

private:
   struct T_Pending
   {
      std::vector<T_Bytes> c_Services;
      uint32_t u32_CyclesLeft;
      bool q_Delivered;
   };

   std::deque<T_Pending> c_Replies;
   std::vector<T_Pending> mc_Pending;
   std::vector<T_Bytes> c_Injected;

   void m_Deliver(const std::vector<T_Bytes> & orc_Services)
   {
      for (const T_Bytes & rc_Reply : orc_Services)
      {
         stw::opensyde_core::C_OscProtocolDriverOsyService c_Response;
         c_Response.c_Data = rc_Reply;
         (void)m_AddToRxQueue(c_Response);
      }
   }
};
}

#endif
