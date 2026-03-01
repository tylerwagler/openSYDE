//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Xapp update package parameters data

   Xapp update package parameters data

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscXceUpdatePackageParameters.hpp"
#include "stwerrors.hpp"
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
C_OscXceUpdatePackageParameters::C_OscXceUpdatePackageParameters() {
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDataStream (binary format)
   
   \param[out] orc_Stream    Output stream for serialization
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceUpdatePackageParameters::ToQDataStream(QDataStream& orc_Stream) const {
   using namespace stw::errors;
   
   orc_Stream << c_Password << c_AuthenticationKeyPath;
   
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
int32_t C_OscXceUpdatePackageParameters::FromQDataStream(QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   orc_Stream >> c_Password >> c_AuthenticationKeyPath;
   
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
QJsonObject C_OscXceUpdatePackageParameters::ToJsonObject() const {
   QJsonObject c_Object;
   
   c_Object["password"] = c_Password;
   c_Object["authentication-key-path"] = c_AuthenticationKeyPath;
   
   return c_Object;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QJsonObject
   
   \param[in] orc_Object    JSON object to deserialize from
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceUpdatePackageParameters::FromJsonObject(const QJsonObject& orc_Object) {
   using namespace stw::errors;
   
   if (orc_Object.contains("password") && orc_Object.contains("authentication-key-path")) {
      c_Password = orc_Object["password"].toString();
      c_AuthenticationKeyPath = orc_Object["authentication-key-path"].toString();
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
QDomElement C_OscXceUpdatePackageParameters::ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
   
   QDomElement c_PasswordElement = orc_Doc.createElement("password");
   QDomText c_PasswordText = orc_Doc.createTextNode(c_Password);
   c_PasswordElement.appendChild(c_PasswordText);
   c_Element.appendChild(c_PasswordElement);
   
   QDomElement c_KeyPathElement = orc_Doc.createElement("authentication-key-path");
   QDomText c_KeyPathText = orc_Doc.createTextNode(c_AuthenticationKeyPath);
   c_KeyPathElement.appendChild(c_KeyPathText);
   c_Element.appendChild(c_KeyPathElement);
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QDomElement
   
   \param[in] orc_Element    XML element to deserialize from
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceUpdatePackageParameters::FromQDomElement(const QDomElement& orc_Element) {
   using namespace stw::errors;
   
   // Parse password
   QDomNode c_PasswordNode = orc_Element.namedItem("password");
   if (c_PasswordNode.isNull()) {
      return C_CONFIG;
   }
   QDomText c_PasswordText = c_PasswordNode.toText();
   if (c_PasswordText.isNull()) {
      return C_CONFIG;
   }
   c_Password = c_PasswordText.data();
   
   // Parse authentication-key-path
   QDomNode c_KeyPathNode = orc_Element.namedItem("authentication-key-path");
   if (c_KeyPathNode.isNull()) {
      return C_CONFIG;
   }
   QDomText c_KeyPathText = c_KeyPathNode.toText();
   if (c_KeyPathText.isNull()) {
      return C_CONFIG;
   }
   c_AuthenticationKeyPath = c_KeyPathText.data();
   
   return C_NO_ERR;
}
