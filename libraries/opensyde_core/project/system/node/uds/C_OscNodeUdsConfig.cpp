//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       A node's UDS (ISO 14229) configuration: addressing, timing, routines and DTCs

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_SclChecksums.hpp"
#include "C_OscNodeUdsConfig.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const std::string C_OscNodeUdsConfig::hc_SEED_KEY_CONSTANT = "constant";

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
C_OscUdsRoutine::C_OscUdsRoutine(void) :
   u16_Identifier(0U),
   q_SupportsStart(true),
   q_SupportsStop(false),
   q_SupportsRequestResults(false)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscUdsRoutine::CalcHash(uint32_t & oru32_HashValue) const
{
   stw::scl::C_SclChecksums::CalcCRC32(&this->u16_Identifier, sizeof(this->u16_Identifier), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(this->c_Name.c_str(), this->c_Name.length(), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(this->c_Comment.c_str(), this->c_Comment.length(), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->q_SupportsStart, sizeof(this->q_SupportsStart), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->q_SupportsStop, sizeof(this->q_SupportsStop), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->q_SupportsRequestResults, sizeof(this->q_SupportsRequestResults),
                                       oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
C_OscUdsDtc::C_OscUdsDtc(void) :
   u32_Code(0U),
   u8_Severity(0U)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscUdsDtc::CalcHash(uint32_t & oru32_HashValue) const
{
   stw::scl::C_SclChecksums::CalcCRC32(&this->u32_Code, sizeof(this->u32_Code), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(this->c_Name.c_str(), this->c_Name.length(), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(this->c_Comment.c_str(), this->c_Comment.length(), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->u8_Severity, sizeof(this->u8_Severity), oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
C_OscNodeUdsConfig::C_OscNodeUdsConfig(void)
{
   this->Initialize();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Defaults: the usual 11 bit tester/ECU pair, padding on, ISO 14229-2 timing, default and extended session
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeUdsConfig::Initialize(void)
{
   u32_RequestId = 0x7E0U;
   u32_ResponseId = 0x7E8U;
   u32_FunctionalId = 0x7DFU;
   q_ExtendedId = false;
   q_PadFrames = true;
   u8_PadByte = 0xCCU;
   u32_P2Ms = 50U;
   u32_P2StarMs = 5000U;
   u32_S3ClientMs = 2000U;
   c_SupportedSessions = {0x01U, 0x03U};
   c_SecurityLevels.clear();
   c_SeedKeyAlgorithm = hc_SEED_KEY_CONSTANT;
   c_Routines.clear();
   c_Dtcs.clear();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeUdsConfig::CalcHash(uint32_t & oru32_HashValue) const
{
   stw::scl::C_SclChecksums::CalcCRC32(&this->u32_RequestId, sizeof(this->u32_RequestId), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->u32_ResponseId, sizeof(this->u32_ResponseId), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->u32_FunctionalId, sizeof(this->u32_FunctionalId), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->q_ExtendedId, sizeof(this->q_ExtendedId), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->q_PadFrames, sizeof(this->q_PadFrames), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->u8_PadByte, sizeof(this->u8_PadByte), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->u32_P2Ms, sizeof(this->u32_P2Ms), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->u32_P2StarMs, sizeof(this->u32_P2StarMs), oru32_HashValue);
   stw::scl::C_SclChecksums::CalcCRC32(&this->u32_S3ClientMs, sizeof(this->u32_S3ClientMs), oru32_HashValue);
   for (const uint8_t u8_Session : this->c_SupportedSessions)
   {
      stw::scl::C_SclChecksums::CalcCRC32(&u8_Session, sizeof(u8_Session), oru32_HashValue);
   }
   for (const uint8_t u8_Level : this->c_SecurityLevels)
   {
      stw::scl::C_SclChecksums::CalcCRC32(&u8_Level, sizeof(u8_Level), oru32_HashValue);
   }
   stw::scl::C_SclChecksums::CalcCRC32(this->c_SeedKeyAlgorithm.c_str(), this->c_SeedKeyAlgorithm.length(),
                                       oru32_HashValue);
   for (const C_OscUdsRoutine & rc_Routine : this->c_Routines)
   {
      rc_Routine.CalcHash(oru32_HashValue);
   }
   for (const C_OscUdsDtc & rc_Dtc : this->c_Dtcs)
   {
      rc_Dtc.CalcHash(oru32_HashValue);
   }
}
