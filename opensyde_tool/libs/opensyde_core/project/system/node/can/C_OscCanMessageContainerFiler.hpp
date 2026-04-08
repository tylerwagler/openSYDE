//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscCanMessageContainer (header)

   Composes the message filer.  Holds the Tx and Rx message lists for one
   CAN interface and the "interface is using this protocol" flag.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANMESSAGECONTAINERFILER_HPP
#define C_OSCCANMESSAGECONTAINERFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanMessageContainer.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscCanMessageContainerFiler
{
public:
   static QJsonObject save(const C_OscCanMessageContainer & orc_Container);
   static int32_t load(const QJsonObject & orc_Json, C_OscCanMessageContainer & orc_Container);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCCANMESSAGECONTAINERFILER_HPP
