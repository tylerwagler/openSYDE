//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefChannelAvailability (header)

   Serializes channel availability with value index and dependent values.

   JSON shape:
     {
       "value_index":       <uint32_t>,
       "dependent_values":  [<array of uint32_t>]
     }

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFCHANNELAVAILABILITYFILER_HPP
#define C_OSCHALCDEFCHANNELAVAILABILITYFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHalcDefChannelAvailability.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefChannelAvailabilityFiler
{
public:
   static QJsonObject save(const C_OscHalcDefChannelAvailability & orc_Availability);
   static int32_t     load(const QJsonObject & orc_Json, C_OscHalcDefChannelAvailability & orc_Availability);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCHALCDEFCHANNELAVAILABILITYFILER_HPP
