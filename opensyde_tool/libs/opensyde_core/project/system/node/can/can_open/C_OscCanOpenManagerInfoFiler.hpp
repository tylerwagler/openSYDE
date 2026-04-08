//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscCanOpenManagerInfo (header)

   Composes the device-info filer.  The CANopen device hash is serialized as
   a JSON array of {key, value} entries (each entry inlines the
   C_OscCanInterfaceId fields and the device sub-object) — preserving order
   for stable diffs and avoiding the JSON-keys-must-be-strings restriction
   on the underlying composite key type.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANOPENMANAGERINFOFILER_HPP
#define C_OSCCANOPENMANAGERINFOFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanOpenManagerInfo.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscCanOpenManagerInfoFiler
{
public:
   static QJsonObject save(const C_OscCanOpenManagerInfo & orc_Manager);
   static int32_t load(const QJsonObject & orc_Json, C_OscCanOpenManagerInfo & orc_Manager);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCCANOPENMANAGERINFOFILER_HPP
