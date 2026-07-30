//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Crypto Agent server class logic reference implementation

   For details see documentation in .hpp file.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <algorithm>
#include <string>

#include "stwtypes.hpp"
#include "stwerrors.hpp"

#include "TglFile.hpp"
#include "C_SclString.hpp"
#include "C_OscUtils.hpp"
#include "C_SclChecksums.hpp"
#include "C_CaServerLogic.hpp"
#include "C_OscSecurityRsa.hpp"
#include "C_OscLoggingHandler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::scl;
using namespace stw::tgl;
using namespace stw::osy_crypto_agent;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
class C_FileInfo
{
public:
   bool operator <(const C_FileInfo & orc_Ref) const
   {
      return this->c_Name < orc_Ref.c_Name;
   }

   std::string c_Name;
   uint64_t u64_LastWriteTimeUtcSeconds;
};

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

C_CaServerLogic::C_CaReferenceConfFile::C_CaReferenceConfFile() :
   C_OscConfFileHandler(),
   u16_Port(C_CaServerLogic::mhu16_SERVER_PORT_DEFAULT),
   c_LogFileFolder("./logs"),
   e_StdOutVerbosityLevel(eSTDOUT_VERBOSITY_FULL),
   c_PemFileFolder("./certificates")
{
   au8_BindAddress[0] = 127U;
   au8_BindAddress[1] = 0U;
   au8_BindAddress[2] = 0U;
   au8_BindAddress[3] = 1U;
}

//----------------------------------------------------------------------------------------------------------------------

C_CaServerLogic::C_CaReferenceConfFile::~C_CaReferenceConfFile()
{
}
//----------------------------------------------------------------------------------------------------------------------

