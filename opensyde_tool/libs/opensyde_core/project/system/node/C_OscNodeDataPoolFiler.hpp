//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscNodeDataPool (header)

   Composes the List filer.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODEDATAPOOLFILER_HPP
#define C_OSCNODEDATAPOOLFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscNodeDataPool.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscNodeDataPoolFiler
{
public:
   static QJsonObject save(const C_OscNodeDataPool & orc_DataPool);
   static int32_t load(const QJsonObject & orc_Json, C_OscNodeDataPool & orc_DataPool);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCNODEDATAPOOLFILER_HPP
