//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE target support package data handling class

   openSYDE target support package data handling class

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscTargetSupportPackage.hpp"
#include "stwerrors.hpp"
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QDomDocument>

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
using namespace stw::opensyde_core;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */

/* -- Module Global Variables
 * ---------------------------------------------------------------------------------------
 */

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscTargetSupportPackage::C_OscTargetSupportPackage() {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Clear elements

   Sets all strings to "" and other values to default.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscTargetSupportPackage::Clear(void) {
  c_DeviceName = "";
  c_Comment = "";
  c_TemplatePath = "";
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDataStream (binary format)

   \param[out] orc_Stream    Output stream for serialization

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackage::ToQDataStream(QDataStream& orc_Stream) const {
   using namespace stw::errors;
   
   orc_Stream << c_DeviceName;
   orc_Stream << c_Comment;
   orc_Stream << c_TemplatePath;
   
   if (orc_Stream.status() == QDataStream::Ok) {
      return C_NO_ERR;
   } else {
      return C_RD_WR;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QDataStream (binary format)

   \param[in,out] orc_Stream    Input stream for deserialization

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackage::FromQDataStream(QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   orc_Stream >> c_DeviceName;
   orc_Stream >> c_Comment;
   orc_Stream >> c_TemplatePath;
   
   if (orc_Stream.status() == QDataStream::Ok) {
      return C_NO_ERR;
   } else {
      return C_RD_WR;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QJsonObject

   \return JSON object containing all data
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscTargetSupportPackage::ToJsonObject() const {
   QJsonObject c_Object;
   
   c_Object["device-name"] = c_DeviceName;
   c_Object["comment"] = c_Comment;
   c_Object["template-path"] = c_TemplatePath;
   
   return c_Object;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QJsonObject

   \param[in] orc_Object    JSON object to deserialize from

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackage::FromJsonObject(const QJsonObject& orc_Object) {
   using namespace stw::errors;
   
   if (orc_Object.contains("device-name")) {
      c_DeviceName = orc_Object["device-name"].toString();
   }
   
   if (orc_Object.contains("comment")) {
      c_Comment = orc_Object["comment"].toString();
   }
   
   if (orc_Object.contains("template-path")) {
      c_TemplatePath = orc_Object["template-path"].toString();
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDomDocument

   \param[in] orc_Doc    XML document to append to
   \param[in] orc_RootElementName    Name of the root element

   \return QDomElement representing the serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscTargetSupportPackage::ToQDomDocument(QDomDocument& orc_Doc, 
                                                      const QString& orc_RootElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
   
   QDomElement c_DeviceNameElement = orc_Doc.createElement("device-name");
   c_DeviceNameElement.appendChild(orc_Doc.createTextNode(c_DeviceName));
   c_Element.appendChild(c_DeviceNameElement);
   
   QDomElement c_CommentElement = orc_Doc.createElement("comment");
   c_CommentElement.appendChild(orc_Doc.createTextNode(c_Comment));
   c_Element.appendChild(c_CommentElement);
   
   QDomElement c_TemplatePathElement = orc_Doc.createElement("template-path");
   c_TemplatePathElement.appendChild(orc_Doc.createTextNode(c_TemplatePath));
   c_Element.appendChild(c_TemplatePathElement);
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QDomElement

   \param[in] orc_Element    XML element to deserialize from

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscTargetSupportPackage::FromQDomElement(const QDomElement& orc_Element) {
   using namespace stw::errors;
   
   QDomNode c_DeviceNameNode = orc_Element.namedItem("device-name");
   if (!c_DeviceNameNode.isNull()) {
      c_DeviceName = c_DeviceNameNode.toElement().text();
   }
   
   QDomNode c_CommentNode = orc_Element.namedItem("comment");
   if (!c_CommentNode.isNull()) {
      c_Comment = c_CommentNode.toElement().text();
   }
   
   QDomNode c_TemplatePathNode = orc_Element.namedItem("template-path");
   if (!c_TemplatePathNode.isNull()) {
      c_TemplatePath = c_TemplatePathNode.toElement().text();
   }
   
   return C_NO_ERR;
}
