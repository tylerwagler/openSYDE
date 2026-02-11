//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief      Wrapper class for QtXml (QDomDocument)

   \class      stw::opensyde_core::C_OscXMLParser
   \brief      Wrapper class for QtXml (QDomDocument)

   Based on an pre-existing implementation for tinyxml2.
   Documented and modified to fit into openSYDE structure and conventions.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXMLPARSER_HPP
#define C_OSCXMLPARSER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QDomDocument>
#include <QDomElement>
#include <QString>
#include <vector>
#include <QList>
#include "stwtypes.hpp"

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
   QString c_Name;  ///< name of attribute
   QString c_Value; ///< value of attribute
};

//----------------------------------------------------------------------------------------------------------------------

///General XML handling
class C_OscXmlParserBase
{
private:
   C_OscXmlParserBase(const C_OscXmlParserBase & orc_Source);               ///< not implemented: prevent copying
   C_OscXmlParserBase & operator = (const C_OscXmlParserBase & orc_Source); ///< not implemented: prevent assignment

   QDomElement mc_CurrentElement;

protected:
   QDomDocument mc_Document;

   void m_Init(void);

public:
   // set up class
   C_OscXmlParserBase(void);
   virtual ~C_OscXmlParserBase(void);

   // init node system - selects root node
   QString SelectRoot(void);
   virtual int32_t SelectRootError(const QString & orc_Name);

   // node navigation - returns current node
   // select first/next node (with name) after current node      (go forward)
   QString SelectNodeNext(const QString & orc_Name = "");
   // select first child node (with name) of current node        (go deeper)
   QString SelectNodeChild(const QString & orc_Name = "");
   virtual int32_t SelectNodeChildError(const QString & orc_Name);
   // select parent node of current node                         (go up)
   QString SelectNodeParent(void);

   //append new child node under the current node with content
   // if the document is still empty this will create the root node
   void CreateNodeChild(const QString & orc_Name, const QString & orc_Content = "");
   //append new child node under the current node and select it; returns new child node name
   QString CreateAndSelectNodeChild(const QString & orc_Name);
   // delete current node
   QString DeleteNode(void);

   // node text content
   void SetNodeContent(const QString & orc_Content);
   QString GetNodeContent(void) const;

   // node attribute operations
   bool AttributeExists(const QString & orc_Name) const;

   QString GetCurrentNodeName(void) const;
   uint32_t GetFileLineForCurrentNode(void) const;

   // set attribute values
   void SetAttributeString(const QString & orc_Name, const QString & orc_Value);
   void SetAttributeSint32(const QString & orc_Name, const int32_t os32_Value);
   void SetAttributeUint32(const QString & orc_Name, const uint32_t ou32_Value);
   void SetAttributeSint64(const QString & orc_Name, const int64_t os64_Value);
   void SetAttributeUint64(const QString & orc_Name, const uint64_t ou64_Value);
   void SetAttributeBool(const QString & orc_Name, const bool oq_Value);
   void SetAttributeFloat32(const QString & orc_Name, const float32_t of32_Value);
   void SetAttributeFloat64(const QString & orc_Name, const float64_t of64_Value);

   // get attribute values
   QString GetAttributeString(const QString & orc_Name,
                                            const QString & orc_Default = "") const;
   int32_t GetAttributeSint32(const QString & orc_Name, const int32_t os32_Default = 0L) const;
   uint32_t GetAttributeUint32(const QString & orc_Name, const uint32_t ou32_Default = 0UL) const;
   int64_t GetAttributeSint64(const QString & orc_Name, const int64_t os64_Default = 0LL) const;
   uint64_t GetAttributeUint64(const QString & orc_Name, const uint64_t ou64_Default = 0ULL) const;
   bool GetAttributeBool(const QString & orc_Name, const bool oq_Default = false) const;
   float32_t GetAttributeFloat32(const QString & orc_Name, const float32_t of32_Default = 0.0F) const;
   float64_t GetAttributeFloat64(const QString & orc_Name, const float64_t of64_Default = 0.0) const;

   // get attribute values (includes error check)
   virtual int32_t GetAttributeStringError(const QString & orc_Name,
                                            QString & orc_Value) const;
   virtual int32_t GetAttributeSint32Error(const QString & orc_Name, int32_t & ors32_Value) const;
   virtual int32_t GetAttributeUint32Error(const QString & orc_Name, uint32_t & oru32_Value) const;
   virtual int32_t GetAttributeSint64Error(const QString & orc_Name, int64_t & ors64_Value) const;
   virtual int32_t GetAttributeUint64Error(const QString & orc_Name, uint64_t & oru64_Value) const;
   virtual int32_t GetAttributeBoolError(const QString & orc_Name, bool & orq_Value) const;
   virtual int32_t GetAttributeFloat32Error(const QString & orc_Name, float32_t & orf32_Value) const;
   virtual int32_t GetAttributeFloat64Error(const QString & orc_Name, float64_t & orf64_Value) const;

   //Base reporting functions
   virtual void ReportErrorForNodeContentAppendXmlContext(const QString & orc_ErrorMessage)
   const;
   virtual void ReportErrorForAttributeContentAppendXmlContext(const QString & orc_Attribute,
                                                                 const QString & orc_ErrorMessage)
   const;
   virtual void ReportErrorForNodeContentStartingWithXmlContext(const QString & orc_ErrorMessage)
   const;
   virtual void ReportErrorForAttributeContentStartingWithXmlContext(const QString & orc_Attribute,
                                                                       const QString & orc_ErrorMessage)
   const;
   virtual void ReportErrorForNodeMissing(const QString & orc_MissingNodeName) const;

   // get all attributes
   QList<C_OscXmlAttribute> GetAttributes(void) const;
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
   virtual int32_t LoadFromFile(const QString & orc_FileName);
   virtual int32_t SaveToFile(const QString & orc_FileName);

   // parse xml string; create XML declaration
   int32_t LoadFromString(const QString & orc_String);
   void SaveToString(QString & orc_String) const;
};

//----------------------------------------------------------------------------------------------------------------------

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
}

#endif
