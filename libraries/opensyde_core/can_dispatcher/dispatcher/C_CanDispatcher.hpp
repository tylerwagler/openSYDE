//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       CAN dispatcher class

   Header for CAN dispatcher class

   Provides a standardized reception mechanism for incoming CAN messages.
   Multiple clients can register in order to receive all incoming CAN traffic.

   \copyright   Copyright 2010 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CCANDISPATCHERHPP
#define CCANDISPATCHERHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <mutex>
#include <deque>
#include <vector>
#include <system_error>
#include <cstdint>
#include "C_CanBase.hpp"
#include "stw_can.hpp"
#include "C_OscErrorCategory.hpp"

namespace stw
{
namespace can
{
/* -- Defines ------------------------------------------------------------------------------------------------------- */
const uint32_t mu32_CAN_QUEUE_DEFAULT_MAX_SIZE = 2048U;

/* -- Types --------------------------------------------------------------------------------------------------------- */
///We wrap up a deque in order to provide thread safety.
//Also we limit the maximum size, to make sure we don't waste all of the heap if nobody clears the RX queue.
class C_CanRxQueue
{
private:
   std::deque<T_STWCAN_Msg_RX> mc_Messages;
   uint32_t mu32_MaxSize;
   std::error_code mc_Status;

public:
   C_CanRxQueue(void);

   std::error_code Push(const T_STWCAN_Msg_RX & orc_Message);
   std::error_code Pop(T_STWCAN_Msg_RX & orc_Message);

   void SetMaxSize(const uint32_t ou32_MaxSize);
   uint32_t GetMaxSize(void) const;

   uint32_t GetSize(void) const;
   void Clear(void);

   std::error_code GetStatus(void);
};

//----------------------------------------------------------------------------------------------------------------------
///Reception filter configuration
//lint -sem(stw::can::C_CanRxFilter::PassAll,initializer)
class C_CanRxFilter
{
public:
   C_CanRxFilter(void);

   uint32_t u32_Code; //pass = ((incoming_ID & mask) == code)
   uint32_t u32_Mask;

   bool q_XTD; //false: 11bit; true: 29bit
   bool q_XTDMustMatch;
   bool q_RTR; //false: plain; true: RTR ID
   bool q_RTRMustMatch;

   void PassAll(void);
   void PassOneID(const uint32_t ou32_ID, const bool oq_ExtID, const bool oq_RTR);

   bool DoesMessagePass(const T_STWCAN_Msg_RX & orc_Message) const;
};

//----------------------------------------------------------------------------------------------------------------------
///Information about one registered dispatch client
class C_CanDispatchClient
{
public:
   C_CanRxQueue c_RXQueue;
   C_CanRxFilter c_RXFilter;
   uint16_t u16_Handle; //for resyncing with pointer list
};

//----------------------------------------------------------------------------------------------------------------------
///Extends C_CAN_Base by queuing and filtering mechanisms
class C_CanDispatcher :
   public C_CanBase
{
private:
   std::vector<C_CanDispatchClient *> mc_ClientsByHandle;
   std::vector<C_CanDispatchClient> mc_InstalledClients;

   void m_ResyncShortcutPointers(void);

   std::mutex mc_CriticalSection;

protected:
   //-----------------------------------------------------------------------------
   /*!
      \brief   Read one incoming CAN message

      Functions shall read and return one incoming CAN message.
      Must be implemented by the CAN driver.

      \param[out]  orc_Message  read message

      \return
      Errc::success   message read
      else            no message read or error
   */
   //-----------------------------------------------------------------------------
   virtual std::error_code m_CAN_Read_Msg(T_STWCAN_Msg_RX & orc_Message) = 0;

public:
   C_CanDispatcher(void);
   C_CanDispatcher(const uint8_t ou8_CommChannel);
   virtual ~C_CanDispatcher(void);

   //returns the number of newly received messages, not an error code -> stays on int32_t
   int32_t DispatchIncoming(void);
   std::error_code RegisterClient(uint16_t & oru16_Handle, const C_CanRxFilter * const opc_RXFilter = nullptr,
                                  const uint32_t & oru32_BufferSize = mu32_CAN_QUEUE_DEFAULT_MAX_SIZE);
   std::error_code RemoveClient(const uint16_t ou16_Handle);
   std::error_code SetRXFilter(const uint16_t ou16_Handle, const C_CanRxFilter & orc_RXFilter);

   std::error_code ReadFromQueue(const uint16_t ou16_Handle, T_STWCAN_Msg_RX & orc_Message);
   std::error_code ClearQueue(const uint16_t ou16_Handle);

   //we hide the base class function on purpose here
   std::error_code CAN_Read_Msg(const uint16_t ou16_Handle, T_STWCAN_Msg_RX & orc_Message); //lint !e1411
   //try to read from CAN driver and add message to all installed RX queues:
   virtual std::error_code CAN_Read_Msg(T_STWCAN_Msg_RX & orc_Message);
};

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Function Prototypes ------------------------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */
}
}
#endif
