//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Store HALC channel information
   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFCHANNEL_HPP
#define C_OSCHALCDEFCHANNEL_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefChannelDef
{
public:
   C_OscHalcDefChannelDef(void);

   QString c_Name; ///< Name of this channel

   void CalcHash(uint32_t & oru32_HashValue) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
