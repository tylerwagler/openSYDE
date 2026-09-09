//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Regression tests for hex string parsing

   The C_SclString -> std::string migration replaced C_SclString::ToInt(), which
   auto-detects a "0x" prefix and parses hex, with std::stoi(), which defaults to
   base 10 and stops at the 'x'. Every affected site therefore produced 0 instead
   of the intended value.

   The worst instance was C_OscSecurityAesFile's key derivation: every byte of the
   AES key became 0, so the user's password was ignored entirely and all encrypted
   output used an all-zero key. ECDSA signature bytes, device serial numbers and
   CANopen EDS object indices were affected the same way.

   These tests pin the parsing behaviour so the substitution cannot silently
   reappear.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <string>
#include <stdexcept>
#include <clocale>

#include "stwtypes.hpp"
#include "C_SclStringUtil.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
using namespace stw::scl;

/* -- Implementation ------------------------------------------------------------------------------------------------ */

/// The exact defect: std::stoi() with a default base returns 0 for "0x.." input.
/// This documents why an explicit base is mandatory at every hex parse site.
TEST(HexStringParsing, StdStoiDefaultBaseDoesNotParseHexPrefix)
{
   EXPECT_EQ(0, std::stoi("0x41"));
   EXPECT_EQ(0, std::stoi("0xFF"));

   // With an explicit base 16 the "0x" prefix is accepted and parsed correctly.
   EXPECT_EQ(0x41, std::stoi("0x41", nullptr, 16));
   EXPECT_EQ(0xFF, std::stoi("0xFF", nullptr, 16));
}

TEST(HexStringParsing, ScanBaseCompatDetectsHexPrefix)
{
   EXPECT_EQ(16, ScanBaseCompat("0x1A"));
   EXPECT_EQ(16, ScanBaseCompat("0X1A"));
   EXPECT_EQ(16, ScanBaseCompat("-0x1A"));

   EXPECT_EQ(10, ScanBaseCompat("42"));
   EXPECT_EQ(10, ScanBaseCompat("-42"));
   EXPECT_EQ(10, ScanBaseCompat(""));
   EXPECT_EQ(10, ScanBaseCompat("0"));
}

TEST(HexStringParsing, ToIntCompatHandlesBothBases)
{
   EXPECT_EQ(255, ToIntCompat("0xFF"));
   EXPECT_EQ(255, ToIntCompat("0XFF"));
   EXPECT_EQ(255, ToIntCompat("255"));
   EXPECT_EQ(-255, ToIntCompat("-0xFF"));
   EXPECT_EQ(-255, ToIntCompat("-255"));
}

/// C_SclString::ToInt() explicitly did NOT treat a leading zero as octal.
/// strtol with base 0 would, which is why ScanBaseCompat never returns 0.
TEST(HexStringParsing, LeadingZeroIsNotOctal)
{
   EXPECT_EQ(10, ToIntCompat("010"));
   EXPECT_EQ(8, ToIntCompat("08"));
   EXPECT_EQ(7, ToIntCompat("007"));
}

TEST(HexStringParsing, ToIntDefCompatReturnsDefaultOnGarbage)
{
   EXPECT_EQ(-1, ToIntDefCompat("", -1));
   EXPECT_EQ(-1, ToIntDefCompat("not a number", -1));
   EXPECT_EQ(0x2A, ToIntDefCompat("0x2A", -1));
   EXPECT_EQ(42, ToIntDefCompat("42", -1));
}

TEST(HexStringParsing, ToInt64CompatHandlesBothBases)
{
   EXPECT_EQ(static_cast<int64_t>(0xABCDEF), ToInt64Compat("0xABCDEF"));
   EXPECT_EQ(static_cast<int64_t>(11259375), ToInt64Compat("11259375"));
}

