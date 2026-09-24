//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Tests for C_OscCryptoAgentAccessUtil against a loopback mock agent

   A minimal TCP server on 127.0.0.1 plays the crypto agent: it captures the request the client sends and answers
   with a canned response. This pins the wire format of the RSA-signature request and the client's handling of the
   success, failure and no-agent cases. POSIX sockets only; skipped on Windows.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>
#include <string>
#include <thread>
#include <cstdint>
#include "gtest/gtest.h"
#include "C_OscErrorCategory.hpp"
#include "C_OscNode.hpp"
#include "C_OscDeviceDefinition.hpp"
#include "C_OscCryptoAgentAccessUtil.hpp"

#ifndef _WIN32
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Types --------------------------------------------------------------------------------------------------------- */

namespace
{
#ifndef _WIN32
/// One-shot mock agent: accepts one connection, reads one size-prefixed request, sends the canned response.
class C_MockAgent
{
public:
   explicit C_MockAgent(const std::vector<uint8_t> & orc_Response) :
      mc_Response(orc_Response)
   {
      sockaddr_in c_Addr = {};

      ms32_Listen = socket(AF_INET, SOCK_STREAM, 0);
      c_Addr.sin_family = AF_INET;
      c_Addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      c_Addr.sin_port = 0; //let the OS choose
      (void)bind(ms32_Listen, reinterpret_cast<sockaddr *>(&c_Addr), sizeof(c_Addr));
      (void)listen(ms32_Listen, 1);
      socklen_t x_Len = sizeof(c_Addr);
      (void)getsockname(ms32_Listen, reinterpret_cast<sockaddr *>(&c_Addr), &x_Len);
      mu16_Port = ntohs(c_Addr.sin_port);
      mc_Thread = std::thread([this]() { m_Serve(); });
   }

   ~C_MockAgent()
   {
      mc_Thread.join();
      (void)close(ms32_Listen);
   }

   uint16_t mu16_Port = 0U;
   std::vector<uint8_t> c_Request; ///< full request including the 4-byte size prefix

private:
   int ms32_Listen = -1;
   std::vector<uint8_t> mc_Response;
   std::thread mc_Thread;

   static bool mh_ReadExact(const int os32_Socket, std::vector<uint8_t> & orc_Buffer, const std::size_t ou32_Count)
   {
      std::size_t u32_Read = 0U;
      const std::size_t u32_Start = orc_Buffer.size();

      orc_Buffer.resize(u32_Start + ou32_Count);
      while (u32_Read < ou32_Count)
      {
         const ssize_t x_Ret = recv(os32_Socket, &orc_Buffer[u32_Start + u32_Read], ou32_Count - u32_Read, 0);
         if (x_Ret <= 0)
         {
            return false;
         }
         u32_Read += static_cast<std::size_t>(x_Ret);
      }
      return true;
   }

   void m_Serve()
   {
      const int s32_Client = accept(ms32_Listen, NULL, NULL);

      if (s32_Client >= 0)
      {
         if (mh_ReadExact(s32_Client, c_Request, 4U))
         {
            const std::size_t u32_Size = (static_cast<std::size_t>(c_Request[0]) << 24U) |
                                         (static_cast<std::size_t>(c_Request[1]) << 16U) |
                                         (static_cast<std::size_t>(c_Request[2]) << 8U) |
                                         static_cast<std::size_t>(c_Request[3]);
            if (mh_ReadExact(s32_Client, c_Request, u32_Size))
            {
               (void)send(s32_Client, mc_Response.data(), mc_Response.size(), 0);
            }
         }
         (void)close(s32_Client);
      }
   }
};

/// Response to a signature request: 4-byte size, service, result, 2-byte signature size, signature; padded to the
/// 136 bytes the client reads.
std::vector<uint8_t> h_SignatureResponse(const uint8_t ou8_Result, const std::vector<uint8_t> & orc_Signature)
{
   std::vector<uint8_t> c_Response = { 0U, 0U, 0U, 132U, 2U, ou8_Result,
                                       static_cast<uint8_t>(orc_Signature.size() >> 8U),
                                       static_cast<uint8_t>(orc_Signature.size() & 0xFFU) };

   c_Response.insert(c_Response.end(), orc_Signature.begin(), orc_Signature.end());
   c_Response.resize(136U, 0U);
   return c_Response;
}

void h_UsePort(const uint16_t ou16_Port)
{
   C_OscCryptoAgentSettings c_Settings;

   c_Settings.u16_CryptoAgentPort = ou16_Port;
   c_Settings.q_CryptoAgentAutoStart = false;
   c_Settings.q_CryptoAgentAutoStop = false;
   C_OscCryptoAgentAccessUtil::h_SetCryptoAgentSettings(c_Settings);
}

/// Reads a 2-byte size-prefixed string at orc_Pos and advances it.
std::string h_ReadSizedString(const std::vector<uint8_t> & orc_Data, std::size_t & oru32_Pos)
{
   const std::size_t u32_Size = (static_cast<std::size_t>(orc_Data[oru32_Pos]) << 8U) | orc_Data[oru32_Pos + 1U];
   const std::string c_Result(orc_Data.begin() + static_cast<std::ptrdiff_t>(oru32_Pos + 2U),
                              orc_Data.begin() + static_cast<std::ptrdiff_t>(oru32_Pos + 2U + u32_Size));

   oru32_Pos += 2U + u32_Size;
   return c_Result;
}
#endif
}

