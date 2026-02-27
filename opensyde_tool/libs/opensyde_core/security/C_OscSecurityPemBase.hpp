//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base class for handling PEM file content
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSECURITYPEMBASE_HPP
#define C_OSCSECURITYPEMBASE_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <vector>

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

class C_OscSecurityPemBase {
public:
  C_OscSecurityPemBase();
  virtual ~C_OscSecurityPemBase();

  void Clear();

  const C_OscSecurityPemKeyInfo &GetKeyInfo(void) const;
  const QString &GetMetaInfos(void) const;

  virtual int32_t LoadFromFile(const QString &orc_FileName,
                               QString &orc_ErrorMessage);

protected:
  static const uint32_t mhu32_DEFAULT_BUFFER_SIZE;

  QString mc_MetaInfo;
  C_OscSecurityPemKeyInfo mc_KeyInfo;

  virtual int32_t m_ReadPublicKey(const QByteArray &orc_FileContent,
                                  QString &orc_ErrorMessage);
  virtual int32_t m_ReadMetaInfos(const QByteArray &orc_FileContent,
                                  QString &orc_ErrorMessage);

  // Pure virtual function. Implementation will differ depending on whether a
  // regular key or an elliptic curve key is
  //  expected.
  virtual int32_t m_ReadPrivateKey(const QByteArray &orc_FileContent,
                                   QString &orc_ErrorMessage) = 0;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