/// Reproduces the key-derivation loop from C_OscSecurityAesFile::mh_HexToKey.
/// Before the fix every byte came out 0 regardless of the password supplied.
TEST(HexStringParsing, AesKeyDerivationProducesNonZeroKey)
{
   const std::string c_Password = "0123456789ABCDEF0123456789ABCDEF";
   uint8_t au8_Key[16];

   for (uint8_t u8_Index = 0U; u8_Index < 16U; u8_Index++)
   {
      const std::string c_Text = "0x" + SubStringCompat(c_Password,
                                                        (static_cast<uint32_t>(u8_Index) * 2U) + 1U, 2U);
      au8_Key[u8_Index] = static_cast<uint8_t>(std::stoi(c_Text, nullptr, 16));
   }

   const uint8_t au8_Expected[16] =
   {
      0x01U, 0x23U, 0x45U, 0x67U, 0x89U, 0xABU, 0xCDU, 0xEFU,
      0x01U, 0x23U, 0x45U, 0x67U, 0x89U, 0xABU, 0xCDU, 0xEFU
   };

   for (uint8_t u8_Index = 0U; u8_Index < 16U; u8_Index++)
   {
      EXPECT_EQ(au8_Expected[u8_Index], au8_Key[u8_Index]) << "key byte " << static_cast<int>(u8_Index);
   }

   // The regression signature: the whole key collapsing to zero.
   bool q_AllZero = true;
   for (uint8_t u8_Index = 0U; u8_Index < 16U; u8_Index++)
   {
      if (au8_Key[u8_Index] != 0U)
      {
         q_AllZero = false;
      }
   }
   EXPECT_FALSE(q_AllZero) << "AES key derived as all zeros — the password is being ignored";
}

/// Two different passwords must not derive the same key. Under the defect they
/// both derived all zeros, so any password decrypted any file.
TEST(HexStringParsing, DifferentPasswordsDeriveDifferentKeys)
{
   const std::string c_PasswordA = "00112233445566778899AABBCCDDEEFF";
   const std::string c_PasswordB = "FFEEDDCCBBAA99887766554433221100";

   uint8_t au8_KeyA[16];
   uint8_t au8_KeyB[16];

   for (uint8_t u8_Index = 0U; u8_Index < 16U; u8_Index++)
   {
      const uint32_t u32_Pos = (static_cast<uint32_t>(u8_Index) * 2U) + 1U;
      au8_KeyA[u8_Index] = static_cast<uint8_t>(std::stoi("0x" + SubStringCompat(c_PasswordA, u32_Pos, 2U),
                                                          nullptr, 16));
      au8_KeyB[u8_Index] = static_cast<uint8_t>(std::stoi("0x" + SubStringCompat(c_PasswordB, u32_Pos, 2U),
                                                          nullptr, 16));
   }

   bool q_Identical = true;
   for (uint8_t u8_Index = 0U; u8_Index < 16U; u8_Index++)
   {
      if (au8_KeyA[u8_Index] != au8_KeyB[u8_Index])
      {
         q_Identical = false;
      }
   }
   EXPECT_FALSE(q_Identical) << "distinct passwords derived identical keys";
}

/// ToIntCompat has to throw as well as honour the base. C_SclString::ToInt() did
/// both: callers wrapped it in try/catch to answer "is this field a number?".
/// The migration split that in two and lost one half at every site - std::stoi()
/// throws but ignores the base, std::strtol() honours the base but never throws.
TEST(HexStringParsing, ToIntCompatThrowsOnNonNumericInput)
{
   EXPECT_THROW(ToIntCompat(""), std::invalid_argument);
   EXPECT_THROW(ToIntCompat("not a number"), std::invalid_argument);
   EXPECT_THROW(ToIntCompat("0x"), std::invalid_argument);
   EXPECT_THROW(ToIntCompat("-"), std::invalid_argument);
}

