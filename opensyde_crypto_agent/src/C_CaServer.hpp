//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE Crypto Agent server class

   Implements the TCP server to provide crypto agent service via TCP.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CASERVERHPP
#define C_CASERVERHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>
#include <vector>
#include "stwtypes.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

namespace stw::osy_crypto_agent
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_CaServer
{
public:
private:
protected:
   static const uint16_t mhu16_SERVER_PORT_DEFAULT = 50963U;

   //service protocol:
   //SOLVE_AUTH_CHALLENGE with all dynamic parts at max
   static const uint16_t mhu16_MAX_REQUEST_SIZE = 388U;

   static const uint8_t mhu8_SERVICE_ID_PING                 = 0x00U;
   static const uint8_t mhu8_SERVICE_ID_SHUTDOWN             = 0x01U;
   static const uint8_t mhu8_SERVICE_ID_SOLVE_AUTH_CHALLENGE = 0x02U;

   static const uint16_t mhu16_PROTOCOL_VERSION = 0x0001U;

   static const uint8_t mhu8_AUTH_VERSION = 0x01U;
   static const uint8_t mhu8_AUTH_ALGO_RSA_1024 = 0x00U;
   static const uint8_t mhu8_AUTH_MANUF_ID_STW = 0x00U;

   //result codes
   static const uint8_t mhu8_SERVICE_RESULT_NO_ERROR             = 0x00U; //yeah
   static const uint8_t mhu8_SERVICE_RESULT_SERVCE_NOT_SUPPORTED = 0x01U; //unknown service requested
   static const uint8_t mhu8_SERVICE_RESULT_MALFORMED_REQUEST    = 0x02U; //protocol error
   static const uint8_t mhu8_SERVICE_RESULT_FAILED               = 0x03U; //failed to perform requested service

   static bool mh_ReadUint8(const std::vector<uint8_t> & orc_Data, uint32_t & oru32_Pos, uint8_t & oru8_Value);
   static bool mh_ReadUint16BigEndian(const std::vector<uint8_t> & orc_Data, uint32_t & oru32_Pos,
                                      uint16_t & oru16_Value);
   static bool mh_ReadByteVector(const std::vector<uint8_t> & orc_Data, uint32_t & oru32_Pos,
                                 std::vector<uint8_t> & orc_Value, const uint16_t ou16_MaxSize);
   static bool mh_ReadString(const std::vector<uint8_t> & orc_Data, uint32_t & oru32_Pos, std::string & orc_Value,
                             const uint16_t ou16_MaxSize);
   static void mh_AppendUint16BigEndian(const uint16_t ou16_Value, std::vector<uint8_t> & orc_Data);
   static void mh_AppendString(const std::string & orc_Text, std::vector<uint8_t> & orc_Data);
   static void mh_AppendByteVector(const std::vector<uint8_t> & orc_Value, std::vector<uint8_t> & orc_Data);

   class C_InputParametersSolveAuthChallenge
   {
   public:
      uint8_t u8_RequestVersion;                            //for future protocol changes
      uint8_t u8_AuthAlgorithm;                             //requested algorithm; see mhu8_AUTH_ALGO_*
      uint8_t u8_SaLevel;                                   //desired security access level
      std::string c_ProjectName;                            //openSYDE project name (max. 255 chars)
      std::string c_DeviceType;                             //device type from system definition (max. 28 chars)
      std::string c_NodeName;                               //node name from system definition (max. 31 chars )
      uint8_t u8_SubNodeId;                                 //sub-node from system definition (for multi-cpu nodes)
      uint8_t u8_EcuSerialNumberManufacturerId;             //See mhu8_AUTH_MANUF_ID_*
      std::vector<uint8_t> c_EcuSerialNumber;               //serial number as reported by node (max. 29 bytes)
                                                            //for STW devices: 6 BCD bytes
      std::vector<uint8_t> c_ServerCertificateSerialNumber; //certificate serial number as reported by server node (max.
                                                            // 20 bytes)
      std::vector<uint8_t> c_ServerChallengeValue;          //authentication challenge value as reported by server node
                                                            //Max size for currently supported algorithms: 8
      C_InputParametersSolveAuthChallenge();
      int32_t FromBinary(const std::vector<uint8_t> & orc_BinaryData);
   };

   //to be called by platform driver if a request was received
   void m_HandleRequest(const std::vector<uint8_t> & orc_Request, std::vector<uint8_t> & orc_Response);

   // handler implementations for received service requests; implementation defines how to solve:
   virtual int32_t m_ServiceHandlerSolveAuthChallenge(const C_InputParametersSolveAuthChallenge & orc_Parameters,
                                                      std::vector<uint8_t> & orc_ResponseValue,
                                                      std::string & orc_ErrorDetail) = 0;
   virtual void m_Shutdown() = 0;

   // different implementation needed depending on target system
   virtual int32_t m_ExecuteListener(const uint16_t ou16_ServerPort, const uint8_t (&orau8_BindAddress)[4]) = 0;

   std::string mc_ApplicationName;
   std::string mc_ApplicationVersion;

public:
   C_CaServer();
   virtual ~C_CaServer();

   int32_t Execute(const uint16_t ou16_ServerPort, const uint8_t (&orau8_BindAddress)[4]);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}

#endif
