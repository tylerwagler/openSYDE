//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefDomain (header)

   Serializes a HALC definition domain including channels, use-cases, and values.

   JSON shape:
     {
       "id":                <string>,
       "name":              <string>,
       "singular_name":     <string>,
       "comment":           <string>,
       "channels":          [<array of channel def objects>],
       "channel_use_cases": [<array of use-case objects>],
       "domain_values":     <channel values object>,
       "channel_values":    <channel values object>,
       "category":          <string - enum from E_Category>
     }

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFDOMAINFILER_HPP
#define C_OSCHALCDEFDOMAINFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHalcDefDomain.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefDomainFiler
{
public:
   static QJsonObject save(const C_OscHalcDefDomain & orc_Domain);
   static int32_t     load(const QJsonObject & orc_Json, C_OscHalcDefDomain & orc_Domain);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCHALCDEFDOMAINFILER_HPP
