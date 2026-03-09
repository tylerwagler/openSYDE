//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for a signal positioning in a CAN message
   (implementation)

   Data class for a signal positioning in a CAN message

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"
#include <QJsonArray>
#include <QJsonDocument>
#include <QDomDocument>

#include "C_OscCanSignal.hpp"
#include "C_SclChecksums.hpp"

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
/*! \brief   Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscCanSignal::C_OscCanSignal(void)
    : e_ComByteOrder(eBYTE_ORDER_INTEL), u16_ComBitLength(8),
      u16_ComBitStart(0), u32_ComDataElementIndex(0),
      e_MultiplexerType(eMUX_DEFAULT), u16_MultiplexValue(0),
      u16_CanOpenManagerObjectDictionaryIndex(0),
      u8_CanOpenManagerObjectDictionarySubIndex(0),
      u32_J1939SuspectParameterNumber(0U) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if current not equal to orc_Cmp

   \param[in]  orc_Cmp  Compared instance

   \return
   Current not equal to orc_Cmp
   Else false
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanSignal::operator!=(const C_OscCanSignal &orc_Cmp) const {
  bool q_Return = false;

  if ((this->u32_ComDataElementIndex != orc_Cmp.u32_ComDataElementIndex) ||
      (this->e_ComByteOrder != orc_Cmp.e_ComByteOrder) ||
      (this->u16_ComBitStart != orc_Cmp.u16_ComBitStart) ||
      (this->u16_ComBitLength != orc_Cmp.u16_ComBitLength) ||
      (this->e_MultiplexerType != orc_Cmp.e_MultiplexerType) ||
      (this->u16_MultiplexValue != orc_Cmp.u16_MultiplexValue)) {
    q_Return = true;
  }

  return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if current is equal to orc_Cmp

   Returns negotiation of != operator.

   \param[in]  orc_Cmp  Compared instance

   \return
   Current is equal to orc_Cmp
   Else false
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanSignal::operator==(const C_OscCanSignal &orc_Cmp) const {
  return !(*this != orc_Cmp);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check if current is less than orc_Cmp. Comparing by start bit

   \param[in]  orc_Cmp  Compared instance

   \return
   Current less than orc_Cmp
   Else false
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanSignal::operator<(const C_OscCanSignal &orc_Cmp) const {
  bool q_Return = false;

  if (this->u16_ComBitStart < orc_Cmp.u16_ComBitStart) {
    q_Return = true;
  }

  return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.
   It is not endian-safe, so it should only be used on the same system it is
   created on.

   \param[in,out]  oru32_HashValue     Hash value with init [in] value and
   result [out] value \param[in]      oq_R20Compatible    Flag to calculate the
   hash of only elements present in R20 to allow compatibility with existing
   hash values from R20 release
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanSignal::CalcHash(uint32_t &oru32_HashValue,
                              const bool oq_R20Compatible) const {
  stw::scl::C_SclChecksums::CalcCRC32(
      &this->e_ComByteOrder, sizeof(this->e_ComByteOrder), oru32_HashValue);
  stw::scl::C_SclChecksums::CalcCRC32(&this->e_MultiplexerType,
                                      sizeof(this->e_MultiplexerType),
                                      oru32_HashValue);
  stw::scl::C_SclChecksums::CalcCRC32(
      &this->u16_ComBitLength, sizeof(this->u16_ComBitLength), oru32_HashValue);
  stw::scl::C_SclChecksums::CalcCRC32(
      &this->u16_ComBitStart, sizeof(this->u16_ComBitStart), oru32_HashValue);
  stw::scl::C_SclChecksums::CalcCRC32(&this->u32_ComDataElementIndex,
                                      sizeof(this->u32_ComDataElementIndex),
                                      oru32_HashValue);
  stw::scl::C_SclChecksums::CalcCRC32(&this->u16_MultiplexValue,
                                      sizeof(this->u16_MultiplexValue),
                                      oru32_HashValue);
  if (oq_R20Compatible == false) {
    stw::scl::C_SclChecksums::CalcCRC32(
        &this->u16_CanOpenManagerObjectDictionaryIndex,
        sizeof(this->u16_CanOpenManagerObjectDictionaryIndex), oru32_HashValue);
    stw::scl::C_SclChecksums::CalcCRC32(
        &this->u8_CanOpenManagerObjectDictionarySubIndex,
        sizeof(this->u8_CanOpenManagerObjectDictionarySubIndex),
        oru32_HashValue);
    stw::scl::C_SclChecksums::CalcCRC32(
        &this->u32_J1939SuspectParameterNumber,
        sizeof(this->u32_J1939SuspectParameterNumber), oru32_HashValue);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the position of a signal bit in the data bytes

   \param[in]  ou16_SignalBitPosition  Signal bit position starting at 0

   \return
   Data bytes bit position
*/
//----------------------------------------------------------------------------------------------------------------------
uint16_t C_OscCanSignal::GetDataBytesBitPosOfSignalBit(
    const uint16_t ou16_SignalBitPosition) const {
  return this->GetDataBytesBitPosOfSignalBit(this->u16_ComBitStart,
                                             ou16_SignalBitPosition);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the position of a signal bit in the data bytes

   \param[in]  ou16_StartBit           New start bit for the signal
   \param[in]  ou16_SignalBitPosition  Signal bit position starting at 0

   \return
   Data bytes bit position
*/
//----------------------------------------------------------------------------------------------------------------------
uint16_t C_OscCanSignal::GetDataBytesBitPosOfSignalBit(
    const uint16_t ou16_StartBit, const uint16_t ou16_SignalBitPosition) const {
  uint16_t u16_DataBytesBitPos;

  if (this->e_ComByteOrder == C_OscCanSignal::eBYTE_ORDER_INTEL) {
    // Intel byte order has an 1:1 mapping
    u16_DataBytesBitPos = (ou16_StartBit + ou16_SignalBitPosition);
  } else {
    // start bit is the only one bit with the "correct" index and the start
    // value for calculation at the same time
    u16_DataBytesBitPos = ou16_StartBit;

    if (ou16_SignalBitPosition > 0U) {
      uint16_t u16_Counter;

      // calculate the position
      for (u16_Counter = 1U; u16_Counter <= ou16_SignalBitPosition;
           ++u16_Counter) {
        if ((u16_DataBytesBitPos % 8U) == 0U) {
          // the end of the "upper" byte reached. jump to start of the next
          // byte.
          u16_DataBytesBitPos += 15U;
        } else {
          // byte order is backwards
          --u16_DataBytesBitPos;
        }
      }
    }
  }

  return u16_DataBytesBitPos;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the positions of all signal bits in the data bytes

   \param[out]  orc_SetPositions    Signal bit positions
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanSignal::GetDataBytesBitPositionsOfSignal(
    QSet<uint16_t> &orc_SetPositions) const {
  uint16_t u16_Counter;
  uint16_t u16_DataBytesBitPos = this->u16_ComBitStart;

  // start bit is the only one bit with the "correct" index and the start value
  // for calculation at the same time
  orc_SetPositions.insert(u16_DataBytesBitPos);

  for (u16_Counter = 1U; u16_Counter < this->u16_ComBitLength; ++u16_Counter) {
    if (this->e_ComByteOrder == C_OscCanSignal::eBYTE_ORDER_INTEL) {
      // Intel byte order has an 1:1 mapping
      ++u16_DataBytesBitPos;
    } else {
      if ((u16_DataBytesBitPos % 8U) == 0U) {
        // the end of the "upper" byte reached. jump to start of the next byte.
        u16_DataBytesBitPos += 15U;
      } else {
        // byte order is backwards
        --u16_DataBytesBitPos;
      }
    }

    orc_SetPositions.insert(u16_DataBytesBitPos);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns the flag if the bit position in the message is part of this
   signal

   \param[in]  ou16_MessageBitPosition           Bin position in message for
   checking

   \retval   True       Bit in message is part of signal
   \retval   False      Bit in message is not part of signal
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscCanSignal::IsBitPosPartOfSignal(
    const uint16_t ou16_MessageBitPosition) const {
  bool q_Return = false;

  if (this->e_ComByteOrder == C_OscCanSignal::eBYTE_ORDER_INTEL) {
    // Intel byte order has an 1:1 mapping
    // Check if the bit is in range
    if ((ou16_MessageBitPosition >= this->u16_ComBitStart) &&
        (ou16_MessageBitPosition <
         (this->u16_ComBitStart + this->u16_ComBitLength))) {
      q_Return = true;
    }
  } else {
    QSet<uint16_t> c_SetPositions;

    this->GetDataBytesBitPositionsOfSignal(c_SetPositions);
    if (c_SetPositions.contains(ou16_MessageBitPosition)) {
      q_Return = true;
    }
  }

  return q_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize signal to QDataStream (binary format)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanSignal::ToQDataStream(QDataStream &ro_DataStream) const {
   ro_DataStream << static_cast<int32_t>(this->e_ComByteOrder);
   ro_DataStream << this->u16_ComBitLength;
   ro_DataStream << this->u16_ComBitStart;
   ro_DataStream << this->u32_ComDataElementIndex;
   ro_DataStream << static_cast<int32_t>(this->e_MultiplexerType);
   ro_DataStream << this->u16_MultiplexValue;
   ro_DataStream << this->u16_CanOpenManagerObjectDictionaryIndex;
   ro_DataStream << this->u8_CanOpenManagerObjectDictionarySubIndex;
   ro_DataStream << this->u32_J1939SuspectParameterNumber;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize signal from QDataStream (binary format)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanSignal::FromQDataStream(QDataStream &ro_DataStream) {
   int32_t s32_ByteOrder;
   int32_t s32_MultiplexerType;

   ro_DataStream >> s32_ByteOrder;
   ro_DataStream >> this->u16_ComBitLength;
   ro_DataStream >> this->u16_ComBitStart;
   ro_DataStream >> this->u32_ComDataElementIndex;
   ro_DataStream >> s32_MultiplexerType;
   ro_DataStream >> this->u16_MultiplexValue;
   ro_DataStream >> this->u16_CanOpenManagerObjectDictionaryIndex;
   ro_DataStream >> this->u8_CanOpenManagerObjectDictionarySubIndex;
   ro_DataStream >> this->u32_J1939SuspectParameterNumber;

   this->e_ComByteOrder = static_cast<E_ByteOrderType>(s32_ByteOrder);
   this->e_MultiplexerType = static_cast<E_MultiplexerType>(s32_MultiplexerType);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize signal to JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscCanSignal::ToJsonObject() const {
   QJsonObject obj;

   obj["byte-order"] = static_cast<qint64>(this->e_ComByteOrder);
   obj["bit-length"] = static_cast<qint64>(this->u16_ComBitLength);
   obj["bit-start"] = static_cast<qint64>(this->u16_ComBitStart);
   obj["data-element-index"] = static_cast<qint64>(this->u32_ComDataElementIndex);
   obj["multiplexer-type"] = static_cast<qint64>(this->e_MultiplexerType);
   obj["multiplex-value"] = static_cast<qint64>(this->u16_MultiplexValue);
   obj["canopen-object-index"] = static_cast<qint64>(this->u16_CanOpenManagerObjectDictionaryIndex);
   obj["canopen-sub-index"] = static_cast<qint64>(this->u8_CanOpenManagerObjectDictionarySubIndex);
   obj["j1939-spn"] = static_cast<qint64>(this->u32_J1939SuspectParameterNumber);

   return obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize signal from JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanSignal::FromJsonObject(const QJsonObject &ro_Json) {
   this->e_ComByteOrder = static_cast<E_ByteOrderType>(ro_Json["byte-order"].toInt());
   this->u16_ComBitLength = static_cast<uint16_t>(ro_Json["bit-length"].toInt());
   this->u16_ComBitStart = static_cast<uint16_t>(ro_Json["bit-start"].toInt());
   this->u32_ComDataElementIndex = static_cast<uint32_t>(ro_Json["data-element-index"].toInt());
   this->e_MultiplexerType = static_cast<E_MultiplexerType>(ro_Json["multiplexer-type"].toInt());
   this->u16_MultiplexValue = static_cast<uint16_t>(ro_Json["multiplex-value"].toInt());
   this->u16_CanOpenManagerObjectDictionaryIndex = static_cast<uint16_t>(ro_Json["canopen-object-index"].toInt());
   this->u8_CanOpenManagerObjectDictionarySubIndex = static_cast<uint8_t>(ro_Json["canopen-sub-index"].toInt());
   this->u32_J1939SuspectParameterNumber = static_cast<uint32_t>(ro_Json["j1939-spn"].toInt());
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize signal to XML DOM element
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscCanSignal::ToQDomDocument(QDomDocument &ro_Doc,
                                           const QString &orc_ElementName) const {
   QDomElement element = ro_Doc.createElement(orc_ElementName);

   element.setAttribute("byte-order", static_cast<int32_t>(this->e_ComByteOrder));
   element.setAttribute("bit-length", this->u16_ComBitLength);
   element.setAttribute("bit-start", this->u16_ComBitStart);
   element.setAttribute("data-element-index", this->u32_ComDataElementIndex);
   element.setAttribute("multiplexer-type", static_cast<int32_t>(this->e_MultiplexerType));
   element.setAttribute("multiplex-value", this->u16_MultiplexValue);
   element.setAttribute("canopen-object-index", this->u16_CanOpenManagerObjectDictionaryIndex);
   element.setAttribute("canopen-sub-index", this->u8_CanOpenManagerObjectDictionarySubIndex);
   element.setAttribute("j1939-spn", this->u32_J1939SuspectParameterNumber);

   return element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize signal from XML DOM element
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanSignal::FromQDomDocument(const QDomElement &ro_Element) {
   this->e_ComByteOrder = static_cast<E_ByteOrderType>(ro_Element.attribute("byte-order").toInt());
   this->u16_ComBitLength = ro_Element.attribute("bit-length").toInt();
   this->u16_ComBitStart = ro_Element.attribute("bit-start").toInt();
   this->u32_ComDataElementIndex = ro_Element.attribute("data-element-index").toInt();
   this->e_MultiplexerType = static_cast<E_MultiplexerType>(ro_Element.attribute("multiplexer-type").toInt());
   this->u16_MultiplexValue = ro_Element.attribute("multiplex-value").toInt();
   this->u16_CanOpenManagerObjectDictionaryIndex = ro_Element.attribute("canopen-object-index").toInt();
   this->u8_CanOpenManagerObjectDictionarySubIndex = ro_Element.attribute("canopen-sub-index").toInt();
   this->u32_J1939SuspectParameterNumber = ro_Element.attribute("j1939-spn").toInt();
}
