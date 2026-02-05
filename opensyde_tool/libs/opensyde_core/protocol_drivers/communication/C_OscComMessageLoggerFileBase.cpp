//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class with generic interface to handle and write a CAN log file (implementation)

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"
#include <QFileInfo>

#include <cstdio>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscUtils.hpp"
#include "C_OscComMessageLoggerFileBase.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::scl;
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
C_OscComMessageLoggerFileBase::C_OscComMessageLoggerFileBase(const QString & orc_FilePath,
                                                             const QString & orc_ProtocolName) :
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
   C_NO_ERR    No errors
   C_RD_WR     Error on creating folders or deleting old file
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscComMessageLoggerFileBase::OpenFile(void)
{
   int32_t s32_Return = C_NO_ERR;
   const QString c_FolderPath = QFileInfo(this->mc_FilePath).absolutePath() + "/";

   // Check and create folder
   if (!QFileInfo(c_FolderPath).isDir())
   {
      s32_Return = C_OscUtils::h_CreateFolderRecursively(c_FolderPath);

      if (s32_Return != C_NO_ERR)
      {
         s32_Return = C_RD_WR;
      }
   }
   else if (QFileInfo(this->mc_FilePath).exists() && QFileInfo(this->mc_FilePath).isFile())
   {
      // Delete the old file
      if (remove(this->mc_FilePath.toUtf8().constData()) != 0)
      {
         s32_Return = C_RD_WR;
      }
   }
   else
   {
      // Nothing to do
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets the current protocol name

   \param[in]     orc_ProtocolName         Current protocol name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscComMessageLoggerFileBase::SetProtocolName(const QString & orc_ProtocolName)
{
   this->mc_ProtocolName = orc_ProtocolName;
}
