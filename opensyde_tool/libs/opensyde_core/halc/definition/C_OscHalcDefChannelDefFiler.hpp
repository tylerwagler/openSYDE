//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefChannelDef (header)

   Serializes a simple channel definition with just a name.

   JSON shape:
     {
       "name": <string>
     }

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFCHANNELDEFFILER_HPP
#define C_OSCHALCDEFCHANNELDEFFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHalcDefChannelDef.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefChannelDefFiler
{
public:
   static QJsonObject save(const C_OscHalcDefChannelDef & orc_Channel);
   static int32_t     load(const QJsonObject & orc_Json, C_OscHalcDefChannelDef & orc_Channel);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCHALCDEFCHANNELDEFFILER_HPP
