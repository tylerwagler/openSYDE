//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle project save and load

   \class       stw::opensyde_core::C_OscProjectFiler
   \brief       Handle project save and load

   (See .cpp file for full description)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPROJECTFILER_HPP
#define C_OSCPROJECTFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include <string>
#include <system_error>
#include "C_OscProject.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscProjectFiler
{
public:
   static std::error_code h_Save(C_OscProject & orc_Project, const std::string & orc_Path,
                                 const std::string & orc_OpenSydeVersion);
   static std::error_code h_Load(C_OscProject & orc_Project, const std::string & orc_Path);

private:
   static std::error_code mh_SaveInternal(C_OscProject & orc_Project, const std::string & orc_Path,
                                          const std::string & orc_OpenSydeVersion, const bool oq_New);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
