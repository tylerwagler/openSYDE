//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscHalcDef (header)

   Serializes the top-level HALC definition including base fields and domains.

   JSON shape:
     {
       // Base C_OscHalcDefBase fields
       "content_version":       <uint32_t>,
       "device_name":           <string>,
       "file_string":           <string>,
       "original_file_name":    <string>,
       "safety_mode":           <string - enum from E_SafetyMode>,
       "num_config_copies":     <uint8_t>,
       "nvm_based_config":      <bool>,
       "nvm_safe_address_offset":      [<array of uint32_t>],
       "nvm_non_safe_address_offset":  [<array of uint32_t>],
       "nvm_reserved_list_size_parameters":       <uint32_t>,
       "nvm_reserved_list_size_input_values":     <uint32_t>,
       "nvm_reserved_list_size_output_values":    <uint32_t>,
       "nvm_reserved_list_size_status_values":    <uint32_t>,

       // Domain-specific
       "domains": [<array of domain objects>]
     }

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
               Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFFILER_HPP
#define C_OSCHALCDEFFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHalcDef.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefFiler
{
public:
   static QJsonObject save(const C_OscHalcDef & orc_Definition);
   static int32_t     load(const QJsonObject & orc_Json, C_OscHalcDef & orc_Definition);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCHALCDEFFILER_HPP
