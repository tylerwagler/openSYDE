//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for signature
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSUPSIGNATUREFILER_HPP
#define C_OSCSUPSIGNATUREFILER_HPP

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

class C_OscSupSignatureFiler {
public:
  static int32_t h_CreateSignatureFile(const QString &orc_Path,
                                       const QString &orc_Signature);
  static int32_t h_LoadSignatureFile(const QString &orc_Path,
                                     QString &orc_Signature);
  static QString h_GetSignatureFileName(void);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
