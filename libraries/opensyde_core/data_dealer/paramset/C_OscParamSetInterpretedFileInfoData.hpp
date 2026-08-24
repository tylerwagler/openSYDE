//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Storage for interpreted param set file info data (header)

   See cpp file for detailed description

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPARAMSETINTERPRETEDFILEINFODATA_HPP
#define C_OSCPARAMSETINTERPRETEDFILEINFODATA_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscParamSetInterpretedFileInfoData
{
public:
   C_OscParamSetInterpretedFileInfoData(void);

   void Clear(void);
   void AddInterpretedFileData(const std::string & orc_DateTime, const std::string & orc_Creator,
                               const std::string & orc_ToolName,
                               const std::string & orc_ToolVersion,
                               const std::string & orc_ProjectName,
                               const std::string & orc_ProjectVersion,
                               const std::string & orc_UserComment);

   //Date and time of creation
   std::string c_DateTime;
   //User who created this file
   std::string c_Creator;
   //Name of tool used for creating this param set file
   std::string c_ToolName;
   //Version of tool used for creating this param set file
   std::string c_ToolVersion;
   //Name of project used for creating this param set file
   std::string c_ProjectName;
   //Version of project used for creating this param set file
   std::string c_ProjectVersion;
   //Optional user comment for this param set file
   std::string c_UserComment;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
