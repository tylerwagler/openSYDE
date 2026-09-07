//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Diagnostic protocol driver for openSYDE protocol

   For details cf. documentation in .h file.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <system_error>
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscDiagProtocolOsy.hpp"
#include "TglTime.hpp"
#include "TglUtils.hpp"
#include <string>

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
/*! \brief   Handle notification about incoming response to event driven data pool reading

   Translates the openSYDE protocol specific notification to a generic notification.
   Invokes the notification function of C_OscDiagProtocolBase to let the application know we have new data.

   \param[in]     ou8_DataPoolIndex    Datapool index
   \param[in]     ou16_ListIndex       List index
   \param[in]     ou16_ElementIndex    Element index
   \param[in]     orc_Value            Value of element stored in uint8 vector
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDiagProtocolOsy::m_OsyReadDataPoolDataEventReceived(const uint8_t ou8_DataPoolIndex,
                                                              const uint16_t ou16_ListIndex,
                                                              const uint16_t ou16_ElementIndex,
                                                              const std::vector<uint8_t> & orc_Value)
{
   if (mpr_OnDataPoolReadEventReceived != nullptr)
   {
      mpr_OnDataPoolReadEventReceived(mpv_OnAsyncInstance, ou8_DataPoolIndex, ou16_ListIndex, ou16_ElementIndex,
                                      orc_Value);
   }
   else
   {
      m_LogErrorWithHeader("Asynchronous communication",
                           "Async ReadDataPool response received, but no handler is installed. Ignoring.",
                           TGL_UTIL_FUNC_ID);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle notification about incoming negative response to event driven data pool reading

   Translates the openSYDE protocol specific notification to a generic notification.
   Invokes the notification function of C_OscDiagProtocolBase to let the application know we have new data.

   \param[in]  ou8_DataPoolIndex    Datapool index
   \param[in]  ou16_ListIndex       List index
   \param[in]  ou16_ElementIndex    Element index
   \param[in]  ou8_NrCode           Negative response code
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDiagProtocolOsy::m_OsyReadDataPoolDataEventErrorReceived(const uint8_t ou8_DataPoolIndex,
                                                                   const uint16_t ou16_ListIndex,
                                                                   const uint16_t ou16_ElementIndex,
                                                                   const uint8_t ou8_NrCode)
{
   if (mpr_OnDataPoolReadEventErrorReceived != nullptr)
   {
      mpr_OnDataPoolReadEventErrorReceived(mpv_OnAsyncInstance, ou8_DataPoolIndex, ou16_ListIndex, ou16_ElementIndex,
                                           ou8_NrCode);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   constructor

   Set up class
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscDiagProtocolOsy::C_OscDiagProtocolOsy(void) :
   C_OscDiagProtocolBase(),
   C_OscProtocolDriverOsy()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   constructor

   Tear down class
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscDiagProtocolOsy::~C_OscDiagProtocolOsy(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get endianness of data returned by and passed to protocol services

   Here: return "big"

   \return
   mhu8_ENDIANNESS_BIG
*/
//----------------------------------------------------------------------------------------------------------------------
uint8_t C_OscDiagProtocolOsy::GetEndianness(void) const
{
   return mhu8_ENDIANNESS_BIG;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Perform cyclic communication tasks

   Invoke protocol's "Cycle" function.

   \return
   Errc::success   at least one service received
   Errc::config    transport protocol not installed
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::Cycle(void)
{
   return C_OscProtocolDriverOsy::Cycle();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read numeric data from server's data pool

   Send request and wait for response.
   In the openSYDE protocol the implementation is the same for array and numeric data.

   \param[in]  ou8_DataPoolIndex   index of data pool to read from
   \param[in]  ou16_ListIndex      list index to read from
   \param[in]  ou16_ElementIndex   element index to read
   \param[in,out] orc_ReadData     in: size of data to read
                                   out: read data
   \param[out]    opu8_NrCode      if != NULL: negative response code in case of an error response

   \return
   Errc::success   request sent, positive response with expected size received
   Errc::range     data pool, list, element index out of range (checked by client-side function)
   Errc::noact     could not send request (Tx buffer full)
   Errc::config    transport protocol not installed
   Errc::warn      error response
   Errc::rd_wr     malformed protocol response
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::DataPoolReadNumeric(const uint8_t ou8_DataPoolIndex,
                                                          const uint16_t ou16_ListIndex,
                                                          const uint16_t ou16_ElementIndex,
                                                          std::vector<uint8_t> & orc_ReadData,
                                                          uint8_t * const opu8_NrCode)
{
   //in this protocol: no need to differentiate between numeric and array ...
   return this->DataPoolReadArray(ou8_DataPoolIndex, ou16_ListIndex, ou16_ElementIndex, orc_ReadData, opu8_NrCode);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read array data from server's data pool

   Send request and wait for response.
   In the openSYDE protocol the implementation is the same for array and numeric data.

   \param[in]     ou8_DataPoolIndex   index of data pool to read from
   \param[in]     ou16_ListIndex      list index to read from
   \param[in]     ou16_ElementIndex   element index to read
   \param[in,out] orc_ReadData        in: size of data to read
                                      out: read data
   \param[out]    opu8_NrCode         if != NULL: negative response code in case of an error response

   \return
   Errc::success   request sent, positive response with expected size received
   Errc::range     data pool, list, element index out of range (checked by client-side function)
   Errc::noact     could not send request (Tx buffer full)
   Errc::config    transport protocol not installed
   Errc::warn      error response
   Errc::rd_wr     malformed protocol response (e.g. reported size does not match expected size)
   Errc::com       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::DataPoolReadArray(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                                        const uint16_t ou16_ElementIndex,
                                                        std::vector<uint8_t> & orc_ReadData,
                                                        uint8_t * const opu8_NrCode)
{
   std::error_code c_Return = Errc::success;

   const uint32_t u32_ExpectedSize = static_cast<uint32_t>(orc_ReadData.size());

   c_Return = this->OsyReadDataPoolData(ou8_DataPoolIndex, ou16_ListIndex, ou16_ElementIndex, orc_ReadData,
                                        opu8_NrCode);
   if (orc_ReadData.size() != u32_ExpectedSize)
   {
      c_Return = Errc::rd_wr;
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write numeric data to server's data pool

   Send request and wait for response.

   \param[in]  ou8_DataPoolIndex   index of data pool to read from
   \param[in]  ou16_ListIndex      list index to read from
   \param[in]  ou16_ElementIndex   element index to read
   \param[out] orc_DataToWrite     data to write
   \param[out] opu8_NrCode         if != NULL: negative response code in case of an error response

   \return
   Errc::success   request sent, positive response received
   Errc::range     data pool, list, element index out of range (checked by client-side function)
   Errc::noact     could not send request (Tx buffer full)
   Errc::config    transport protocol not installed
   Errc::warn      error response
   Errc::rd_wr     malformed protocol response
   Errc::com       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::DataPoolWriteNumeric(const uint8_t ou8_DataPoolIndex,
                                                           const uint16_t ou16_ListIndex,
                                                           const uint16_t ou16_ElementIndex,
                                                           const std::vector<uint8_t> & orc_DataToWrite,
                                                           uint8_t * const opu8_NrCode)
{
   return this->OsyWriteDataPoolData(ou8_DataPoolIndex, ou16_ListIndex, ou16_ElementIndex,
                                     orc_DataToWrite, opu8_NrCode);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write array data to server's data pool

   Send request and wait for response.
   In the openSYDE protocol this results in the same protocol service as writing numeric data.

   \param[in]  ou8_DataPoolIndex   index of data pool to write to
   \param[in]  ou16_ListIndex      list index to write to
   \param[in]  ou16_ElementIndex   element index to write
   \param[out] orc_DataToWrite     data to write
   \param[out] opu8_NrCode         if != NULL: negative response code in case of an error response

   \return
   Errc::success   request sent, positive response received
   Errc::range     data pool, list, element index out of range (checked by client-side function)
   Errc::noact     could not send request (Tx buffer full)
   Errc::config    transport protocol not installed
   Errc::warn      error response
   Errc::rd_wr     malformed protocol response
   Errc::com       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::DataPoolWriteArray(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                                         const uint16_t ou16_ElementIndex,
                                                         const std::vector<uint8_t> & orc_DataToWrite,
                                                         uint8_t * const opu8_NrCode)
{
   return this->DataPoolWriteNumeric(ou8_DataPoolIndex, ou16_ListIndex, ou16_ElementIndex, orc_DataToWrite,
                                     opu8_NrCode);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set rate of event driven transmissions

   Configure update rate of event driven transmissions in ms.
   Three "rails" can be configured for event driven transmissions.
   Each transmission can be assigned to one of these rails.

   Depending on the protocol capabilities the implementation shall either:
   * configure the rail time of the server driver
   * remember the configured rail time for starting subsequent event driven transmissions

   \param[in]  ou8_Rail            0..2 "rail" to configure
   \param[in]  ou16_IntervalMs     interval to configure (in ms)

   \return
   Errc::success   request sent, positive response received (or: rail value remembered)
   Errc::range     parameter out of range (checked by client-side function)
   Errc::noact     could not send request (e.g. Tx buffer full)
   Errc::config    pre-requisites not correct; e.g. driver not initialized
   Errc::warn      error response
   Errc::rd_wr     malformed protocol response
   Errc::com       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::DataPoolSetEventDataRate(const uint8_t ou8_Rail, const uint16_t ou16_IntervalMs)
{
   return this->OsyWriteDataPoolEventDataRate(ou8_Rail, ou16_IntervalMs);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Request cyclic driven transmission of data pool element.

   Request cyclic transmission of data pool element.
   Shall send the request and wait for the initial confirmation (or error) response.

   Data responses shall be handled asynchronously using the Cycle() function and
   be reported via calls to m_DataPoolReadEventReceived().

   \param[in]  ou8_DataPoolIndex   index of data pool to read from
   \param[in]  ou16_ListIndex      list index to read from
   \param[in]  ou16_ElementIndex   element index to read
   \param[in]  ou8_Rail            0..2   "rail" referring to time interval to use
   \param[out] opu8_NrCode         if != NULL: negative response code in case of an error response

   \return
   Errc::success   request sent
   Errc::range     parameter out of range (checked by client-side function)
   Errc::noact     could not send request (e.g. Tx buffer full)
   Errc::config    pre-requisites not correct; e.g. driver not initialized
   Errc::warn      error response
   Errc::rd_wr     malformed protocol response
   Errc::com       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::DataPoolReadCyclic(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                                         const uint16_t ou16_ElementIndex, const uint8_t ou8_Rail,
                                                         uint8_t * const opu8_NrCode)
{
   return this->OsyReadDataPoolDataCyclic(ou8_DataPoolIndex, ou16_ListIndex, ou16_ElementIndex, ou8_Rail, opu8_NrCode);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Request change driven transmission of data pool element.

   Request change driven transmission of data pool element.

   Shall send the request and wait for the initial confirmation (or error) response.

   Data responses shall be handled asynchronously using the Cycle() function and
   be reported via calls to m_DataPoolReadEventReceived().

   \param[in]  ou8_DataPoolIndex   index of data pool to read from
   \param[in]  ou16_ListIndex      list index to read from
   \param[in]  ou16_ElementIndex   element index to read
   \param[in]  ou8_Rail            0..2   "rail" referring to time interval to use for maximum timeout
   \param[in]  ou32_Threshold      threshold the value must cross before getting sent again
   \param[out] opu8_NrCode         if != NULL: negative response code in case of an error response

   \return
   Errc::success   request sent
   Errc::range     parameter out of range (checked by client-side function)
   Errc::noact     could not send request (e.g. Tx buffer full)
   Errc::config    pre-requisites not correct; e.g. driver not initialized
   Errc::warn      error response
   Errc::rd_wr     malformed protocol response
   Errc::com       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::DataPoolReadChangeDriven(const uint8_t ou8_DataPoolIndex,
                                                               const uint16_t ou16_ListIndex,
                                                               const uint16_t ou16_ElementIndex, const uint8_t ou8_Rail,
                                                               const uint32_t ou32_Threshold,
                                                               uint8_t * const opu8_NrCode)
{
   return this->OsyReadDataPoolDataChangeDriven(ou8_DataPoolIndex, ou16_ListIndex, ou16_ElementIndex, ou8_Rail,
                                                ou32_Threshold, opu8_NrCode);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Request stop all change driven transmissions.

   Request stop all change driven transmissions.

   Shall send the request and wait for the initial confirmation (or error) response.

   \return
   Errc::success   request sent
   Errc::timeout   expected response not received within timeout
   Errc::noact     could not send request (e.g. Tx buffer full)
   Errc::config    pre-requisites not correct; e.g. driver not initialized
   Errc::warn      error response
   Errc::rd_wr     malformed protocol response
   Errc::com       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::DataPoolStopEventDriven(void)
{
   return this->OsyStopDataPoolEvents();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Request cyclic driven transmission of NVM content.

   \param[in]     ou32_MemoryAddress NVM memory address to read (first read byte)
   \param[in,out] orc_DataRecord     in: size defines number of data bytes to read
                                     out: read data bytes
   \param[out]    opu8_NrCode        if != NULL: negative response code in case of an error response

   \return
   Errc::success   request sent, positive response received
   Errc::timeout   expected response not received within timeout
   Errc::noact     could not put request in Tx queue ...
   Errc::config    no transport protocol installed
   Errc::warn      error response
   Errc::com       expected server response not received because of communication error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::NvmRead(const uint32_t ou32_MemoryAddress, std::vector<uint8_t> & orc_DataRecord,
                                              uint8_t * const opu8_NrCode)
{
   std::error_code c_Retval = Errc::success;

   c_Retval = this->OsyReadMemoryByAddress(ou32_MemoryAddress, orc_DataRecord, opu8_NrCode);

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Start NVM write sequence

   For the openSYDE protocol there's nothing to do ...

   \param[in]  ou8_DataPoolIndex   Accessed data pool index
   \param[in]  ou16_NvmAccessCount Expected NVM access count

   \return
   Errc::success   write sequence started
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::NvmWriteStartTransaction(const uint8_t ou8_DataPoolIndex,
                                                               const uint16_t ou16_NvmAccessCount)
{
   (void)ou8_DataPoolIndex;
   (void)ou16_NvmAccessCount;
   return Errc::success;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Write to NVM

   Usage:
   * First call NvmWriteStartTransaction() (Here you can register any non zero amount of NVM write operations)
   * Execute the specified amount NVM write operations
   * Afterwards call NvmWriteFinalizeTransaction() to complete this transaction.

   \param[in]  ou32_MemoryAddress NVM memory address to write to (first written byte)
   \param[in]  orc_DataRecord     Data bytes to write
   \param[out] opu8_NrCode        if != NULL: negative response code in case of an error response

   \return
   Errc::success   request sent, positive response received
   Errc::timeout   expected response not received within timeout
   Errc::noact     could not put request in Tx queue ...
   Errc::config    no transport protocol installed
   Errc::warn      error response
   Errc::rd_wr     unexpected content in response
   Errc::range     orc_DataRecord empty
   Errc::com       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::NvmWrite(const uint32_t ou32_MemoryAddress,
                                               const std::vector<uint8_t> & orc_DataRecord, uint8_t * const opu8_NrCode)
{
   std::error_code c_Retval = Errc::success;

   c_Retval = this->OsyWriteMemoryByAddress(ou32_MemoryAddress, orc_DataRecord, opu8_NrCode);

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   NVM write operation finalization

   For the openSYDE protocol there's nothing to do ...

   \return
   Errc::success   no problem
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::NvmWriteFinalizeTransaction(void)
{
   return Errc::success;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read data pool version

   Version format: One byte for Major, Minor, Release

   Example: v1.23r4 in 3 Bytes   -> (0x01, 0x17, 0x04)

   \param[in]  ou8_DataPoolIndex   Data pool index
   \param[out] orau8_Version       Read version; format: see function description
   \param[out] opu8_NrCode         if != NULL: negative response code in case of an error response

   \return
   Errc::success   request sent, positive response received
   Errc::timeout   expected response not received within timeout
   Errc::noact     could not put request in Tx queue ...
   Errc::config    no transport protocol installed
   Errc::warn      error response
   Errc::com       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::DataPoolReadVersion(const uint8_t ou8_DataPoolIndex, uint8_t (&orau8_Version)[3],
                                                          uint8_t * const opu8_NrCode)
{
   std::error_code c_Retval = Errc::success;
   C_DataPoolMetaData c_MetaData;

   c_Retval = this->OsyReadDataPoolMetaData(ou8_DataPoolIndex, c_MetaData, opu8_NrCode);

   if (c_Retval == Errc::success)
   {
      orau8_Version[0] = c_MetaData.au8_Version[0];
      orau8_Version[1] = c_MetaData.au8_Version[1];
      orau8_Version[2] = c_MetaData.au8_Version[2];
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read Datapool meta data

   Version format: One byte for Major, Minor, Release

   Example: v1.23r4 in 3 Bytes   -> (0x01, 0x17, 0x04)

   \param[in]  ou8_DataPoolIndex   Datapool index
   \param[out] orau8_Version       Read version; format: see function description
   \param[out] orc_Name            Read name of Datapool
   \param[out] opu8_NrCode         if != NULL: negative response code in case of an error response

   \return
   Errc::success   request sent, positive response received; or: no action required
   Errc::timeout   expected response not received within timeout
   Errc::noact     could not send protocol request
   Errc::config    CAN dispatcher not installed
   Errc::warn      error response
   Errc::rd_wr     unexpected content in response (here: wrong data pool index)
   Errc::com       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::DataPoolReadMetaData(const uint8_t ou8_DataPoolIndex, uint8_t (&orau8_Version)[3],
                                                           std::string & orc_Name, uint8_t * const opu8_NrCode)
{
   std::error_code c_Retval = Errc::success;
   C_DataPoolMetaData c_MetaData;

   c_Retval = this->OsyReadDataPoolMetaData(ou8_DataPoolIndex, c_MetaData, opu8_NrCode);

   if (c_Retval == Errc::success)
   {
      orau8_Version[0] = c_MetaData.au8_Version[0];
      orau8_Version[1] = c_MetaData.au8_Version[1];
      orau8_Version[2] = c_MetaData.au8_Version[2];

      orc_Name = c_MetaData.c_Name;
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Verify data pool consistency

   Used to check whether the referenced data pool matches with the passed criteria.
   In the openSYDE protocol we verify:
   * a checksum over data data pool definition

   \param[in]  ou8_DataPoolIndex              Data pool index
   \param[in]  ou16_NumberOfDataPoolElements  number of elements in data pool (not used in this protocol)
   \param[in]  ou16_DataPoolVersion           version number of data pool (not used in this protocol)
   \param[in]  ou32_DataPoolChecksum          Data pool checksum
   \param[out] orq_Match                      Data pool checksum match response

   \return
   Errc::success   request sent, positive response received
   Errc::timeout   expected response not received within timeout
   Errc::noact     could not put request in Tx queue ...
   Errc::config    no transport protocol installed
   Errc::warn      error response
   Errc::rd_wr     unexpected content in response (here: wrong data pool index)
   Errc::com       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::DataPoolVerify(const uint8_t ou8_DataPoolIndex,
                                                     const uint16_t ou16_NumberOfDataPoolElements,
                                                     const uint16_t ou16_DataPoolVersion,
                                                     const uint32_t ou32_DataPoolChecksum, bool & orq_Match)
{
   std::error_code c_Retval = Errc::success;

   (void)ou16_NumberOfDataPoolElements; //not supported by this protocol (checksum should be good enough)
   (void)ou16_DataPoolVersion;          //not supported by this protocol (checksum should be good enough)

   c_Retval = this->OsyVerifyDataPool(ou8_DataPoolIndex, ou32_DataPoolChecksum, orq_Match);

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Notify NVM data changes

   \param[in]  ou8_DataPoolIndex           Data pool index
   \param[in]  ou8_ListIndex               List index
   \param[out] orq_ApplicationAcknowledge  true: positive acknowledge from server
                                           false: negative acknowledge from server
   \param[out] opu8_NrCode                 if != NULL: negative response code in case of an error response

   \return
   Errc::success   request sent, positive response received
   Errc::timeout   expected response not received within timeout
   Errc::noact     could not put request in Tx queue ...
   Errc::config    no transport protocol installed
   Errc::warn      error response
   Errc::rd_wr     unexpected content in response (here: wrong data pool or list index)
   Errc::com       communication driver reported error
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscDiagProtocolOsy::NvmNotifyOfChanges(const uint8_t ou8_DataPoolIndex, const uint8_t ou8_ListIndex,
                                                         bool & orq_ApplicationAcknowledge, uint8_t * const opu8_NrCode)
{
   std::error_code c_Retval = Errc::success;

   c_Retval =
      this->OsyNotifyNvmDataChanges(ou8_DataPoolIndex, ou8_ListIndex, orq_ApplicationAcknowledge, opu8_NrCode);

   return c_Retval;
}
//----------------------------------------------------------------------------------------------------------------------
