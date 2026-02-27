//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Class to handle parsing of PEM file folder and stores all
   necessary information about all read PEM files \copyright   Copyright 2021
   Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSECURITYPEMDATABASE_HPP
#define C_OSCSECURITYPEMDATABASE_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QList>
#include <QString>

#include "C_OscSecurityPemKeyInfo.hpp"
#include "stwtypes.hpp"

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

class C_OscSecurityPemDatabase {
public:
  C_OscSecurityPemDatabase();

  uint32_t GetSizeOfDatabase(void) const;
  const C_OscSecurityPemKeyInfo *
  GetPemFileBySerialNumber(const QByteArray &orc_SerialNumber) const;
  const C_OscSecurityPemKeyInfo *GetLevel7PemInformation(void) const;

  int32_t AddLevel7PemFile(const QString &orc_Path);
  int32_t ParseFolder(const QString &orc_FolderPath);

private:
  bool mq_StoredLevel7PemInformationValid;
  C_OscSecurityPemKeyInfo mc_StoredLevel7PemInformation;
  QList<C_OscSecurityPemKeyInfo> mc_StoredPemFiles;

  int32_t m_TryAddKeyFromPath(const QString &orc_Path,
                              const bool oq_AddToList);
  int32_t m_TryAddKey(const C_OscSecurityPemKeyInfo &orc_NewKey,
                      QString &orc_ErrorMessage, const bool oq_AddToList);
  static QStringList mh_GetPemFiles(const QString &orc_FolderPath);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
