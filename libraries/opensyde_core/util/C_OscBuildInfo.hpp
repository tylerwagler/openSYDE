//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Build identification of the running binary (header)

   See cpp file for detailed description

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCBUILDINFO_HPP
#define C_OSCBUILDINFO_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
///Identifies the source a binary was built from: git describe and the commit date
class C_OscBuildInfo
{
public:
   static const char * h_GetVersionControlId(void);
   static const char * h_GetSourceDate(void);
   static std::string h_GetSummary(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
