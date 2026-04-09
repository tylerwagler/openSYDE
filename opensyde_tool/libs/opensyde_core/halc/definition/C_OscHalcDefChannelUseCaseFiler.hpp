//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefChannelUseCase (header)

   Serializes a channel use-case including value and availability settings.

   JSON shape:
     {
       "id":           <string>,
       "display":      <string>,
       "comment":      <string>,
       "value":        <content object>,
       "availability": [<array of availability objects>],
       "default_channels": [<array of uint32_t indices>]
     }

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFCHANNELUSECASEFILER_HPP
#define C_OSCHALCDEFCHANNELUSECASEFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHalcDefChannelUseCase.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefChannelUseCaseFiler
{
public:
   static QJsonObject save(const C_OscHalcDefChannelUseCase & orc_UseCase);
   static int32_t     load(const QJsonObject & orc_Json, C_OscHalcDefChannelUseCase & orc_UseCase);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCHALCDEFCHANNELUSECASEFILER_HPP
