//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       HALC definition domain channel use-case
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFCHANNELUSECASE_HPP
#define C_OSCHALCDEFCHANNELUSECASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QList>
#include <QString>
#include "C_OscNodeDataPoolContent.hpp"
#include "C_OscHalcDefChannelAvailability.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefChannelUseCase
{
public:
   C_OscHalcDefChannelUseCase(void);
   virtual ~C_OscHalcDefChannelUseCase();

   virtual void CalcHash(uint32_t & oru32_HashValue) const;

   QString c_Id;                                  ///< Unique ID for later reference of this use-case
   QString c_Display;                             ///< Name to display to user
   QString c_Comment;                             ///< Description to display to user
   C_OscNodeDataPoolContent c_Value;                            ///< Internal value to use
   QList<C_OscHalcDefChannelAvailability> c_Availability; ///< Channel availability settings
   QList<uint32_t> c_DefaultChannels;                     ///< Indices of channels which use this as default
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
