//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Message data structure with additional UI/ configuration info
   (header)

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CAMPROMESSAGEDATA_HPP
#define C_CAMPROMESSAGEDATA_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */

#include "stw_can.hpp"
#include "stwtypes.hpp"
#include <QString>
#include <array>

/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_gui_logic {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_CamProMessageData {
public:
  C_CamProMessageData(void);

  QString c_DataBaseFilePath;
  QString c_Name;
  bool q_ContainsValidHash;
  uint32_t u32_Hash;
  bool q_DoCyclicTrigger;
  uint32_t u32_CyclicTriggerTime;
  QString c_Key;
  uint32_t u32_KeyPressOffset;
  bool q_IsExtended;
  bool q_IsRtr;
  uint16_t u16_Dlc;
  uint32_t u32_Id;
  QList<uint8_t> c_Bytes;

  int32_t SetMessageDataBytes(const QList<uint8_t> &orc_DataBytes);
  static uint8_t h_GetBoolValue(const bool oq_Value);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_gui_logic
} // namespace stw

#endif
