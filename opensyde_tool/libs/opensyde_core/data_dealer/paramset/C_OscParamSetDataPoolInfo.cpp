//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class, containing information about the data pools in a
   parameter set file (implementation)

   Data class, containing information about the data pools in a parameter set
   file

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscParamSetDataPoolInfo.hpp"
#include "stwerrors.hpp"
#include <QJsonArray>
#include <QJsonValue>

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
C_OscParamSetDataPoolInfo::C_OscParamSetDataPoolInfo(void)
    : u32_DataPoolCrc(0), u32_NvmStartAddress(0), u32_NvmSize(0) {
  au8_Version[0] = 0;
  au8_Version[1] = 0;
  au8_Version[2] = 0;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDataStream (binary format)
   
   \param[out] orc_Stream    Output stream for serialization
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetDataPoolInfo::ToQDataStream(QDataStream& orc_Stream) const {
   orc_Stream << c_Name;
   orc_Stream << u32_DataPoolCrc;
   orc_Stream << u32_NvmStartAddress;
   orc_Stream << u32_NvmSize;
   orc_Stream.writeRawData(reinterpret_cast<const char*>(au8_Version), 3);
   
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
int32_t C_OscParamSetDataPoolInfo::FromQDataStream(QDataStream& orc_Stream) {
   orc_Stream >> c_Name;
   orc_Stream >> u32_DataPoolCrc;
   orc_Stream >> u32_NvmStartAddress;
   orc_Stream >> u32_NvmSize;
   orc_Stream.readRawData(reinterpret_cast<char*>(au8_Version), 3);
   
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
QJsonObject C_OscParamSetDataPoolInfo::ToJsonObject() const {
   QJsonObject c_Object;
   
   c_Object["name"] = c_Name;
   c_Object["data-pool-crc"] = static_cast<qint64>(u32_DataPoolCrc);
   c_Object["nvm-start-address"] = static_cast<qint64>(u32_NvmStartAddress);
   c_Object["nvm-size"] = static_cast<qint64>(u32_NvmSize);
   
   // Version array
   QJsonArray c_VersionArray;
   c_VersionArray.append(static_cast<qint64>(au8_Version[0]));
   c_VersionArray.append(static_cast<qint64>(au8_Version[1]));
   c_VersionArray.append(static_cast<qint64>(au8_Version[2]));
   c_Object["version"] = c_VersionArray;
   
   return c_Object;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QJsonObject
   
   \param[in] orc_Object    JSON object to deserialize from
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetDataPoolInfo::FromJsonObject(const QJsonObject& orc_Object) {
   if (orc_Object.contains("name") && 
       orc_Object.contains("data-pool-crc") && 
       orc_Object.contains("nvm-start-address") && 
       orc_Object.contains("nvm-size") && 
       orc_Object.contains("version")) {
      
      c_Name = orc_Object["name"].toString();
      u32_DataPoolCrc = static_cast<uint32_t>(orc_Object["data-pool-crc"].toInt());
      u32_NvmStartAddress = static_cast<uint32_t>(orc_Object["nvm-start-address"].toInt());
      u32_NvmSize = static_cast<uint32_t>(orc_Object["nvm-size"].toInt());
      
      QJsonArray c_VersionArray = orc_Object["version"].toArray();
      if (c_VersionArray.size() >= 3) {
         au8_Version[0] = static_cast<uint8_t>(c_VersionArray[0].toInt());
         au8_Version[1] = static_cast<uint8_t>(c_VersionArray[1].toInt());
         au8_Version[2] = static_cast<uint8_t>(c_VersionArray[2].toInt());
      }
      
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
QDomElement C_OscParamSetDataPoolInfo::ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
   
   // Add name
   QDomElement c_NameElement = orc_Doc.createElement("name");
   QDomText c_NameText = orc_Doc.createTextNode(c_Name);
   c_NameElement.appendChild(c_NameText);
   c_Element.appendChild(c_NameElement);
   
   // Add data pool CRC
   QDomElement c_DataPoolCrcElement = orc_Doc.createElement("data-pool-crc");
   QDomText c_DataPoolCrcText = orc_Doc.createTextNode(QString::number(u32_DataPoolCrc));
   c_DataPoolCrcElement.appendChild(c_DataPoolCrcText);
   c_Element.appendChild(c_DataPoolCrcElement);
   
   // Add NVM start address
   QDomElement c_NvmStartAddrElement = orc_Doc.createElement("nvm-start-address");
   QDomText c_NvmStartAddrText = orc_Doc.createTextNode(QString::number(u32_NvmStartAddress));
   c_NvmStartAddrElement.appendChild(c_NvmStartAddrText);
   c_Element.appendChild(c_NvmStartAddrElement);
   
   // Add NVM size
   QDomElement c_NvmSizeElement = orc_Doc.createElement("nvm-size");
   QDomText c_NvmSizeText = orc_Doc.createTextNode(QString::number(u32_NvmSize));
   c_NvmSizeElement.appendChild(c_NvmSizeText);
   c_Element.appendChild(c_NvmSizeElement);
   
   // Add version
   QDomElement c_VersionElement = orc_Doc.createElement("version");
   QDomText c_VersionText1 = orc_Doc.createTextNode(QString::number(au8_Version[0]));
   QDomText c_VersionText2 = orc_Doc.createTextNode(QString::number(au8_Version[1]));
   QDomText c_VersionText3 = orc_Doc.createTextNode(QString::number(au8_Version[2]));
   c_VersionElement.appendChild(c_VersionText1);
   c_VersionElement.appendChild(c_VersionText2);
   c_VersionElement.appendChild(c_VersionText3);
   c_Element.appendChild(c_VersionElement);
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QDomElement
   
   \param[in] orc_Element    XML element to deserialize from
   
   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetDataPoolInfo::FromQDomElement(const QDomElement& orc_Element) {
   QDomNode c_NameNode = orc_Element.namedItem("name");
   QDomNode c_DataPoolCrcNode = orc_Element.namedItem("data-pool-crc");
   QDomNode c_NvmStartAddrNode = orc_Element.namedItem("nvm-start-address");
   QDomNode c_NvmSizeNode = orc_Element.namedItem("nvm-size");
   QDomNode c_VersionNode = orc_Element.namedItem("version");
   
   if (!c_NameNode.isNull() && 
       !c_DataPoolCrcNode.isNull() && 
       !c_NvmStartAddrNode.isNull() && 
       !c_NvmSizeNode.isNull() &&
       !c_VersionNode.isNull()) {
      
      QDomText c_NameText = c_NameNode.toText();
      QDomText c_DataPoolCrcText = c_DataPoolCrcNode.toText();
      QDomText c_NvmStartAddrText = c_NvmStartAddrNode.toText();
      QDomText c_NvmSizeText = c_NvmSizeNode.toText();
      
      if (!c_NameText.isNull() && !c_DataPoolCrcText.isNull() && 
          !c_NvmStartAddrText.isNull() && !c_NvmSizeText.isNull()) {
         
         c_Name = c_NameText.data();
         u32_DataPoolCrc = static_cast<uint32_t>(c_DataPoolCrcText.data().toInt());
         u32_NvmStartAddress = static_cast<uint32_t>(c_NvmStartAddrText.data().toInt());
         u32_NvmSize = static_cast<uint32_t>(c_NvmSizeText.data().toInt());
         
         // Handle version
         QDomNode c_VersionChild1 = c_VersionNode.firstChild();
         QDomNode c_VersionChild2 = c_VersionChild1.nextSibling();
         QDomNode c_VersionChild3 = c_VersionChild2.nextSibling();
         
         if (!c_VersionChild1.isNull() && !c_VersionChild2.isNull() && !c_VersionChild3.isNull()) {
            QDomText c_VersionText1 = c_VersionChild1.toText();
            QDomText c_VersionText2 = c_VersionChild2.toText();
            QDomText c_VersionText3 = c_VersionChild3.toText();
            
            if (!c_VersionText1.isNull() && !c_VersionText2.isNull() && !c_VersionText3.isNull()) {
               au8_Version[0] = static_cast<uint8_t>(c_VersionText1.data().toInt());
               au8_Version[1] = static_cast<uint8_t>(c_VersionText2.data().toInt());
               au8_Version[2] = static_cast<uint8_t>(c_VersionText3.data().toInt());
               
               return C_NO_ERR;
            }
         }
      }
   }
   
   return C_CONFIG;
}
