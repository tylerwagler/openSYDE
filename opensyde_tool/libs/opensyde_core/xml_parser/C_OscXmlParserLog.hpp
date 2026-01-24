//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Wrapper for C_OscXMLParser including error logging
   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXMLPARSERLOG_HPP
#define C_OSCXMLPARSERLOG_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscXmlParser.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscXmlParserLog :
   public C_OscXmlParser
{
public:
   C_OscXmlParserLog();

   void SetLogHeading(const QString & orc_Text);

   // init node system - selects second node, if available; first node is declaration
   virtual int32_t SelectRootError(const QString & orc_Name);

   // select first child node (with name) of current node        (go deeper)
   virtual int32_t SelectNodeChildError(const QString & orc_Name);

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

   //Base error reporting functions
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

private:
   QString mc_LogHeading;

   void m_ReportErrorForRootNodeMissing(const QString & orc_RootNodeName) const;
   void m_ReportErrorForAttributeMissing(const QString & orc_AttributeName) const;
   QString m_GetCurrentXmlLineInfoText(void) const;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
