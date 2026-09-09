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
#include <system_error>
#include "C_OscXmlParser.hpp"
#include <string>

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

    void SetLogHeading(const std::string & orc_Text);


   // init node system - selects second node, if available; first node is declaration
   virtual std::error_code SelectRootError(const std::string & orc_Name);

   // select first child node (with name) of current node        (go deeper)
   virtual std::error_code SelectNodeChildError(const std::string & orc_Name);

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

   //Base error reporting functions
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

private:
    std::string mc_LogHeading;


    void m_ReportErrorForRootNodeMissing(const std::string & orc_RootNodeName) const;
    void m_ReportErrorForAttributeMissing(const std::string & orc_AttributeName) const;
    std::string m_GetCurrentXmlLineInfoText(void) const;

};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
