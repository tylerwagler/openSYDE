//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE: Service Update Package base

   For details cf. documentation in .h file.

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <fstream>
#include <iterator>
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include <string>
#include <sstream>
#include <iomanip>
#include <system_error>
#include "C_OscErrorCategory.hpp"
#include "C_OscSupServiceUpdatePackageBase.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscSystemDefinition.hpp"
#include "C_OscSystemDefinitionFiler.hpp"
#include "C_OscDeviceDefinition.hpp"
#include "C_OscDeviceDefinitionFiler.hpp"
#include "C_OscSuSequences.hpp"
#include "TglFile.hpp"
#include "C_SclIniFile.hpp"
#include "C_OscSuSequences.hpp"
#include "C_OscUtils.hpp"
#include "C_OscZipFile.hpp"
#include "C_OscSecurityAesFile.hpp"
#include "C_OscSupSignatureFiler.hpp"
#include "C_OscSupDefinitionFiler.hpp"
#include "C_OscSecurityPemSecUpdate.hpp"
#include "C_OscSupNodeDefinitionFiler.hpp"
#include "C_OscSecurityEcdsa.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::opensyde_core;
using namespace stw::scl;
using namespace stw::tgl;
using namespace std;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const std::string C_OscSupServiceUpdatePackageBase::mhc_PACKAGE_EXT = ".syde_sup";
const std::string C_OscSupServiceUpdatePackageBase::mhc_PACKAGE_EXT_TMP = ".syde_sup_tmp"; // intermediate directory
                                                                                           // before creating zip
                                                                                           // archive
const std::string C_OscSupServiceUpdatePackageBase::mhc_SUP_SYSDEF = "sup_system_definition.syde_sysdef";

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */
stw::scl::C_SclStringList C_OscSupServiceUpdatePackageBase::mhc_WarningMessages; // global warnings e.g. if update
                                                                                 // position
                                                                                 // of
                                                                                 // active node is not available
