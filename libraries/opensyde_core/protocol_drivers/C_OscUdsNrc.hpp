//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       UDS (ISO 14229-1) negative response codes and their names

   One table for every place that names a negative response code: the openSYDE protocol driver, the
   CAN Monitor's openSYDE and UDS interpreters, and the update sequences that react to a specific code.
   Before this file each of those carried its own copy and the wording had already drifted.

   The names are the ones in ISO 14229-1 Table A.1, verbatim, including its hyphens.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCUDSNRC_HPP
#define C_OSCUDSNRC_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <cstdint>
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
///UDS negative response codes
class C_OscUdsNrc
{
public:
   static constexpr uint8_t hu8_POSITIVE_RESPONSE                               = 0x00U;
   static constexpr uint8_t hu8_GENERAL_REJECT                                  = 0x10U;
   static constexpr uint8_t hu8_SERVICE_NOT_SUPPORTED                           = 0x11U;
   static constexpr uint8_t hu8_SUB_FUNCTION_NOT_SUPPORTED                      = 0x12U;
   static constexpr uint8_t hu8_INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT      = 0x13U;
   static constexpr uint8_t hu8_RESPONSE_TOO_LONG                               = 0x14U;
   static constexpr uint8_t hu8_BUSY_REPEAT_REQUEST                             = 0x21U;
   static constexpr uint8_t hu8_CONDITIONS_NOT_CORRECT                          = 0x22U;
   static constexpr uint8_t hu8_REQUEST_SEQUENCE_ERROR                          = 0x24U;
   static constexpr uint8_t hu8_NO_RESPONSE_FROM_SUBNET_COMPONENT               = 0x25U;
   static constexpr uint8_t hu8_FAILURE_PREVENTS_EXECUTION_OF_REQUESTED_ACTION  = 0x26U;
   static constexpr uint8_t hu8_REQUEST_OUT_OF_RANGE                            = 0x31U;
   static constexpr uint8_t hu8_SECURITY_ACCESS_DENIED                          = 0x33U;
   static constexpr uint8_t hu8_AUTHENTICATION_REQUIRED                         = 0x34U;
   static constexpr uint8_t hu8_INVALID_KEY                                     = 0x35U;
   static constexpr uint8_t hu8_EXCEED_NUMBER_OF_ATTEMPTS                       = 0x36U;
   static constexpr uint8_t hu8_REQUIRED_TIME_DELAY_NOT_EXPIRED                 = 0x37U;
   static constexpr uint8_t hu8_SECURE_DATA_TRANSMISSION_REQUIRED               = 0x38U;
   static constexpr uint8_t hu8_SECURE_DATA_TRANSMISSION_NOT_ALLOWED            = 0x39U;
   static constexpr uint8_t hu8_SECURE_DATA_VERIFICATION_FAILED                 = 0x3AU;
   static constexpr uint8_t hu8_UPLOAD_DOWNLOAD_NOT_ACCEPTED                    = 0x70U;
   static constexpr uint8_t hu8_TRANSFER_DATA_SUSPENDED                         = 0x71U;
   static constexpr uint8_t hu8_GENERAL_PROGRAMMING_FAILURE                     = 0x72U;
   static constexpr uint8_t hu8_WRONG_BLOCK_SEQUENCE_COUNTER                    = 0x73U;
   static constexpr uint8_t hu8_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING     = 0x78U;
   static constexpr uint8_t hu8_SUB_FUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION    = 0x7EU;
   static constexpr uint8_t hu8_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION         = 0x7FU;
   static constexpr uint8_t hu8_RPM_TOO_HIGH                                    = 0x81U;
   static constexpr uint8_t hu8_RPM_TOO_LOW                                     = 0x82U;
   static constexpr uint8_t hu8_ENGINE_IS_RUNNING                               = 0x83U;
   static constexpr uint8_t hu8_ENGINE_IS_NOT_RUNNING                           = 0x84U;
   static constexpr uint8_t hu8_ENGINE_RUN_TIME_TOO_LOW                         = 0x85U;
   static constexpr uint8_t hu8_TEMPERATURE_TOO_HIGH                            = 0x86U;
   static constexpr uint8_t hu8_TEMPERATURE_TOO_LOW                             = 0x87U;
   static constexpr uint8_t hu8_VEHICLE_SPEED_TOO_HIGH                          = 0x88U;
   static constexpr uint8_t hu8_VEHICLE_SPEED_TOO_LOW                           = 0x89U;
   static constexpr uint8_t hu8_THROTTLE_PEDAL_TOO_HIGH                         = 0x8AU;
   static constexpr uint8_t hu8_THROTTLE_PEDAL_TOO_LOW                          = 0x8BU;
   static constexpr uint8_t hu8_TRANSMISSION_RANGE_NOT_IN_NEUTRAL               = 0x8CU;
   static constexpr uint8_t hu8_TRANSMISSION_RANGE_NOT_IN_GEAR                  = 0x8DU;
   static constexpr uint8_t hu8_BRAKE_SWITCHES_NOT_CLOSED                       = 0x8FU;
   static constexpr uint8_t hu8_SHIFTER_LEVER_NOT_IN_PARK                       = 0x90U;
   static constexpr uint8_t hu8_TORQUE_CONVERTER_CLUTCH_LOCKED                  = 0x91U;
   static constexpr uint8_t hu8_VOLTAGE_TOO_HIGH                                = 0x92U;
   static constexpr uint8_t hu8_VOLTAGE_TOO_LOW                                 = 0x93U;
   static constexpr uint8_t hu8_RESOURCE_TEMPORARILY_NOT_AVAILABLE              = 0x94U;

   static std::string h_ToText(const uint8_t ou8_Nrc);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
