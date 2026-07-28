#include <cstring>
#include "gtest/gtest.h"
#include "C_OscXmlParser.hpp"

// Key API semantics:
// - CreateNodeChild() appends a child but does NOT select it (current node unchanged)
// - CreateAndSelectNodeChild() appends AND selects the new child
// - SetAttribute*/GetAttribute* operate on the currently selected node
// - After creating the first node, call SelectRoot() to make it current

static std::string CompactXml(const std::string & or_Xml)
{
   std::string c_Result;
   bool q_InTag = false;
   for (char ch : or_Xml)
   {
      if (ch == '<') { q_InTag = true; c_Result += ch; }
      else if (ch == '>') { q_InTag = false; c_Result += ch; }
      else if (q_InTag) { c_Result += ch; }
      else if (ch != '\n' && ch != '\r' && ch != '\t') { c_Result += ch; }
   }
   return c_Result;
}

TEST(XmlParser, CreateEmptyDocument)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   stw::scl::C_SclString c_Xml;
   c_Parser.SaveToString(c_Xml);
   EXPECT_TRUE(std::string(c_Xml.c_str()).find("<?xml") != std::string::npos);
}

TEST(XmlParser, CreateSingleRoot)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateNodeChild("root");
   const std::string c_Compact = CompactXml(std::string(c_Parser.GetCurrentNodeName().c_str()));
   // CreateNodeChild does NOT select the new node; current node is still NULL
   EXPECT_TRUE(std::string(c_Parser.GetCurrentNodeName().c_str()).empty());
}

TEST(XmlParser, CreateNodeWithContent)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateNodeChild("root", "Hello World");
   stw::scl::C_SclString c_Xml;
   c_Parser.SaveToString(c_Xml);
   EXPECT_TRUE(std::string(c_Xml.c_str()).find("Hello World") != std::string::npos);
}

TEST(XmlParser, CreateAndSelectRoot)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateAndSelectNodeChild("root");
   EXPECT_EQ("root", std::string(c_Parser.GetCurrentNodeName().c_str()));
}

TEST(XmlParser, CreateNestedNodes)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateNodeChild("root");
   c_Parser.SelectRoot();
   c_Parser.CreateNodeChild("child", "value");
   stw::scl::C_SclString c_Xml;
   c_Parser.SaveToString(c_Xml);
   EXPECT_TRUE(std::string(c_Xml.c_str()).find("<child>value</child>") != std::string::npos);
}

TEST(XmlParser, SelectNodeChild_ByName)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateNodeChild("root");
   c_Parser.SelectRoot();
   c_Parser.CreateAndSelectNodeChild("alpha");
   c_Parser.SelectNodeParent();
   c_Parser.CreateAndSelectNodeChild("beta");
   c_Parser.SelectNodeParent();

   c_Parser.SelectNodeChild("beta");
   EXPECT_EQ("beta", std::string(c_Parser.GetCurrentNodeName().c_str()));
}

TEST(XmlParser, SelectNodeNext)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateNodeChild("root");
   c_Parser.SelectRoot();
   c_Parser.CreateAndSelectNodeChild("first");
   c_Parser.SelectNodeParent();
   c_Parser.CreateAndSelectNodeChild("second");
   c_Parser.SelectNodeParent();
   c_Parser.CreateAndSelectNodeChild("third");
   c_Parser.SelectNodeParent();

   c_Parser.SelectNodeChild(""); // select first child
   EXPECT_EQ("first", std::string(c_Parser.GetCurrentNodeName().c_str()));

   stw::scl::C_SclString c_Name = c_Parser.SelectNodeNext("");
   EXPECT_EQ("second", std::string(c_Name.c_str()));

   c_Name = c_Parser.SelectNodeNext("");
   EXPECT_EQ("third", std::string(c_Name.c_str()));

   c_Name = c_Parser.SelectNodeNext("");
   EXPECT_TRUE(std::string(c_Name.c_str()).empty());
}

TEST(XmlParser, SetGetStringAttribute)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateAndSelectNodeChild("root");
   c_Parser.SetAttributeString("name", "test_value");
   EXPECT_EQ("test_value", std::string(c_Parser.GetAttributeString("name").c_str()));
}

