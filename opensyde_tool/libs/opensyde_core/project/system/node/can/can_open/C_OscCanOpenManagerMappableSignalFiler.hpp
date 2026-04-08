//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscCanOpenManagerMappableSignal (header)

   Composes the existing C_OscCanSignalFiler and C_OscNodeDataPoolListElementFiler.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANOPENMANAGERMAPPABLESIGNALFILER_HPP
#define C_OSCCANOPENMANAGERMAPPABLESIGNALFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanOpenManagerMappableSignal.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscCanOpenManagerMappableSignalFiler
{
public:
   static QJsonObject save(const C_OscCanOpenManagerMappableSignal & orc_Signal);
   static int32_t load(const QJsonObject & orc_Json, C_OscCanOpenManagerMappableSignal & orc_Signal);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCCANOPENMANAGERMAPPABLESIGNALFILER_HPP
