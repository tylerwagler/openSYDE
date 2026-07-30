//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Sequences for basic system update.

   \copyright   Copyright 2023 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCBUSEQUENCES_HPP
#define C_OSCBUSEQUENCES_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_SclString.hpp"
#include "C_CanDispatcher.hpp"
#include "C_OscIpDispatcher.hpp"
#include "C_OscProtocolDriverOsyTpCan.hpp"
#include "C_OscProtocolDriverOsyTpIp.hpp"
#include "C_OscProtocolDriverOsy.hpp"
#include "C_OscComFlashloaderInformation.hpp"
#include "C_OscHexFile.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscBuSequences
{
public:
   C_OscBuSequences(void);
   virtual ~C_OscBuSequences(void);
   int32_t Init(stw::can::C_CanDispatcher * const opc_CanDispatcher, const int32_t os32_CanBitrate,
                const uint8_t ou8_NodeId);
   int32_t Init(stw::can::C_CanDispatcher * const opc_CanDispatcher, C_OscIpDispatcher * const opc_IpDispatcher,
                const int32_t os32_CanBitrate, const uint8_t (&orau8_IpAddress)[4], const uint8_t ou8_NodeId,
                const uint8_t ou8_BusId = 0U);
   int32_t ActivateFlashLoader(const uint32_t ou32_FlashloaderResetWaitTime);
   int32_t ReadDeviceInformation(void);
   int32_t UpdateNode(const stw::scl::C_SclString & orc_HexFilePath, const uint32_t ou32_RequestDownloadTimeout,
                      const uint32_t ou32_TransferDataTimeout);
   int32_t ResetSystem(void);

   static int32_t h_ReadHexFile(const stw::scl::C_SclString & orc_HexFilePath, C_OscHexFile & orc_HexFile,
                                uint32_t & oru32_SignatureBlockAddress);

   void PrepareForDestruction(void);
   static uint64_t h_GetAllHexFilesSize(const std::vector<std::string> & orc_HexFiles);

protected:
   virtual void m_ReportProgressPercentage(const uint8_t ou8_ProgressInPercentage,
                                           const bool oq_IsCaluclatedPercentage);
   virtual void m_ReportProgress(const int32_t os32_Result, const stw::scl::C_SclString & orc_Information);
   virtual void m_ReportFlashloaderInformationRead(const stw::scl::C_SclString & orc_DeviceName,
                                                   const C_OscComFlashloaderInformation & orc_Information);
   virtual void m_CurrentHexFileSizeInBytes(const uint64_t ou64_CurrentHexFileSizeInBytes);

private:
   // driver instances:
   stw::can::C_CanDispatcher * mpc_CanDispatcher;
   C_OscIpDispatcher * mpc_IpDispatcher;
   C_OscProtocolDriverOsyTpCan * mpc_TpCan;
   C_OscProtocolDriverOsyTpIp * mpc_TpIp;

   C_OscProtocolDriverOsy mc_OsyProtocol;

   int32_t ms32_CanBitrate; // CAN bitrate in kBit/s
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