TEST(XmlParser, SetGetIntegerAttributes)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateAndSelectNodeChild("root");

   c_Parser.SetAttributeSint32("s32", -12345);
   c_Parser.SetAttributeUint32("u32", 54321U);
   c_Parser.SetAttributeSint64("s64", -9876543210LL);
   c_Parser.SetAttributeUint64("u64", 9876543210ULL);

   EXPECT_EQ(-12345,        c_Parser.GetAttributeSint32("s32"));
   EXPECT_EQ(54321U,        c_Parser.GetAttributeUint32("u32"));
   EXPECT_EQ(-9876543210LL, c_Parser.GetAttributeSint64("s64"));
   EXPECT_EQ(9876543210ULL, c_Parser.GetAttributeUint64("u64"));
}

TEST(XmlParser, SetGetBoolAttribute)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateAndSelectNodeChild("root");

   c_Parser.SetAttributeBool("flag_true", true);
   c_Parser.SetAttributeBool("flag_false", false);

   EXPECT_TRUE( c_Parser.GetAttributeBool("flag_true"));
   EXPECT_FALSE(c_Parser.GetAttributeBool("flag_false"));
}

TEST(XmlParser, SetGetFloatAttributes)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateAndSelectNodeChild("root");

   c_Parser.SetAttributeFloat32("f32", 3.14F);
   c_Parser.SetAttributeFloat64("f64", 2.718281828459045);

   EXPECT_FLOAT_EQ(3.14F,           c_Parser.GetAttributeFloat32("f32"));
   EXPECT_DOUBLE_EQ(2.718281828459045, c_Parser.GetAttributeFloat64("f64"));
}

TEST(XmlParser, AttributeDefaults)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateAndSelectNodeChild("root");

   EXPECT_EQ("default_str", std::string(c_Parser.GetAttributeString("nonexistent", "default_str").c_str()));
   EXPECT_EQ(42,            c_Parser.GetAttributeSint32("nonexistent", 42));
   EXPECT_EQ(99U,           c_Parser.GetAttributeUint32("nonexistent", 99U));
   EXPECT_TRUE(             c_Parser.GetAttributeBool("nonexistent", true));
}

TEST(XmlParser, AttributeExists)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateAndSelectNodeChild("root");
   c_Parser.SetAttributeString("existing", "yes");

   EXPECT_TRUE( c_Parser.AttributeExists("existing"));
   EXPECT_FALSE(c_Parser.AttributeExists("missing"));
}

TEST(XmlParser, RoundTrip_Simple)
{
   stw::opensyde_core::C_OscXmlParser c_Writer;
   c_Writer.CreateAndSelectNodeChild("config");
   c_Writer.SetAttributeString("version", "1.0");
   c_Writer.CreateNodeChild("mode", "production");

   stw::scl::C_SclString c_Xml;
   c_Writer.SaveToString(c_Xml);

   stw::opensyde_core::C_OscXmlParser c_Reader;
   ASSERT_EQ(0, c_Reader.LoadFromString(c_Xml));

   EXPECT_EQ("config", std::string(c_Reader.SelectRoot().c_str()));
   EXPECT_EQ("1.0", std::string(c_Reader.GetAttributeString("version").c_str()));

   c_Reader.SelectNodeChild("mode");
   EXPECT_EQ("production", std::string(c_Reader.GetNodeContent().c_str()));
}

TEST(XmlParser, RoundTrip_ComplexHierarchy)
{
   stw::opensyde_core::C_OscXmlParser c_Writer;
   c_Writer.CreateAndSelectNodeChild("system");
   c_Writer.SetAttributeString("name", "test_system");

   c_Writer.CreateAndSelectNodeChild("node");
   c_Writer.SetAttributeString("id", "42");
   c_Writer.CreateNodeChild("param", "value1");
   // CreateNodeChild does NOT select; current node is still "node"
   c_Writer.SelectNodeParent();

   c_Writer.CreateAndSelectNodeChild("bus");
   c_Writer.SetAttributeString("type", "CAN");

   stw::scl::C_SclString c_Xml;
   c_Writer.SaveToString(c_Xml);

   stw::opensyde_core::C_OscXmlParser c_Reader;
   ASSERT_EQ(0, c_Reader.LoadFromString(c_Xml));

   EXPECT_EQ("system", std::string(c_Reader.SelectRoot().c_str()));
   EXPECT_EQ("test_system", std::string(c_Reader.GetAttributeString("name").c_str()));

   c_Reader.SelectNodeChild("node");
   EXPECT_EQ("42", std::string(c_Reader.GetAttributeString("id").c_str()));
   c_Reader.SelectNodeChild("param");
   EXPECT_EQ("value1", std::string(c_Reader.GetNodeContent().c_str()));
   c_Reader.SelectNodeParent();
   c_Reader.SelectNodeParent();

   c_Reader.SelectNodeChild("bus");
   EXPECT_EQ("CAN", std::string(c_Reader.GetAttributeString("type").c_str()));
}

