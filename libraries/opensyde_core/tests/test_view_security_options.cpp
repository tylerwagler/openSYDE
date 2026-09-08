//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Regression tests for C_OscViewFiler's security option parsing

   These three functions map the strings stored in a .syde view file onto the
   per-node security settings: whether the debugger is enabled, whether
   authentication is required, and whether traffic is encrypted.

   They were inverted. Upstream compared with C_SclString::AnsiCompare, which
   returns 0 on equality:

       if (orc_String.AnsiCompare("activate") == 0)

   The phase 3b C_SclString retirement rewrote the call as `== "activate"` but
   left the trailing `== 0`, giving `(orc_String == "activate") == 0` — true when
   the string does NOT match. "activate" then fell through to the "deactivate"
   branch, which matched for the same inverted reason, so the two states came out
   swapped.

   GCC reports this as -Wparentheses when building the tool trees, but not under
   the core build's warning set, which is how it survived.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <string>

#include "stwtypes.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscViewFiler.hpp"
#include "C_OscViewNodeUpdate.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::errors;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

TEST(ViewSecurityOptions, DebuggerStatesAreNotSwapped)
{
   C_OscViewNodeUpdate::E_StateDebugger e_State = C_OscViewNodeUpdate::eST_DEB_NO_CHANGE;

   ASSERT_FALSE(static_cast<bool>(C_OscViewFiler::h_StringToSecurityOptionDebugger("activate", e_State)));
   EXPECT_EQ(C_OscViewNodeUpdate::eST_DEB_ACTIVATE, e_State) << "\"activate\" did not select ACTIVATE";

   ASSERT_FALSE(static_cast<bool>(C_OscViewFiler::h_StringToSecurityOptionDebugger("deactivate", e_State)));
   EXPECT_EQ(C_OscViewNodeUpdate::eST_DEB_DEACTIVATE, e_State) << "\"deactivate\" did not select DEACTIVATE";

   ASSERT_FALSE(static_cast<bool>(C_OscViewFiler::h_StringToSecurityOptionDebugger("no-change", e_State)));
   EXPECT_EQ(C_OscViewNodeUpdate::eST_DEB_NO_CHANGE, e_State);
}

TEST(ViewSecurityOptions, AuthenticationStatesAreNotSwapped)
{
   C_OscViewNodeUpdate::E_StateSecureAuthentication e_State = C_OscViewNodeUpdate::eST_SEC_NO_CHANGE;

   ASSERT_FALSE(static_cast<bool>(C_OscViewFiler::h_StringToSecurityOptionAuthentication("activate", e_State)));
   EXPECT_EQ(C_OscViewNodeUpdate::eST_SEC_ACTIVATE, e_State) << "\"activate\" did not select ACTIVATE";

   ASSERT_FALSE(static_cast<bool>(C_OscViewFiler::h_StringToSecurityOptionAuthentication("deactivate", e_State)));
   EXPECT_EQ(C_OscViewNodeUpdate::eST_SEC_DEACTIVATE, e_State) << "\"deactivate\" did not select DEACTIVATE";

   ASSERT_FALSE(static_cast<bool>(C_OscViewFiler::h_StringToSecurityOptionAuthentication("no-change", e_State)));
   EXPECT_EQ(C_OscViewNodeUpdate::eST_SEC_NO_CHANGE, e_State);
}

TEST(ViewSecurityOptions, EncryptionStatesAreNotSwapped)
{
   C_OscViewNodeUpdate::E_StateTrafficEncryption e_State = C_OscViewNodeUpdate::eST_TEN_NO_CHANGE;

   ASSERT_FALSE(static_cast<bool>(C_OscViewFiler::h_StringToSecurityOptionEncryption("activate", e_State)));
   EXPECT_EQ(C_OscViewNodeUpdate::eST_TEN_ACTIVATE, e_State) << "\"activate\" did not select ACTIVATE";

   ASSERT_FALSE(static_cast<bool>(C_OscViewFiler::h_StringToSecurityOptionEncryption("deactivate", e_State)));
   EXPECT_EQ(C_OscViewNodeUpdate::eST_TEN_DEACTIVATE, e_State) << "\"deactivate\" did not select DEACTIVATE";

   ASSERT_FALSE(static_cast<bool>(C_OscViewFiler::h_StringToSecurityOptionEncryption("no-change", e_State)));
   EXPECT_EQ(C_OscViewNodeUpdate::eST_TEN_NO_CHANGE, e_State);
}

/// An unrecognised string must report Errc::range and fall back to NO_CHANGE --
/// the safe default. Under the inverted comparison it matched the first branch
/// instead and silently returned success.
TEST(ViewSecurityOptions, UnknownStringReportsRangeAndDefaultsToNoChange)
{
   C_OscViewNodeUpdate::E_StateDebugger e_Debugger = C_OscViewNodeUpdate::eST_DEB_ACTIVATE;
   C_OscViewNodeUpdate::E_StateSecureAuthentication e_Auth = C_OscViewNodeUpdate::eST_SEC_ACTIVATE;
   C_OscViewNodeUpdate::E_StateTrafficEncryption e_Encryption = C_OscViewNodeUpdate::eST_TEN_ACTIVATE;

   EXPECT_EQ(Errc::range, C_OscViewFiler::h_StringToSecurityOptionDebugger("nonsense", e_Debugger));
   EXPECT_EQ(C_OscViewNodeUpdate::eST_DEB_NO_CHANGE, e_Debugger);

   EXPECT_EQ(Errc::range, C_OscViewFiler::h_StringToSecurityOptionAuthentication("nonsense", e_Auth));
   EXPECT_EQ(C_OscViewNodeUpdate::eST_SEC_NO_CHANGE, e_Auth);

   EXPECT_EQ(Errc::range, C_OscViewFiler::h_StringToSecurityOptionEncryption("nonsense", e_Encryption));
   EXPECT_EQ(C_OscViewNodeUpdate::eST_TEN_NO_CHANGE, e_Encryption);
}

/// The empty string is what a missing attribute yields; it must not be mistaken
/// for a valid option.
TEST(ViewSecurityOptions, EmptyStringIsRejected)
{
   C_OscViewNodeUpdate::E_StateDebugger e_State = C_OscViewNodeUpdate::eST_DEB_ACTIVATE;

   EXPECT_EQ(Errc::range, C_OscViewFiler::h_StringToSecurityOptionDebugger("", e_State));
   EXPECT_EQ(C_OscViewNodeUpdate::eST_DEB_NO_CHANGE, e_State);
}
