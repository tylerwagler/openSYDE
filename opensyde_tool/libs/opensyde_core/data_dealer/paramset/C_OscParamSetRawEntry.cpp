//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for one raw entry in a parameter set file
   (implementation)

   Data class for one raw entry in a parameter set file

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscParamSetRawEntry.hpp"
#include "stwerrors.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
using namespace stw::opensyde_core;
using namespace stw::errors;

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
/*! \brief   Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscParamSetRawEntry::C_OscParamSetRawEntry(void) : u32_StartAddress(0) {}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDataStream (binary format)
   
   \param[out] orc_Stream    Output stream for serialization
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawEntry::ToQDataStream(QDataStream& orc_Stream) const {
   orc_Stream << u32_StartAddress;
   orc_Stream << c_Bytes;
   
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
int32_t C_OscParamSetRawEntry::FromQDataStream(QDataStream& orc_Stream) {
   orc_Stream >> u32_StartAddress;
   orc_Stream >> c_Bytes;
   
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
QJsonObject C_OscParamSetRawEntry::ToJsonObject() const {
   QJsonObject c_Object;
   
   c_Object["start-address"] = static_cast<qint64>(u32_StartAddress);
   c_Object["bytes"] = QString(c_Bytes.toBase64());
   
   return c_Object;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QJsonObject
   
   \param[in] orc_Object    JSON object to deserialize from
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawEntry::FromJsonObject(const QJsonObject& orc_Object) {
   if (orc_Object.contains("start-address") && orc_Object.contains("bytes")) {
      u32_StartAddress = static_cast<uint32_t>(orc_Object["start-address"].toInt());
      
      QString c_Base64Bytes = orc_Object["bytes"].toString();
      c_Bytes = QByteArray::fromBase64(c_Base64Bytes.toLatin1());
      
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
QDomElement C_OscParamSetRawEntry::ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
   
   // Add start address
   QDomElement c_StartAddrElement = orc_Doc.createElement("start-address");
   QDomText c_StartAddrText = orc_Doc.createTextNode(QString::number(u32_StartAddress));
   c_StartAddrElement.appendChild(c_StartAddrText);
   c_Element.appendChild(c_StartAddrElement);
   
   // Add bytes
   QDomElement c_BytesElement = orc_Doc.createElement("bytes");
   QDomText c_BytesText = orc_Doc.createTextNode(c_Bytes.toBase64());
   c_BytesElement.appendChild(c_BytesText);
   c_Element.appendChild(c_BytesElement);
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QDomElement
   
   \param[in] orc_Element    XML element to deserialize from
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetRawEntry::FromQDomElement(const QDomElement& orc_Element) {
   QDomNode c_StartAddrNode = orc_Element.namedItem("start-address");
   QDomNode c_BytesNode = orc_Element.namedItem("bytes");
   
   if (!c_StartAddrNode.isNull() && !c_BytesNode.isNull()) {
      QDomText c_StartAddrText = c_StartAddrNode.toText();
      QDomText c_BytesText = c_BytesNode.toText();
      
      if (!c_StartAddrText.isNull() && !c_BytesText.isNull()) {
         u32_StartAddress = static_cast<uint32_t>(c_StartAddrText.data().toInt());
         c_Bytes = QByteArray::fromBase64(c_BytesText.data().toLatin1());
         
         return C_NO_ERR;
      }
   }
   
   return C_CONFIG;
}