TEST(XmlParser, LoadInvalidXml_ReturnsError)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   EXPECT_NE(0, c_Parser.LoadFromString("this is not valid xml"));
}

TEST(XmlParser, SelectRootOnEmptyDocument_ReturnsEmpty)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.LoadFromString("<?xml version=\"1.0\"?>");
   EXPECT_TRUE(std::string(c_Parser.SelectRoot().c_str()).empty());
}

TEST(XmlParser, GetNodeContent_NoContent)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateAndSelectNodeChild("empty");
   EXPECT_EQ("", std::string(c_Parser.GetNodeContent().c_str()));
}

TEST(XmlParser, GetAttributes_Enumerate)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateAndSelectNodeChild("root");
   c_Parser.SetAttributeString("a", "1");
   c_Parser.SetAttributeString("b", "2");
   c_Parser.SetAttributeString("c", "3");

   const std::vector<stw::opensyde_core::C_OscXmlAttribute> c_Attrs = c_Parser.GetAttributes();
   EXPECT_EQ(3U, c_Attrs.size());

   bool q_FoundA = false, q_FoundB = false, q_FoundC = false;
   for (const auto & rc_Attr : c_Attrs)
   {
      const std::string c_Name(rc_Attr.c_Name.c_str());
      if (c_Name == "a") { q_FoundA = true; EXPECT_EQ("1", std::string(rc_Attr.c_Value.c_str())); }
      if (c_Name == "b") { q_FoundB = true; EXPECT_EQ("2", std::string(rc_Attr.c_Value.c_str())); }
      if (c_Name == "c") { q_FoundC = true; EXPECT_EQ("3", std::string(rc_Attr.c_Value.c_str())); }
   }
   EXPECT_TRUE(q_FoundA);
   EXPECT_TRUE(q_FoundB);
   EXPECT_TRUE(q_FoundC);
}

TEST(XmlParser, DeleteNode)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   c_Parser.CreateAndSelectNodeChild("root");
   c_Parser.CreateAndSelectNodeChild("todelete");
   c_Parser.SelectNodeParent();
   c_Parser.CreateAndSelectNodeChild("keep");
   c_Parser.SelectNodeParent();

   c_Parser.SelectNodeChild("todelete");
   c_Parser.DeleteNode();

   c_Parser.SelectNodeChild("");
   EXPECT_EQ("keep", std::string(c_Parser.GetCurrentNodeName().c_str()));
}

TEST(XmlParser, SaveAndLoadFile)
{
   const stw::scl::C_SclString c_TmpFile = "/tmp/opensyde_test_xml.xml";

   stw::opensyde_core::C_OscXmlParser c_Writer;
   c_Writer.CreateAndSelectNodeChild("testdata");
   c_Writer.SetAttributeString("key", "value");
   ASSERT_EQ(0, c_Writer.SaveToFile(c_TmpFile));

   stw::opensyde_core::C_OscXmlParser c_Reader;
   ASSERT_EQ(0, c_Reader.LoadFromFile(c_TmpFile));

   EXPECT_EQ("testdata", std::string(c_Reader.SelectRoot().c_str()));
   EXPECT_EQ("value", std::string(c_Reader.GetAttributeString("key").c_str()));

   std::remove(c_TmpFile.c_str());
}

TEST(XmlParser, LoadNonexistentFile_ReturnsError)
{
   stw::opensyde_core::C_OscXmlParser c_Parser;
   EXPECT_NE(0, c_Parser.LoadFromFile("/nonexistent/path/file.xml"));
}
