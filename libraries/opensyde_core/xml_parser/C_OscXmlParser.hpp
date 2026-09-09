//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief      Wrapper class for tinyxml2

   \class      stw::opensyde_core::C_OscXMLParser
   \brief      Wrapper class for tinyxml2

   Based on an pre-existing implementation for rapidxml.
   Documented and modified to fit into openSYDE structure and conventions.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXMLPARSER_HPP
#define C_OSCXMLPARSER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <list>
#include <system_error>
#include "tinyxml2.h"
#include <cstdint>
#include "C_OscErrorCategory.hpp"
#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

///one XML attribute with its value
class C_OscXmlAttribute
{
public:
   std::string c_Name;  ///< name of attribute
   std::string c_Value; ///< value of attribute
};

//----------------------------------------------------------------------------------------------------------------------

///General XML handling
class C_OscXmlParserBase
{
private:
   C_OscXmlParserBase(const C_OscXmlParserBase & orc_Source);               ///< not implemented: prevent copying
   C_OscXmlParserBase & operator = (const C_OscXmlParserBase & orc_Source); ///< not implemented: prevent assignment

   tinyxml2::XMLElement * mpc_CurrentNode;

protected:
   tinyxml2::XMLDocument mc_Document;

   void m_Init(void);

public:
   // set up class
   C_OscXmlParserBase(void);
   virtual ~C_OscXmlParserBase(void);

   // init node system - selects second node, if available; first node is declaration
   std::string SelectRoot(void);
   virtual std::error_code SelectRootError(const std::string & orc_Name);

   // node navigation - returns current node
   // select first/next node (with name) after current node      (go forward)
   std::string SelectNodeNext(const std::string & orc_Name = "");
   // select first child node (with name) of current node        (go deeper)
   std::string SelectNodeChild(const std::string & orc_Name = "");
   virtual std::error_code SelectNodeChildError(const std::string & orc_Name);
   // select parent node of current node                         (go up)
   std::string SelectNodeParent(void);

   //append new child node under the current node with content
   // if the document is still empty this will create the root node
   void CreateNodeChild(const std::string & orc_Name, const std::string & orc_Content = "");
   //append new child node under the current node and select it; returns new child node name
   std::string CreateAndSelectNodeChild(const std::string & orc_Name);
   // delete current node
   std::string DeleteNode(void);

   // node text content
   void SetNodeContent(const std::string & orc_Content);
   std::string GetNodeContent(void) const;

   // node attribute operations
   bool AttributeExists(const std::string & orc_Name) const;

   std::string GetCurrentNodeName(void) const;
   uint32_t GetFileLineForCurrentNode(void) const;

   // set attribute values
   void SetAttributeString(const std::string & orc_Name, const std::string & orc_Value);
   void SetAttributeSint32(const std::string & orc_Name, const int32_t os32_Value);
   void SetAttributeUint32(const std::string & orc_Name, const uint32_t ou32_Value);
   void SetAttributeSint64(const std::string & orc_Name, const int64_t os64_Value);
   void SetAttributeUint64(const std::string & orc_Name, const uint64_t ou64_Value);
   void SetAttributeBool(const std::string & orc_Name, const bool oq_Value);
   void SetAttributeFloat32(const std::string & orc_Name, const float of32_Value);
   void SetAttributeFloat64(const std::string & orc_Name, const double of64_Value);

   // get attribute values
   std::string GetAttributeString(const std::string & orc_Name,
                                            const std::string & orc_Default = "") const;
   int32_t GetAttributeSint32(const std::string & orc_Name, const int32_t os32_Default = 0L) const;
   uint32_t GetAttributeUint32(const std::string & orc_Name, const uint32_t ou32_Default = 0UL) const;
   int64_t GetAttributeSint64(const std::string & orc_Name, const int64_t os64_Default = 0LL) const;
   uint64_t GetAttributeUint64(const std::string & orc_Name, const uint64_t ou64_Default = 0ULL) const;
   bool GetAttributeBool(const std::string & orc_Name, const bool oq_Default = false) const;
   float GetAttributeFloat32(const std::string & orc_Name, const float of32_Default = 0.0F) const;
   double GetAttributeFloat64(const std::string & orc_Name, const double of64_Default = 0.0) const;

   // get attribute values (includes error check)
   virtual std::error_code GetAttributeStringError(const std::string & orc_Name,
                                                   std::string & orc_Value) const;
   virtual std::error_code GetAttributeSint32Error(const std::string & orc_Name, int32_t & ors32_Value) const;
   virtual std::error_code GetAttributeUint32Error(const std::string & orc_Name, uint32_t & oru32_Value) const;
   virtual std::error_code GetAttributeSint64Error(const std::string & orc_Name, int64_t & ors64_Value) const;
   virtual std::error_code GetAttributeUint64Error(const std::string & orc_Name, uint64_t & oru64_Value) const;
   virtual std::error_code GetAttributeBoolError(const std::string & orc_Name, bool & orq_Value) const;
   virtual std::error_code GetAttributeFloat32Error(const std::string & orc_Name, float & orf32_Value) const;
   virtual std::error_code GetAttributeFloat64Error(const std::string & orc_Name, double & orf64_Value) const;

   //Base reporting functions
   virtual void ReportErrorForNodeContentAppendXmlContext(const std::string & orc_ErrorMessage)
   const;
   virtual void ReportErrorForAttributeContentAppendXmlContext(const std::string & orc_Attribute,
                                                               const std::string & orc_ErrorMessage)
   const;
   virtual void ReportErrorForNodeContentStartingWithXmlContext(const std::string & orc_ErrorMessage)
   const;
   virtual void ReportErrorForAttributeContentStartingWithXmlContext(const std::string & orc_Attribute,
                                                                     const std::string & orc_ErrorMessage)
   const;
   virtual void ReportErrorForNodeMissing(const std::string & orc_MissingNodeName) const;

   // get all attributes
   std::vector<C_OscXmlAttribute> GetAttributes(void) const;
};

//----------------------------------------------------------------------------------------------------------------------

///file based parsing
class C_OscXmlParser :
   public C_OscXmlParserBase
{
private:
   //not implemented -> prevent copying
   C_OscXmlParser(const C_OscXmlParser & orc_Source);
   //not implemented -> prevent assignment
   C_OscXmlParser & operator = (const C_OscXmlParser & orc_Source); //lint !e1511 //we want to hide the base function

public:
   C_OscXmlParser(void);
   virtual ~C_OscXmlParser(void);

   // open xml file; create XML declaration
   virtual std::error_code LoadFromFile(const std::string & orc_FileName);
   virtual std::error_code SaveToFile(const std::string & orc_FileName);

   // parse xml string; create XML declaration
   std::error_code LoadFromString(const std::string & orc_String);
   void SaveToString(std::string & orc_String) const;
};

//----------------------------------------------------------------------------------------------------------------------

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
