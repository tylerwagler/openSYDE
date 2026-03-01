//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Xapp certificates package data

   Xapp certificates package data

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscXceManifest.hpp"
#include "stwerrors.hpp"
#include "stwtypes.hpp"
#include <QJsonArray>
#include <QJsonValue>

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
C_OscXceManifest::C_OscXceManifest() {
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDataStream (binary format)
   
   \param[out] orc_Stream    Output stream for serialization
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifest::ToQDataStream(QDataStream& orc_Stream) const {
   using namespace stw::errors;
   
   orc_Stream << c_CertificatesPath;
   
   // Serialize list
   orc_Stream << qint32(c_UpdatePackageParameters.size());
   for (const C_OscXceUpdatePackageParameters& c_Param : c_UpdatePackageParameters) {
      c_Param.ToQDataStream(orc_Stream);
   }
   
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
int32_t C_OscXceManifest::FromQDataStream(QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   orc_Stream >> c_CertificatesPath;
   
   // Deserialize list
   qint32 s32_Size;
   orc_Stream >> s32_Size;
   c_UpdatePackageParameters.clear();
   for (qint32 s32_Index = 0; s32_Index < s32_Size; s32_Index++) {
      C_OscXceUpdatePackageParameters c_Param;
      c_Param.FromQDataStream(orc_Stream);
      c_UpdatePackageParameters.append(c_Param);
   }
   
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
QJsonObject C_OscXceManifest::ToJsonObject() const {
   QJsonObject c_Object;
   
   c_Object["certificates-path"] = c_CertificatesPath;
   
   QJsonArray c_ParamsArray;
   for (const C_OscXceUpdatePackageParameters& c_Param : c_UpdatePackageParameters) {
      c_ParamsArray.append(c_Param.ToJsonObject());
   }
   c_Object["update-package-parameters"] = c_ParamsArray;
   
   return c_Object;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QJsonObject
   
   \param[in] orc_Object    JSON object to deserialize from
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifest::FromJsonObject(const QJsonObject& orc_Object) {
   using namespace stw::errors;
   
   if (!orc_Object.contains("certificates-path")) {
      return C_CONFIG;
   }
   
   c_CertificatesPath = orc_Object["certificates-path"].toString();
   
   // Parse array
   if (!orc_Object.contains("update-package-parameters")) {
      return C_CONFIG;
   }
   
   QJsonArray c_ParamsArray = orc_Object["update-package-parameters"].toArray();
   c_UpdatePackageParameters.clear();
   for (const QJsonValue& c_Value : c_ParamsArray) {
      C_OscXceUpdatePackageParameters c_Param;
      if (c_Param.FromJsonObject(c_Value.toObject()) != C_NO_ERR) {
         return C_CONFIG;
      }
      c_UpdatePackageParameters.append(c_Param);
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDomDocument
   
   \param[in] orc_Doc    XML document to append to
   \param[in] orc_RootElementName    Name of the root element to create
   
   \return QDomElement representing the serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscXceManifest::ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
   
   // Certificates path
   QDomElement c_PathElement = orc_Doc.createElement("certificates-path");
   QDomText c_PathText = orc_Doc.createTextNode(c_CertificatesPath);
   c_PathElement.appendChild(c_PathText);
   c_Element.appendChild(c_PathElement);
   
   // Update package parameters
   QDomElement c_ParamsElement = orc_Doc.createElement("update-package-parameters");
   for (const C_OscXceUpdatePackageParameters& c_Param : c_UpdatePackageParameters) {
      QDomElement c_ParamElement = c_Param.ToQDomDocument(orc_Doc, "parameter");
      c_ParamsElement.appendChild(c_ParamElement);
   }
   c_Element.appendChild(c_ParamsElement);
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QDomElement
   
   \param[in] orc_Element    XML element to deserialize from
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXceManifest::FromQDomElement(const QDomElement& orc_Element) {
   using namespace stw::errors;
   
   // Parse certificates-path
   QDomNode c_PathNode = orc_Element.namedItem("certificates-path");
   if (c_PathNode.isNull()) {
      return C_CONFIG;
   }
   QDomText c_PathText = c_PathNode.toText();
   if (c_PathText.isNull()) {
      return C_CONFIG;
   }
   c_CertificatesPath = c_PathText.data();
   
   // Parse update-package-parameters
   QDomNode c_ParamsNode = orc_Element.namedItem("update-package-parameters");
   if (c_ParamsNode.isNull()) {
      return C_CONFIG;
   }
   
   QDomElement c_ParamsElement = c_ParamsNode.toElement();
   QDomNode c_ParamNode = c_ParamsElement.firstChild();
   
   c_UpdatePackageParameters.clear();
   while (!c_ParamNode.isNull()) {
      QDomElement c_ParamElement = c_ParamNode.toElement();
      if (!c_ParamElement.isNull()) {
         C_OscXceUpdatePackageParameters c_Param;
         if (c_Param.FromQDomElement(c_ParamElement) != C_NO_ERR) {
            return C_CONFIG;
         }
         c_UpdatePackageParameters.append(c_Param);
      }
      c_ParamNode = c_ParamNode.nextSibling();
   }
   
   return C_NO_ERR;
}
