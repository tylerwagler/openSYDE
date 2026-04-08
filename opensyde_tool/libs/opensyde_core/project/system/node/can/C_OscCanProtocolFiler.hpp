//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscCanProtocol (header)

   Top of the CAN message family.  Composes the message-container filer over
   the per-interface container list.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANPROTOCOLFILER_HPP
#define C_OSCCANPROTOCOLFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanProtocol.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscCanProtocolFiler
{
public:
   static QJsonObject save(const C_OscCanProtocol & orc_Protocol);
   static int32_t load(const QJsonObject & orc_Json, C_OscCanProtocol & orc_Protocol);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCCANPROTOCOLFILER_HPP
