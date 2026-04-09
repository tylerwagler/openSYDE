//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscNodeDataPoolListElement (header)

   Composes C_OscNodeDataPoolContentFiler for the min/max/value/dataset
   value fields.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODEDATAPOOLLISTELEMENTFILER_HPP
#define C_OSCNODEDATAPOOLLISTELEMENTFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscNodeDataPoolListElement.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscNodeDataPoolListElementFiler
{
public:
   static QJsonObject save(const C_OscNodeDataPoolListElement & orc_Element);
   static int32_t load(const QJsonObject & orc_Json, C_OscNodeDataPoolListElement & orc_Element);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCNODEDATAPOOLLISTELEMENTFILER_HPP
