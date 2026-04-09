//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefContent (header)

   Serializes the HALC definition content type which extends C_OscNodeDataPoolContent
   with complex type information (plain, enum, bitmask, string).

   JSON shape:
     {
       // Base C_OscNodeDataPoolContent fields
       "type":     <string - enum from C_OscNodeDataPoolContent::E_Type>,
       "is_array": <bool>,
       "value":    <encoded value per C_OscNodeDataPoolContentFiler>,

       // HALC-specific extensions
       "complex_type": <string - enum from E_ComplexType>,
       "enum_items":   [<array of {display, value} objects>],
       "bitmask_items": [<array of bitmask item objects>],
       "string_value": <string - only for string type>
     }

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFCONTENTFILER_HPP
#define C_OSCHALCDEFCONTENTFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHalcDefContent.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefContentFiler
{
public:
   static QJsonObject save(const C_OscHalcDefContent & orc_Content);
   static int32_t     load(const QJsonObject & orc_Json, C_OscHalcDefContent & orc_Content);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCHALCDEFCONTENTFILER_HPP
