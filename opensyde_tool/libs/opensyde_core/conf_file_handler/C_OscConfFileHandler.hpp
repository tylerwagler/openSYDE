//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE .conf file handler
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
 */
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCCFGFILEHANDLER_HPP
#define C_OSCCFGFILEHANDLER_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <vector>
#include <QList>

#include <QString>
#include <QStringList>


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

/// Generic config class handler
class C_OscConfFileHandler {
public:
  virtual ~C_OscConfFileHandler();

  int32_t LoadSettings(const QString &orc_Path);

protected:
  // Function for loading concrete settings. Must return C_CONFIG in error case
  virtual int32_t m_LoadSettings(
      const QStringList &orc_SettingsWithoutComments) = 0;
  static int32_t mh_ReplaceSettings(
      const QString &orc_Path,
      const QList<std::pair<QString, QString>> &orc_Configs);

  QString mc_ConfigFilePath; // path where the config file was loaded from
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
