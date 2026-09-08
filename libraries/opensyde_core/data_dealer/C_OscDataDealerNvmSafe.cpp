//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Encapsulates safety relevant sequences for NVM access. (implementation)

   As the "NVM" classes inherit from the data dealer we can only handle the safe data of one node;
   as a result the application will need to handle parameters for multiple nodes
   (e.g. multiple files with parameter set values; separate sequences for each node)

   Example for editing individual values:
   - read values from all nodes
   - write changes values to all nodes
   - read back values from all nodes
   - list the read back values from all nodes for the user to confirm
   - update the CRCs of all nodes

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"
#include "C_SclStringCompat.hpp"

#include <system_error>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "TglFile.hpp"
#include "TglUtils.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscParamSetHandler.hpp"
#include "C_OscParamSetRawNodeFiler.hpp"
#include "C_OscParamSetInterpretedNodeFiler.hpp"
#include "C_OscDataDealerNvmSafe.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::scl;
using namespace stw::tgl;

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
C_OscDataDealerNvmSafe::C_OscDataDealerNvmSafe(void) :
   C_OscDataDealerNvm(),
   me_CreateParameterSetWorkflowState(C_OscDataDealerNvmSafe::eCPSFS_IDLE),
   me_ParameterSetFileState(C_OscDataDealerNvmSafe::ePSFS_IDLE)
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
C_OscDataDealerNvmSafe::C_OscDataDealerNvmSafe(C_OscNode * const opc_Node, const uint32_t ou32_NodeIndex,
                                               C_OscDiagProtocolBase * const opc_DiagProtocol) :
   C_OscDataDealerNvm(opc_Node, ou32_NodeIndex, opc_DiagProtocol),
   me_CreateParameterSetWorkflowState(C_OscDataDealerNvmSafe::eCPSFS_IDLE),
   me_ParameterSetFileState(C_OscDataDealerNvmSafe::ePSFS_IDLE)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Clean up class
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscDataDealerNvmSafe::~C_OscDataDealerNvmSafe(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Checking all CRCs of "NVM" datapool lists of one node

   The function calculates and checks the CRCs over all datapool lists with the flag q_NvmCrcActive is set to true
   of all datapools of type "NVM".

   \param[in]  orc_Node    Node with datapools for checking the CRCs

   \return
   Errc::success  All list checksums are valid
   Errc::checksum At least one checksum of a list is invalid
   Errc::range    At least one datapool has the flag q_IsSafety set to true and
                  at least one of its list has the flag q_NvmCrcActive set to false
   Errc::config   No diagnostic protocol are known (was this class properly Initialize()d ?)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvmSafe::NvmSafeCheckCrcs(const C_OscNode & orc_Node) const
{
   std::error_code c_Return = Errc::success;

   if (this->mpc_DiagProtocol == nullptr)
   {
      c_Return = Errc::config;
   }
   else
   {
      uint32_t u32_DataPoolCounter;

      for (u32_DataPoolCounter = 0U; u32_DataPoolCounter < orc_Node.c_DataPools.size(); ++u32_DataPoolCounter)
      {
         if ((orc_Node.c_DataPools[u32_DataPoolCounter].e_Type == C_OscNodeDataPool::eNVM) ||
             (orc_Node.c_DataPools[u32_DataPoolCounter].e_Type == C_OscNodeDataPool::eHALC_NVM))
         {
            uint32_t u32_ListCounter;

            for (u32_ListCounter = 0U;
                 u32_ListCounter < orc_Node.c_DataPools[u32_DataPoolCounter].c_Lists.size();
                 ++u32_ListCounter)
            {
               const C_OscNodeDataPoolList & rc_List =
                  orc_Node.c_DataPools[u32_DataPoolCounter].c_Lists[u32_ListCounter];
               if (rc_List.q_NvmCrcActive == true)
               {
                  const uint16_t u16_CalcCrc = this->NvmCalcCrc(rc_List);

                  if (rc_List.u32_NvmCrc != static_cast<uint32_t>(u16_CalcCrc))
                  {
                     // Checksum of list is invalid
                     c_Return = Errc::checksum;
                  }
               }
               else if (orc_Node.c_DataPools[u32_DataPoolCounter].q_IsSafety == true)
               {
                  // A list in a safety datapool shall have a CRC
                  c_Return = Errc::range;
               }
               else
               {
                  // Nothing to do
               }

               if (c_Return)
               {
                  break;
               }
            }

            if (c_Return)
            {
               break;
            }
         }
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Writing of changed NVM values

   The function writes all values of all datapool elements of all lists of all datapools of
   the original node of the "C_OscDataDealer" marked as "changed" to the ECU NVM without updating the CRC(s).
   Only lists in datapools of type "NVM" will be considered.

   \param[out]  orc_ChangedElements         Container with all detected changed elements.
                                            The container will not be cleared.
   \param[in]   opc_AdditionalListsToUpdate Container with additional lists to update via this process
                                            even in the case they don't have any changed elements
                                            The specified node indices will be ignored.
   \param[out]    opu8_NrCode               if != NULL: negative response code in case of an error response

   \return
   Errc::success  Writing successful
   Errc::config   no node or diagnostic protocol are known (was this class properly Initialized ?)
                  protocol driver reported configuration error (was the protocol driver properly initialized ?)
   Errc::overflow At least one value lies outside of the defined minimum and maximum range. (checked by client side)
   Errc::busy     No changed value found and no additional lists specified (checked by client side)
   Errc::timeout  Expected server response not received within timeout
   Errc::noact    Could not send request (e.g. Tx buffer full)
   Errc::warn     server sent error response
   Errc::rd_wr    unexpected content in server response
   Errc::com      expected server response not received because of communication error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvmSafe::NvmSafeWriteChangedValues(
   std::vector<C_OscNodeDataPoolListElementId> & orc_ChangedElements,
   const std::vector<C_OscNodeDataPoolListId> * const opc_AdditionalListsToUpdate, uint8_t * const opu8_NrCode)
{
   std::error_code c_Return = Errc::success;

   // Reset the container for changed lists. Will be used by NvmSafeReadValues.
   this->mc_ChangedLists.clear();

   if ((this->mpc_Node == nullptr) || (this->mpc_DiagProtocol == nullptr))
   {
      c_Return = Errc::config;
   }
   else
   {
      //Check the additional parameter first to not write anything if this one does contain invalid indices
      if ((opc_AdditionalListsToUpdate != nullptr) && (opc_AdditionalListsToUpdate->size() > 0UL))
      {
         c_Return = Errc::success;
         for (uint32_t u32_ItAdditionalIndex = 0;
              (u32_ItAdditionalIndex < opc_AdditionalListsToUpdate->size()) && (!c_Return);
              ++u32_ItAdditionalIndex)
         {
            const C_OscNodeDataPoolListId & rc_CurAdditionalIndex =
               (*opc_AdditionalListsToUpdate)[u32_ItAdditionalIndex];
            if (rc_CurAdditionalIndex.u32_DataPoolIndex < this->mpc_Node->c_DataPools.size())
            {
               const C_OscNodeDataPool & rc_CurDataPool =
                  this->mpc_Node->c_DataPools[rc_CurAdditionalIndex.u32_DataPoolIndex];
               if (rc_CurAdditionalIndex.u32_ListIndex < rc_CurDataPool.c_Lists.size())
               {
                  //No problem found
                  this->mc_ChangedLists.insert(rc_CurAdditionalIndex);
               }
               else
               {
                  c_Return = Errc::config;
               }
            }
            else
            {
               c_Return = Errc::config;
            }
         }
      }
      else
      {
         c_Return = Errc::busy;
      }

      if (c_Return != Errc::config)
      {
         uint32_t u32_DataPoolCounter;
         C_OscNodeDataPoolListElementId c_ElementId;

         c_ElementId.u32_NodeIndex = this->mu32_NodeIndex;

         for (u32_DataPoolCounter = 0U; u32_DataPoolCounter < this->mpc_Node->c_DataPools.size(); ++u32_DataPoolCounter)
         {
            c_ElementId.u32_DataPoolIndex = u32_DataPoolCounter;

            C_OscNodeDataPool * const pc_DataPool = &this->mpc_Node->c_DataPools[u32_DataPoolCounter];
            if ((pc_DataPool->e_Type == C_OscNodeDataPool::eNVM) ||
                (pc_DataPool->e_Type == C_OscNodeDataPool::eHALC_NVM))
            {
               uint32_t u32_ListCounter;
               uint32_t u32_Elementcounter;
               uint16_t u16_AccessCount = 0U;

               // Count the changed elements
               for (u32_ListCounter = 0U; u32_ListCounter < pc_DataPool->c_Lists.size(); ++u32_ListCounter)
               {
                  for (u32_Elementcounter = 0U;
                       u32_Elementcounter < pc_DataPool->c_Lists[u32_ListCounter].c_Elements.size();
                       ++u32_Elementcounter)
                  {
                     if (pc_DataPool->c_Lists[u32_ListCounter].c_Elements[u32_Elementcounter].q_NvmValueChanged == true)
                     {
                        // Element was changed and must be written.
                        ++u16_AccessCount;
                     }
                  }
               }

               if (u16_AccessCount > 0U)
               {
                  // Prepare the transaction for this datapool
                  // Adapt errorcode
                  c_Return = C_OscDataDealerNvm::mh_AdaptProtocolReturnValue(
                     this->mpc_DiagProtocol->NvmWriteStartTransaction(static_cast<uint8_t>(u32_DataPoolCounter),
                                                                      u16_AccessCount));
                  if (!c_Return)
                  {
                     // Write the concrete elements
                     for (u32_ListCounter = 0U; u32_ListCounter < pc_DataPool->c_Lists.size(); ++u32_ListCounter)
                     {
                        C_OscNodeDataPoolList * const pc_List = &pc_DataPool->c_Lists[u32_ListCounter];
                        C_OscNodeDataPoolListId c_ListId;

                        c_ElementId.u32_ListIndex = u32_ListCounter;
                        c_ListId = c_ElementId;

                        for (u32_Elementcounter = 0U;
                             u32_Elementcounter < pc_List->c_Elements.size();
                             ++u32_Elementcounter)
                        {
                           C_OscNodeDataPoolListElement * const pc_Element = &pc_List->c_Elements[u32_Elementcounter];

                           // Check range if value was changed
                           if (pc_Element->q_NvmValueChanged == true)
                           {
                              if (!pc_Element->CheckNvmValueRange())
                              {
                                 std::vector<uint8_t> c_ElementData;

                                 //convert to native endianness depending on the type ...
                                 //no possible problem we did not check for already ...
                                 if (this->mpc_DiagProtocol->GetEndianness() ==
                                     C_OscDiagProtocolBase::mhu8_ENDIANNESS_BIG)
                                 {
                                    pc_Element->c_NvmValue.GetValueAsBigEndianBlob(c_ElementData);
                                 }
                                 else
                                 {
                                    pc_Element->c_NvmValue.GetValueAsLittleEndianBlob(c_ElementData);
                                 }

                                 // Element value was changed and is valid
                                 // Adapt errorcode
                                 c_Return = C_OscDataDealerNvm::mh_AdaptProtocolReturnValue(
                                    this->mpc_DiagProtocol->NvmWrite(pc_Element->u32_NvmStartAddress, c_ElementData,
                                                                     opu8_NrCode));

                                 if (!c_Return)
                                 {
                                    // Element written
                                    // Return the entire element id
                                    c_ElementId.u32_ElementIndex = u32_Elementcounter;
                                    orc_ChangedElements.push_back(c_ElementId);

                                    // Reset the flag
                                    pc_Element->q_NvmValueChanged = false;

                                    // Save the list index as changed list. Will be used by NvmSafeReadValues.
                                    this->mc_ChangedLists.insert(c_ListId);
                                 }
                              }
                              else
                              {
                                 c_Return = Errc::overflow;
                              }
                           }

                           if (c_Return)
                           {
                              // Service failed. Abort writing.
                              break;
                           }
                        }

                        if (c_Return)
                        {
                           // Service failed. Abort writing.
                           break;
                        }
                     }
                  }

                  if (!c_Return)
                  {
                     // All elements of datapool written. Finish this transaction.
                     // Adapt return value
                     c_Return = C_OscDataDealerNvm::mh_AdaptProtocolReturnValue(
                        this->mpc_DiagProtocol->NvmWriteFinalizeTransaction());
                  }
               }

               //Stop if service failure, continue with Errc::busy (don't stop if no elements found in first datapool)
               if ((c_Return) && (c_Return != Errc::busy))
               {
                  // Service failed. Abort writing.
                  break;
               }
            }
         }
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Reads values of all changed lists of ECU

   The function reads the values of all datapool elements of all datapool lists that
   contain datapool elements that were written by the preceding call to
   "NvmSafeWriteChangedValues". The function stores the read values in the copy of
   the original "C_OscNode" instance of "C_OscDataDealer".
   Read values are marked by setting the "Valid" flag to true.

   \param[out] orpc_NodeCopy Parameter to access the read values
   \param[out] opu8_NrCode   if != NULL: negative response code in case of an error response

   \return
   Errc::success  Values read successful
   Errc::config   No node or diagnostic protocol are known (was this class properly Initialize()d ?)
                  protocol driver reported configuration error (was the protocol driver properly initialized ?)
   Errc::overflow At least one list has no elements.
   Errc::rd_wr    No list contains datapool elements that were
                  written by the preceding call to "NvmSafeWriteChangedValues"
   Errc::range    At least one index of a datapool or a list of changed lists is invalid or
                  datapool element size configuration does not match with count of read bytes
   Errc::timeout  Expected server response not received within timeout
   Errc::noact    Could not send request (e.g. Tx buffer full)
   Errc::warn     Server sent error response
   Errc::com      expected server response not received because of communication error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvmSafe::NvmSafeReadValues(const C_OscNode * (&orpc_NodeCopy),
                                                          uint8_t * const opu8_NrCode)
{
   std::error_code c_Return = Errc::success;

   if ((this->mpc_Node == nullptr) || (this->mpc_DiagProtocol == nullptr))
   {
      c_Return = Errc::config;
   }
   else
   {
      c_Return = Errc::rd_wr;

      // Copy the original instance
      this->mc_NodeCopy = *this->mpc_Node;

      if (this->mc_ChangedLists.size() > 0)
      {
         std::set<C_OscNodeDataPoolListId>::const_iterator c_ItChangedList;

         // Read all changed lists
         for (c_ItChangedList = this->mc_ChangedLists.begin();
              c_ItChangedList != this->mc_ChangedLists.end();
              ++c_ItChangedList)
         {
            if (((*c_ItChangedList).u32_DataPoolIndex < this->mc_NodeCopy.c_DataPools.size()) &&
                ((*c_ItChangedList).u32_ListIndex <
                 this->mc_NodeCopy.c_DataPools[(*c_ItChangedList).u32_DataPoolIndex].c_Lists.size()))
            {
               C_OscNodeDataPoolList & rc_List =
                  this->mc_NodeCopy.c_DataPools[(*c_ItChangedList).u32_DataPoolIndex].c_Lists[(*c_ItChangedList).
                                                                                              u32_ListIndex];
               std::vector<uint8_t> c_Values;
               uint32_t u32_ElementCounter;

               // Reset of all valid flags
               for (u32_ElementCounter = 0U; u32_ElementCounter < rc_List.c_Elements.size(); ++u32_ElementCounter)
               {
                  rc_List.c_Elements[u32_ElementCounter].q_NvmValueIsValid = false;
               }

               c_Return = this->m_NvmReadListRaw(rc_List, c_Values, opu8_NrCode);

               if (!c_Return)
               {
                  // Store the read values into the copy without checking and updating the CRC.
                  // The valid flag will be set to true if the element was read
                  c_Return = this->m_SaveDumpValuesToListValues(c_Values, rc_List);

                  if (c_Return == Errc::rd_wr)
                  {
                     // Remap the error
                     c_Return = Errc::range;
                  }
               }
            }
            else
            {
               c_Return = Errc::range;
            }

            if (c_Return)
            {
               break;
            }
         }
      }
   }
   if (!c_Return)
   {
      orpc_NodeCopy = &this->mc_NodeCopy;
   }
   else
   {
      orpc_NodeCopy = nullptr;
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the CRCs of changed lists and writes them to the ECU

   The function calculates the CRCs over all datapool lists read by the preceding call to
   "NvmSafeReadValues" and writes them to the ECU NVM.

   \param[out]   opu8_NrCode   if != NULL: negative response code in case of an error response

   \return
   Errc::success  CRCs written successfully
   Errc::config   No node or diagnostic protocol are known (was this class properly Initialized ?)
                  protocol driver reported configuration error (was the protocol driver properly initialized ?)
   Errc::checksum At least one datapool of type "NVM" has the flag q_IsSafety set to true and
                  at least one of its lists has the flag q_NvmCrcActive set to false.
   Errc::range    The size of at least one list of NVM datapools of the copy of C_OscNode differs to the original instance.
                  The count of lists of NVM datapools of the copy of C_OscNode differs to the original instance.
                  The count of the datapools of the copy of C_OscNode differs to the original instance.
   Errc::busy     No list contains datapool elements that were
                  written by the preceding call to "NvmSafeWriteChangedValues"
   Errc::overflow At least one changed list has no elements.
   Errc::default_ At least one element of the changed lists has the flag q_IsValid set to false.
   Errc::timeout  Expected server response not received within timeout
   Errc::noact    Could not send request (e.g. Tx buffer full)
   Errc::warn     Server sent error response
   Errc::rd_wr    unexpected content in server response
   Errc::com      expected server response not received because of communication error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvmSafe::NvmSafeWriteCrcs(uint8_t * const opu8_NrCode)
{
   std::error_code c_Return = Errc::success;

   if ((this->mpc_Node == nullptr) || (this->mpc_DiagProtocol == nullptr))
   {
      c_Return = Errc::config;
   }
   else
   {
      uint32_t u32_DataPoolCounter;
      uint32_t u32_ListCounter;
      std::set<C_OscNodeDataPoolListId>::const_iterator c_ItChangedList;

      c_Return = Errc::success;

      // Check all common preconditions
      if (this->mc_NodeCopy.c_DataPools.size() == this->mpc_Node->c_DataPools.size())
      {
         // Check the datapool configuartion
         for (u32_DataPoolCounter = 0U;
              u32_DataPoolCounter < this->mc_NodeCopy.c_DataPools.size();
              ++u32_DataPoolCounter)
         {
            if (this->mc_NodeCopy.c_DataPools[u32_DataPoolCounter].c_Lists.size() ==
                this->mpc_Node->c_DataPools[u32_DataPoolCounter].c_Lists.size())
            {
               // Check the list configuration
               for (u32_ListCounter = 0U;
                    u32_ListCounter < this->mc_NodeCopy.c_DataPools[u32_DataPoolCounter].c_Lists.size();
                    ++u32_ListCounter)
               {
                  if (this->mc_NodeCopy.c_DataPools[u32_DataPoolCounter].c_Lists[u32_ListCounter].u32_NvmSize !=
                      this->mpc_Node->c_DataPools[u32_DataPoolCounter].c_Lists[u32_ListCounter].u32_NvmSize)
                  {
                     // NVM size of list different
                     c_Return = Errc::range;
                     break;
                  }
               }
            }
            else
            {
               c_Return = Errc::range;
            }

            if (c_Return)
            {
               break;
            }
         }
      }
      else
      {
         c_Return = Errc::range;
      }

      // Check pre conditions of changed lists
      if (!c_Return)
      {
         // Read all changed lists
         for (c_ItChangedList = this->mc_ChangedLists.begin();
              c_ItChangedList != this->mc_ChangedLists.end();
              ++c_ItChangedList)
         {
            u32_DataPoolCounter = (*c_ItChangedList).u32_DataPoolIndex;
            u32_ListCounter = (*c_ItChangedList).u32_ListIndex;

            if ((u32_DataPoolCounter < this->mc_NodeCopy.c_DataPools.size()) &&
                (u32_ListCounter < this->mc_NodeCopy.c_DataPools[(*c_ItChangedList).u32_DataPoolIndex].c_Lists.size()))
            {
               if (this->mc_NodeCopy.c_DataPools[u32_DataPoolCounter].c_Lists[u32_ListCounter].c_Elements.size() == 0)
               {
                  // No elements in list
                  c_Return = Errc::overflow;
               }
               else if ((this->mc_NodeCopy.c_DataPools[u32_DataPoolCounter].q_IsSafety == true) &&
                        (this->mc_NodeCopy.c_DataPools[u32_DataPoolCounter].c_Lists[u32_ListCounter].q_NvmCrcActive
                         ==
                         false))
               {
                  // Checksum is deactivated in a safety datapool
                  c_Return = Errc::checksum;
               }
               else
               {
                  uint32_t u32_ElementCounter;

                  for (u32_ElementCounter = 0U;
                       u32_ElementCounter <
                       this->mc_NodeCopy.c_DataPools[u32_DataPoolCounter].c_Lists[u32_ListCounter].c_Elements.size();
                       ++u32_ElementCounter)
                  {
                     if (this->mc_NodeCopy.c_DataPools[u32_DataPoolCounter].c_Lists[u32_ListCounter].c_Elements[
                            u32_ElementCounter].q_NvmValueIsValid == false)
                     {
                        // Element was not read from ECU
                        c_Return = Errc::busy;
                        break;
                     }
                  }
               }
            }
            else
            {
               c_Return = Errc::range;
            }

            if (c_Return)
            {
               break;
            }
         }
      }

      // Writing of CRC to ECU
      if (!c_Return)
      {
         c_Return = Errc::busy;

         for (c_ItChangedList = this->mc_ChangedLists.begin();
              c_ItChangedList != this->mc_ChangedLists.end();
              ++c_ItChangedList)
         {
            C_OscNodeDataPoolList & rc_List =
               this->mc_NodeCopy.c_DataPools[(*c_ItChangedList).u32_DataPoolIndex].c_Lists[(*c_ItChangedList).
                                                                                           u32_ListIndex];

            if (rc_List.q_NvmCrcActive == true)
            {
               std::vector<uint8_t> c_CrcData;

               // Calc the CRC
               rc_List.u32_NvmCrc = this->NvmCalcCrc(rc_List);

               if (this->mpc_DiagProtocol->GetEndianness() == C_OscDiagProtocolBase::mhu8_ENDIANNESS_BIG)
               {
                  rc_List.GetCrcAsBigEndianBlob(c_CrcData);
               }
               else
               {
                  rc_List.GetCrcAsLittleEndianBlob(c_CrcData);
               }

               // Adapt errorcode
               c_Return = C_OscDataDealerNvm::mh_AdaptProtocolReturnValue(
                  this->mpc_DiagProtocol->NvmWrite(rc_List.u32_NvmStartAddress, c_CrcData, opu8_NrCode));

               if (c_Return)
               {
                  break;
               }
            }
            else
            {
               // No CRC active
               c_Return = Errc::success;
            }
         }
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Clear internally stored content
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDataDealerNvmSafe::NvmSafeClearInternalContent(void)
{
   //Update state
   this->me_ParameterSetFileState = C_OscDataDealerNvmSafe::ePSFS_DATA_RESET;
   this->mc_ImageFileHandler.ClearContent();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create parameter set file based on current ECU values

   Note: Not set CRCs are supported

   \param[in]   orc_ListIds   List IDs
   \param[out]  opu8_NrCode   if != NULL: negative response code in case of an error response

   \return
   Errc::success  Data prepared for file
   Errc::overflow Wrong sequence of function calls
   Errc::range    Data pool list IDs invalid
   Errc::checksum CRC over the values of a parameter list read from the ECU does not match those values
   Errc::config   No valid diagnostic protocol is set
                  or no valid pointer to the original instance of "C_OscNode" is set in "C_OscDataDealer"
   Errc::noact    Server communication protocol service could not be requested
   Errc::timeout  Server communication protocol service has timed out
   Errc::warn     Server communication protocol service error response was received
   Errc::com      expected server response not received because of communication error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvmSafe::NvmSafeReadParameterValues(
   const std::vector<C_OscNodeDataPoolListId> & orc_ListIds, uint8_t * const opu8_NrCode)
{
   std::error_code c_Retval = Errc::success;

   //Reset state
   this->me_CreateParameterSetWorkflowState = C_OscDataDealerNvmSafe::eCPSFS_IDLE;
   if (C_OscDataDealerNvmSafe::me_ParameterSetFileState == C_OscDataDealerNvmSafe::ePSFS_DATA_RESET)
   {
      if (orc_ListIds.size() > 0)
      {
         if ((this->mpc_Node != nullptr) && (this->mpc_DiagProtocol != nullptr))
         {
            C_OscParamSetRawNode c_RawNode;
            c_RawNode.c_Entries.reserve(orc_ListIds.size() * 2);
            //Add invalid CRC entries
            for (uint32_t u32_ItList = 0; (u32_ItList < orc_ListIds.size()) && (!c_Retval); ++u32_ItList)
            {
               const C_OscNodeDataPoolListId & rc_DataPoolListId = orc_ListIds[u32_ItList];
               if ((rc_DataPoolListId.u32_NodeIndex == this->mu32_NodeIndex) &&
                   (rc_DataPoolListId.u32_DataPoolIndex < this->mpc_Node->c_DataPools.size()))
               {
                  C_OscNodeDataPool & rc_DataPool =
                     this->mpc_Node->c_DataPools[rc_DataPoolListId.u32_DataPoolIndex];
                  if (rc_DataPoolListId.u32_ListIndex < rc_DataPool.c_Lists.size())
                  {
                     const C_OscNodeDataPoolList & rc_List = rc_DataPool.c_Lists[rc_DataPoolListId.u32_ListIndex];
                     if (rc_List.q_NvmCrcActive == true)
                     {
                        C_OscParamSetRawEntry c_RawEntry;
                        c_RawEntry.u32_StartAddress = rc_List.u32_NvmStartAddress;
                        c_RawEntry.c_Bytes.clear();
                        c_RawEntry.c_Bytes.reserve(2);
                        c_RawEntry.c_Bytes.push_back(0);
                        c_RawEntry.c_Bytes.push_back(0);
                        c_RawNode.c_Entries.push_back(c_RawEntry);
                     }
                  }
                  else
                  {
                     c_Retval = Errc::range;
                  }
               }
               else
               {
                  c_Retval = Errc::range;
               }
            }
            if (!c_Retval)
            {
               //Read list values
               for (uint32_t u32_ItList = 0; (u32_ItList < orc_ListIds.size()) && (!c_Retval);
                    ++u32_ItList)
               {
                  const C_OscNodeDataPoolListId & rc_DataPoolListId = orc_ListIds[u32_ItList];
                  if ((rc_DataPoolListId.u32_NodeIndex == this->mu32_NodeIndex) &&
                      (rc_DataPoolListId.u32_DataPoolIndex < this->mpc_Node->c_DataPools.size()))
                  {
                     C_OscNodeDataPool & rc_DataPool =
                        this->mpc_Node->c_DataPools[rc_DataPoolListId.u32_DataPoolIndex];
                     if (rc_DataPoolListId.u32_ListIndex < rc_DataPool.c_Lists.size())
                     {
                        C_OscParamSetRawEntry c_RawEntry;
                        C_OscNodeDataPoolList & rc_List = rc_DataPool.c_Lists[rc_DataPoolListId.u32_ListIndex];
                        c_Retval = m_CreateRawEntryAndPrepareInterpretedData(rc_List, c_RawEntry, opu8_NrCode);
                        c_RawNode.c_Entries.push_back(c_RawEntry);
                     }
                     else
                     {
                        c_Retval = Errc::range;
                     }
                  }
                  else
                  {
                     c_Retval = Errc::range;
                  }
               }
               if (!c_Retval)
               {
                  std::vector<uint32_t> c_AlreadyUsedDataPoolIndices;
                  C_OscParamSetInterpretedNode c_InterpretedNode;
                  //Prepare data
                  //Node
                  c_RawNode.c_Name = this->mpc_Node->c_Properties.c_Name;
                  c_InterpretedNode.c_Name = this->mpc_Node->c_Properties.c_Name;
                  //Data pools
                  for (uint32_t u32_ItCurListId = 0; (u32_ItCurListId < orc_ListIds.size()) && (!c_Retval);
                       ++u32_ItCurListId)
                  {
                     const C_OscNodeDataPoolListId & rc_DataPoolListId = orc_ListIds[u32_ItCurListId];
                     if (rc_DataPoolListId.u32_DataPoolIndex < this->mpc_Node->c_DataPools.size())
                     {
                        C_OscNodeDataPool & rc_DataPool =
                           this->mpc_Node->c_DataPools[rc_DataPoolListId.u32_DataPoolIndex];
                        if (rc_DataPoolListId.u32_ListIndex < rc_DataPool.c_Lists.size())
                        {
                           C_OscNodeDataPoolList & rc_List = rc_DataPool.c_Lists[rc_DataPoolListId.u32_ListIndex];
                           bool q_DataPoolFound = false;
                           //Case 1: Data pool exists
                           //------------------------
                           tgl_assert(c_AlreadyUsedDataPoolIndices.size() == c_InterpretedNode.c_DataPools.size());
                           for (uint32_t u32_ItCurrentDataPool = 0;
                                (u32_ItCurrentDataPool < c_InterpretedNode.c_DataPools.size()) &&
                                (!c_Retval);
                                ++u32_ItCurrentDataPool)
                           {
                              if (c_AlreadyUsedDataPoolIndices[u32_ItCurrentDataPool] ==
                                  rc_DataPoolListId.u32_DataPoolIndex)
                              {
                                 bool q_ListFound = false;
                                 q_DataPoolFound = true;
                                 //Interpreted
                                 for (uint32_t u32_ItListId = 0; u32_ItListId < orc_ListIds.size(); ++u32_ItListId)
                                 {
                                    const C_OscNodeDataPoolListId & rc_ListId = orc_ListIds[u32_ItListId];
                                    //If not same index but otherwise same ID abort
                                    if ((u32_ItCurListId != u32_ItListId) &&
                                        ((rc_ListId.u32_DataPoolIndex == rc_DataPoolListId.u32_DataPoolIndex) &&
                                         (rc_ListId.u32_ListIndex ==
                                          rc_DataPoolListId.u32_ListIndex)))
                                    {
                                       q_ListFound = true;
                                    }
                                 }
                                 if (q_ListFound == false)
                                 {
                                    //List
                                    C_OscParamSetInterpretedDataPool & rc_InterpretedDataPool =
                                       c_InterpretedNode.c_DataPools[u32_ItCurrentDataPool];
                                    C_OscParamSetInterpretedList c_NewList;
                                    mh_CreateInterpretedList(rc_List, c_NewList);
                                    rc_InterpretedDataPool.c_Lists.push_back(c_NewList);
                                 }
                                 else
                                 {
                                    c_Retval = Errc::range;
                                 }
                              }
                           }
                           //Case 2: New data pool
                           //---------------------
                           if (q_DataPoolFound == false)
                           {
                              C_OscParamSetDataPoolInfo c_DataPoolInfo;
                              C_OscParamSetInterpretedDataPool c_NewInterpretedDataPool;
                              C_OscParamSetInterpretedList c_NewList;
                              //Info
                              //Data pool crc
                              c_DataPoolInfo.u32_DataPoolCrc = 0;
                              rc_DataPool.CalcGeneratedDefinitionHash(c_DataPoolInfo.u32_DataPoolCrc);
                              c_DataPoolInfo.c_Name = rc_DataPool.c_Name;
                              c_DataPoolInfo.u32_NvmSize = rc_DataPool.u32_NvmSize;
                              c_DataPoolInfo.u32_NvmStartAddress = rc_DataPool.u32_NvmStartAddress;
                              c_DataPoolInfo.au8_Version[0] = rc_DataPool.au8_Version[0];
                              c_DataPoolInfo.au8_Version[1] = rc_DataPool.au8_Version[1];
                              c_DataPoolInfo.au8_Version[2] = rc_DataPool.au8_Version[2];
                              //Raw
                              c_RawNode.c_DataPools.push_back(c_DataPoolInfo);
                              //Interpreted
                              c_NewInterpretedDataPool.c_DataPoolInfo = c_DataPoolInfo;
                              //List
                              mh_CreateInterpretedList(rc_List, c_NewList);
                              c_NewInterpretedDataPool.c_Lists.push_back(c_NewList);
                              c_InterpretedNode.c_DataPools.push_back(c_NewInterpretedDataPool);
                              //Update indices
                              c_AlreadyUsedDataPoolIndices.push_back(rc_DataPoolListId.u32_DataPoolIndex);
                           }
                        }
                        else
                        {
                           c_Retval = Errc::range;
                        }
                     }
                     else
                     {
                        c_Retval = Errc::range;
                     }
                  }
                  //Write data
                  if (!c_Retval)
                  {
                     if (!this->mc_ImageFileHandler.AddRawDataForNode(c_RawNode))
                     {
                        if (!this->mc_ImageFileHandler.AddInterpretedDataForNode(c_InterpretedNode))
                        {
                           //Finished
                           //Update state
                           this->me_CreateParameterSetWorkflowState = C_OscDataDealerNvmSafe::eCPSFS_FILE_CREATED;
                        }
                        else
                        {
                           c_Retval = Errc::range;
                        }
                     }
                     else
                     {
                        c_Retval = Errc::range;
                     }
                  }
               }
            }
         }
         else
         {
            c_Retval = Errc::config;
         }
      }
      else
      {
         c_Retval = Errc::range;
      }
   }
   else
   {
      c_Retval = Errc::overflow;
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create clean file from internally stored content without adding a CRC

   Note: this function handles a file step and there is only one parameter set file for each parametrization process,
         so this function needs to only be called once,
         so all participating data dealers can continue with the next step (if there is any)

   \param[in] orc_Path     Parameter set file path
   \param[in] orc_FileInfo Optional general file information

   \return
   Errc::success  data saved
   Errc::range    file already exists
   Errc::overflow Wrong sequence of function calls
   Errc::busy     file already exists
   Errc::rd_wr    could not write to file (e.g. missing write permissions; missing folder)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvmSafe::NvmSafeCreateCleanFileWithoutCrc(
   const std::string & orc_Path, const C_OscParamSetInterpretedFileInfoData & orc_FileInfo)
{
   std::error_code c_Retval = Errc::success;

   if (TglFileExists(orc_Path) == false)
   {
      if (this->me_ParameterSetFileState == C_OscDataDealerNvmSafe::ePSFS_DATA_RESET)
      {
         this->mc_ImageFileHandler.AddInterpretedFileData(orc_FileInfo);
         c_Retval = this->mc_ImageFileHandler.CreateCleanFileWithoutCrc(orc_Path);
         if (!c_Retval)
         {
            //Update internal variable
            this->mc_ParameterSetFilePath = orc_Path;
            //Update state
            this->me_ParameterSetFileState = C_OscDataDealerNvmSafe::ePSFS_FILE_CREATED;
         }
      }
      else
      {
         c_Retval = Errc::overflow;
      }
   }
   else
   {
      c_Retval = Errc::busy;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read file and update internally stored content (cleared at start)

   Warning: CRC is not checked

   Note: this function handles a file step and there is only one parameter set file for each parametrization process,
         so this function needs to only be called once,
         so all participating data dealers can continue with the next step (if there is any)

   \param[in] orc_Path   Parameter file path

   \return
   Errc::success  data read
   Errc::overflow Wrong sequence of function calls
   Errc::range    Path does not match the path of the preceding function calls
   Errc::rd_wr    specified file does not exist
                  specified file is present but structure is invalid (e.g. invalid XML file)
   Errc::config   file does not contain essential information
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvmSafe::NvmSafeReadFileWithoutCrc(const std::string & orc_Path)
{
   std::error_code c_Retval = Errc::success;

   if (this->me_ParameterSetFileState == C_OscDataDealerNvmSafe::ePSFS_FILE_CREATED)
   {
      if (this->mc_ParameterSetFilePath == orc_Path)
      {
         c_Retval = this->mc_ImageFileHandler.ReadFile(orc_Path, true);
         if (!c_Retval)
         {
            //Update state
            this->me_ParameterSetFileState = C_OscDataDealerNvmSafe::ePSFS_FILE_READ_WITHOUT_CRC;
         }
      }
      else
      {
         c_Retval = Errc::range;
      }
   }
   else
   {
      c_Retval = Errc::overflow;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   The function reads the contents of a parameter set file without checking the file CRC

   \param[in]  orc_Path          File path
   \param[out] orc_DataPoolLists Loaded data pool lists (Always cleared at start)

   \return
   Errc::success  Lists valid
   Errc::overflow Wrong sequence of function calls
   Errc::range    Path does not match the path of the preceding function calls
   Errc::config   Mismatch of data with current node
                  or no valid pointer to the original instance of "C_OscNode" is set in "C_OscDataDealer"
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvmSafe::NvmSafeCheckParameterFileContents(
   const std::string & orc_Path, std::vector<C_OscNodeDataPoolListId> & orc_DataPoolLists)
{
   std::error_code c_Retval = Errc::success;

   orc_DataPoolLists.clear();
   if (this->me_CreateParameterSetWorkflowState == C_OscDataDealerNvmSafe::eCPSFS_FILE_CREATED)
   {
      if (C_OscDataDealerNvmSafe::mc_ParameterSetFilePath == orc_Path)
      {
         if (this->mpc_Node != nullptr)
         {
            const C_OscParamSetRawNode * const pc_Node = this->mc_ImageFileHandler.GetRawDataForNode(
               this->mpc_Node->c_Properties.c_Name);
            if (pc_Node != nullptr)
            {
               if (!m_CheckParameterFileContent(*pc_Node))
               {
                  //For each raw data pool
                  //For each raw entry
                  for (uint32_t u32_ItRawEntry =
                          0; (u32_ItRawEntry < pc_Node->c_Entries.size()) && (!c_Retval);
                       ++u32_ItRawEntry)
                  {
                     bool q_Found = false;
                     const C_OscParamSetRawEntry & rc_CurRawEntry = pc_Node->c_Entries[u32_ItRawEntry];
                     //Find data pool list
                     for (uint32_t u32_ItDataPool = 0;
                          (u32_ItDataPool < this->mpc_Node->c_DataPools.size()) && (!c_Retval);
                          ++u32_ItDataPool)
                     {
                        const C_OscNodeDataPool & rc_DataPool = this->mpc_Node->c_DataPools[u32_ItDataPool];
                        //Check data pool address range and for NVM datapool
                        if (((rc_DataPool.e_Type == C_OscNodeDataPool::eNVM) ||
                             (rc_DataPool.e_Type == C_OscNodeDataPool::eHALC_NVM)) &&
                            (rc_DataPool.u32_NvmStartAddress <= rc_CurRawEntry.u32_StartAddress) &&
                            ((rc_DataPool.u32_NvmStartAddress + rc_DataPool.u32_NvmSize) >
                             rc_CurRawEntry.u32_StartAddress))
                        {
                           for (uint32_t u32_ItList = 0;
                                (u32_ItList < rc_DataPool.c_Lists.size()) && (!c_Retval); ++u32_ItList)
                           {
                              const C_OscNodeDataPoolList & rc_List = rc_DataPool.c_Lists[u32_ItList];
                              //Check list address range
                              if ((rc_List.u32_NvmStartAddress <= rc_CurRawEntry.u32_StartAddress) &&
                                  ((rc_List.u32_NvmStartAddress + rc_List.u32_NvmSize) >
                                   rc_CurRawEntry.u32_StartAddress))
                              {
                                 if ((rc_CurRawEntry.c_Bytes.size() == rc_List.GetNumBytesUsed()) ||
                                     (rc_CurRawEntry.c_Bytes.size() == 2))
                                 {
                                    //Data pool list found
                                    const C_OscNodeDataPoolListId c_Value(this->mu32_NodeIndex, u32_ItDataPool,
                                                                          u32_ItList);
                                    q_Found = true;
                                    orc_DataPoolLists.push_back(c_Value);
                                 }
                                 else
                                 {
                                    c_Retval = Errc::config;
                                 }
                              }
                           }
                        }
                     }
                     if (q_Found == false)
                     {
                        c_Retval = Errc::config;
                     }
                  }
                  if (!c_Retval)
                  {
                     //Remove duplicates
                     const std::vector<C_OscNodeDataPoolListId> c_Copy = orc_DataPoolLists;
                     orc_DataPoolLists.clear();
                     orc_DataPoolLists.reserve(c_Copy.size());
                     for (uint32_t u32_ItCopy = 0; u32_ItCopy < c_Copy.size(); ++u32_ItCopy)
                     {
                        bool q_Found = false;
                        const C_OscNodeDataPoolListId & rc_CopyId = c_Copy[u32_ItCopy];
                        //Search if already appended
                        for (uint32_t u32_ItExisting = 0; u32_ItExisting < orc_DataPoolLists.size(); ++u32_ItExisting)
                        {
                           const C_OscNodeDataPoolListId & rc_ExistingId = orc_DataPoolLists[u32_ItExisting];
                           if (((rc_ExistingId.u32_NodeIndex == rc_CopyId.u32_NodeIndex) &&
                                (rc_ExistingId.u32_DataPoolIndex == rc_CopyId.u32_DataPoolIndex)) &&
                               (rc_ExistingId.u32_ListIndex == rc_CopyId.u32_ListIndex))
                           {
                              q_Found = true;
                           }
                        }
                        if (q_Found == false)
                        {
                           orc_DataPoolLists.push_back(rc_CopyId);
                        }
                     }
                     //As not every list might require a CRC the new list size cannot be exactly matched
                     if ((orc_DataPoolLists.size() >= (c_Copy.size() / 2)) &&
                         (orc_DataPoolLists.size() <= c_Copy.size()))
                     {
                        //Finished
                        this->me_CreateParameterSetWorkflowState = C_OscDataDealerNvmSafe::eCPSFS_FILE_CHECKED;
                     }
                     else
                     {
                        c_Retval = Errc::config;
                     }
                  }
               }
               else
               {
                  c_Retval = Errc::config;
               }
            }
            else
            {
               c_Retval = Errc::config;
            }
         }
         else
         {
            c_Retval = Errc::config;
         }
      }
      else
      {
         c_Retval = Errc::range;
      }
   }
   else
   {
      c_Retval = Errc::overflow;
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update/add CRC for provided file

   Note: this function handles a file step and there is only one parameter set file for each parametrization process,
         so this function needs to only be called once,
         so all participating data dealers can continue with the next step (if there is any)

   \param[in] orc_Path   Parameter set file path

   \return
   Errc::success  CRC updated
   Errc::overflow Wrong sequence of function calls
   Errc::range    Path does not match the path of the preceding function calls
   Errc::rd_wr    specified file does not exist
                  specified file is present but structure is invalid (e.g. invalid XML file)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvmSafe::NvmSafeUpdateCrcForFile(const std::string & orc_Path)
{
   std::error_code c_Retval = Errc::success;

   if (this->me_ParameterSetFileState == C_OscDataDealerNvmSafe::ePSFS_FILE_READ_WITHOUT_CRC)
   {
      if (this->mc_ParameterSetFilePath == orc_Path)
      {
         c_Retval = C_OscParamSetHandler::h_UpdateCrcForFile(orc_Path);
         if (!c_Retval)
         {
            //Update state
            this->me_ParameterSetFileState = C_OscDataDealerNvmSafe::ePSFS_IDLE;
         }
         else
         {
            c_Retval = Errc::rd_wr;
         }
      }
      else
      {
         c_Retval = Errc::range;
      }
   }
   else
   {
      c_Retval = Errc::overflow;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read file and update internally stored content (cleared at start)

   After loading this functions checks whether the file contains data for exactly one node.
   Otherwise it will fail.

   \param[in] orc_Path   Parameter set file path

   \return
   Errc::success  data read
   Errc::rd_wr    specified file does not exist
                  specified file is present but structure is invalid (e.g. invalid XML file)
   Errc::config   file does not contain data for exactly one node (zero or more than one)
   Errc::checksum specified file is present but checksum is invalid
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvmSafe::NvmSafeReadFileWithCrc(const std::string & orc_Path)
{
   std::error_code c_Retval = this->mc_ImageFileHandler.ReadFile(orc_Path, false);

   if (!c_Retval)
   {
      //data for one file contained ?
      if (this->mc_ImageFileHandler.GetNumberOfNodes() != 1U)
      {
         std::string c_Error;
         c_Error = PrintFormattedCompat(
            "File \"%s\"  Expected: contains parameters for one device  Found: contains parameters for %u devices\n",
            orc_Path.c_str(), this->mc_ImageFileHandler.GetNumberOfNodes());
         this->mc_ImageFileHandler.ClearContent();
         c_Retval = Errc::config;
         osc_write_log_error("Loading parameter set file", c_Error);
      }
      else
      {
         //Update state
         this->me_ParameterSetFileState = C_OscDataDealerNvmSafe::ePSFS_FILE_READ_WITH_CRC;
         this->mc_ParameterSetFilePath = orc_Path;
      }
   }
   else if (c_Retval == Errc::checksum)
   {
      //Same return value
   }
   else
   {
      c_Retval = Errc::rd_wr;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read a parameter set file and writes the contained values and CRCs to the ECU's NVM

   Sequence:
   * read parameter set file
   * write to NVM

   \param[in]  orc_Path           File path
   \param[out] ors32_ResultDetail Result detail

   \return
   Return            Error Detail
   Errc::success     1            File successfully written to ECU
   Errc::overflow    5            Wrong sequence of function calls
                     6            Path mismatch with previous function call
   Errc::config      1            No valid diagnostic protocol is set in "C_OscDataDealer"
                     2            No valid pointer to the original instance of "C_OscNode" is set in "C_OscDataDealer"
   Errc::noact       1            Communication protocol service could not be requested
   Errc::timeout     1            Communication protocol service has timed out
   Errc::warn        1            Communication protocol service error response was received
   Errc::unknown_err <undefined>  Communication protocol failed with non-specified error code
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvmSafe::NvmSafeWriteParameterSetFile(const std::string & orc_Path,
                                                                     int32_t & ors32_ResultDetail)
{
   std::error_code c_Retval = Errc::success;

   ors32_ResultDetail = -1;
   if (this->mpc_Node != nullptr)
   {
      if (this->mpc_DiagProtocol != nullptr)
      {
         if (C_OscDataDealerNvmSafe::me_ParameterSetFileState == C_OscDataDealerNvmSafe::ePSFS_FILE_READ_WITH_CRC)
         {
            if (C_OscDataDealerNvmSafe::mc_ParameterSetFilePath == orc_Path)
            {
               //when we get here we can be sure there was data for exactly one node contained in the file
               // as this is checked by NvmSafeReadFileWithCRC
               const C_OscParamSetRawNode * const pc_Node = this->mc_ImageFileHandler.GetRawDataForNode(0U);
               tgl_assert(pc_Node != nullptr);
               if (pc_Node != nullptr)
               {
                  //write the actual values:
                  for (uint32_t u32_ItEntry = 0; u32_ItEntry < pc_Node->c_Entries.size(); u32_ItEntry++)
                  {
                     const C_OscParamSetRawEntry & rc_Entry = pc_Node->c_Entries[u32_ItEntry];
                     const std::error_code c_ProtReturn =
                        this->mpc_DiagProtocol->NvmWrite(rc_Entry.u32_StartAddress, rc_Entry.c_Bytes, nullptr);
                     //Map error codes
                     if (c_ProtReturn == Errc::timeout)
                     {
                        c_Retval = Errc::timeout;
                        ors32_ResultDetail = 1;
                     }
                     else if ((c_ProtReturn == Errc::noact) || (c_ProtReturn == Errc::config))
                     {
                        c_Retval = Errc::noact;
                        ors32_ResultDetail = 1;
                     }
                     else if ((c_ProtReturn == Errc::range) || (c_ProtReturn == Errc::rd_wr) ||
                              (c_ProtReturn == Errc::warn))
                     {
                        c_Retval = Errc::warn;
                        ors32_ResultDetail = 1;
                     }
                     else if (c_ProtReturn == Errc::success) //positive result
                     {
                        c_Retval = Errc::success;
                        ors32_ResultDetail = 1;
                     }
                     else
                     {
                        //Not documented error was returned by function
                        c_Retval = Errc::unknown_err;
                        osc_write_log_info("Parametrization", "Not documented error code " +
                                           std::to_string(c_ProtReturn.value()) + " was returned by NvmWrite");
                     }
                     if (c_Retval)
                     {
                        break;
                     }
                  }
               }
            }
            else
            {
               c_Retval = Errc::overflow;
               ors32_ResultDetail = 6;
            }
         }
         else
         {
            c_Retval = Errc::overflow;
            ors32_ResultDetail = 5;
         }
      }
      else
      {
         c_Retval = Errc::config;
         ors32_ResultDetail = 1;
      }
   }
   else
   {
      c_Retval = Errc::config;
      ors32_ResultDetail = 2;
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create interpreted list

   \param[in]  orc_List            List data
   \param[out] orc_InterpretedList New interpreted list
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDataDealerNvmSafe::mh_CreateInterpretedList(const C_OscNodeDataPoolList & orc_List,
                                                      C_OscParamSetInterpretedList & orc_InterpretedList)
{
   orc_InterpretedList.c_Name = orc_List.c_Name;
   orc_InterpretedList.c_Elements.reserve(orc_List.c_Elements.size());
   for (uint32_t u32_ItElement = 0; u32_ItElement < orc_List.c_Elements.size(); ++u32_ItElement)
   {
      C_OscParamSetInterpretedElement c_NewElement;
      const C_OscNodeDataPoolListElement & rc_Element = orc_List.c_Elements[u32_ItElement];
      c_NewElement.c_Name = rc_Element.c_Name;
      c_NewElement.c_NvmValue = rc_Element.c_NvmValue;
      orc_InterpretedList.c_Elements.push_back(c_NewElement);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if raw node parameters match internal node

   \param[in]  orc_Node    Raw node parameters to check

   \return
   Errc::success  Match
   Errc::config   Node not set
   Errc::range    Node mismatch
   Errc::timeout  Data pool not found
   Errc::checksum Data pool CRC mismatch
   Errc::rd_wr    Data pool version mismatch
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvmSafe::m_CheckParameterFileContent(const C_OscParamSetRawNode & orc_Node) const
{
   std::error_code c_Retval = Errc::success;

   if (this->mpc_Node != nullptr)
   {
      if ((this->mpc_Node->c_Properties.c_Name == orc_Node.c_Name) && (orc_Node.c_DataPools.size() > 0))
      {
         //Check all data pool info segments
         for (uint32_t u32_ItReadDataPool = 0U;
              (u32_ItReadDataPool < orc_Node.c_DataPools.size()) && (!c_Retval);
              ++u32_ItReadDataPool)
         {
            const C_OscParamSetDataPoolInfo & rc_DataPoolInfo = orc_Node.c_DataPools[u32_ItReadDataPool];
            bool q_Found = false;
            //Find matching data pool
            for (uint32_t u32_ItNodeDataPool = 0U;
                 (u32_ItNodeDataPool < this->mpc_Node->c_DataPools.size()) && (!c_Retval);
                 ++u32_ItNodeDataPool)
            {
               const C_OscNodeDataPool & rc_NodeDataPool = this->mpc_Node->c_DataPools[u32_ItNodeDataPool];
               if (rc_NodeDataPool.c_Name == rc_DataPoolInfo.c_Name)
               {
                  uint32_t u32_Crc = 0U;
                  rc_NodeDataPool.CalcGeneratedDefinitionHash(u32_Crc);
                  q_Found = true;
                  //Check content
                  if (rc_DataPoolInfo.u32_DataPoolCrc != u32_Crc)
                  {
                     c_Retval = Errc::checksum;
                  }
                  else if ((((rc_DataPoolInfo.au8_Version[0] != rc_NodeDataPool.au8_Version[0])) ||
                            (rc_DataPoolInfo.au8_Version[1] != rc_NodeDataPool.au8_Version[1])) ||
                           (rc_DataPoolInfo.au8_Version[2] != rc_NodeDataPool.au8_Version[2]))
                  {
                     c_Retval = Errc::rd_wr;
                  }
                  else
                  {
                     //Match
                  }
               }
            }
            if (q_Found == false)
            {
               c_Retval = Errc::timeout;
            }
         }
      }
      else
      {
         c_Retval = Errc::range;
      }
   }
   else
   {
      c_Retval = Errc::config;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create raw entry for list and write bytes to interpreted data

   \param[in]  orc_List    List to set raw entry from
   \param[out] orc_Entry   Raw entry from list
   \param[out] opu8_NrCode if != NULL: negative response code in case of an error response

   \return
   Errc::success  Entry successfully created
   Errc::range    List has no elements. Nothing to read.
                  Datapool element size configuration does not match with count of read bytes
   Errc::timeout  Expected response not received within timeout
   Errc::noact    Could not send request (e.g. Tx buffer full)
   Errc::warn     Error response or malformed protocol response
   Errc::config   Pre-requisites not correct; e.g. driver not initialized or
                  parameter out of range (checked by client side)
   Errc::rd_wr    Datapool element size configuration does not match with count of read bytes
   Errc::checksum Checksum of read datapool list is invalid
   Errc::com      expected server response not received because of communication error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDataDealerNvmSafe::m_CreateRawEntryAndPrepareInterpretedData(C_OscNodeDataPoolList & orc_List,
                                                                                  C_OscParamSetRawEntry & orc_Entry,
                                                                                  uint8_t * const opu8_NrCode)
{
   std::vector<uint8_t> c_Values;
   std::error_code c_Retval = this->m_NvmReadListRaw(orc_List, c_Values, opu8_NrCode);
   if (!c_Retval)
   {
      //Raw
      orc_Entry.u32_StartAddress = orc_List.u32_NvmStartAddress;
      orc_Entry.c_Bytes = c_Values;
      c_Retval = this->m_SaveDumpToList(c_Values, orc_List);
      if (c_Retval)
      {
         //Translate error values if necessary
         if (c_Retval == Errc::rd_wr)
         {
            c_Retval = Errc::range;
         }
         else if (c_Retval == Errc::checksum)
         {
            c_Retval = Errc::checksum;
         }
         else
         {
            //Not documented error was returned by function
            c_Retval = Errc::unknown_err;
            osc_write_log_info("parametrization",
                               "Not documented error was returned by m_InterpretDumbToList");
         }
      }
   }
   else
   {
      //Translate error values if necessary
      if (((c_Retval == Errc::timeout) || (c_Retval == Errc::warn)) || (c_Retval == Errc::noact))
      {
         //Same error value
      }
      else if (c_Retval == Errc::overflow)
      {
         c_Retval = Errc::range;
      }
      else
      {
         //Not documented error was returned by function
         c_Retval = Errc::unknown_err;
         osc_write_log_info("parametrization",
                            "Not documented error was returned by m_NvmReadListRaw");
      }
   }
   return c_Retval;
}
