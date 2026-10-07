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
#include "TglTime.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace osy_scripted_transport
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A transport that answers from a script

   Cycle() is where a real transport talks to a bus. This one moves every queued request into the
   record, and for each request hands out the next scripted reply (if any) as if the server had
   answered at once, or after a wall-clock delay if the reply was scripted with one (a polling cycle is a
   different length on every platform, so delays are milliseconds, not cycles). Injected services are
   unsolicited: they land in the Rx queue on the next cycle whether or not anything was sent.
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
         if ((rc_Pending.q_Delivered == false) &&
             (static_cast<int32_t>(stw::tgl::TglGetTickCount() - rc_Pending.u32_DueMs) >= 0))
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
            if (c_Pending.u32_DelayMs == 0U)
            {
               m_Deliver(c_Pending.c_Services);
            }
            else
            {
               c_Pending.u32_DueMs = stw::tgl::TglGetTickCount() + c_Pending.u32_DelayMs;
               mc_Pending.push_back(c_Pending);
            }
         }
      }
      return stw::errors::Errc::success;
   }

   ///the next request gets exactly these services back, in this order
   void Reply(const std::initializer_list<T_Bytes> oc_Services)
   {
      c_Replies.push_back({std::vector<T_Bytes>(oc_Services), 0U, 0U, false});
   }

   ///the next request gets these services back, but only ou32_DelayMs milliseconds after it was sent
   void ReplyAfterMs(const uint32_t ou32_DelayMs, const std::initializer_list<T_Bytes> oc_Services)
   {
      c_Replies.push_back({std::vector<T_Bytes>(oc_Services), ou32_DelayMs, 0U, false});
   }

   ///arrives on the next cycle, unasked
   void Inject(const T_Bytes & orc_Service)
   {
      c_Injected.push_back(orc_Service);
   }

   ///arrives ou32_DelayMs milliseconds from now, unasked: a server that answers late
   void InjectAfterMs(const uint32_t ou32_DelayMs, const T_Bytes & orc_Service)
   {
      mc_Pending.push_back({std::vector<T_Bytes>{orc_Service}, ou32_DelayMs,
                            stw::tgl::TglGetTickCount() + ou32_DelayMs, false});
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
      uint32_t u32_DelayMs; ///< for a scripted reply: delay after the request it answers
      uint32_t u32_DueMs;   ///< absolute time to deliver, once known
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
