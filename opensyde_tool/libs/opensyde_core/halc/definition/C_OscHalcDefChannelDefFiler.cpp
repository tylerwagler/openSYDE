//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefChannelDef (impl)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "C_OscHalcDefChannelDefFiler.hpp"
#include "C_OscJsonUtil.hpp"
#include "stwerrors.hpp"

using namespace stw::opensyde_core;
using namespace stw::errors;

//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscHalcDefChannelDefFiler::save(const C_OscHalcDefChannelDef & orc_Channel)
{
   QJsonObject c_Json;
   c_Json["name"] = orc_Channel.c_Name;
   return c_Json;
}

//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscHalcDefChannelDefFiler::load(const QJsonObject & orc_Json, C_OscHalcDefChannelDef & orc_Channel)
{
   return C_OscJsonUtil::h_GetString(orc_Json, "name", orc_Channel.c_Name);
}
