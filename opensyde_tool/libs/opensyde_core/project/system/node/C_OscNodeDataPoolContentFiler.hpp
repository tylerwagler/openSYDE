//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscNodeDataPoolContent (header)

   Serializes the polymorphic data pool content type (any of the supported
   primitive types, scalar or array) to and from JSON.

   JSON shape:
     {
       "type":     "uint8" | "uint16" | "uint32" | "uint64" |
                   "sint8" | "sint16" | "sint32" | "sint64" |
                   "float32" | "float64",
       "is_array": <bool>,
       "value":    <encoding depends on type+is_array, see below>
     }

   Value encoding:
     - Scalar uint8/16/32, sint8/16/32, float32/64 .... JSON number
     - Scalar uint64, sint64 .......................... JSON string (decimal)
     - Array  uint8 ................................... JSON string (Base64)
     - Array  uint16/32, sint8/16/32, float32/64 ...... JSON array of numbers
     - Array  uint64, sint64 .......................... JSON array of strings

   The uint8 array uses Base64 because the underlying storage is a
   QByteArray and Base64 is the natural compact encoding for byte blobs.
   The 64-bit types are string-encoded for the same reason as elsewhere
   in the filer subsystem (JSON numbers are doubles internally).

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODEDATAPOOLCONTENTFILER_HPP
#define C_OSCNODEDATAPOOLCONTENTFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscNodeDataPoolContent.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscNodeDataPoolContentFiler
{
public:
   static QJsonObject save(const C_OscNodeDataPoolContent & orc_Content);
   static int32_t load(const QJsonObject & orc_Json, C_OscNodeDataPoolContent & orc_Content);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCNODEDATAPOOLCONTENTFILER_HPP