/// A partial parse answers the caller's question wrongly: std::stoi("1000junk")
/// is 1000 with no complaint, so a malformed field reads as valid.
TEST(HexStringParsing, ToIntCompatRejectsTrailingCharacters)
{
   EXPECT_THROW(ToIntCompat("1000junk"), std::invalid_argument);
   EXPECT_THROW(ToIntCompat("0x10ZZ"), std::invalid_argument);
   EXPECT_THROW(ToIntCompat("12 34"), std::invalid_argument);

   // sanity: std::stoi is the lax behaviour being rejected here
   EXPECT_EQ(1000, std::stoi("1000junk"));
}

TEST(HexStringParsing, ToIntCompatThrowsOnOverflow)
{
   EXPECT_THROW(ToIntCompat("99999999999999999999"), std::out_of_range);
   EXPECT_THROW(ToIntCompat("0xFFFFFFFFFFFFFFFFFF"), std::out_of_range);

   // the boundaries themselves must still parse
   EXPECT_EQ(2147483647, ToIntCompat("2147483647"));
   EXPECT_EQ(-2147483648, ToIntCompat("-2147483648"));
}

TEST(HexStringParsing, ToInt64CompatThrowsOnNonNumericInput)
{
   EXPECT_THROW(ToInt64Compat(""), std::invalid_argument);
   EXPECT_THROW(ToInt64Compat("nope"), std::invalid_argument);
   EXPECT_THROW(ToInt64Compat("123abc"), std::invalid_argument);
}

/// CiA 306 writes object indices in the MandatoryObjects / OptionalObjects /
/// ManufacturerObjects blocks as "0x"-prefixed hex:
///
///   [MandatoryObjects]
///   SupportedObjects=2
///   1=0x1000
///   2=0x1018
///
/// C_OscCanOpenObjectDictionary keys its object map by hex-parsed section names,
/// then cross-checks those entries. Under std::stoi every entry resolved to 0,
/// so the lookup missed and a conforming EDS file was reported as referencing a
/// non-existent object.
TEST(HexStringParsing, EdsSupportedObjectIndicesParseAsHex)
{
   EXPECT_EQ(0x1000, ToIntCompat("0x1000"));
   EXPECT_EQ(0x1018, ToIntCompat("0x1018"));
   EXPECT_EQ(0x6040, ToIntCompat("0x6040"));

   // EDS DataType fields are hex too, e.g. DataType=0x0007 (UNSIGNED32)
   EXPECT_EQ(7, ToIntCompat("0x0007"));

   // decimal spellings must keep working - both appear in the wild
   EXPECT_EQ(4096, ToIntCompat("4096"));

   // and this is what the defect produced for every one of them
   EXPECT_EQ(0, std::stoi("0x1018"));
}

/// The migration replaced C_SclString::ToDouble() with strtod()/stod(), which
/// take their decimal separator from the *current* C locale. Qt sets that from
/// the environment at application startup, so on a German or French desktop
/// "1.5" parsed as 1 - and every number in a .syde project, a DBC file or an EDS
/// file is written with a '.'. Silent truncation of real data, in exactly the
/// locales most of openSYDE's users are in.
TEST(HexStringParsing, ToDoubleCompatIsLocaleIndependent)
{
   const char * const pcn_Original = std::setlocale(LC_NUMERIC, nullptr);
   const std::string c_Saved = (pcn_Original != nullptr) ? pcn_Original : "C";

   // Try a few spellings; CI images do not all generate the same locales.
   const char * const apcn_Candidates[] = {"de_DE.UTF-8", "de_DE.utf8", "de_DE", "fr_FR.UTF-8"};
   bool q_LocaleApplied = false;
   for (const char * const pcn_Candidate : apcn_Candidates)
   {
      if (std::setlocale(LC_NUMERIC, pcn_Candidate) != nullptr)
      {
         q_LocaleApplied = true;
         break;
      }
   }

   if (q_LocaleApplied)
   {
      // strtod is the behaviour being rejected: under a comma locale it stops at
      // the '.'. If this ever stops holding, the C library changed, not us.
      EXPECT_DOUBLE_EQ(1.0, std::strtod("1.5", nullptr)) << "comma locale did not take effect";

      EXPECT_DOUBLE_EQ(1.5, ToDoubleCompat("1.5")) << "ToDoubleCompat followed the process locale";
      EXPECT_DOUBLE_EQ(-2.25, ToDoubleCompat("-2.25"));
   }

   std::setlocale(LC_NUMERIC, c_Saved.c_str());

   // Holds regardless of whether a comma locale was available.
   EXPECT_DOUBLE_EQ(1.5, ToDoubleCompat("1.5"));
   if (!q_LocaleApplied)
   {
      GTEST_SKIP() << "no comma-decimal locale available; only the C-locale path was exercised";
   }
}

