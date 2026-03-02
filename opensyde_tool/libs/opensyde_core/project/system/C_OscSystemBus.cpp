//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Store bus

   Data container class for all information describing a bus.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscSystemBus.hpp"

#include "C_SclChecksums.hpp"
#include "stwerrors.hpp"
#include <QString>
#include <QJsonArray>
#include <QJsonValue>
#include <QDomDocument>
#include <QDomElement>

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
/*! \brief  Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscSystemBus::C_OscSystemBus(void)
    : e_Type(eCAN), c_Name("NewBus"), c_Comment(""), u64_BitRate(125000ULL),
      q_UseCanFd(false), u64_CanFdBitRate(1000000ULL), u8_BusId(0),
      u16_RxTimeoutOffsetMs(0), q_UseableForRouting(true) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default destructor

   Clean up.
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscSystemBus::~C_OscSystemBus(void) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over all data

   The hash value is a 32 bit CRC value.
   It is not endian-safe, so it should only be used on the same system it is
   created on.

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and
   result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSystemBus::CalcHash(uint32_t &oru32_HashValue) const {
  C_SclChecksums::CalcCRC32(&this->e_Type, sizeof(this->e_Type),
                            oru32_HashValue);
  // Convert QString to C string for checksum calculation
  const QByteArray nameBytes = this->c_Name.toLatin1();
  const QByteArray commentBytes = this->c_Comment.toLatin1();
  C_SclChecksums::CalcCRC32(nameBytes.constData(), nameBytes.size(),
                            oru32_HashValue);
  C_SclChecksums::CalcCRC32(commentBytes.constData(), commentBytes.size(),
                            oru32_HashValue);
  C_SclChecksums::CalcCRC32(&this->u64_BitRate, sizeof(this->u64_BitRate),
                            oru32_HashValue);
  C_SclChecksums::CalcCRC32(&this->q_UseCanFd, sizeof(this->q_UseCanFd),
                            oru32_HashValue);
  C_SclChecksums::CalcCRC32(&this->u64_CanFdBitRate,
                            sizeof(this->u64_CanFdBitRate), oru32_HashValue);
  C_SclChecksums::CalcCRC32(&this->u8_BusId, sizeof(this->u8_BusId),
                            oru32_HashValue);
  C_SclChecksums::CalcCRC32(&this->u16_RxTimeoutOffsetMs,
                            sizeof(this->u16_RxTimeoutOffsetMs),
                            oru32_HashValue);
  C_SclChecksums::CalcCRC32(&this->q_UseableForRouting,
                            sizeof(this->q_UseableForRouting), oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Compare bus names for greater

   Buses are compared by name.
   Primary sorting criteria: Name length: Shortest first
   Secondary sorting criteria: First difference in alphabetic ordering

   \param[in]  orc_Bus1    Bus 1
   \param[in]  orc_Bus2    Bus 2

   \return
   true:  Bus 1 smaller than Bus 2
   false: Else
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscSystemBus::h_CompareNameGreater(const C_OscSystemBus &orc_Bus1,
                                          const C_OscSystemBus &orc_Bus2) {
  bool q_Retval;

  if (orc_Bus1.c_Name.size() == orc_Bus2.c_Name.size()) {
    q_Retval = (orc_Bus1.c_Name < orc_Bus2.c_Name);
  } else {
    q_Retval = orc_Bus1.c_Name.size() < orc_Bus2.c_Name.size();
  }
  return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if bus ID invalid

   \return
   true  Error
   false No error
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscSystemBus::CheckErrorBusId(void) const {
  return (this->u8_BusId > 15);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream (binary format)
   
   \param[in,out]  orc_Stream    Data stream to write to
   
   \return Error code
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBus::ToQDataStream(QDataStream& orc_Stream) const {
   int32_t s32_Retval = stw::errors::C_NO_ERR;
   
   // Serialize enum as uint32_t
   orc_Stream << static_cast<uint32_t>(this->e_Type);
   orc_Stream << this->c_Name;
   orc_Stream << this->c_Comment;
   orc_Stream << this->u64_BitRate;
   orc_Stream << this->q_UseCanFd;
   orc_Stream << this->u64_CanFdBitRate;
   orc_Stream << this->u8_BusId;
   orc_Stream << this->u16_RxTimeoutOffsetMs;
   orc_Stream << this->q_UseableForRouting;
   
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream (binary format)
   
   \param[in,out]  orc_Stream    Data stream to read from
   
   \return Error code
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBus::FromQDataStream(QDataStream& orc_Stream) {
   int32_t s32_Retval = stw::errors::C_NO_ERR;
   uint32_t u32_Type = 0;
   
   orc_Stream >> u32_Type;
   orc_Stream >> this->c_Name;
   orc_Stream >> this->c_Comment;
   orc_Stream >> this->u64_BitRate;
   orc_Stream >> this->q_UseCanFd;
   orc_Stream >> this->u64_CanFdBitRate;
   orc_Stream >> this->u8_BusId;
   orc_Stream >> this->u16_RxTimeoutOffsetMs;
   orc_Stream >> this->q_UseableForRouting;
   
   // Convert enum back from uint32_t
   this->e_Type = static_cast<E_Type>(u32_Type);
   
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QJsonObject (JSON format)
   
   \return JSON object containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscSystemBus::ToJsonObject() const {
   QJsonObject c_Object;
   
   // Serialize enum as human-readable string
   c_Object["type"] = (this->e_Type == eCAN) ? "CAN" : "ETHERNET";
   c_Object["name"] = this->c_Name;
   c_Object["comment"] = this->c_Comment;
   c_Object["bitRate"] = static_cast<qint64>(this->u64_BitRate);
   c_Object["useCanFd"] = this->q_UseCanFd;
   c_Object["canFdBitRate"] = static_cast<qint64>(this->u64_CanFdBitRate);
   c_Object["busId"] = static_cast<qint64>(this->u8_BusId);
   c_Object["rxTimeoutOffsetMs"] = static_cast<qint64>(this->u16_RxTimeoutOffsetMs);
   c_Object["useableForRouting"] = this->q_UseableForRouting;
   
   return c_Object;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QJsonObject (JSON format)
   
   \param[in]  orc_Object    JSON object containing serialized data
   
   \return Error code
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBus::FromJsonObject(const QJsonObject& orc_Object) {
   int32_t s32_Retval = stw::errors::C_NO_ERR;
   
   // Deserialize enum from string
   QString c_Type = orc_Object["type"].toString();
   if (c_Type == "CAN") {
      this->e_Type = eCAN;
   } else if (c_Type == "ETHERNET") {
      this->e_Type = eETHERNET;
   } else {
      s32_Retval = stw::errors::C_CONFIG;
      return s32_Retval;
   }
   
   this->c_Name = orc_Object["name"].toString();
   this->c_Comment = orc_Object["comment"].toString();
   this->u64_BitRate = static_cast<uint64_t>(orc_Object["bitRate"].toVariant().toULongLong());
   this->q_UseCanFd = orc_Object["useCanFd"].toBool();
   this->u64_CanFdBitRate = static_cast<uint64_t>(orc_Object["canFdBitRate"].toVariant().toULongLong());
   this->u8_BusId = static_cast<uint8_t>(orc_Object["busId"].toVariant().toUInt());
   this->u16_RxTimeoutOffsetMs = static_cast<uint16_t>(orc_Object["rxTimeoutOffsetMs"].toVariant().toUInt());
   this->q_UseableForRouting = orc_Object["useableForRouting"].toBool();
   
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDomElement (XML format)
   
   \param[in,out]  orc_Doc              XML document
   \param[in]      orc_RootElementName  Name for the root element
   
   \return XML element containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscSystemBus::ToQDomDocument(QDomDocument& orc_Doc, 
                                           const QString& orc_RootElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
   
   // Serialize enum as string
   c_Element.setAttribute("type", (this->e_Type == eCAN) ? "CAN" : "ETHERNET");
   c_Element.setAttribute("name", this->c_Name);
   c_Element.setAttribute("comment", this->c_Comment);
   c_Element.setAttribute("bitRate", QString::number(this->u64_BitRate));
   c_Element.setAttribute("useCanFd", this->q_UseCanFd);
   c_Element.setAttribute("canFdBitRate", QString::number(this->u64_CanFdBitRate));
   c_Element.setAttribute("busId", QString::number(this->u8_BusId));
   c_Element.setAttribute("rxTimeoutOffsetMs", QString::number(this->u16_RxTimeoutOffsetMs));
   c_Element.setAttribute("useableForRouting", this->q_UseableForRouting);
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDomElement (XML format)
   
   \param[in]  orc_Element    XML element containing serialized data
   
   \return Error code
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscSystemBus::FromQDomElement(const QDomElement& orc_Element) {
   int32_t s32_Retval = stw::errors::C_NO_ERR;
   
   // Deserialize enum from string
   QString c_Type = orc_Element.attribute("type");
   if (c_Type == "CAN") {
      this->e_Type = eCAN;
   } else if (c_Type == "ETHERNET") {
      this->e_Type = eETHERNET;
   } else {
      s32_Retval = stw::errors::C_CONFIG;
      return s32_Retval;
   }
   
   this->c_Name = orc_Element.attribute("name");
   this->c_Comment = orc_Element.attribute("comment");
   this->u64_BitRate = static_cast<uint64_t>(orc_Element.attribute("bitRate").toULongLong());
   this->q_UseCanFd = orc_Element.attribute("useCanFd").toBool();
   this->u64_CanFdBitRate = static_cast<uint64_t>(orc_Element.attribute("canFdBitRate").toULongLong());
   this->u8_BusId = static_cast<uint8_t>(orc_Element.attribute("busId").toUInt());
   this->u16_RxTimeoutOffsetMs = static_cast<uint16_t>(orc_Element.attribute("rxTimeoutOffsetMs").toUInt());
   this->q_UseableForRouting = orc_Element.attribute("useableForRouting").toBool();
   
   return s32_Retval;
}
