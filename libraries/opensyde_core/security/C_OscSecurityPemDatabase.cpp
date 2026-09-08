//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class to handle parsing of PEM file folder and stores all necessary information about all read PEM files

   Class to handle parsing of PEM file folder and stores all necessary information about all read PEM files

   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <system_error>

#include "TglFile.hpp"
#include "stwerrors.hpp"
#include <string>
#include <vector>
#include "C_OscErrorCategory.hpp"
#include "C_SclStringList.hpp"
#include "C_OscSecurityPem.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscSecurityPemDatabase.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
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
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscSecurityPemDatabase::C_OscSecurityPemDatabase() :
   mq_StoredLevel7PemInformationValid(false)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get size of database

   \return
   Size of database
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscSecurityPemDatabase::GetSizeOfDatabase() const
{
   return static_cast<uint32_t>(this->mc_StoredPemFiles.size());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get pem file by serial number

   \param[in]  orc_SerialNumber  Serial number

   \return
   PEM file by serial number
*/
//----------------------------------------------------------------------------------------------------------------------
const C_OscSecurityPemKeyInfo * C_OscSecurityPemDatabase::GetPemFileBySerialNumber(
   const std::vector<uint8_t> & orc_SerialNumber) const
{
   const C_OscSecurityPemKeyInfo * pc_Retval = nullptr;

   for (uint32_t u32_ItFile = 0UL; u32_ItFile < this->mc_StoredPemFiles.size(); ++u32_ItFile)
   {
      const C_OscSecurityPemKeyInfo & rc_KeyFile = this->mc_StoredPemFiles[u32_ItFile];
      const std::vector<uint8_t> & rc_CurSerialNumber = rc_KeyFile.GetCertificateSerialNumber();
      if (rc_CurSerialNumber.size() == orc_SerialNumber.size())
      {
         bool q_Matches = true;
         for (uint32_t u32_ItKey = 0UL; (u32_ItKey < rc_CurSerialNumber.size()) && (q_Matches == true); ++u32_ItKey)
         {
            if (rc_CurSerialNumber[u32_ItKey] != orc_SerialNumber[u32_ItKey])
            {
               q_Matches = false;
            }
         }
         if (q_Matches)
         {
            pc_Retval = &this->mc_StoredPemFiles[u32_ItFile];
         }
      }
   }

   return pc_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get level 7 pem information

   \return
   Level 7 pem information
*/
//----------------------------------------------------------------------------------------------------------------------
const C_OscSecurityPemKeyInfo * C_OscSecurityPemDatabase::GetLevel7PemInformation() const
{
   const C_OscSecurityPemKeyInfo * pc_Retval = nullptr;

   if (this->mq_StoredLevel7PemInformationValid)
   {
      pc_Retval = &this->mc_StoredLevel7PemInformation;
   }
   return pc_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Add level 7 pem file

   \param[in]  orc_Path    Path

   \return
   std::error_code with Errc::success if the information was extracted,
   Errc::range if the file was not found,
   Errc::config on invalid file content
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityPemDatabase::AddLevel7PemFile(const std::string & orc_Path)
{
   std::error_code c_Retval = Errc::success;

   if (TglFileExists(orc_Path))
   {
      c_Retval = C_OscSecurityPemDatabase::m_TryAddKeyFromPath(orc_Path, false);
   }
   else
   {
      c_Retval = Errc::range;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Parse folder

   \param[in]  orc_FolderPath    Folder path

   \return
   std::error_code with Errc::success if the information was extracted, Errc::range if the folder was not found
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityPemDatabase::ParseFolder(const std::string & orc_FolderPath)
{
   std::error_code c_Retval = Errc::success;

   const std::string c_SclFolderPathWithDelimiter =
      stw::tgl::TglFileIncludeTrailingDelimiter(orc_FolderPath);
   const std::string c_FolderPathWithDelimiter = c_SclFolderPathWithDelimiter.c_str();

   // Remove previous results
   this->mc_StoredPemFiles.clear();

   if (TglDirectoryExists(c_FolderPathWithDelimiter))
   {
      const std::vector<std::string> c_Files = C_OscSecurityPemDatabase::mh_GetPemFiles(c_FolderPathWithDelimiter);
      for (uint32_t u32_It = 0UL; u32_It < c_Files.size(); ++u32_It)
      {
         const std::string c_CurFolderPath = c_Files[u32_It];
         (void)C_OscSecurityPemDatabase::m_TryAddKeyFromPath(c_CurFolderPath, true);
      }
      osc_write_log_info("Read PEM database",
                         "Imported " + std::to_string(
                            this->mc_StoredPemFiles.size()) + " valid PEM files of the total seen " +
                         std::to_string(
                            c_Files.size()) + " PEM files in folder \"" + c_FolderPathWithDelimiter + "\".");
   }
   else
   {
      c_Retval = Errc::range;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Try add key from path

   \param[in]  orc_Path       Path
   \param[in]  oq_AddToList   Add to list (or alternative level 7 file)

   \return
   std::error_code with Errc::success if the information was extracted,
   Errc::range if the file was not found,
   Errc::config on invalid file content
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityPemDatabase::m_TryAddKeyFromPath(const std::string & orc_Path, const bool oq_AddToList)
{
   C_OscSecurityPem c_NewFile;

   std::string c_ErrorMessage;
   std::error_code c_Retval = c_NewFile.LoadFromFile(orc_Path, c_ErrorMessage);
   if (!c_Retval)
   {
      c_Retval = m_TryAddKey(c_NewFile.GetKeyInfo(), c_ErrorMessage, oq_AddToList);
   }
   if (c_ErrorMessage.size() > 0UL)
   {
      std::string c_Heading;
      if (oq_AddToList)
      {
         c_Heading = "Read PEM database";
      }
      else
      {
         c_Heading = "Read PEM level 7 key";
      }
      osc_write_log_warning(c_Heading,
                            "Error reading file \"" + orc_Path + "\": " + c_ErrorMessage);
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Try add key

   \param[in]      orc_NewKey          New key
   \param[in,out]  orc_ErrorMessage    Error message
   \param[in]      oq_AddToList        Add to list (or alternative level 7 file)

   \return
   std::error_code with Errc::success if the information was extracted, Errc::config on invalid file content
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscSecurityPemDatabase::m_TryAddKey(const C_OscSecurityPemKeyInfo & orc_NewKey,
                                                      std::string & orc_ErrorMessage, const bool oq_AddToList)
{
   std::error_code c_Retval = Errc::success;

   if (orc_NewKey.AreKeysAvailable(orc_ErrorMessage, oq_AddToList))
   {
      if (oq_AddToList)
      {
         if (this->GetPemFileBySerialNumber(orc_NewKey.GetCertificateSerialNumber()) == nullptr)
         {
            this->mc_StoredPemFiles.push_back(orc_NewKey);
         }
         else
         {
            c_Retval = Errc::config;
            orc_ErrorMessage = "ignored because serial number already exists";
         }
      }
      else
      {
         this->mq_StoredLevel7PemInformationValid = true;
         this->mc_StoredLevel7PemInformation = orc_NewKey;
      }
   }
   else
   {
      c_Retval = Errc::config;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get PEM files

   \param[in]  orc_FolderPath    Folder path with path delimiter at the end

   \return
   PEM files
*/
//----------------------------------------------------------------------------------------------------------------------
std::vector<std::string> C_OscSecurityPemDatabase::mh_GetPemFiles(const std::string & orc_FolderPath)
{
   std::vector<std::string> c_Retval;
   std::vector<C_TglFileSearchRecord> c_FilesScl;

   TglFileFind(orc_FolderPath + "*", c_FilesScl);
   for (int32_t s32_It = 0; s32_It < static_cast<int32_t>(c_FilesScl.size()); ++s32_It)
   {
      const C_TglFileSearchRecord & rc_FileRecord = c_FilesScl[s32_It];
      const std::string c_FileNameScl = rc_FileRecord.c_FileName;
      const std::string c_Extension = TglExtractFileExtension(c_FileNameScl);
      if (c_Extension == ".pem")
      {
         c_Retval.push_back(orc_FolderPath + c_FileNameScl.c_str());
      }
   }
   return c_Retval;
}
