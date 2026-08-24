//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       This class uses a platform-specific path of the executable, which is running in the current process and
                creates a MD5-Checksum from this file. It also sets an boolean global state, if the Checksum has already
                been calculated.
   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCUTILBINARYHASH_HPP
#define C_OSCUTILBINARYHASH_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscUtilBinaryHash
{
public:
   static std::string h_CreateBinaryHash(void);
   static void h_SetHashBool(const bool oq_State);
   static bool h_GetHashBool(void);

private:
   static std::string mhc_BinaryHash;
   static bool mhq_HashCompleted;
   C_OscUtilBinaryHash();
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
