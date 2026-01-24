//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Group of device descriptions (header)

   Group of device descriptions (note: main module description should be in .cpp
   file)

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDEVICEGROUP_HPP
#define C_OSCDEVICEGROUP_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */

#include "C_OscDeviceDefinition.hpp"
#include <QSettings>
#include <QString>


/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_core {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_OscDeviceGroup {
public:
  C_OscDeviceGroup(void);

  const C_OscDeviceDefinition *
  LookForDevice(const QString &orc_Name, const QString &orc_MainDeviceName,
                uint32_t &oru32_SubDeviceIndex) const;
  bool PreCheckDevice(const QString &orc_DeviceName,
                      const QString &orc_DeviceNameAlias,
                      const QString &orc_DevicePath) const;
  int32_t LoadGroup(QSettings &orc_Ini, const QString &orc_BasePath);

  void SetGroupName(const QString &orc_GroupName);
  QString GetGroupName(void) const;

  const std::vector<C_OscDeviceDefinition> &GetDevices(void) const;

private:
  QString mc_GroupName; ///< Group name
  std::vector<C_OscDeviceDefinition>
      mc_Devices; ///< All devices belonging to this group
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
