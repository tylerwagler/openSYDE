//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       UDS (ISO 14229-1) negative response codes and their names

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <format>

#include "C_OscUdsNrc.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
namespace
{
struct T_NrcName
{
   uint8_t u8_Code;
   const char * pcn_Name;
};

///ISO 14229-1 Table A.1, in code order
constexpr T_NrcName hac_NAMES[] =
{
   { C_OscUdsNrc::hu8_POSITIVE_RESPONSE,                              "positiveResponse"                              },
   { C_OscUdsNrc::hu8_GENERAL_REJECT,                                 "generalReject"                                 },
   { C_OscUdsNrc::hu8_SERVICE_NOT_SUPPORTED,                          "serviceNotSupported"                           },
   { C_OscUdsNrc::hu8_SUB_FUNCTION_NOT_SUPPORTED,                     "sub-functionNotSupported"                      },
   { C_OscUdsNrc::hu8_INCORRECT_MESSAGE_LENGTH_OR_INVALID_FORMAT,     "incorrectMessageLengthOrInvalidFormat"         },
   { C_OscUdsNrc::hu8_RESPONSE_TOO_LONG,                              "responseTooLong"                               },
   { C_OscUdsNrc::hu8_BUSY_REPEAT_REQUEST,                            "busyRepeatRequest"                             },
   { C_OscUdsNrc::hu8_CONDITIONS_NOT_CORRECT,                         "conditionsNotCorrect"                          },
   { C_OscUdsNrc::hu8_REQUEST_SEQUENCE_ERROR,                         "requestSequenceError"                          },
   { C_OscUdsNrc::hu8_NO_RESPONSE_FROM_SUBNET_COMPONENT,              "noResponseFromSubnetComponent"                 },
   { C_OscUdsNrc::hu8_FAILURE_PREVENTS_EXECUTION_OF_REQUESTED_ACTION, "failurePreventsExecutionOfRequestedAction"     },
   { C_OscUdsNrc::hu8_REQUEST_OUT_OF_RANGE,                           "requestOutOfRange"                             },
   { C_OscUdsNrc::hu8_SECURITY_ACCESS_DENIED,                         "securityAccessDenied"                          },
   { C_OscUdsNrc::hu8_AUTHENTICATION_REQUIRED,                        "authenticationRequired"                        },
   { C_OscUdsNrc::hu8_INVALID_KEY,                                    "invalidKey"                                    },
   { C_OscUdsNrc::hu8_EXCEED_NUMBER_OF_ATTEMPTS,                      "exceedNumberOfAttempts"                        },
   { C_OscUdsNrc::hu8_REQUIRED_TIME_DELAY_NOT_EXPIRED,                "requiredTimeDelayNotExpired"                   },
   { C_OscUdsNrc::hu8_SECURE_DATA_TRANSMISSION_REQUIRED,              "secureDataTransmissionRequired"                },
   { C_OscUdsNrc::hu8_SECURE_DATA_TRANSMISSION_NOT_ALLOWED,           "secureDataTransmissionNotAllowed"              },
   { C_OscUdsNrc::hu8_SECURE_DATA_VERIFICATION_FAILED,                "secureDataVerificationFailed"                  },
   { C_OscUdsNrc::hu8_UPLOAD_DOWNLOAD_NOT_ACCEPTED,                   "uploadDownloadNotAccepted"                     },
   { C_OscUdsNrc::hu8_TRANSFER_DATA_SUSPENDED,                        "transferDataSuspended"                         },
   { C_OscUdsNrc::hu8_GENERAL_PROGRAMMING_FAILURE,                    "generalProgrammingFailure"                     },
   { C_OscUdsNrc::hu8_WRONG_BLOCK_SEQUENCE_COUNTER,                   "wrongBlockSequenceCounter"                     },
   { C_OscUdsNrc::hu8_REQUEST_CORRECTLY_RECEIVED_RESPONSE_PENDING,    "requestCorrectlyReceived-ResponsePending"      },
   { C_OscUdsNrc::hu8_SUB_FUNCTION_NOT_SUPPORTED_IN_ACTIVE_SESSION,   "sub-functionNotSupportedInActiveSession"       },
   { C_OscUdsNrc::hu8_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION,        "serviceNotSupportedInActiveSession"            },
   { C_OscUdsNrc::hu8_RPM_TOO_HIGH,                                   "rpmTooHigh"                                    },
   { C_OscUdsNrc::hu8_RPM_TOO_LOW,                                    "rpmTooLow"                                     },
   { C_OscUdsNrc::hu8_ENGINE_IS_RUNNING,                              "engineIsRunning"                               },
   { C_OscUdsNrc::hu8_ENGINE_IS_NOT_RUNNING,                          "engineIsNotRunning"                            },
   { C_OscUdsNrc::hu8_ENGINE_RUN_TIME_TOO_LOW,                        "engineRunTimeTooLow"                           },
   { C_OscUdsNrc::hu8_TEMPERATURE_TOO_HIGH,                           "temperatureTooHigh"                            },
   { C_OscUdsNrc::hu8_TEMPERATURE_TOO_LOW,                            "temperatureTooLow"                             },
   { C_OscUdsNrc::hu8_VEHICLE_SPEED_TOO_HIGH,                         "vehicleSpeedTooHigh"                           },
   { C_OscUdsNrc::hu8_VEHICLE_SPEED_TOO_LOW,                          "vehicleSpeedTooLow"                            },
   { C_OscUdsNrc::hu8_THROTTLE_PEDAL_TOO_HIGH,                        "throttle/PedalTooHigh"                         },
   { C_OscUdsNrc::hu8_THROTTLE_PEDAL_TOO_LOW,                         "throttle/PedalTooLow"                          },
   { C_OscUdsNrc::hu8_TRANSMISSION_RANGE_NOT_IN_NEUTRAL,              "transmissionRangeNotInNeutral"                 },
   { C_OscUdsNrc::hu8_TRANSMISSION_RANGE_NOT_IN_GEAR,                 "transmissionRangeNotInGear"                    },
   { C_OscUdsNrc::hu8_BRAKE_SWITCHES_NOT_CLOSED,                      "brakeSwitch(es)NotClosed"                      },
   { C_OscUdsNrc::hu8_SHIFTER_LEVER_NOT_IN_PARK,                      "shifterLeverNotInPark"                         },
   { C_OscUdsNrc::hu8_TORQUE_CONVERTER_CLUTCH_LOCKED,                 "torqueConverterClutchLocked"                   },
   { C_OscUdsNrc::hu8_VOLTAGE_TOO_HIGH,                               "voltageTooHigh"                                },
   { C_OscUdsNrc::hu8_VOLTAGE_TOO_LOW,                                "voltageTooLow"                                 },
   { C_OscUdsNrc::hu8_RESOURCE_TEMPORARILY_NOT_AVAILABLE,             "resourceTemporarilyNotAvailable"               }
};
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Name of a negative response code

   \param[in]  ou8_Nrc  negative response code

   \return
   ISO 14229-1 name; "unknownNegativeResponseCode(0xNN)" for a code the standard does not define
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscUdsNrc::h_ToText(const uint8_t ou8_Nrc)
{
   for (const T_NrcName & rc_Entry : hac_NAMES)
   {
      if (rc_Entry.u8_Code == ou8_Nrc)
      {
         return rc_Entry.pcn_Name;
      }
   }
   return std::format("unknownNegativeResponseCode(0x{:02X})", ou8_Nrc);
}
