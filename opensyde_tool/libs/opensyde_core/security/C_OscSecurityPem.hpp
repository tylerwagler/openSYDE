//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle PEM content
   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSECURITYPEM_HPP
#define C_OSCSECURITYPEM_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <vector>

#include "C_OscSecurityPemBase.hpp"
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

class C_OscSecurityPem : public C_OscSecurityPemBase {
public:
  C_OscSecurityPem();

  static int32_t h_ExtractModulusAndExponentFromFile(
      const QString &orc_FileName, QByteArray &orc_Modulus,
      QByteArray &orc_Exponent, QString &orc_ErrorMessage);
  static int32_t
  h_ExtractModulusAndExponent(const QByteArray &orc_PubKeyTextDecoded,
                              QByteArray &orc_Modulus, QByteArray &orc_Exponent,
                              QString &orc_ErrorMessage);

protected:
  virtual int32_t m_ReadPrivateKey(const QByteArray &orc_FileContent,
                                   QString &orc_ErrorMessage);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
