//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Float attributes must not follow the process locale

   tinyxml2 formats and parses floating point through snprintf and sscanf, both of which
   honour LC_NUMERIC. openSYDE never calls setlocale itself, so it inherits whatever Qt
   set from the environment. On a German or French desktop that made the XML writer emit

       <element factor="1,5"/>

   which is still well-formed XML. Read back anywhere else, sscanf stops at the comma and
   the value silently becomes 1. Every f64_Factor, f64_Offset, min, max and float Datapool
   value in a shared project was exposed.

   C_SclStringUtil already defended its own parsing against exactly this with strtod_l
   against an explicit "C" locale; the XML parser had been left out.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <gtest/gtest.h>

#include <clocale>
#include <cstdlib>
#include <string>

#include <cstdint>
#include "C_OscXmlParser.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

namespace
{
/// Applies the first comma-decimal locale the host actually provides. CI images do not
/// all generate the same ones, so this reports failure rather than assuming.
class C_ScopedCommaLocale
{
public:
   C_ScopedCommaLocale(void) :
      mq_Applied(false)
   {
      const char * const pcn_Original = std::setlocale(LC_NUMERIC, nullptr);

      mc_Saved = (pcn_Original != nullptr) ? pcn_Original : "C";

      //Spellings differ per platform: glibc wants de_DE.UTF-8, macOS ships it too, and the
      //Windows CRT uses its own names entirely. Try all of them so this test actually runs
      //on all three targets rather than skipping into a false pass.
      const char * const apcn_Candidates[] =
      {
         "de_DE.UTF-8", "de_DE.utf8", "de_DE", "fr_FR.UTF-8", "fr_FR",
         "German_Germany.1252", "German_Germany.utf8", "German", "French_France.1252"
      };
      for (const char * const pcn_Candidate : apcn_Candidates)
      {
         if (std::setlocale(LC_NUMERIC, pcn_Candidate) != nullptr)
         {
            mq_Applied = true;
            break;
         }
      }
   }

   ~C_ScopedCommaLocale(void)
   {
      (void)std::setlocale(LC_NUMERIC, mc_Saved.c_str());
   }

   bool IsApplied(void) const
   {
      //a locale can be set and still not use a comma; confirm by observing strtod
      return mq_Applied && (std::strtod("1.5", nullptr) == 1.0);
   }

private:
   bool mq_Applied;
   std::string mc_Saved;
};
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Floats are always written with a '.' regardless of the process locale

   Regression pin. This is the assertion that failed: the serialised document contained
   `v="1,5"`.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(XmlFloatLocale, WritesDotUnderCommaLocale)
{
   const C_ScopedCommaLocale c_Locale;

   if (c_Locale.IsApplied() == false)
   {
      GTEST_SKIP() << "no comma-decimal locale available on this host";
   }

   C_OscXmlParser c_Xml;
   std::string c_Serialised;

   c_Xml.CreateAndSelectNodeChild("root");
   c_Xml.SetAttributeFloat64("f64", 1.5);
   c_Xml.SetAttributeFloat32("f32", 2.25F);
   c_Xml.SaveToString(c_Serialised);

   EXPECT_EQ(std::string::npos, c_Serialised.find(',')) << "written with a comma separator: " << c_Serialised;
   EXPECT_NE(std::string::npos, c_Serialised.find("1.5"));
   EXPECT_NE(std::string::npos, c_Serialised.find("2.25"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Floats read back correctly under a comma locale
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(XmlFloatLocale, ReadsDotUnderCommaLocale)
{
   const C_ScopedCommaLocale c_Locale;

   if (c_Locale.IsApplied() == false)
   {
      GTEST_SKIP() << "no comma-decimal locale available on this host";
   }

   C_OscXmlParser c_Xml;
   ASSERT_FALSE(static_cast<bool>(c_Xml.LoadFromString("<root f64=\"1.5\" f32=\"2.25\"/>")));
   ASSERT_EQ("root", c_Xml.SelectRoot());

   EXPECT_DOUBLE_EQ(1.5, c_Xml.GetAttributeFloat64("f64"));
   EXPECT_FLOAT_EQ(2.25F, c_Xml.GetAttributeFloat32("f32"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   A file already written with a comma is recovered rather than truncated

   Projects saved before this fix exist. ToDoubleCompat accepts one ',' as a decimal
   separator, so those files read back with their real values instead of silently
   losing everything after the separator.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(XmlFloatLocale, RecoversCommaWrittenFiles)
{
   C_OscXmlParser c_Xml;

   ASSERT_FALSE(static_cast<bool>(c_Xml.LoadFromString("<root f64=\"1,5\" f32=\"2,25\"/>")));
   ASSERT_EQ("root", c_Xml.SelectRoot());

   EXPECT_DOUBLE_EQ(1.5, c_Xml.GetAttributeFloat64("f64"));
   EXPECT_FLOAT_EQ(2.25F, c_Xml.GetAttributeFloat32("f32"));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Ordinary values, defaults and precision are unaffected

   The write path changed from tinyxml2's "%.17g" to std::format("{}"), so the exact
   round trip of awkward values is worth pinning.
*/
//----------------------------------------------------------------------------------------------------------------------
TEST(XmlFloatLocale, RoundTripsAwkwardValues)
{
   const double af64_Values[] = {0.0, -0.0, 1.0, 0.1, -2.25, 1e-300, 1e300, 3.14159265358979,
                                 1234567.891011, -0.000001};

   for (const double f64_Value : af64_Values)
   {
      C_OscXmlParser c_Write;
      std::string c_Serialised;
      c_Write.CreateAndSelectNodeChild("root");
      c_Write.SetAttributeFloat64("v", f64_Value);
      c_Write.SaveToString(c_Serialised);

      C_OscXmlParser c_Read;
      ASSERT_FALSE(static_cast<bool>(c_Read.LoadFromString(c_Serialised)));
      ASSERT_EQ("root", c_Read.SelectRoot());
      EXPECT_DOUBLE_EQ(f64_Value, c_Read.GetAttributeFloat64("v")) << "lost in round trip";
   }

   //a missing attribute still yields the supplied default
   C_OscXmlParser c_Empty;
   c_Empty.CreateAndSelectNodeChild("root");
   EXPECT_DOUBLE_EQ(7.5, c_Empty.GetAttributeFloat64("absent", 7.5));
   EXPECT_FLOAT_EQ(8.5F, c_Empty.GetAttributeFloat32("absent", 8.5F));

   //so does an attribute that is not a number at all
   C_OscXmlParser c_Junk;
   ASSERT_FALSE(static_cast<bool>(c_Junk.LoadFromString("<root v=\"not a number\"/>")));
   ASSERT_EQ("root", c_Junk.SelectRoot());
   EXPECT_DOUBLE_EQ(9.5, c_Junk.GetAttributeFloat64("v", 9.5));
}
