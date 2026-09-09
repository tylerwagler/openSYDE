//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Common base for all graphics items
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_GIBIBASE_HPP
#define C_GIBIBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_GiBiBase
{
public:
   C_GiBiBase(void);
   virtual ~C_GiBiBase(void);

   virtual void SetZetValueCustom(const double of64_ZetValue) = 0;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
