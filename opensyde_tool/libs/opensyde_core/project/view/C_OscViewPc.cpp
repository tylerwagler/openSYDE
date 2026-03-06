//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       System view PC item data element (implementation)

   System view PC item data element

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include <limits>

#include "C_OscViewPc.hpp"
#include "C_OscHashUtil.hpp"
#include "C_SclChecksums.hpp"
#include "stwtypes.hpp"
#include "stwerrors.hpp"
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
C_OscViewPc::C_OscViewPc(void)
    : mq_Connected(false), mu32_BusIndex(std::numeric_limits<uint32_t>::max()) {
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with init [in] value and result
   [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewPc::CalcHash(uint32_t & oru32_HashValue) const
{
   hash_util::CalcHashMembers(oru32_HashValue, this->mq_Connected, this->mu32_BusIndex);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get connected flag

   \return
   Current connected flag
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscViewPc::GetConnected(void) const { return this->mq_Connected; }

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get bus index

   \return
   Current bus index
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscViewPc::GetBusIndex(void) const { return this->mu32_BusIndex; }

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set connected state of pc in view

   \param[in]  oq_Connected         Flag if pc is connected
   \param[in]  ou32_BusIndex        Bus index PC is connected to
   \param[in]  orq_ForceSimpleSet   Optional flag to indicate if this function
   is used as a simple set or a logic operation
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewPc::SetConnected(const bool oq_Connected,
                               const uint32_t ou32_BusIndex,
                               const bool &orq_ForceSimpleSet) {
  this->mq_Connected = oq_Connected;
  if ((orq_ForceSimpleSet == true) || (this->mq_Connected == true)) {
    this->mu32_BusIndex = ou32_BusIndex;
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sync view bus index to added bus index

   \param[in]  ou32_Index  Added bus index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewPc::OnSyncBusAdded(const uint32_t ou32_Index) {
  if (ou32_Index < this->mu32_BusIndex) {
    ++this->mu32_BusIndex;
  } else if (ou32_Index == this->mu32_BusIndex) {
    if (this->mq_Connected == false) {
      this->mq_Connected = true;
    } else {
      ++this->mu32_BusIndex;
    }
  } else {
    // No handling necessary
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sync view bus index to deleted bus index

   \param[in]  ou32_Index  Deleted bus index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewPc::OnSyncBusDeleted(const uint32_t ou32_Index) {
  if (ou32_Index < this->mu32_BusIndex) {
    --this->mu32_BusIndex;
  } else if (ou32_Index == this->mu32_BusIndex) {
    this->mq_Connected = false;
  } else {
    // No handling necessary
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility function to check if disconnect was only triggered
   indirectly

   \return
   False All valid state
   True  Disconnect was not triggered properly
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscViewPc::CheckIndirectDisconnection(void) const {
  bool q_Retval;

  if ((this->mq_Connected == false) &&
      (this->mu32_BusIndex != std::numeric_limits<uint32_t>::max())) {
    q_Retval = true;
  } else {
    q_Retval = false;
  }
  return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDataStream (binary format)

   \param[out] orc_Stream    Output stream for serialization

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewPc::ToQDataStream(QDataStream& orc_Stream) const {
   using namespace stw::errors;
   
   orc_Stream << mq_Connected;
   orc_Stream << mu32_BusIndex;
   
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
int32_t C_OscViewPc::FromQDataStream(QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   orc_Stream >> mq_Connected;
   orc_Stream >> mu32_BusIndex;
   
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
QJsonObject C_OscViewPc::ToJsonObject() const {
   QJsonObject c_Object;
   
   c_Object["connected"] = mq_Connected;
   c_Object["bus-index"] = static_cast<qlonglong>(mu32_BusIndex);
   
   return c_Object;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QJsonObject

   \param[in] orc_Object    JSON object to deserialize from

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewPc::FromJsonObject(const QJsonObject& orc_Object) {
   using namespace stw::errors;
   
   if (orc_Object.contains("connected")) {
      mq_Connected = orc_Object["connected"].toBool();
   }
   
   if (orc_Object.contains("bus-index")) {
      mu32_BusIndex = static_cast<uint32_t>(orc_Object["bus-index"].toInteger());
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
QDomElement C_OscViewPc::ToQDomDocument(QDomDocument& orc_Doc, 
                                        const QString& orc_RootElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
   
   QDomElement c_ConnectedElement = orc_Doc.createElement("connected");
   c_ConnectedElement.appendChild(orc_Doc.createTextNode(mq_Connected ? "1" : "0"));
   c_Element.appendChild(c_ConnectedElement);
   
   QDomElement c_BusIndexElement = orc_Doc.createElement("bus-index");
   c_BusIndexElement.appendChild(orc_Doc.createTextNode(QString::number(mu32_BusIndex)));
   c_Element.appendChild(c_BusIndexElement);
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QDomElement

   \param[in] orc_Element    XML element to deserialize from

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewPc::FromQDomElement(const QDomElement& orc_Element) {
   using namespace stw::errors;
   
   QDomNode c_ConnectedNode = orc_Element.namedItem("connected");
   if (!c_ConnectedNode.isNull()) {
      mq_Connected = (c_ConnectedNode.toElement().text() == "1");
   }
   
   QDomNode c_BusIndexNode = orc_Element.namedItem("bus-index");
   if (!c_BusIndexNode.isNull()) {
      mu32_BusIndex = static_cast<uint32_t>(c_BusIndexNode.toElement().text().toUInt());
   }
   
   return C_NO_ERR;
}
