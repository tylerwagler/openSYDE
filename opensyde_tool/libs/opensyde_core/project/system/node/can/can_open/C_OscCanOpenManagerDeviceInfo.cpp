//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for CANopen manager device specific data information

   Data class for CANopen manager device specific data information

   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"
#include <QFileInfo>

#include "C_OscCanOpenManagerDeviceInfo.hpp"
#include "C_OscHashUtil.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_SclChecksums.hpp"
#include "stwerrors.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
using namespace stw::opensyde_core;
using namespace stw::errors;
using namespace stw::scl;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */

/* -- Module Global Variables
 * ---------------------------------------------------------------------------------------
 */

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscCanOpenManagerDeviceInfo::C_OscCanOpenManagerDeviceInfo()
    : mq_EdsFileContentLoaded(false), q_DeviceOptional(false),
      q_NoInitialization(false), q_FactorySettingsActive(false),
      u8_ResetNodeObjectDictionarySubIndex(0), q_EnableHeartbeatProducing(true),
      u16_HeartbeatProducerTimeMs(100), q_UseOpenSydeNodeId(true),
      u8_NodeIdValue(0), q_EnableHeartbeatConsuming(true),
      u16_HeartbeatConsumerTimeMs(100),
      q_EnableHeartbeatConsumingAutoCalculation(true) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with unit [in] value and result
   [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerDeviceInfo::CalcHash(uint32_t & oru32_HashValue) const
{
   hash_util::CalcHashMembers(oru32_HashValue,
                              this->c_OriginalEdsFileName, this->mc_EdsFileContent,
                              this->q_DeviceOptional, this->q_NoInitialization,
                              this->q_FactorySettingsActive, this->u8_ResetNodeObjectDictionarySubIndex,
                              this->q_EnableHeartbeatProducing, this->u16_HeartbeatProducerTimeMs,
                              this->q_UseOpenSydeNodeId, this->u8_NodeIdValue,
                              this->q_EnableHeartbeatConsuming, this->u16_HeartbeatConsumerTimeMs,
                              this->q_EnableHeartbeatConsumingAutoCalculation);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load EDS file if needed and return reference to data

   Check whether EDS file is already loaded. If not do so.
   Return reference to loaded data.
   This dynamic approach is chosen as:
   * even with optimized loading routines parsing EDS files is slow due to the
   non-linear file structure
   * EDS file data is not needed by all applications

   Error handling:
   * problems when loading the file will be logged using the openSYDE core
   logger engine
   * in case of an error a reference to an empty OD will be returned

   \return  object dictionary data from loaded EDS file
*/
//----------------------------------------------------------------------------------------------------------------------
const C_OscCanOpenObjectDictionary &
C_OscCanOpenManagerDeviceInfo::GetEdsFileContent() const {
  if (this->mq_EdsFileContentLoaded == false) {
    if (QFileInfo(this->c_ProjectEdsFilePath).exists() &&
        QFileInfo(this->c_ProjectEdsFilePath).isFile()) {
      const int32_t s32_Retval = this->mc_EdsFileContent.LoadFromFile(
          this->c_ProjectEdsFilePath.toUtf8().constData());
      if (s32_Retval != C_NO_ERR) {
        osc_write_log_error("CANopen manager device information",
                            "Failed to load from EDS file \"" +
                                this->c_ProjectEdsFilePath + "\" Error: \"" +
                                this->mc_EdsFileContent.GetLastErrorText() +
                                "\".");
        this->mc_EdsFileContent.c_OdObjects.clear();
        this->mc_EdsFileContent.c_TextFileContent.clear();
      } else {
        osc_write_log_info("CANopen manager device information",
                           "Loaded EDS file \"" + this->c_ProjectEdsFilePath +
                               "\"");
      }
    } else {
      osc_write_log_error("CANopen manager device information",
                          "Failed to load from EDS file \"" +
                              this->c_ProjectEdsFilePath +
                              "\" Error: File does not exist.");
      this->mc_EdsFileContent.c_OdObjects.clear();
      this->mc_EdsFileContent.c_TextFileContent.clear();
    }
    this->mq_EdsFileContentLoaded = true;
  }

  return this->mc_EdsFileContent;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set EDS OD content from parameter

   Will only set the OD elements.
   All other elements can be accessed manually via the public fields.

   \param[in]  orc_NewContent   new content to store
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerDeviceInfo::SetEdsFileContent(
    const C_OscCanOpenObjectDictionary &orc_NewContent) {
  this->mc_EdsFileContent = orc_NewContent;
}
