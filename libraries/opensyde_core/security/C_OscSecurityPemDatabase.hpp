//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class to handle parsing of PEM file folder and stores all necessary information about all read PEM files
   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSECURITYPEMDATABASE_HPP
#define C_OSCSECURITYPEMDATABASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>
#include <vector>
#include <system_error>

#include <cstdint>
#include "C_OscSecurityPemKeyInfo.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscSecurityPemDatabase
{
public:
   C_OscSecurityPemDatabase();

   uint32_t GetSizeOfDatabase(void) const;
   const C_OscSecurityPemKeyInfo * GetPemFileBySerialNumber(const std::vector<uint8_t> & orc_SerialNumber) const;
   const C_OscSecurityPemKeyInfo * GetLevel7PemInformation(void) const;

   std::error_code AddLevel7PemFile(const std::string & orc_Path);
   std::error_code ParseFolder(const std::string & orc_FolderPath);

private:
   bool mq_StoredLevel7PemInformationValid;
   C_OscSecurityPemKeyInfo mc_StoredLevel7PemInformation;
   std::vector<C_OscSecurityPemKeyInfo> mc_StoredPemFiles;

   std::error_code m_TryAddKeyFromPath(const std::string & orc_Path, const bool oq_AddToList);
   std::error_code m_TryAddKey(const C_OscSecurityPemKeyInfo & orc_NewKey, std::string & orc_ErrorMessage,
                               const bool oq_AddToList);
   static std::vector<std::string> mh_GetPemFiles(const std::string & orc_FolderPath);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