std::string C_OscSupServiceUpdatePackageBase::mhc_ErrorMessage;        // description of error which caused
                                                                                 // the
                                                                                 // service update package to fail

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns fix defined service update package extension.

   \return
   service update package extension
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscSupServiceUpdatePackageBase::h_GetPackageExtension()
{
   return mhc_PACKAGE_EXT;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check common security parameters

   \param[in]  orc_EncryptNodes           Encrypt nodes
   \param[in]  orc_EncryptNodesPassword   Encrypt nodes password
   \param[in]  orc_NodeSignatureKeys      Node signature keys
   \param[in]  ou32_NumNodes              Num nodes
   \param[in]  orc_Mode                   Mode
   \param[in]  orc_Function               Function

   \return
   Errc::success    success
   Errc::checksum   size of orc_EncryptNodes does not match system definition
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupServiceUpdatePackageBase::mh_CheckCommonSecurityParameters(
   const std::vector<uint8_t> & orc_EncryptNodes, const std::vector<std::string> & orc_EncryptNodesPassword,
   const std::vector<std::vector<uint8_t> > & orc_NodeSignatureKeys, const uint32_t ou32_NumNodes,
   const std::string & orc_Mode, const std::string & orc_Function)
{
   std::error_code c_Return;

   if (orc_EncryptNodes.size() != orc_EncryptNodesPassword.size())
   {
      mhc_ErrorMessage = "The container of " + orc_Mode + "ed nodes and " + orc_Mode +
                         "ed passwords have not the same size.";
      osc_write_log_error(orc_Function, mhc_ErrorMessage);
      c_Return = Errc::checksum;
   }

   if (!c_Return)
   {
      if (((orc_EncryptNodes.size() != 0UL) && (orc_EncryptNodes.size() != 1UL)) &&
          (orc_EncryptNodes.size() != ou32_NumNodes))
      {
         mhc_ErrorMessage = "The container of " + orc_Mode + "ed nodes and nodes have not the same size.";
         osc_write_log_error(orc_Function, mhc_ErrorMessage);
         c_Return = Errc::checksum;
      }
   }

   if (!c_Return)
   {
      if (((orc_NodeSignatureKeys.size() != 0UL) && (orc_NodeSignatureKeys.size() != 1UL)) &&
          (orc_NodeSignatureKeys.size() != ou32_NumNodes))
      {
         mhc_ErrorMessage = "The container of signature keys and nodes have not the same size.";
         osc_write_log_error(orc_Function, mhc_ErrorMessage);
         c_Return = Errc::checksum;
      }
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get sydeSUP secure def file names

   \param[in]      orc_SystemDefinition   System definition
   \param[in]      orc_TargetPath         Target path
   \param[in,out]  orc_AbsPath            Abs path
   \param[in,out]  orc_RelPath            Rel path
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupServiceUpdatePackageBase::mh_GetSydeSecureDefFileNames(const C_OscSystemDefinition & orc_SystemDefinition,
                                                                    const std::string & orc_TargetPath,
                                                                    std::vector<std::string> & orc_AbsPath,
                                                                    std::vector<std::string> & orc_RelPath)
{
   for (uint32_t u32_ItNode = 0UL; u32_ItNode < orc_SystemDefinition.c_Nodes.size(); ++u32_ItNode)
   {
      const C_OscNode & rc_Node = orc_SystemDefinition.c_Nodes[u32_ItNode];
      const std::string c_Folder = TglFileIncludeTrailingDelimiter(C_OscUtils::h_NiceifyStringForFileName(
                                                                      rc_Node.c_Properties.c_Name));
      const std::string c_File = "secure_update_collection.syde_sucdef";
      const std::string c_RelPath = c_File;
      const std::string c_AbsPath = orc_TargetPath + c_Folder + c_RelPath;
      orc_AbsPath.push_back(c_AbsPath);
      orc_RelPath.push_back(c_RelPath);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get node folder names

   \param[in]      orc_SystemDefinition   System definition
   \param[in]      orc_TargetPath         Target path
   \param[in,out]  orc_AbsPath            Abs path
   \param[in,out]  orc_RelPath            Rel path
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupServiceUpdatePackageBase::mh_GetNodeFolderNames(const C_OscSystemDefinition & orc_SystemDefinition,
                                                             const std::string & orc_TargetPath,
                                                             std::vector<std::string> & orc_AbsPath,
                                                             std::vector<std::string> & orc_RelPath)
{
   for (uint32_t u32_ItNode = 0UL; u32_ItNode < orc_SystemDefinition.c_Nodes.size(); ++u32_ItNode)
   {
      const C_OscNode & rc_Node = orc_SystemDefinition.c_Nodes[u32_ItNode];
      const std::string c_RelFile = C_OscUtils::h_NiceifyStringForFileName(rc_Node.c_Properties.c_Name);
      const std::string c_AbsFile = TglFileIncludeTrailingDelimiter(orc_TargetPath + c_RelFile);
      orc_AbsPath.push_back(c_AbsFile);
      orc_RelPath.push_back(c_RelFile);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Adapt encryption parameters

   \param[in]      orc_InEncryptNodes           In encrypt nodes
   \param[in]      orc_InEncryptNodesPassword   In encrypt nodes password
   \param[in]      ou32_NodeCount               Node count
   \param[in,out]  orc_OutEncryptNodes          Out encrypt nodes
   \param[in,out]  orc_OutEncryptNodesPassword  Out encrypt nodes password
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupServiceUpdatePackageBase::mh_AdaptEncryptionParameters(const std::vector<uint8_t> & orc_InEncryptNodes,
                                                                    const std::vector<std::string> & orc_InEncryptNodesPassword, const uint32_t ou32_NodeCount, std::vector<uint8_t> & orc_OutEncryptNodes,
                                                                    std::vector<std::string> & orc_OutEncryptNodesPassword)
{
   orc_OutEncryptNodes = orc_InEncryptNodes;
   orc_OutEncryptNodesPassword = orc_InEncryptNodesPassword;
   if (orc_OutEncryptNodes.size() == 0UL)
   {
      //Default
      orc_OutEncryptNodes.resize(ou32_NodeCount, 0U);
      orc_OutEncryptNodesPassword.resize(ou32_NodeCount, "");
   }
   else if (orc_OutEncryptNodes.size() == 1UL)
   {
      //All same
      orc_OutEncryptNodes.resize(ou32_NodeCount, orc_InEncryptNodes[0U]);
      orc_OutEncryptNodesPassword.resize(ou32_NodeCount, orc_InEncryptNodesPassword[0U]);
   }
   else
   {
      //No change necessary
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Adapt common signature parameters

   \param[in]      orc_InNodeSignatureKeys   In node signature keys
   \param[in]      ou32_NodeCount            Node count
   \param[in,out]  orc_OutNodeSignatureKeys  Out node signature keys
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupServiceUpdatePackageBase::mh_AdaptCommonSignatureParameters(
   const std::vector<std::vector<uint8_t> > & orc_InNodeSignatureKeys, const uint32_t ou32_NodeCount,
   std::vector<std::vector<uint8_t> > & orc_OutNodeSignatureKeys)
{
   orc_OutNodeSignatureKeys = orc_InNodeSignatureKeys;
   if (orc_OutNodeSignatureKeys.size() == 0UL)
   {
      //Default
      orc_OutNodeSignatureKeys.resize(ou32_NodeCount, std::vector<uint8_t>());
   }
   else if (orc_OutNodeSignatureKeys.size() == 1UL)
   {
      //All same
      orc_OutNodeSignatureKeys.resize(ou32_NodeCount, orc_OutNodeSignatureKeys[0U]);
   }
   else
   {
      //No change necessary
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calc digest

   \param[in]   orc_SourcePath         Source path
   \param[in]   orc_SupFiles           Sup files
   \param[out]  orau8_Digest           Digest
   \param[in]   oq_PathsAreAbsolute    Paths are absolute

   \return
   std::error_code

   \retval   Errc::success   No err
   \retval   Errc::rd_wr     File not found
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupServiceUpdatePackageBase::mh_CalcDigest(const std::string & orc_SourcePath,
                                                                const std::set<std::string> & orc_SupFiles,
                                                                uint8_t (&orau8_Digest)[C_OscSecurityEcdsa::hu32_SHA256_FINAL_LENGTH],
                                                                const bool oq_PathsAreAbsolute)
{
   C_OscSecurityEcdsa c_Signature;

   std::error_code c_Retval = static_cast<Errc>(c_Signature.Sha256Init());

   if (!c_Retval)
   {
      for (std::set<std::string>::const_iterator c_ItFile = orc_SupFiles.begin();
           (c_ItFile != orc_SupFiles.end()) && (!c_Retval);
           ++c_ItFile)
      {
         const std::string c_CompleteFilePath = oq_PathsAreAbsolute ? *c_ItFile : orc_SourcePath + *c_ItFile;
         c_Retval = mh_AddFileToDigest(c_CompleteFilePath, c_Signature);
      }
   }
   if (!c_Retval)
   {
      c_Retval = static_cast<Errc>(c_Signature.Sha256GetDigest(orau8_Digest));
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Add file to digest

   \param[in]      orc_FilePath     File path
   \param[in,out]  orc_Signature    Signature

   \return
   std::error_code

   \retval   Errc::success   No err
   \retval   Errc::rd_wr     File not found
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupServiceUpdatePackageBase::mh_AddFileToDigest(const std::string & orc_FilePath,
                                                                     C_OscSecurityEcdsa & orc_Signature)
{
   std::error_code c_Retval;
   const uint32_t u32_SECTION_SIZE = 256;

   std::ifstream c_InputFileStream;
   const uint32_t u32_InputFileSize = static_cast<uint32_t>(TglFileSize(orc_FilePath));

   c_InputFileStream.open(orc_FilePath.c_str(), std::ifstream::binary);

   if (c_InputFileStream.is_open() == false)
   {
      c_Retval = Errc::rd_wr;
   }
   else
   {
      uint32_t u32_RemainingFileCount = u32_InputFileSize;
      while ((u32_RemainingFileCount / u32_SECTION_SIZE) >= 1UL)
      {
         c_Retval = mh_AddFileSectionToDigest(c_InputFileStream, orc_Signature, u32_SECTION_SIZE);
         u32_RemainingFileCount = u32_RemainingFileCount - u32_SECTION_SIZE;
      }
      if (!c_Retval)
      {
         if (u32_RemainingFileCount > 0UL)
         {
            c_Retval = mh_AddFileSectionToDigest(c_InputFileStream, orc_Signature, u32_RemainingFileCount);
         }
      }
      //close file
      c_InputFileStream.close();
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Add file section to digest

   \param[in,out]  orc_File            File
   \param[in,out]  orc_Signature       Signature
   \param[in]      ou32_SectionLength  Section length

   \return
   std::error_code

   \retval   Errc::success   Everything read
   \retval   Errc::rd_wr     File could not be read
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSupServiceUpdatePackageBase::mh_AddFileSectionToDigest(ifstream & orc_File,
                                                                            C_OscSecurityEcdsa & orc_Signature,
                                                                            const uint32_t ou32_SectionLength)
{
   std::error_code c_Retval;
   //read file content
   bool q_HasFailed;

   std::vector<uint8_t> c_InputData;
   c_InputData.resize(static_cast<size_t>(ou32_SectionLength));
   //lint -e{9176} //no problems as long as charn has the same size as uint8; if not we'd be in deep !"=?& anyway
   orc_File.read(reinterpret_cast<char_t *>(&c_InputData[0]), c_InputData.size());
   //check for error
   q_HasFailed = orc_File.fail();
   if (q_HasFailed == true)
   {
      c_Retval = Errc::rd_wr;
   }
   else
   {
      c_Retval = static_cast<Errc>(orc_Signature.Sha256Update(&c_InputData[0],
                                                              static_cast<uint32_t>(c_InputData.size())));
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Init
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupServiceUpdatePackageBase::mh_Init()
{
   mhc_WarningMessages.Clear(); // clear old warning messages
   mhc_ErrorMessage = "";       // clear old error message
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get warnings and errors

   \param[in,out]  orc_WarningMessages    Warning messages
   \param[in,out]  orc_ErrorMessage       Error message
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupServiceUpdatePackageBase::mh_GetWarningsAndErrors(C_SclStringList & orc_WarningMessages,
                                                               std::string & orc_ErrorMessage)
{
   orc_WarningMessages = mhc_WarningMessages; // set warning messages for caller
   orc_ErrorMessage = mhc_ErrorMessage;       // set error message for caller
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Digest to string

   \param[in]      orau8_DigestBin  Digest bin
   \param[in,out]  orc_Digest       Digest
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupServiceUpdatePackageBase::mh_DigestToString(
   const uint8_t (&orau8_DigestBin)[C_OscSecurityEcdsa::hu32_SHA256_FINAL_LENGTH], std::string & orc_Digest)
{
   orc_Digest = "";
   for (uint32_t u32_ItByte = 0UL; u32_ItByte < C_OscSecurityEcdsa::hu32_SHA256_FINAL_LENGTH; ++u32_ItByte)
   {
      std::stringstream c_Stream;
      c_Stream << std::setw(2) << std::setfill('0') << std::hex << std::uppercase <<
         static_cast<int32_t>(orau8_DigestBin[u32_ItByte]);
      orc_Digest += c_Stream.str();
   }
}
