//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscCanSignal (header)

   Serializes a single CAN signal (positioning, byte order, multiplex info,
   CANopen / J1939 specifics) to and from JSON.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANSIGNALFILER_HPP
#define C_OSCCANSIGNALFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanSignal.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscCanSignalFiler
{
public:
   static QJsonObject save(const C_OscCanSignal & orc_Signal);
   static int32_t load(const QJsonObject & orc_Json, C_OscCanSignal & orc_Signal);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCCANSIGNALFILER_HPP
