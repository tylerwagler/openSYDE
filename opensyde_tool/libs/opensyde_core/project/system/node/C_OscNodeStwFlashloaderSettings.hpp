//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       short description (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODESTWFLASHLOADEROPTIONS_HPP
#define C_OSCNODESTWFLASHLOADEROPTIONS_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "stwtypes.hpp"
#include <vector>

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

class C_OscNodeStwFlashloaderSettings {
public:
  C_OscNodeStwFlashloaderSettings(void);

  // lint
  // -sem(stw::opensyde_core::C_OscNodeStwFlashloaderSettings::Initialize,initializer)
  void Initialize(void);

  void CalcHash(uint32_t &oru32_HashValue) const;

  bool q_ResetMessageActive;
  bool q_ResetMessageExtendedId;
  uint32_t u32_ResetMessageId;
  uint8_t u8_ResetMessageDlc;
  QByteArray c_Data;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