int32_t C_CaServerLogic::C_CaReferenceConfFile::m_LoadSettings(const C_SclStringList & orc_SettingsWithoutComments)
{
   int32_t s32_Result;
   C_SclString c_BindAddress;
   C_SclString c_StdOutVerbosityLevel;

   this->u16_Port =
      static_cast<uint16_t>(orc_SettingsWithoutComments.Values("OSY_CRYPTO_AGENT_TCP_SERVER_PORT").ToIntDef(
                               C_CaServer::mhu16_SERVER_PORT_DEFAULT));

   this->c_LogFileFolder = orc_SettingsWithoutComments.Values("OSY_CRYPTO_AGENT_LOG_FILE_FOLDER").c_str();
   if (this->c_LogFileFolder == "")
   {
      osc_write_log_info("Configuration",
                         "No setting for OSY_CRYPTO_AGENT_LOG_FILE_FOLDER found. Using default './logs'.");
      this->c_LogFileFolder = "./logs";
   }
   this->c_PemFileFolder = orc_SettingsWithoutComments.Values("OSY_CRYPTO_AGENT_AUTH_PEM_FILE_FOLDER").c_str();
   if (this->c_PemFileFolder == "")
   {
      osc_write_log_info("Configuration",
                         "No setting for OSY_CRYPTO_AGENT_AUTH_PEM_FILE_FOLDER found. Using default './certificates'.");

      this->c_PemFileFolder = "./certificates";
   }

   c_StdOutVerbosityLevel =
      orc_SettingsWithoutComments.Values("OSY_CRYPTO_AGENT_STDOUT_VERBOSITY_LEVEL").LowerCase();
   if ((c_StdOutVerbosityLevel == "") || (c_StdOutVerbosityLevel == "full"))
   {
      this->e_StdOutVerbosityLevel = eSTDOUT_VERBOSITY_FULL;
   }
   else if (c_StdOutVerbosityLevel == "low")
   {
      this->e_StdOutVerbosityLevel = eSTDOUT_VERBOSITY_LOW;
   }
   else
   {
      this->e_StdOutVerbosityLevel = eSTDOUT_VERBOSITY_FULL;
      osc_write_log_info("Configuration",
                         "Invalid setting for OSY_CRYPTO_AGENT_STDOUT_VERBOSITY_LEVEL found. Using default 'full'.");
   }

   c_BindAddress = orc_SettingsWithoutComments.Values("OSY_CRYPTO_AGENT_TCP_SERVER_BIND_ADDRESS").c_str();
   s32_Result = C_OscUtils::h_StringToIp4(c_BindAddress, this->au8_BindAddress);
   if (s32_Result != C_NO_ERR)
   {
      osc_write_log_info("Configuration",
                         "None or invalid setting for OSY_CRYPTO_AGENT_TCP_SERVER_BIND_ADDRESS found. Using default '127.0.0.1'.");
      this->au8_BindAddress[0] = 127U;
      this->au8_BindAddress[1] = 0U;
      this->au8_BindAddress[2] = 0U;
      this->au8_BindAddress[3] = 1U;
   }

   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------

C_CaServerLogic::C_CaServerLogic() :
   C_CaServerPlatform(),
   mu32_PemFolderStateHash(0U)
{
}

//----------------------------------------------------------------------------------------------------------------------
C_CaServerLogic::~C_CaServerLogic()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check whether reloading of the .pem folder is needed

   We do not perform a 100% test. Just a quick hash over all *.pem files names and last write times.
   This should be sufficient to detect changes like added/removed/changed .pem files in the folder.
   If this hash has changed since last time, we assume that the content of the folder has changed and we need to reload.

   \return  true:  .pem folder content has changed since last call; reload needed
            false: .pem folder content has not changed since last call; no reload needed
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_CaServerLogic::m_PemFolderReloadNeeded()
{
   std::vector<C_FileInfo> c_FileStates;
   C_SclDynamicArray<C_TglFileSearchRecord> c_FoundFiles;
   bool q_ReloadNeeded = false;
   uint32_t u32_CurrentHash = 0x12345678UL; //non-zero start value to distinguish from empty folder

   const C_SclString c_SearchPath =
      stw::tgl::TglFileIncludeTrailingDelimiter(this->c_Settings.c_PemFileFolder.c_str()) + "*.pem";

   if (stw::tgl::TglFileFind(c_SearchPath, c_FoundFiles) == C_NO_ERR)
   {
      for (int32_t s32_It = 0; s32_It < c_FoundFiles.GetLength(); ++s32_It)
      {
         C_FileInfo c_FileState;
         c_FileState.c_Name = c_FoundFiles[s32_It].c_FileName.c_str();
         c_FileState.u64_LastWriteTimeUtcSeconds = c_FoundFiles[s32_It].u64_LastWriteTimeUtcSeconds;
         c_FileStates.push_back(c_FileState);
      }
   }

   //sort by file name, to make sure the order is always the same and we get a stable hash value
   std::sort(c_FileStates.begin(), c_FileStates.end());
   //calculate hash value over file names and last write times to detect changes
   for (uint32_t u32_It = 0U; u32_It < c_FileStates.size(); ++u32_It)
   {
      C_SclChecksums::CalcCRC32(c_FileStates[u32_It].c_Name.c_str(),
                                static_cast<uint32_t>(c_FileStates[u32_It].c_Name.size()), u32_CurrentHash);
      C_SclChecksums::CalcCRC32(&c_FileStates[u32_It].u64_LastWriteTimeUtcSeconds,
                                sizeof(C_FileInfo::u64_LastWriteTimeUtcSeconds),
                                u32_CurrentHash);
   }

   if (this->mu32_PemFolderStateHash != u32_CurrentHash)
   {
      q_ReloadNeeded = true;
   }

   this->mu32_PemFolderStateHash = u32_CurrentHash;

   return q_ReloadNeeded;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Initialize

   Load PEM files from configured folder and populate our "database" of known certificates.

   \retval   C_NO_ERR   Files loaded; information extracted; or: folder not changed since last call; no release done
   \retval   C_RANGE    Folder not found
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_CaServerLogic::Initialize()
{
   int32_t s32_Result = C_NO_ERR;
   const bool q_ReloadNeeded = this->m_PemFolderReloadNeeded();

   if (q_ReloadNeeded == true)
   {
      osc_write_log_info("Initialization", "PEM folder changed. Reloading PEM files.");

      s32_Result = mc_PemDb.ParseFolder(c_Settings.c_PemFileFolder);
      if (s32_Result != C_NO_ERR)
      {
         osc_write_log_error("Initialization", "Error parsing PEM files from folder \"" + c_Settings.c_PemFileFolder +
                             "\". Check if folder exists.");
      }
   }

   return s32_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Solve authentication challenge

   Here:
   * use provided certificate serial number to find private key in our PEM-based data base
   * calculate response value using the found private key and provided challenge value

   \param[in]   orc_Parameters     Input parameters to use for matching against known certificates
   \param[out]  orc_ResponseValue  Resulting response value to send back to server device for authentication
   \param[out]  orc_ErrorDetail    If error is returned: details about error

   \return  result of operation

   \retval  C_NO_ERR  challenge solved; result in orc_ResponseValue
   \retval  C_NOACT   operation failed; details in orc_ErrorDetail
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_CaServerLogic::m_ServiceHandlerSolveAuthChallenge(const C_InputParametersSolveAuthChallenge & orc_Parameters,
                                                            std::vector<uint8_t> & orc_ResponseValue,
                                                            std::string & orc_ErrorDetail)
{
   int32_t s32_Return = C_NOACT;

   orc_ErrorDetail = "";
   orc_ResponseValue.resize(0);

   if (orc_Parameters.u8_AuthAlgorithm != C_CaServer::mhu8_AUTH_ALGO_RSA_1024)
   {
      orc_ErrorDetail = "Unknown algorithm requested";
   }
   else
   {
      //maybe the .pem folder has changed since last time
      //e.g. added/removed/changed .pem files
      this->Initialize();

      // In this implementation we only use the certificate SNR as parameter.
      // Other implementations can decide to check other parameters for more fine tuning.
      const C_OscSecurityPemKeyInfo * const pc_PemKeyInfo =
         mc_PemDb.GetPemFileBySerialNumber(orc_Parameters.c_ServerCertificateSerialNumber);

      if (pc_PemKeyInfo != NULL)
      {
         std::vector<uint8_t> c_RandomValue = orc_Parameters.c_ServerChallengeValue;
         std::vector<uint8_t> c_PrivKey;
         orc_ResponseValue.resize(128, 0U);
         c_RandomValue.resize(8, 0U);

         //get private authentication key from PEM file:
         c_PrivKey = pc_PemKeyInfo->GetPrivateKey();

         //calculate RSA signature with private key and random value from server (u64_Seed)
         s32_Return = C_OscSecurityRsa::h_SignSignature(c_PrivKey, c_RandomValue, orc_ResponseValue);
         if ((s32_Return != C_NO_ERR) || (orc_ResponseValue.size() != 128U))
         {
            orc_ErrorDetail = "Error on calculating RSA signature.";
         }
      }
      else
      {
         orc_ErrorDetail = "No key with specified certificate serial number found.";
      }
   }
   if (s32_Return != C_NO_ERR)
   {
      osc_write_log_error("Challenge solver", orc_ErrorDetail);
   }
   else
   {
      osc_write_log_info("Challenge solver", "Authentication challenge solved successfully.");
   }

   return s32_Return;
}
