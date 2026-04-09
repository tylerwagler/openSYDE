//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefContentBitmaskItem (header)

   Serializes a bitmask item (display name, comment, apply flag, value) to JSON.

   JSON shape:
     {
       "display":           <string>,
       "comment":          <string>,
       "apply_value_setting": <bool>,
       "value":            <string> (decimal-encoded uint64_t)
     }

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFCONTENTBITMASKITEMFILER_HPP
#define C_OSCHALCDEFCONTENTBITMASKITEMFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHalcDefContentBitmaskItem.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefContentBitmaskItemFiler
{
public:
   static QJsonObject save(const C_OscHalcDefContentBitmaskItem & orc_Item);
   static int32_t     load(const QJsonObject & orc_Json, C_OscHalcDefContentBitmaskItem & orc_Item);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCHALCDEFCONTENTBITMASKITEMFILER_HPP
