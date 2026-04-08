//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscCanMessage (header)

   Composes the signal filer.  Embeds the CANopen owner-node interface id
   inline as a small sub-object.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANMESSAGEFILER_HPP
#define C_OSCCANMESSAGEFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanMessage.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscCanMessageFiler
{
public:
   static QJsonObject save(const C_OscCanMessage & orc_Message);
   static int32_t load(const QJsonObject & orc_Json, C_OscCanMessage & orc_Message);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCCANMESSAGEFILER_HPP
