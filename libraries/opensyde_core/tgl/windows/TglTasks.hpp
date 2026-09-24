//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Target Glue Layer: Task system related functionality

   Target Glue Layer module containing task system related functionality.
   Implementation for Windows.

   \copyright   Copyright 2009 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef TGLTASKSHPP
#define TGLTASKSHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <windows.h>
#include <string>
#include <cstdint>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace tgl
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
///Implements a handler for critical sections
class C_TglCriticalSection
{
protected:
   CRITICAL_SECTION mc_CriticalSection; ///< synchronization object

private:
   //this class can not be copied:
   C_TglCriticalSection(const C_TglCriticalSection & orc_Source);
   C_TglCriticalSection & operator = (const C_TglCriticalSection & orc_Source);

public:
   C_TglCriticalSection(void);
   virtual ~C_TglCriticalSection(void);

   void Acquire(void);
   bool TryAcquire(void);
   void Release(void);
};

int32_t TglStartProcessDetached(const std::string & orc_BinaryPath, const std::string & orc_Parameters);

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
