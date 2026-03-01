//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Manifest data

   Manifest data

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscXcoManifest.hpp"
#include "stwtypes.hpp"

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
C_OscXcoManifest::C_OscXcoManifest() {
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDataStream (binary format)
   
   \param[out] orc_Stream    Output stream for serialization
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXcoManifest::ToQDataStream(QDataStream& orc_Stream) const {
   using namespace stw::errors;
   
   orc_Stream << c_NodeName;
   
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
int32_t C_OscXcoManifest::FromQDataStream(QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   orc_Stream >> c_NodeName;
   
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
QJsonObject C_OscXcoManifest::ToJsonObject() const {
   QJsonObject c_Object;
   
   c_Object["node-name"] = c_NodeName;
   
   return c_Object;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QJsonObject
   
   \param[in] orc_Object    JSON object to deserialize from
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXcoManifest::FromJsonObject(const QJsonObject& orc_Object) {
   using namespace stw::errors;
   
   if (orc_Object.contains("node-name")) {
      c_NodeName = orc_Object["node-name"].toString();
      return C_NO_ERR;
   } else {
      return C_CONFIG;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDomDocument
   
   \param[in] orc_Doc    XML document to append to
   \param[in] orc_RootElementName    Name of the root element to create
   
   \return QDomElement representing the serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscXcoManifest::ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
   
   QDomElement c_NodeNameElement = orc_Doc.createElement("node-name");
   QDomText c_NodeNameText = orc_Doc.createTextNode(c_NodeName);
   c_NodeNameElement.appendChild(c_NodeNameText);
   c_Element.appendChild(c_NodeNameElement);
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QDomElement
   
   \param[in] orc_Element    XML element to deserialize from
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXcoManifest::FromQDomElement(const QDomElement& orc_Element) {
   using namespace stw::errors;
   
   QDomNode c_NodeNameNode = orc_Element.namedItem("node-name");
   if (!c_NodeNameNode.isNull()) {
      QDomText c_Text = c_NodeNameNode.toText();
      if (!c_Text.isNull()) {
         c_NodeName = c_Text.data();
         return C_NO_ERR;
      }
   }
   
   return C_CONFIG;
}
