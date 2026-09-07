//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Encapsulates non-trivial sequences for NVM access.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstring>
#include <system_error>

#include "stwtypes.hpp"

#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscDataDealerNvm.hpp"
#include "C_SclChecksums.hpp"

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
/*! \brief   Set up class

   Initializes class elements
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscDataDealerNvm::C_OscDataDealerNvm(void) :
   C_OscDataDealer()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set up class

   Initializes class elements

   \param[in]     opc_Node          Pointer to node of data dealer
   \param[in]     ou32_NodeIndex    Index of node of data dealer
   \param[in]     opc_DiagProtocol  Pointer to used diagnostic protocol
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscDataDealerNvm::C_OscDataDealerNvm(C_OscNode * const opc_Node, const uint32_t ou32_NodeIndex,
                                       C_OscDiagProtocolBase * const opc_DiagProtocol) :
   C_OscDataDealer(opc_Node, ou32_NodeIndex, opc_DiagProtocol)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Clean up class
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscDataDealerNvm::~C_OscDataDealerNvm(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the CRC for the list

   The 16 bit CRC_CCITT is used with start value 0x1D0F.

   \param[in]     orc_List       List to calculate CRC over

   \return
   Calculated CRC
*/
//----------------------------------------------------------------------------------------------------------------------
uint16_t C_OscDataDealerNvm::NvmCalcCrc(const C_OscNodeDataPoolList & orc_List) const
{
   uint16_t u16_Crc = 0x1D0FU;

   if (this->mpc_DiagProtocol != nullptr)
   {
      uint32_t u32_Counter;
      const uint8_t u8_Endianness = this->mpc_DiagProtocol->GetEndianness();

      for (u32_Counter = 0U; u32_Counter < orc_List.c_Elements.size(); ++u32_Counter)
      {
         const C_OscNodeDataPoolListElement * const pc_Element = &orc_List.c_Elements[u32_Counter];
         std::vector<uint8_t> c_Data;

         //convert to native endianness depending on the type ...
         //no possible problem we did not check for already ...
         if (u8_Endianness == C_OscDiagProtocolBase::mhu8_ENDIANNESS_BIG)
         {
            pc_Element->c_NvmValue.GetValueAsBigEndianBlob(c_Data);
         }
         else
         {
            pc_Element->c_NvmValue.GetValueAsLittleEndianBlob(c_Data);
         }
         stw::scl::C_SclChecksums::CalcCRC16(&c_Data[0], static_cast<uint32_t>(c_Data.size()), u16_Crc);
      }
   }

   return u16_Crc;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reads a specific NVM list of ECU

   The CRC of the list will be checked.

   \param[in]     ou32_DataPoolIndex   Node datapool index
   \param[in]     ou32_ListIndex       Node datapool list index
   \param[out]    opu8_NrCode          if != NULL: negative response code in case of an error response

   \return
   Errc::success    Reading of list successful
   Errc::config     no node or diagnostic protocol are known (was this class properly Initialize()d ?)
                    protocol driver reported configuration error (was the protocol driver properly initialized ?)
   Errc::range      Input parameter invalid
   Errc::overflow   List has no elements. Nothing to read
   Errc::rd_wr      Datapool element size configuration does not match with count of read bytes
   Errc::checksum   Checksum of read datapool list is invalid
   Errc::timeout    Expected response not received within timeout
   Errc::noact      Could not send request (e.g. Tx buffer full)
   Errc::warn       Error response or malformed protocol response
   Errc::com        Pre-requisites not correct; e.g. driver not initialized or
                    parameter out of range (checked by client side)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvm::NvmReadList(const uint32_t ou32_DataPoolIndex, const uint32_t ou32_ListIndex,
                                                uint8_t * const opu8_NrCode)
{
   std::error_code c_Return = Errc::success;

   if ((mpc_Node == nullptr) || (mpc_DiagProtocol == nullptr))
   {
      c_Return = Errc::config;
   }
   else if ((this->mpc_Node->c_DataPools.size() > ou32_DataPoolIndex) &&
            (this->mpc_Node->c_DataPools[ou32_DataPoolIndex].c_Lists.size() > ou32_ListIndex))
   {
      std::vector<uint8_t> c_Values;
      C_OscNodeDataPoolList & rc_List = this->mpc_Node->c_DataPools[ou32_DataPoolIndex].c_Lists[ou32_ListIndex];

      c_Return = this->m_NvmReadListRaw(rc_List, c_Values, opu8_NrCode);

      if (!c_Return)
      {
         c_Return = this->m_SaveDumpToList(c_Values, rc_List);
      }
   }
   else
   {
      c_Return = Errc::range;
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Notify server application about NVM data changes

   \param[in]     ou8_DataPoolIndex             Node datapool index
   \param[in]     ou8_ListIndex                 Node datapool list index
   \param[out]    orq_ApplicationAcknowledge    True: positive acknowledge from server
                                                False: negative acknowledge from server
   \param[out]    opu8_NrCode                   if != NULL: negative response code in case of an error response

   \return
   Errc::success    Request sent, positive response received
   Errc::config     no node or diagnostic protocol are known (was this class properly Initialize()d ?)
                    protocol driver reported configuration error (was the protocol driver properly initialized ?)
   Errc::timeout    Expected server response not received within timeout
   Errc::noact      Could not send request (e.g. Tx buffer full)
   Errc::warn       Server sent error response
   Errc::rd_wr      unexpected content in server response (here: wrong data pool index)
   Errc::com        communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvm::NvmNotifyOfChanges(const uint8_t ou8_DataPoolIndex, const uint8_t ou8_ListIndex,
                                                       bool & orq_ApplicationAcknowledge,
                                                       uint8_t * const opu8_NrCode)
{
   std::error_code c_Return = Errc::success;

   if ((mpc_Node == nullptr) || (mpc_DiagProtocol == nullptr))
   {
      c_Return = Errc::config;
   }
   else
   {
      c_Return = this->mpc_DiagProtocol->NvmNotifyOfChanges(ou8_DataPoolIndex, ou8_ListIndex,
                                                            orq_ApplicationAcknowledge, opu8_NrCode);
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reads the dump of a specific NVM list of ECU

   \param[in]      orc_List       List with read information
   \param[out]     orc_Values     List dump
   \param[out]    opu8_NrCode          if != NULL: negative response code in case of an error response

   \return
   Errc::success    Request sent, positive response received
   Errc::overflow   List has no elements. Nothing to read
   Errc::timeout    Expected response not received within timeout
   Errc::noact      Could not send request (e.g. Tx buffer full)
   Errc::warn       Error response
   Errc::config     Pre-requisites not correct; e.g. driver not initialized or
                    parameter out of range (checked by client side)
   Errc::com        expected server response not received because of communication error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvm::m_NvmReadListRaw(const C_OscNodeDataPoolList & orc_List,
                                                     std::vector<uint8_t> & orc_Values,
                                                     uint8_t * const opu8_NrCode)
{
   std::error_code c_Return = Errc::success;

   // If CRC is active, at least 2 byte are necessary for the CRC
   if ((orc_List.u32_NvmSize > 2U) ||
       ((orc_List.u32_NvmSize > 0U) && (orc_List.q_NvmCrcActive == false)))
   {
      // Size is input parameter for NvmRead
      const uint32_t u32_NumBytesToRead = orc_List.GetNumBytesUsed();
      orc_Values.resize(u32_NumBytesToRead);

      // Read the entire list
      const std::error_code c_ProtReturn = this->mpc_DiagProtocol->NvmRead(orc_List.u32_NvmStartAddress, orc_Values,
                                                                           opu8_NrCode);

      // Adapt return value
      c_Return = C_OscDataDealerNvm::mh_AdaptProtocolReturnValue(c_ProtReturn);
   }
   else
   {
      c_Return = Errc::overflow;
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Interprets the value and if necessary the CRC of the container orc_Values into the datapool list

   If CRC is active, the CRC will be checked and updated.

   \param[in]     orc_Values     List dump
   \param[in,out] orc_List       List for filling up

   \return
   Errc::success    Filling of list successful
   Errc::rd_wr      Datapool element size configuration does not match with count of read bytes
   Errc::checksum   Checksum of read datapool list is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvm::m_SaveDumpToList(const std::vector<uint8_t> & orc_Values,
                                                     C_OscNodeDataPoolList & orc_List) const
{
   std::error_code c_Return = this->m_SaveDumpValuesToListValues(orc_Values, orc_List);

   if ((orc_List.q_NvmCrcActive == true) && (!c_Return))
   {
      if (orc_Values.size() >= 2)
      {
         // Update CRC
         std::vector<uint8_t> c_CrcData;
         uint16_t u16_CalcCrc;

         c_CrcData.resize(2);
         (void)std::memcpy(&c_CrcData[0], &orc_Values[0], 2);

         if (this->mpc_DiagProtocol->GetEndianness() == C_OscDiagProtocolBase::mhu8_ENDIANNESS_BIG)
         {
            orc_List.SetCrcFromBigEndianBlob(c_CrcData);
         }
         else
         {
            orc_List.SetCrcFromLittleEndianBlob(c_CrcData);
         }

         // Check CRC
         u16_CalcCrc = this->NvmCalcCrc(orc_List);
         if (u16_CalcCrc != orc_List.u32_NvmCrc)
         {
            c_Return = Errc::checksum;
         }
      }
      else
      {
         c_Return = Errc::checksum;
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Interprets the value of the container orc_Values into the NVM datapool list

   The flag q_IsValid of each successfully read element will be set to true.
   The CRC will not be checked, calculated and updated.

   \param[in]     orc_Values     List dump
   \param[in,out] orc_List       List for filling up

   \return
   Errc::success    Filling of list successful
   Errc::rd_wr      Datapool element size configuration does not match with count of read bytes
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvm::m_SaveDumpValuesToListValues(const std::vector<uint8_t> & orc_Values,
                                                                 C_OscNodeDataPoolList & orc_List) const
{
   std::error_code c_Return = Errc::success;
   uint32_t u32_Counter;

   for (u32_Counter = 0U; u32_Counter < orc_List.c_Elements.size(); ++u32_Counter)
   {
      C_OscNodeDataPoolListElement * const pc_Element = &orc_List.c_Elements[u32_Counter];
      // Index of data container
      const uint32_t u32_SizeElement = pc_Element->GetSizeByte();
      const uint32_t u32_Index = pc_Element->u32_NvmStartAddress - orc_List.u32_NvmStartAddress;

      if (orc_Values.size() >= (static_cast<size_t>(u32_Index) + u32_SizeElement))
      {
         std::vector<uint8_t> c_ElementData;

         // Get the relevant data for this element
         c_ElementData.resize(u32_SizeElement);
         (void)std::memcpy(&c_ElementData[0], &orc_Values[u32_Index], u32_SizeElement);

         //we have data
         //convert to native endianness depending on the type ...
         //no possible problem we did not check for already ...
         if (this->mpc_DiagProtocol->GetEndianness() == C_OscDiagProtocolBase::mhu8_ENDIANNESS_BIG)
         {
            (void)pc_Element->c_NvmValue.SetValueFromBigEndianBlob(c_ElementData);
         }
         else
         {
            (void)pc_Element->c_NvmValue.SetValueFromLittleEndianBlob(c_ElementData);
         }
         pc_Element->q_NvmValueIsValid = true;
      }
      else
      {
         c_Return = Errc::rd_wr;
         break;
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Adapts the return value of a diag protocol base function

   \param[in]     os32_ProtReturnValue   Return value of diag protocol base function

   \return
   Errc::success    Request sent, positive response received
   Errc::timeout    Expected response not received within timeout
   Errc::noact      Could not send request (e.g. Tx buffer full)
   Errc::warn       Error response or malformed protocol response
   Errc::com        Expected server response not received because of communication error
   Errc::config     Pre-requisites not correct; e.g. driver not initialized or
                    parameter out of range (checked by client side)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvm::mh_AdaptProtocolReturnValue(const std::error_code & orc_ProtReturnValue)
{
   std::error_code c_Return = Errc::success;

   if ((orc_ProtReturnValue == Errc::success) || (orc_ProtReturnValue == Errc::timeout) ||
       (orc_ProtReturnValue == Errc::warn) || (orc_ProtReturnValue == Errc::noact) ||
       (orc_ProtReturnValue == Errc::com))
   {
      // Nothing to adapt
      c_Return = orc_ProtReturnValue;
   }
   else if (orc_ProtReturnValue == Errc::rd_wr)
   {
      c_Return = Errc::warn;
   }
   else
   {
      // All other errors
      c_Return = Errc::config;
   }

   return c_Return;
}
