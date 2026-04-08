//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       JSON filer for C_OscCanOpenManagerDeviceInfo (header)

   The dynamically-loaded EDS file content (mc_EdsFileContent) is intentionally
   NOT serialized.  Only the EDS file path and original name are stored; the
   actual EDS content is loaded on demand from the project's side files.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCANOPENMANAGERDEVICEINFOFILER_HPP
#define C_OSCCANOPENMANAGERDEVICEINFOFILER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscCanOpenManagerDeviceInfo.hpp"
#include "stwtypes.hpp"

#include <QJsonObject>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscCanOpenManagerDeviceInfoFiler
{
public:
   static QJsonObject save(const C_OscCanOpenManagerDeviceInfo & orc_Device);
   static int32_t load(const QJsonObject & orc_Json, C_OscCanOpenManagerDeviceInfo & orc_Device);
};

} // namespace opensyde_core
} // namespace stw

#endif // C_OSCCANOPENMANAGERDEVICEINFOFILER_HPP
