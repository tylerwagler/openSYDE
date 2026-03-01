//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node update information for parameter sets (implementation)

   Node update information for parameter sets

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscViewNodeUpdateParamInfo.hpp"
#include "C_SclChecksums.hpp"
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include <QString>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QDomDocument>

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
using namespace stw::scl;
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
/*! \brief   Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscViewNodeUpdateParamInfo::C_OscViewNodeUpdateParamInfo(void)
    : mc_FilePath(""), mu32_LastKnownCrc(0UL) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue Hash value with init [in] value and result
   [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdateParamInfo::CalcHash(uint32_t &oru32_HashValue) const {
  C_SclChecksums::CalcCRC32(&this->mu32_LastKnownCrc,
                            sizeof(this->mu32_LastKnownCrc), oru32_HashValue);
  C_SclChecksums::CalcCRC32(this->mc_FilePath.toUtf8().constData(),
                            this->mc_FilePath.length(), oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set file information

   \param[in] orc_FilePath      File path
   \param[in] ou32_LastKnownCrc Last known CRC value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdateParamInfo::SetContent(
    const QString &orc_FilePath, const uint32_t ou32_LastKnownCrc) {
  this->mc_FilePath = orc_FilePath;
  this->mu32_LastKnownCrc = ou32_LastKnownCrc;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current path

   \return
   Current path
*/
//----------------------------------------------------------------------------------------------------------------------
const QString &C_OscViewNodeUpdateParamInfo::GetPath(void) const {
  return this->mc_FilePath;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get last known CRC value

   \return
   Last known CRC value
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscViewNodeUpdateParamInfo::GetLastKnownCrc(void) const {
  return this->mu32_LastKnownCrc;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDataStream (binary format)

   \param[out] orc_Stream    Output stream for serialization

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewNodeUpdateParamInfo::ToQDataStream(QDataStream& orc_Stream) const {
   using namespace stw::errors;
   
   orc_Stream << mc_FilePath;
   orc_Stream << mu32_LastKnownCrc;
   
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
int32_t C_OscViewNodeUpdateParamInfo::FromQDataStream(QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   orc_Stream >> mc_FilePath;
   orc_Stream >> mu32_LastKnownCrc;
   
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
QJsonObject C_OscViewNodeUpdateParamInfo::ToJsonObject() const {
   QJsonObject c_Object;
   
   c_Object["file-path"] = mc_FilePath;
   c_Object["last-known-crc"] = static_cast<qlonglong>(mu32_LastKnownCrc);
   
   return c_Object;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QJsonObject

   \param[in] orc_Object    JSON object to deserialize from

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewNodeUpdateParamInfo::FromJsonObject(const QJsonObject& orc_Object) {
   using namespace stw::errors;
   
   if (orc_Object.contains("file-path")) {
      mc_FilePath = orc_Object["file-path"].toString();
   }
   
   if (orc_Object.contains("last-known-crc")) {
      mu32_LastKnownCrc = static_cast<uint32_t>(orc_Object["last-known-crc"].toInteger());
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
QDomElement C_OscViewNodeUpdateParamInfo::ToQDomDocument(QDomDocument& orc_Doc, 
                                                         const QString& orc_RootElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
   
   QDomElement c_FilePathElement = orc_Doc.createElement("file-path");
   c_FilePathElement.appendChild(orc_Doc.createTextNode(mc_FilePath));
   c_Element.appendChild(c_FilePathElement);
   
   QDomElement c_CrcElement = orc_Doc.createElement("last-known-crc");
   c_CrcElement.appendChild(orc_Doc.createTextNode(QString::number(mu32_LastKnownCrc)));
   c_Element.appendChild(c_CrcElement);
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QDomElement

   \param[in] orc_Element    XML element to deserialize from

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewNodeUpdateParamInfo::FromQDomElement(const QDomElement& orc_Element) {
   using namespace stw::errors;
   
   QDomNode c_FilePathNode = orc_Element.namedItem("file-path");
   if (!c_FilePathNode.isNull()) {
      mc_FilePath = c_FilePathNode.toElement().text();
   }
   
   QDomNode c_CrcNode = orc_Element.namedItem("last-known-crc");
   if (!c_CrcNode.isNull()) {
      mu32_LastKnownCrc = static_cast<uint32_t>(c_CrcNode.toElement().text().toUInt());
   }
   
   return C_NO_ERR;
}
