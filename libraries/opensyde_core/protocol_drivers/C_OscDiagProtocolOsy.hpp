//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Diagnostic protocol driver for openSYDE protocol

   \class       stw::opensyde_core::C_OscDiagProtocolOsy
   \brief       openSYDE: Diagnostic protocol driver for openSYDE protocol

   Implements openSYDE protocol.

   Concept for handling of incoming responses when waiting for a specific service response:
   - non event-driven responses not matching the expected service:
     - unexpected service will be dumped ...
     - unexpected length will be dumped
     - unexpected content will lead to error
   - event-driven responses will be reported via function pointers (see Cycle())

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDIAGPROTOCOLOSY_HPP
#define C_OSCDIAGPROTOCOLOSY_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include "C_OscDiagProtocolBase.hpp"
#include "C_OscProtocolDriverOsy.hpp"
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

//----------------------------------------------------------------------------------------------------------------------
///interface class
class C_OscDiagProtocolOsy :
   public C_OscDiagProtocolBase,
   public C_OscProtocolDriverOsy
{
private:
   //not implemented -> prevent copying
   C_OscDiagProtocolOsy(const C_OscDiagProtocolOsy & orc_Source);
   //not implemented -> prevent assignment
   C_OscDiagProtocolOsy & operator = (const C_OscDiagProtocolOsy & orc_Source); //lint !e1511 //we want to hide the base
                                                                                // function

protected:
   //implemented function from C_OscProtocolDriverOsy:
   virtual void m_OsyReadDataPoolDataEventReceived(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                                   const uint16_t ou16_ElementIndex,
                                                   const std::vector<uint8_t> & orc_Value);
   virtual void m_OsyReadDataPoolDataEventErrorReceived(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                                        const uint16_t ou16_ElementIndex, const uint8_t ou8_NrCode);

public:
   C_OscDiagProtocolOsy(void);
   virtual ~C_OscDiagProtocolOsy();

   //implemented functions from C_OscDiagProtocolBase:
   virtual uint8_t GetEndianness(void) const;

   //this is intended to implement the pure virtual function from C_OscDiagProtocolBase
   virtual std::error_code Cycle(void); //lint !e1511

   virtual std::error_code DataPoolReadNumeric(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                               const uint16_t ou16_ElementIndex, std::vector<uint8_t> & orc_ReadData,
                                               uint8_t * const opu8_NrCode);
   virtual std::error_code DataPoolReadArray(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                             const uint16_t ou16_ElementIndex, std::vector<uint8_t> & orc_ReadData,
                                             uint8_t * const opu8_NrCode);
   virtual std::error_code DataPoolWriteNumeric(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                                const uint16_t ou16_ElementIndex,
                                                const std::vector<uint8_t> & orc_DataToWrite,
                                                uint8_t * const opu8_NrCode);
   virtual std::error_code DataPoolWriteArray(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                              const uint16_t ou16_ElementIndex,
                                              const std::vector<uint8_t> & orc_DataToWrite,
                                              uint8_t * const opu8_NrCode);
   virtual std::error_code DataPoolSetEventDataRate(const uint8_t ou8_Rail, const uint16_t ou16_IntervalMs);
   virtual std::error_code DataPoolReadCyclic(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                              const uint16_t ou16_ElementIndex, const uint8_t ou8_Rail,
                                              uint8_t * const opu8_NrCode);
   virtual std::error_code DataPoolReadChangeDriven(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ListIndex,
                                                    const uint16_t ou16_ElementIndex, const uint8_t ou8_Rail,
                                                    const uint32_t ou32_Threshold, uint8_t * const opu8_NrCode);
   virtual std::error_code DataPoolStopEventDriven(void);
   virtual std::error_code NvmRead(const uint32_t ou32_MemoryAddress, std::vector<uint8_t> & orc_DataRecord,
                                   uint8_t * const opu8_NrCode);
   virtual std::error_code NvmWriteStartTransaction(const uint8_t ou8_DataPoolIndex,
                                                    const uint16_t ou16_NvmAccessCount);
   virtual std::error_code NvmWrite(const uint32_t ou32_MemoryAddress, const std::vector<uint8_t> & orc_DataRecord,
                                    uint8_t * const opu8_NrCode);
   virtual std::error_code NvmWriteFinalizeTransaction(void);
   virtual std::error_code DataPoolReadVersion(const uint8_t ou8_DataPoolIndex,
                                               uint8_t(&orau8_Version)[3],
                                               uint8_t * const opu8_NrCode);
   virtual std::error_code DataPoolReadMetaData(const uint8_t ou8_DataPoolIndex,
                                                uint8_t(&orau8_Version)[3],
                                                std::string & orc_Name,
                                                uint8_t * const opu8_NrCode);
   virtual std::error_code DataPoolVerify(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_NumberOfDataPoolElements,
                                          const uint16_t ou16_DataPoolVersion, const uint32_t ou32_DataPoolChecksum,
                                          bool & orq_Match);
   virtual std::error_code NvmNotifyOfChanges(const uint8_t ou8_DataPoolIndex, const uint8_t ou8_ListIndex,
                                              bool & orq_ApplicationAcknowledge, uint8_t * const opu8_NrCode);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
