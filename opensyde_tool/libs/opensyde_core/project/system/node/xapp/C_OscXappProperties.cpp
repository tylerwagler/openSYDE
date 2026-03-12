//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       X-app properties

   X-app properties

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscXappProperties.hpp"
#include "C_OscHashUtil.hpp"
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
C_OscXappProperties::C_OscXappProperties()
   : u32_PollingIntervalMs(10U), u32_DataRequestIntervalMs(100U),
     e_ConnectedInterfaceType(C_OscSystemBus::eCAN),
     u8_ConnectedInterfaceNumber(0U) {
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Initialize
 */
//----------------------------------------------------------------------------------------------------------------------
void C_OscXappProperties::Initialize() {
   u32_PollingIntervalMs = 10U;
   u32_DataRequestIntervalMs = 100U;
   e_ConnectedInterfaceType = C_OscSystemBus::eCAN;
   u8_ConnectedInterfaceNumber = 0U;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over all data

   The hash value is a 32 bit CRC value.
   It is not endian-safe, so it should only be used on the same system it is
   created on.

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and
   result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscXappProperties::CalcHash(uint32_t & oru32_HashValue) const
{
   hash_util::CalcHashMembers(oru32_HashValue,
                              this->u32_PollingIntervalMs, this->u32_DataRequestIntervalMs,
                              this->e_ConnectedInterfaceType, this->u8_ConnectedInterfaceNumber);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDataStream (binary format)
   
   \param[out] orc_Stream    Output stream for serialization
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXappProperties::ToQDataStream(QDataStream& orc_Stream) const {
   using namespace stw::errors;
   
   orc_Stream << u32_PollingIntervalMs << u32_DataRequestIntervalMs
              << static_cast<int32_t>(e_ConnectedInterfaceType)
              << u8_ConnectedInterfaceNumber;
   
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
int32_t C_OscXappProperties::FromQDataStream(QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   int32_t s32_InterfaceType;
   orc_Stream >> u32_PollingIntervalMs >> u32_DataRequestIntervalMs
              >> s32_InterfaceType >> u8_ConnectedInterfaceNumber;
   
   e_ConnectedInterfaceType = static_cast<C_OscSystemBus::E_Type>(s32_InterfaceType);
   
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
QJsonObject C_OscXappProperties::ToJsonObject() const {
   QJsonObject c_Object;
   
   c_Object["polling-interval-ms"] = static_cast<int>(u32_PollingIntervalMs);
   c_Object["data-request-interval-ms"] = static_cast<int>(u32_DataRequestIntervalMs);
   c_Object["connected-interface-type"] = static_cast<int>(e_ConnectedInterfaceType);
   c_Object["connected-interface-number"] = static_cast<int>(u8_ConnectedInterfaceNumber);
   
   return c_Object;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QJsonObject
   
   \param[in] orc_Object    JSON object to deserialize from
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXappProperties::FromJsonObject(const QJsonObject& orc_Object) {
   using namespace stw::errors;
   
   if (!orc_Object.contains("polling-interval-ms") ||
       !orc_Object.contains("data-request-interval-ms") ||
       !orc_Object.contains("connected-interface-type") ||
       !orc_Object.contains("connected-interface-number")) {
      return C_CONFIG;
   }
   
   u32_PollingIntervalMs = static_cast<uint32_t>(orc_Object["polling-interval-ms"].toInt());
   u32_DataRequestIntervalMs = static_cast<uint32_t>(orc_Object["data-request-interval-ms"].toInt());
   e_ConnectedInterfaceType = static_cast<C_OscSystemBus::E_Type>(orc_Object["connected-interface-type"].toInt());
   u8_ConnectedInterfaceNumber = static_cast<uint8_t>(orc_Object["connected-interface-number"].toInt());
   
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
QDomElement C_OscXappProperties::ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
   
   QDomElement c_PollingElement = orc_Doc.createElement("polling-interval-ms");
   QDomText c_PollingText = orc_Doc.createTextNode(QString::number(u32_PollingIntervalMs));
   c_PollingElement.appendChild(c_PollingText);
   c_Element.appendChild(c_PollingElement);
   
   QDomElement c_RequestElement = orc_Doc.createElement("data-request-interval-ms");
   QDomText c_RequestText = orc_Doc.createTextNode(QString::number(u32_DataRequestIntervalMs));
   c_RequestElement.appendChild(c_RequestText);
   c_Element.appendChild(c_RequestElement);
   
   QDomElement c_TypeElement = orc_Doc.createElement("connected-interface-type");
   QDomText c_TypeText = orc_Doc.createTextNode(QString::number(static_cast<int>(e_ConnectedInterfaceType)));
   c_TypeElement.appendChild(c_TypeText);
   c_Element.appendChild(c_TypeElement);
   
   QDomElement c_NumberElement = orc_Doc.createElement("connected-interface-number");
   QDomText c_NumberText = orc_Doc.createTextNode(QString::number(u8_ConnectedInterfaceNumber));
   c_NumberElement.appendChild(c_NumberText);
   c_Element.appendChild(c_NumberElement);
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QDomElement
   
   \param[in] orc_Element    XML element to deserialize from
   
    \return QDomElement representing the serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscXappProperties::ToQDomElement(QDomDocument& orc_Doc, const QString& orc_RootElementName) const {
    QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
    
    // Serialize polling-interval-ms
    QDomElement c_PollingElement = orc_Doc.createElement("polling-interval-ms");
    QDomText c_PollingText = orc_Doc.createTextNode(QString::number(u32_PollingIntervalMs));
    c_PollingElement.appendChild(c_PollingText);
    c_Element.appendChild(c_PollingElement);
    
    // Serialize data-request-interval-ms
    QDomElement c_RequestElement = orc_Doc.createElement("data-request-interval-ms");
    QDomText c_RequestText = orc_Doc.createTextNode(QString::number(u32_DataRequestIntervalMs));
    c_RequestElement.appendChild(c_RequestText);
    c_Element.appendChild(c_RequestElement);
    
    // Serialize connected-interface-type
    QDomElement c_TypeElement = orc_Doc.createElement("connected-interface-type");
    QString c_TypeText;
    switch (e_ConnectedInterfaceType) {
        case C_OscSystemBus::eCAN:
            c_TypeText = "can";
            break;
        case C_OscSystemBus::eETHERNET:
            c_TypeText = "ethernet";
            break;
        default:
            c_TypeText = "unknown";
            break;
    }
    QDomText c_TypeTextNode = orc_Doc.createTextNode(c_TypeText);
    c_TypeElement.appendChild(c_TypeTextNode);
    c_Element.appendChild(c_TypeElement);
    
    // Serialize connected-interface-number
    QDomElement c_NumberElement = orc_Doc.createElement("connected-interface-number");
    QDomText c_NumberText = orc_Doc.createTextNode(QString::number(u8_ConnectedInterfaceNumber));
    c_NumberElement.appendChild(c_NumberText);
    c_Element.appendChild(c_NumberElement);
    
    return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
    \brief Deserialize from QDomElement

    \param[in] orc_Element    XML element to deserialize from
    
    \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscXappProperties::FromQDomElement(const QDomElement& orc_Element) {
   using namespace stw::errors;
   
   // Parse polling-interval-ms
   QDomNode c_PollingNode = orc_Element.namedItem("polling-interval-ms");
   if (c_PollingNode.isNull()) return C_CONFIG;
   u32_PollingIntervalMs = c_PollingNode.toElement().text().toUInt();
   
   // Parse data-request-interval-ms
   QDomNode c_RequestNode = orc_Element.namedItem("data-request-interval-ms");
   if (c_RequestNode.isNull()) return C_CONFIG;
   u32_DataRequestIntervalMs = c_RequestNode.toElement().text().toUInt();
   
   // Parse connected-interface-type
   QDomNode c_TypeNode = orc_Element.namedItem("connected-interface-type");
   if (c_TypeNode.isNull()) return C_CONFIG;
   e_ConnectedInterfaceType = static_cast<C_OscSystemBus::E_Type>(c_TypeNode.toElement().text().toInt());
   
   // Parse connected-interface-number
   QDomNode c_NumberNode = orc_Element.namedItem("connected-interface-number");
   if (c_NumberNode.isNull()) return C_CONFIG;
   u8_ConnectedInterfaceNumber = static_cast<uint8_t>(c_NumberNode.toElement().text().toUInt());
   
   return C_NO_ERR;
}
