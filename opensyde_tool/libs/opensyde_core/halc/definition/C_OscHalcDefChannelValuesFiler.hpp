//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefChannelValues (header)

   Serializes channel values containing 4 lists of struct elements:
   parameters, input values, output values, and status values.

   JSON shape:
     {
       "parameters":      [<array of struct objects>],
       "input_values":    [<array of struct objects>],
       "output_values":   [<array of struct objects>],
       "status_values":   [<array of struct objects>]
     }

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFCHANNELVALUESFILER_HPP
#define C_OSCHALCDEFCHANNELVALUESFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHalcDefChannelValues.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefChannelValuesFiler
{
public:
   static QJsonObject save(const C_OscHalcDefChannelValues & orc_Values);
   static int32_t     load(const QJsonObject & orc_Json, C_OscHalcDefChannelValues & orc_Values);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCHALCDEFCHANNELVALUESFILER_HPP