/* -- Tests --------------------------------------------------------------------------------------------------------- */

TEST(CryptoAgentAccess, SignatureRequestEncodingAndSuccess)
{
#ifdef _WIN32
   GTEST_SKIP() << "mock agent uses POSIX sockets";
#else
   std::vector<uint8_t> c_ExpectedSignature(128U);
   for (std::size_t u32_It = 0U; u32_It < c_ExpectedSignature.size(); ++u32_It)
   {
      c_ExpectedSignature[u32_It] = static_cast<uint8_t>(u32_It);
   }
   C_MockAgent c_Agent(h_SignatureResponse(0U, c_ExpectedSignature));
   h_UsePort(c_Agent.mu16_Port);

   C_OscDeviceDefinition c_Device;
   c_Device.c_DeviceName = "ESX3CM";
   C_OscNode c_Node;
   c_Node.pc_DeviceDefinition = &c_Device;
   c_Node.c_Properties.c_Name = "Node1";

   std::vector<uint8_t> c_Signature;
   const std::error_code c_Result = C_OscCryptoAgentAccessUtil::h_GetRsaSignatureBySerialNumber(
      { 0xAAU, 0xBBU }, 5U, { 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U }, c_Signature, "/proj/dir/MyProject.syde_sysdef",
      c_Node, 7U, "SN-123", 2U);

   EXPECT_EQ(c_Result, Errc::success);
   EXPECT_EQ(c_Signature, c_ExpectedSignature);

   const std::vector<uint8_t> & rc_Req = c_Agent.c_Request;
   ASSERT_GE(rc_Req.size(), 8U);
   EXPECT_EQ(rc_Req[3], static_cast<uint8_t>(rc_Req.size() - 4U)); //size prefix excludes itself
   EXPECT_EQ(rc_Req[4], 2U); //service: RSA signature by serial number
   EXPECT_EQ(rc_Req[5], 1U); //request version
   EXPECT_EQ(rc_Req[6], 0U); //RSA 1024
   EXPECT_EQ(rc_Req[7], 5U); //security level
   std::size_t u32_Pos = 8U;
   EXPECT_EQ(h_ReadSizedString(rc_Req, u32_Pos), "MyProject"); //extension stripped
   EXPECT_EQ(h_ReadSizedString(rc_Req, u32_Pos), "ESX3CM");
   EXPECT_EQ(h_ReadSizedString(rc_Req, u32_Pos), "Node1");
   EXPECT_EQ(rc_Req[u32_Pos], 7U); //node id
   EXPECT_EQ(rc_Req[u32_Pos + 1U], 2U); //manufacturer format
   u32_Pos += 2U;
   EXPECT_EQ(h_ReadSizedString(rc_Req, u32_Pos), "SN-123");
   EXPECT_EQ(h_ReadSizedString(rc_Req, u32_Pos), std::string("\xAA\xBB"));
   EXPECT_EQ(h_ReadSizedString(rc_Req, u32_Pos), std::string("\x01\x02\x03\x04\x05\x06\x07\x08"));
   EXPECT_EQ(u32_Pos, rc_Req.size());
#endif
}

TEST(CryptoAgentAccess, AgentReportsFailure)
{
#ifdef _WIN32
   GTEST_SKIP() << "mock agent uses POSIX sockets";
#else
   C_MockAgent c_Agent(h_SignatureResponse(3U, {}));
   h_UsePort(c_Agent.mu16_Port);

   const C_OscNode c_Node;
   std::vector<uint8_t> c_Signature;
   const std::error_code c_Result = C_OscCryptoAgentAccessUtil::h_GetRsaSignatureBySerialNumber(
      { 1U }, 1U, { 1U }, c_Signature, "p.syde_sysdef", c_Node, 1U, "", 0U);

   EXPECT_EQ(c_Result, Errc::com);
   EXPECT_TRUE(c_Signature.empty());
#endif
}

TEST(CryptoAgentAccess, NoAgentListeningIsNoact)
{
#ifdef _WIN32
   GTEST_SKIP() << "mock agent uses POSIX sockets";
#else
   //reserve a free port, then release it so nothing is listening there
   uint16_t u16_Port;
   {
      const int s32_Socket = socket(AF_INET, SOCK_STREAM, 0);
      sockaddr_in c_Addr = {};
      c_Addr.sin_family = AF_INET;
      c_Addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
      (void)bind(s32_Socket, reinterpret_cast<sockaddr *>(&c_Addr), sizeof(c_Addr));
      socklen_t x_Len = sizeof(c_Addr);
      (void)getsockname(s32_Socket, reinterpret_cast<sockaddr *>(&c_Addr), &x_Len);
      u16_Port = ntohs(c_Addr.sin_port);
      (void)close(s32_Socket);
   }
   h_UsePort(u16_Port);

   const C_OscNode c_Node;
   std::vector<uint8_t> c_Signature;
   const std::error_code c_Result = C_OscCryptoAgentAccessUtil::h_GetRsaSignatureBySerialNumber(
      { 1U }, 1U, { 1U }, c_Signature, "p.syde_sysdef", c_Node, 1U, "", 0U);

   EXPECT_EQ(c_Result, Errc::noact);
#endif
}
