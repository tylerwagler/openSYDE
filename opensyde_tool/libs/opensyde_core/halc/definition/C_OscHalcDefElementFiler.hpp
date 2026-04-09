//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDefElement (header)

   Serializes a HALC definition element including common fields (id, display, comment)
   and content (type, initial/min/max values, use-case availabilities).

   JSON shape:
     {
       "id":              <string>,
       "display":         <string>,
       "comment":         <string>,
       "initial_value":   <content object>,
       "min_value":       <content object>,
       "max_value":       <content object>,
       "type":            <string - enum from C_OscNodeDataPoolContent::E_Type>,
       "is_array":        <bool>,
       "complex_type":    <string - enum from C_OscHalcDefContent::E_ComplexType>,
       "enum_items":      [<array of enum item objects>],
       "bitmask_items":   [<array of bitmask item objects>],
       "use_case_availabilities": [<array of uint32_t indices>]
     }

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFELEMENTFILER_HPP
#define C_OSCHALCDEFELEMENTFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHalcDefElement.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefElementFiler
{
public:
   static QJsonObject save(const C_OscHalcDefElement & orc_Element);
   static int32_t     load(const QJsonObject & orc_Json, C_OscHalcDefElement & orc_Element);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCHALCDEFELEMENTFILER_HPP
