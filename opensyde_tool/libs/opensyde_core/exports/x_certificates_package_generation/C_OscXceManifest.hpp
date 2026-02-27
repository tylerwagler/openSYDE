//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Xapp certificates package data
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXCEMANIFEST_HPP
#define C_OSCXCEMANIFEST_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <vector>

#include "C_OscXceUpdatePackageParameters.hpp"
#include <QList>
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

class C_OscXceManifest {
public:
  C_OscXceManifest();

  QString c_CertificatesPath;
  QList<C_OscXceUpdatePackageParameters> c_UpdatePackageParameters;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
