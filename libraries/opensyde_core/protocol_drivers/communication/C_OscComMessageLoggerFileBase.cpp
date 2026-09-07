//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class with generic interface to handle and write a CAN log file (implementation)

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstdio>
#include <system_error>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "TglFile.hpp"
#include "C_OscUtils.hpp"
#include "C_OscComMessageLoggerFileBase.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::tgl;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   \param[in]  orc_FilePath      Complete path with file name of logging file
   \param[in]  orc_ProtocolName  Name of current set protocol
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscComMessageLoggerFileBase::C_OscComMessageLoggerFileBase(const std::string & orc_FilePath,
                                                             const std::string & orc_ProtocolName) :
   mc_FilePath(orc_FilePath),
   mc_ProtocolName(orc_ProtocolName)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscComMessageLoggerFileBase::~C_OscComMessageLoggerFileBase(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Creates, if necessary, and opens file and adds the default header of the file.

   Base implementation only creates the folders and deletes an already existing file

   \return
   Errc::success    No errors
   Errc::rd_wr      Error on creating folders or deleting old file
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscComMessageLoggerFileBase::OpenFile(void)
{
   std::error_code c_Return = Errc::success;
   const std::string c_FolderPath = TglExtractFilePath(this->mc_FilePath);

   // Check and create folder
   if (TglDirectoryExists(c_FolderPath) == false)
   {
      if (C_OscUtils::h_CreateFolderRecursively(c_FolderPath))
      {
         c_Return = Errc::rd_wr;
      }
   }
   else if (TglFileExists(this->mc_FilePath) == true)
   {
      // Delete the old file
      // remove() is the C library function: returns 0 on success, non-zero on failure (not an STW code)
      if (remove(this->mc_FilePath.c_str()) != 0)
      {
         c_Return = Errc::rd_wr;
      }
   }
   else
   {
      // Nothing to do
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets the current protocol name

   \param[in]     orc_ProtocolName         Current protocol name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscComMessageLoggerFileBase::SetProtocolName(const std::string & orc_ProtocolName)
{
   this->mc_ProtocolName = orc_ProtocolName;
}
