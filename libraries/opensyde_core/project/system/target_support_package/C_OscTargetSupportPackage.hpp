//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE target support package data handling class
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCTARGETSUPPORTPACKAGE_HPP
#define C_OSCTARGETSUPPORTPACKAGE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants -----------------------------------------------------------  -----------------------------------
   */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscTargetSupportPackage
{
public:
   C_OscTargetSupportPackage();
   std::string c_DeviceName; ///< name of device this package it intended for
   std::string c_Comment;    ///< comment about this package

   std::string c_TemplatePath; ///< path to template package archive

   void Clear(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