/// The original replaced the first comma with a '.', so a hand-entered "1,5"
/// still parsed. Worth keeping for a tool whose users type in those locales.
TEST(HexStringParsing, ToDoubleCompatAcceptsCommaDecimalSeparator)
{
   EXPECT_DOUBLE_EQ(1.5, ToDoubleCompat("1,5"));
   EXPECT_DOUBLE_EQ(-0.25, ToDoubleCompat("-0,25"));

   // only the first comma is replaced, matching the original: parsing then stops
   EXPECT_DOUBLE_EQ(1.234, ToDoubleCompat("1,234,5"));
}

/// strtod returns 0.0 for garbage, so a malformed value read as a legitimate
/// zero. The original threw, and callers wrap it to detect exactly that.
TEST(HexStringParsing, ToDoubleCompatThrowsOnNonNumericInput)
{
   EXPECT_THROW(ToDoubleCompat(""), std::invalid_argument);
   EXPECT_THROW(ToDoubleCompat("not a number"), std::invalid_argument);
   EXPECT_THROW(ToDoubleCompat("abc1.5"), std::invalid_argument);

   // a real zero still parses, and must not be confused with the failure value
   EXPECT_DOUBLE_EQ(0.0, ToDoubleCompat("0"));
   EXPECT_DOUBLE_EQ(0.0, ToDoubleCompat("0.0"));
}

/// Deliberately laxer than ToIntCompat: the original extracted through a stream
/// and stopped at the first unusable character. Kept so project files that have
/// always loaded keep loading.
TEST(HexStringParsing, ToDoubleCompatToleratesTrailingCharacters)
{
   EXPECT_DOUBLE_EQ(1.5, ToDoubleCompat("1.5abc"));
   EXPECT_DOUBLE_EQ(42.0, ToDoubleCompat("42 units"));
}

/// The pair has to round-trip: a value written into a project file must read
/// back as the same value on a machine with a different locale. Streams take
/// their separator from the global std::locale rather than the C locale, so this
/// was not actually broken - but nothing pinned it either, and one call to
/// std::locale::global() anywhere would have silently started writing "1,5"
/// into .syde files. Both directions are now explicit, and this test says so.
TEST(HexStringParsing, FloatRoundTripsUnderAnyLocale)
{
   const char * const pcn_Original = std::setlocale(LC_NUMERIC, nullptr);
   const std::string c_Saved = (pcn_Original != nullptr) ? pcn_Original : "C";

   const char * const apcn_Candidates[] = {"de_DE.UTF-8", "de_DE.utf8", "de_DE", "fr_FR.UTF-8"};
   for (const char * const pcn_Candidate : apcn_Candidates)
   {
      if (std::setlocale(LC_NUMERIC, pcn_Candidate) != nullptr)
      {
         break;
      }
   }

   const double af64_Values[] = {1.5, -2.25, 0.0, 1234.5678, -0.001};
   for (const double f64_Value : af64_Values)
   {
      const std::string c_Written = FloatToStrCompat(f64_Value, 6);
      EXPECT_EQ(std::string::npos, c_Written.find(',')) << "wrote a comma decimal separator: " << c_Written;
      EXPECT_DOUBLE_EQ(f64_Value, ToDoubleCompat(c_Written)) << "round trip failed for " << c_Written;
   }

   std::setlocale(LC_NUMERIC, c_Saved.c_str());
}
