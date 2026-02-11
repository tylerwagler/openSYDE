//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief      Wrapper class for QtXml (QDomDocument)

   cf. .h file header for details

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp" //pre-compiled headers

#include <QFile>
#include <QTextStream>
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscXmlParser.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscXmlParserBase::C_OscXmlParserBase(void)
{
   m_Init();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Init of common xml structure
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::m_Init(void)
{
   //empty file ?
   if (mc_Document.documentElement().isNull())
   {
      //create header:
      mc_Document.appendChild(mc_Document.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"utf-8\""));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscXmlParserBase::~C_OscXmlParserBase(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscXmlParser::C_OscXmlParser(void) :
   C_OscXmlParserBase()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscXmlParser::~C_OscXmlParser(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Open XML data from file
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParser::LoadFromFile(const QString & orc_FileName)
{
   int32_t s32_Return = C_NO_ERR;
   mc_Document.clear();

   QFile c_File(orc_FileName);
   if (!c_File.open(QIODevice::ReadOnly | QIODevice::Text))
   {
      s32_Return = C_NOACT;
      m_Init();
   }
   else
   {
      QDomDocument::ParseResult c_ParseResult = mc_Document.setContent(&c_File, QDomDocument::ParseOption::Default);
      if (!c_ParseResult)
      {
         s32_Return = C_NOACT;
         m_Init();
      }
      c_File.close();
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Write XML data to file
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParser::SaveToFile(const QString & orc_FileName)
{
   int32_t s32_Return = C_NO_ERR;
   QFile c_File(orc_FileName);
   if (!c_File.open(QIODevice::WriteOnly | QIODevice::Text))
   {
      s32_Return = C_NOACT;
   }
   else
   {
      QTextStream c_Stream(&c_File);
      mc_Document.save(c_Stream, 3); //indent 3 spaces
      c_File.close();
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Select root node as active element
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscXmlParserBase::SelectRoot(void)
{
   QString c_RootName;

   mc_CurrentElement = mc_Document.documentElement();
   if (!mc_CurrentElement.isNull())
   {
      c_RootName = mc_CurrentElement.tagName();
   }
   return c_RootName;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Select root node as active element
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::SelectRootError(const QString & orc_Name)
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->SelectRoot() != orc_Name)
   {
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Select next node as active element
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscXmlParserBase::SelectNodeNext(const QString & orc_Name)
{
   QString c_Name;
   QDomElement c_Save = mc_CurrentElement;

   if (!mc_CurrentElement.isNull())
   {
      if (orc_Name != "")
      {
         mc_CurrentElement = mc_CurrentElement.nextSiblingElement(orc_Name);
      }
      else
      {
         mc_CurrentElement = mc_CurrentElement.nextSiblingElement();
      }
   }

   if (!mc_CurrentElement.isNull())
   {
      c_Name = mc_CurrentElement.tagName();
   }
   else
   {
      mc_CurrentElement = c_Save;
   }
   return c_Name;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Select child node as active element
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscXmlParserBase::SelectNodeChild(const QString & orc_Name)
{
   QString c_Name;
   QDomElement c_Element;

   if (!mc_CurrentElement.isNull())
   {
      c_Element = mc_CurrentElement;
   }
   else
   {
      c_Element = mc_Document.documentElement();
   }

   if (!c_Element.isNull())
   {
      if (orc_Name == "")
      {
         c_Element = c_Element.firstChildElement();
      }
      else
      {
         c_Element = c_Element.firstChildElement(orc_Name);
      }
   }

   if (!c_Element.isNull())
   {
      mc_CurrentElement = c_Element;
      c_Name = mc_CurrentElement.tagName();
   }
   return c_Name;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Select child node as active element
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::SelectNodeChildError(const QString & orc_Name)
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->SelectNodeChild(orc_Name) != orc_Name)
   {
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Select parent of active node as active element
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscXmlParserBase::SelectNodeParent(void)
{
   QString c_Name;

   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement = mc_CurrentElement.parentNode().toElement();
   }
   if (!mc_CurrentElement.isNull())
   {
      c_Name = mc_CurrentElement.tagName();
   }
   return c_Name;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get content of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscXmlParserBase::GetNodeContent(void) const
{
   QString c_Content;

   if (!mc_CurrentElement.isNull())
   {
      c_Content = mc_CurrentElement.text();
   }

   return c_Content;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check whether specified attribute exists
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscXmlParserBase::AttributeExists(const QString & orc_Name) const
{
   bool q_Return = false;

   if (!mc_CurrentElement.isNull())
   {
      q_Return = mc_CurrentElement.hasAttribute(orc_Name);
   }
   return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get current node name
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscXmlParserBase::GetCurrentNodeName(void) const
{
   return (mc_CurrentElement.isNull()) ? "" : mc_CurrentElement.tagName();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get file line for current node
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscXmlParserBase::GetFileLineForCurrentNode(void) const
{
   // QDomDocument does not store line numbers for elements.
   return 0U;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscXmlParserBase::GetAttributeString(const QString & orc_Name, const QString & orc_Default) const
{
   QString c_Value = orc_Default;

   if (!mc_CurrentElement.isNull())
   {
      if (mc_CurrentElement.hasAttribute(orc_Name))
      {
         c_Value = mc_CurrentElement.attribute(orc_Name);
      }
   }
   return c_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeSint32(const QString & orc_Name, const int32_t os32_Default) const
{
   int32_t s32_Value = os32_Default;
   QString c_Text = this->GetAttributeString(orc_Name);
   if (c_Text != "")
   {
      bool q_Ok = false;
      s32_Value = c_Text.toInt(&q_Ok);
      if (!q_Ok)
      {
         s32_Value = os32_Default;
      }
   }
   return s32_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscXmlParserBase::GetAttributeUint32(const QString & orc_Name, const uint32_t ou32_Default) const
{
   uint32_t u32_Value = ou32_Default;
   QString c_Text = this->GetAttributeString(orc_Name);
   if (c_Text != "")
   {
      bool q_Ok = false;
      u32_Value = c_Text.toUInt(&q_Ok);
      if (!q_Ok)
      {
         u32_Value = ou32_Default;
      }
   }
   return u32_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int64_t C_OscXmlParserBase::GetAttributeSint64(const QString & orc_Name, const int64_t os64_Default) const
{
   int64_t s64_Value = os64_Default;
   QString c_Text = this->GetAttributeString(orc_Name);
   if (c_Text != "")
   {
      bool q_Ok = false;
      s64_Value = c_Text.toLongLong(&q_Ok);
      if (!q_Ok)
      {
         s64_Value = os64_Default;
      }
   }
   return s64_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
uint64_t C_OscXmlParserBase::GetAttributeUint64(const QString & orc_Name, const uint64_t ou64_Default) const
{
   uint64_t u64_Value = ou64_Default;
   QString c_Text = this->GetAttributeString(orc_Name);
   if (c_Text != "")
   {
      bool q_Ok = false;
      if (c_Text.startsWith("0x"))
      {
         u64_Value = c_Text.toULongLong(&q_Ok, 16);
      }
      else
      {
         u64_Value = c_Text.toULongLong(&q_Ok, 10);
      }
      if (!q_Ok)
      {
         u64_Value = ou64_Default;
      }
   }
   return u64_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscXmlParserBase::GetAttributeBool(const QString & orc_Name, const bool oq_Default) const
{
   bool q_Value = oq_Default;
   QString c_Text = this->GetAttributeString(orc_Name);
   if (c_Text != "")
   {
      QString c_Qs = c_Text.toLower();
      if (c_Qs == "true" || c_Qs == "1")
      {
         q_Value = true;
      }
      else if (c_Qs == "false" || c_Qs == "0")
      {
         q_Value = false;
      }
   }
   return q_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
float32_t C_OscXmlParserBase::GetAttributeFloat32(const QString & orc_Name, const float32_t of32_Default) const
{
   float32_t f32_Value = of32_Default;
   QString c_Text = this->GetAttributeString(orc_Name);
   if (c_Text != "")
   {
      bool q_Ok = false;
      f32_Value = c_Text.toFloat(&q_Ok);
      if (!q_Ok)
      {
         f32_Value = of32_Default;
      }
   }
   return f32_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
float64_t C_OscXmlParserBase::GetAttributeFloat64(const QString & orc_Name, const float64_t of64_Default) const
{
   float64_t f64_Value = of64_Default;
   QString c_Text = this->GetAttributeString(orc_Name);
   if (c_Text != "")
   {
      bool q_Ok = false;
      f64_Value = c_Text.toDouble(&q_Ok);
      if (!q_Ok)
      {
         f64_Value = of64_Default;
      }
   }
   return f64_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeStringError(const QString & orc_Name, QString & orc_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      orc_Value = this->GetAttributeString(orc_Name);
   }
   else
   {
      orc_Value = "";
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeSint32Error(const QString & orc_Name, int32_t & ors32_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      ors32_Value = this->GetAttributeSint32(orc_Name);
   }
   else
   {
      ors32_Value = 0L;
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeUint32Error(const QString & orc_Name, uint32_t & oru32_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      oru32_Value = this->GetAttributeUint32(orc_Name);
   }
   else
   {
      oru32_Value = 0UL;
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeSint64Error(const QString & orc_Name, int64_t & ors64_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      ors64_Value = this->GetAttributeSint64(orc_Name);
   }
   else
   {
      ors64_Value = 0LL;
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeUint64Error(const QString & orc_Name, uint64_t & oru64_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      oru64_Value = this->GetAttributeUint64(orc_Name);
   }
   else
   {
      oru64_Value = 0ULL;
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeBoolError(const QString & orc_Name, bool & orq_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      orq_Value = this->GetAttributeBool(orc_Name);
   }
   else
   {
      orq_Value = false;
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeFloat32Error(const QString & orc_Name, float32_t & orf32_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      orf32_Value = this->GetAttributeFloat32(orc_Name);
   }
   else
   {
      orf32_Value = 0.0F;
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get attribute value of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParserBase::GetAttributeFloat64Error(const QString & orc_Name, float64_t & orf64_Value) const
{
   int32_t s32_Retval = C_NO_ERR;

   if (this->AttributeExists(orc_Name))
   {
      orf64_Value = this->GetAttributeFloat64(orc_Name);
   }
   else
   {
      orf64_Value = 0.0;
      s32_Retval = C_CONFIG;
   }
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report error for node content
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::ReportErrorForNodeContentAppendXmlContext(const QString & orc_ErrorMessage) const
{
   (void)orc_ErrorMessage;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report error for attribute content
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::ReportErrorForAttributeContentAppendXmlContext(const QString & orc_Attribute,
                                                                        const QString & orc_ErrorMessage) const
{
   (void)orc_Attribute;
   (void)orc_ErrorMessage;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report error for node content
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::ReportErrorForNodeContentStartingWithXmlContext(const QString & orc_ErrorMessage) const
{
   (void)orc_ErrorMessage;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report error for attribute content
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::ReportErrorForAttributeContentStartingWithXmlContext(const QString & orc_Attribute,
                                                                              const QString & orc_ErrorMessage) const
{
   (void)orc_Attribute;
   (void)orc_ErrorMessage;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Report error for node missing
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::ReportErrorForNodeMissing(const QString & orc_MissingNodeName) const
{
   (void)orc_MissingNodeName;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get all attribute values of selected node
*/
//----------------------------------------------------------------------------------------------------------------------
QList<C_OscXmlAttribute> C_OscXmlParserBase::GetAttributes(void) const
{
   QList<C_OscXmlAttribute> c_AttributeList;
   if (!mc_CurrentElement.isNull())
   {
      QDomNamedNodeMap c_Attrs = mc_CurrentElement.attributes();
      for (int i = 0; i < c_Attrs.length(); ++i)
      {
         QDomNode c_Node = c_Attrs.item(i);
         if (c_Node.isAttr())
         {
            QDomAttr c_Attr = c_Node.toAttr();
            C_OscXmlAttribute c_Data;
            c_Data.c_Name = c_Attr.name();
            c_Data.c_Value = c_Attr.value();
            c_AttributeList.push_back(c_Data);
         }
      }
   }

   return c_AttributeList;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Create a node under the currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::CreateNodeChild(const QString & orc_Name, const QString & orc_Content)
{
   QDomElement c_NewNode = mc_Document.createElement(orc_Name);
   if (orc_Content != "")
   {
      c_NewNode.appendChild(mc_Document.createTextNode(orc_Content));
   }
   
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.appendChild(c_NewNode);
   }
   else
   {
      mc_Document.appendChild(c_NewNode);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Create a node under the currently selected node and select it.
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscXmlParserBase::CreateAndSelectNodeChild(const QString & orc_Name)
{
   QString c_ResultName;

   this->CreateNodeChild(orc_Name);
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement = mc_CurrentElement.lastChildElement();
   }
   else
   {
      mc_CurrentElement = mc_Document.documentElement();
   }

   if (!mc_CurrentElement.isNull())
   {
      c_ResultName = mc_CurrentElement.tagName();
   }
   return c_ResultName;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Delete the current node.
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscXmlParserBase::DeleteNode(void)
{
   QString c_Name;
 
   if (!mc_CurrentElement.isNull())
   {
      c_Name = mc_CurrentElement.tagName();
      QDomNode c_Parent = mc_CurrentElement.parentNode();
      c_Parent.removeChild(mc_CurrentElement);
      mc_CurrentElement = QDomElement();
   }

   return c_Name;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set content of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetNodeContent(const QString & orc_Content)
{
   if (!mc_CurrentElement.isNull())
   {
      // Remove existing text nodes
      QDomNodeList c_Children = mc_CurrentElement.childNodes();
      for (int i = 0; i < c_Children.count(); ++i)
      {
         if (c_Children.at(i).isText())
         {
            mc_CurrentElement.removeChild(c_Children.at(i));
            --i;
         }
      }
      mc_CurrentElement.appendChild(mc_Document.createTextNode(orc_Content));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set string content of one attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeString(const QString & orc_Name, const QString & orc_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(orc_Name, 
                                     orc_Value);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set sint32 content of attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeSint32(const QString & orc_Name, const int32_t os32_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(orc_Name, os32_Value);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set uint32 content of attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeUint32(const QString & orc_Name, const uint32_t ou32_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(orc_Name, ou32_Value);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set sint64 content of attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeSint64(const QString & orc_Name, const int64_t os64_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(orc_Name, static_cast<long long>(os64_Value));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set uint64 content of attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeUint64(const QString & orc_Name, const uint64_t ou64_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(orc_Name, static_cast<unsigned long long>(ou64_Value));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set bool content of attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeBool(const QString & orc_Name, const bool oq_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(orc_Name, oq_Value ? "true" : "false");
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set float32 content of attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeFloat32(const QString & orc_Name, const float32_t of32_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(orc_Name, static_cast<double>(of32_Value));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set float64 content of attribute of currently selected node.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParserBase::SetAttributeFloat64(const QString & orc_Name, const float64_t of64_Value)
{
   if (!mc_CurrentElement.isNull())
   {
      mc_CurrentElement.setAttribute(orc_Name, of64_Value);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Open XML data from string
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXmlParser::LoadFromString(const QString & orc_String)
{
   int32_t s32_Return = C_NO_ERR;
   mc_Document.clear();

   QDomDocument::ParseResult c_ParseResult = mc_Document.setContent(orc_String, QDomDocument::ParseOption::Default);
   if (!c_ParseResult)
   {
      s32_Return = C_NOACT;
      m_Init();
   }
   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Write XML data to string
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXmlParser::SaveToString(QString & orc_String) const
{
   orc_String = mc_Document.toString(3);
}
