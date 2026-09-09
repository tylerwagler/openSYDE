//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Target Glue Layer: File functions

   Target Glue Layer module containing file functions:
   - file timestamps
   - file size
   - file exists

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef TGLFILEHPP
#define TGLFILEHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include <string>
#include <vector>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace tgl
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

///Information about found files
class C_TglFileSearchRecord
{
public:
   std::string c_FileName; ///< name of found file (with full path)
   //for now we only need the name ...
};

bool TglFileAgeString(const std::string & orc_FileName, std::string & orc_String);
int32_t TglFileSize(const std::string & orc_FileName);
bool TglDirectoryExists(const std::string & orc_Path);
bool TglFileExists(const std::string & orc_FileName);

int32_t TglFileFind(const std::string & orc_SearchPattern,
                    std::vector<C_TglFileSearchRecord> & orc_FoundFiles);

std::string TglFileIncludeTrailingDelimiter(const std::string & orc_Path);
std::string TglExtractFileExtension(const std::string & orc_Path);
std::string TglChangeFileExtension(const std::string & orc_Path,
                                              const std::string & orc_Extension);
std::string TglGetExePath(void);
std::string TglExtractFilePath(const std::string & orc_Path);
std::string TglExtractFileName(const std::string & orc_Path);
std::string TglExpandFileName(const std::string & orc_RelativePath,
                                         const std::string & orc_BasePath);
bool TglIsRelativePath(const std::string & orc_Path);

int32_t TglCreateDirectory(const std::string & orc_Directory);
int32_t TglRemoveDirectory(const std::string & orc_Directory, const bool oq_ContentOnly);

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
