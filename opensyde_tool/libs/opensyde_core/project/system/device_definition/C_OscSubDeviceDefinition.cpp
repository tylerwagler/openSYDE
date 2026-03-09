//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Sub device part of device definition

   Sub device part of device definition

   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights
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

#include "C_OscSubDeviceDefinition.hpp"

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
C_OscSubDeviceDefinition::C_OscSubDeviceDefinition() { this->Clear(); }

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Clear
 */
//----------------------------------------------------------------------------------------------------------------------
void C_OscSubDeviceDefinition::Clear() {
  c_SubDeviceName = "";
  c_ConnectedInterfaces.clear();
  c_OtherAcceptedNames.clear();
  q_ProgrammingSupport = false;
  q_DiagnosticProtocolKefex = false;
  q_DiagnosticProtocolOpenSydeCan = false;
  q_DiagnosticProtocolOpenSydeEthernet = false;
  q_FlashloaderStwCan = false;
  q_FlashloaderOpenSydeCan = false;
  q_FlashloaderOpenSydeEthernet = false;
  q_FlashloaderOpenSydeIsFileBased = false;
  u32_UserEepromSizeBytes = 0U;
  // Special default values
  u32_FlashloaderResetWaitTimeNoChangesCan = 1000U;
  u32_FlashloaderResetWaitTimeNoChangesEthernet = 5500U;
  u32_FlashloaderResetWaitTimeNoFundamentalChangesCan = 1000U;
  u32_FlashloaderResetWaitTimeNoFundamentalChangesEthernet = 5500U;
  u32_FlashloaderResetWaitTimeFundamentalChangesCan = 1000U;
  u32_FlashloaderResetWaitTimeFundamentalChangesEthernet = 5500U;
  u32_FlashloaderOpenSydeRequestDownloadTimeout = 20000U;
  u32_FlashloaderOpenSydeTransferDataTimeout = 2000U;
}

//----------------------------------------------------------------------------------------------------------------------

bool C_OscSubDeviceDefinition::IsUpdateAvailable(
    const C_OscSystemBus::E_Type oe_Type) const {
  bool q_Retval = false;

  // flashloader support?
  switch (oe_Type) {
  case C_OscSystemBus::eCAN:
    if (this->q_FlashloaderOpenSydeCan || this->q_FlashloaderStwCan) {
      q_Retval = true;
    } else {
      q_Retval = false;
    }
    break;
  case C_OscSystemBus::eETHERNET:
    if (this->q_FlashloaderOpenSydeEthernet) {
      q_Retval = true;
    } else {
      q_Retval = false;
    }
    break;
  default:
    break;
  }
  return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------

bool C_OscSubDeviceDefinition::IsRoutingAvailable(
    const C_OscSystemBus::E_Type oe_Type) const {
  bool q_Retval = false;

  switch (oe_Type) {
  case C_OscSystemBus::eCAN:
    if (this->q_DiagnosticProtocolOpenSydeCan) {
      q_Retval = true;
    } else {
      q_Retval = false;
    }
    break;
  case C_OscSystemBus::eETHERNET:
    if (this->q_DiagnosticProtocolOpenSydeEthernet) {
      q_Retval = true;
    } else {
      q_Retval = false;
    }
    break;
  default:
    break;
  }
  return q_Retval;
}
//----------------------------------------------------------------------------------------------------------------------

bool C_OscSubDeviceDefinition::IsDiagnosisAvailable(
    const C_OscSystemBus::E_Type oe_Type) const {
  bool q_Retval = false;

  switch (oe_Type) {
  case C_OscSystemBus::eCAN:
    if (this->q_DiagnosticProtocolKefex ||
        this->q_DiagnosticProtocolOpenSydeCan) {
      q_Retval = true;
    } else {
      q_Retval = false;
    }
    break;
  case C_OscSystemBus::eETHERNET:
    if (this->q_DiagnosticProtocolOpenSydeEthernet) {
      q_Retval = true;
    } else {
      q_Retval = false;
    }
    break;
  default:
    break;
  }
  return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Is connected

   \param[in]  oe_Type              Type
   \param[in]  ou8_InterfaceNumber  Interface number

   \return
   State if interface is connected
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscSubDeviceDefinition::IsConnected(
    const C_OscSystemBus::E_Type oe_Type,
    const uint8_t ou8_InterfaceNumber) const {
  bool q_Retval = false;

  const QString c_Interface = C_OscSubDeviceDefinition::h_GetInterfaceNameLower(
      oe_Type, ou8_InterfaceNumber);
  {
    const QHash<QString, bool>::const_iterator c_It =
        this->c_ConnectedInterfaces.find(c_Interface);
    if (c_It != this->c_ConnectedInterfaces.end()) {
      q_Retval = c_It.value();
    }
  }

  return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get interface name (lowercase)

   \param[in]  oe_Type              Type
   \param[in]  ou8_InterfaceNumber  Interface number

   \return
   Interface name (lowercase)
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscSubDeviceDefinition::h_GetInterfaceNameLower(
    const C_OscSystemBus::E_Type oe_Type, const uint8_t ou8_InterfaceNumber) {
  QString c_Interface =
      QString::number(static_cast<uint32_t>(ou8_InterfaceNumber) + 1U);

  if (oe_Type == C_OscSystemBus::eCAN) {
    c_Interface = "can" + c_Interface;
  } else {
    c_Interface = "eth" + c_Interface;
  }
  return c_Interface;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSubDeviceDefinition::ToQDataStream(QDataStream &ro_DataStream) const {
   ro_DataStream << this->c_SubDeviceName;
   
   // Serialize QHash<QString, bool>
   ro_DataStream << static_cast<int32_t>(this->c_ConnectedInterfaces.size());
   for (auto it = this->c_ConnectedInterfaces.constBegin(); it != this->c_ConnectedInterfaces.constEnd(); ++it) {
      ro_DataStream << it.key();
      ro_DataStream << static_cast<int32_t>(it.value());
   }
   
   ro_DataStream << this->c_OtherAcceptedNames;
   ro_DataStream << static_cast<int32_t>(this->q_ProgrammingSupport);
   ro_DataStream << static_cast<int32_t>(this->q_DiagnosticProtocolKefex);
   ro_DataStream << static_cast<int32_t>(this->q_DiagnosticProtocolOpenSydeCan);
   ro_DataStream << static_cast<int32_t>(this->q_DiagnosticProtocolOpenSydeEthernet);
   ro_DataStream << static_cast<int32_t>(this->q_FlashloaderStwCan);
   ro_DataStream << static_cast<int32_t>(this->q_FlashloaderOpenSydeCan);
   ro_DataStream << static_cast<int32_t>(this->q_FlashloaderOpenSydeEthernet);
   ro_DataStream << static_cast<int32_t>(this->q_FlashloaderOpenSydeIsFileBased);
   ro_DataStream << this->u32_FlashloaderResetWaitTimeNoChangesCan;
   ro_DataStream << this->u32_FlashloaderResetWaitTimeNoChangesEthernet;
   ro_DataStream << this->u32_FlashloaderResetWaitTimeNoFundamentalChangesCan;
   ro_DataStream << this->u32_FlashloaderResetWaitTimeNoFundamentalChangesEthernet;
   ro_DataStream << this->u32_FlashloaderResetWaitTimeFundamentalChangesCan;
   ro_DataStream << this->u32_FlashloaderResetWaitTimeFundamentalChangesEthernet;
   ro_DataStream << this->u32_FlashloaderOpenSydeRequestDownloadTimeout;
   ro_DataStream << this->u32_FlashloaderOpenSydeTransferDataTimeout;
   ro_DataStream << this->u32_UserEepromSizeBytes;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSubDeviceDefinition::FromQDataStream(QDataStream &ro_DataStream) {
   int32_t s32_Supports;
   int32_t s32_HashSize;
   
   ro_DataStream >> this->c_SubDeviceName;
   
   // Deserialize QHash
   ro_DataStream >> s32_HashSize;
   this->c_ConnectedInterfaces.clear();
   for (int32_t i = 0; i < s32_HashSize; ++i) {
      QString c_Key;
      int32_t s32_Value;
      ro_DataStream >> c_Key;
      ro_DataStream >> s32_Value;
      this->c_ConnectedInterfaces.insert(c_Key, (s32_Value != 0));
   }
   
   ro_DataStream >> this->c_OtherAcceptedNames;
   
   ro_DataStream >> s32_Supports; this->q_ProgrammingSupport = (s32_Supports != 0);
   ro_DataStream >> s32_Supports; this->q_DiagnosticProtocolKefex = (s32_Supports != 0);
   ro_DataStream >> s32_Supports; this->q_DiagnosticProtocolOpenSydeCan = (s32_Supports != 0);
   ro_DataStream >> s32_Supports; this->q_DiagnosticProtocolOpenSydeEthernet = (s32_Supports != 0);
   ro_DataStream >> s32_Supports; this->q_FlashloaderStwCan = (s32_Supports != 0);
   ro_DataStream >> s32_Supports; this->q_FlashloaderOpenSydeCan = (s32_Supports != 0);
   ro_DataStream >> s32_Supports; this->q_FlashloaderOpenSydeEthernet = (s32_Supports != 0);
   ro_DataStream >> s32_Supports; this->q_FlashloaderOpenSydeIsFileBased = (s32_Supports != 0);
   
   ro_DataStream >> this->u32_FlashloaderResetWaitTimeNoChangesCan;
   ro_DataStream >> this->u32_FlashloaderResetWaitTimeNoChangesEthernet;
   ro_DataStream >> this->u32_FlashloaderResetWaitTimeNoFundamentalChangesCan;
   ro_DataStream >> this->u32_FlashloaderResetWaitTimeNoFundamentalChangesEthernet;
   ro_DataStream >> this->u32_FlashloaderResetWaitTimeFundamentalChangesCan;
   ro_DataStream >> this->u32_FlashloaderResetWaitTimeFundamentalChangesEthernet;
   ro_DataStream >> this->u32_FlashloaderOpenSydeRequestDownloadTimeout;
   ro_DataStream >> this->u32_FlashloaderOpenSydeTransferDataTimeout;
   ro_DataStream >> this->u32_UserEepromSizeBytes;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to JSON
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscSubDeviceDefinition::ToJsonObject() const {
   QJsonObject obj;
   QJsonArray connectedArray;
   
   obj["sub-device-name"] = this->c_SubDeviceName;
   
   for (auto it = this->c_ConnectedInterfaces.constBegin(); it != this->c_ConnectedInterfaces.constEnd(); ++it) {
      QJsonObject item;
      item["interface"] = it.key();
      item["connected"] = it.value();
      connectedArray.append(item);
   }
   obj["connected-interfaces"] = connectedArray;
   
   QJsonArray namesArr; for (const auto &n : this->c_OtherAcceptedNames) { namesArr.append(n); } obj["other-accepted-names"] = namesArr;
   obj["programming-support"] = this->q_ProgrammingSupport;
   obj["diagnostic-protocol-kefex"] = this->q_DiagnosticProtocolKefex;
   obj["diagnostic-protocol-opensyde-can"] = this->q_DiagnosticProtocolOpenSydeCan;
   obj["diagnostic-protocol-opensyde-ethernet"] = this->q_DiagnosticProtocolOpenSydeEthernet;
   obj["flashloader-stw-can"] = this->q_FlashloaderStwCan;
   obj["flashloader-opensyde-can"] = this->q_FlashloaderOpenSydeCan;
   obj["flashloader-opensyde-ethernet"] = this->q_FlashloaderOpenSydeEthernet;
   obj["flashloader-opensyde-file-based"] = this->q_FlashloaderOpenSydeIsFileBased;
   obj["flashloader-reset-wait-time-no-changes-can"] = static_cast<qint64>(this->u32_FlashloaderResetWaitTimeNoChangesCan);
   obj["flashloader-reset-wait-time-no-changes-ethernet"] = static_cast<qint64>(this->u32_FlashloaderResetWaitTimeNoChangesEthernet);
   obj["flashloader-reset-wait-time-no-fundamental-changes-can"] = static_cast<qint64>(this->u32_FlashloaderResetWaitTimeNoFundamentalChangesCan);
   obj["flashloader-reset-wait-time-no-fundamental-changes-ethernet"] = static_cast<qint64>(this->u32_FlashloaderResetWaitTimeNoFundamentalChangesEthernet);
   obj["flashloader-reset-wait-time-fundamental-changes-can"] = static_cast<qint64>(this->u32_FlashloaderResetWaitTimeFundamentalChangesCan);
   obj["flashloader-reset-wait-time-fundamental-changes-ethernet"] = static_cast<qint64>(this->u32_FlashloaderResetWaitTimeFundamentalChangesEthernet);
   obj["flashloader-request-download-timeout"] = static_cast<qint64>(this->u32_FlashloaderOpenSydeRequestDownloadTimeout);
   obj["flashloader-transfer-data-timeout"] = static_cast<qint64>(this->u32_FlashloaderOpenSydeTransferDataTimeout);
   obj["user-eeprom-size-bytes"] = static_cast<qint64>(this->u32_UserEepromSizeBytes);
   
   return obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from JSON
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSubDeviceDefinition::FromJsonObject(const QJsonObject &ro_Json) {
   this->c_SubDeviceName = ro_Json["sub-device-name"].toString();
   
   this->c_ConnectedInterfaces.clear();
   QJsonArray connectedArray = ro_Json["connected-interfaces"].toArray();
   for (const auto &item : connectedArray) {
      QJsonObject obj = item.toObject();
      this->c_ConnectedInterfaces.insert(obj["interface"].toString(), obj["connected"].toBool());
   }
   
   QJsonArray arr = ro_Json["other-accepted-names"].toArray();
   this->c_OtherAcceptedNames.clear();
   for (const auto &v : arr) {
      this->c_OtherAcceptedNames.append(v.toString());
   }
   this->q_ProgrammingSupport = ro_Json["programming-support"].toBool();
   this->q_DiagnosticProtocolKefex = ro_Json["diagnostic-protocol-kefex"].toBool();
   this->q_DiagnosticProtocolOpenSydeCan = ro_Json["diagnostic-protocol-opensyde-can"].toBool();
   this->q_DiagnosticProtocolOpenSydeEthernet = ro_Json["diagnostic-protocol-opensyde-ethernet"].toBool();
   this->q_FlashloaderStwCan = ro_Json["flashloader-stw-can"].toBool();
   this->q_FlashloaderOpenSydeCan = ro_Json["flashloader-opensyde-can"].toBool();
   this->q_FlashloaderOpenSydeEthernet = ro_Json["flashloader-opensyde-ethernet"].toBool();
   this->q_FlashloaderOpenSydeIsFileBased = ro_Json["flashloader-opensyde-file-based"].toBool();
   this->u32_FlashloaderResetWaitTimeNoChangesCan = static_cast<uint32_t>(ro_Json["flashloader-reset-wait-time-no-changes-can"].toInt());
   this->u32_FlashloaderResetWaitTimeNoChangesEthernet = static_cast<uint32_t>(ro_Json["flashloader-reset-wait-time-no-changes-ethernet"].toInt());
   this->u32_FlashloaderResetWaitTimeNoFundamentalChangesCan = static_cast<uint32_t>(ro_Json["flashloader-reset-wait-time-no-fundamental-changes-can"].toInt());
   this->u32_FlashloaderResetWaitTimeNoFundamentalChangesEthernet = static_cast<uint32_t>(ro_Json["flashloader-reset-wait-time-no-fundamental-changes-ethernet"].toInt());
   this->u32_FlashloaderResetWaitTimeFundamentalChangesCan = static_cast<uint32_t>(ro_Json["flashloader-reset-wait-time-fundamental-changes-can"].toInt());
   this->u32_FlashloaderResetWaitTimeFundamentalChangesEthernet = static_cast<uint32_t>(ro_Json["flashloader-reset-wait-time-fundamental-changes-ethernet"].toInt());
   this->u32_FlashloaderOpenSydeRequestDownloadTimeout = static_cast<uint32_t>(ro_Json["flashloader-request-download-timeout"].toInt());
   this->u32_FlashloaderOpenSydeTransferDataTimeout = static_cast<uint32_t>(ro_Json["flashloader-transfer-data-timeout"].toInt());
   this->u32_UserEepromSizeBytes = static_cast<uint32_t>(ro_Json["user-eeprom-size-bytes"].toInt());
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to XML
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscSubDeviceDefinition::ToQDomDocument(QDomDocument &ro_Doc,
                                                      const QString &orc_ElementName) const {
   QDomElement element = ro_Doc.createElement(orc_ElementName);
   
   element.setAttribute("sub-device-name", this->c_SubDeviceName);
   
   QDomElement connectedElement = ro_Doc.createElement("connected-interfaces");
   for (auto it = this->c_ConnectedInterfaces.constBegin(); it != this->c_ConnectedInterfaces.constEnd(); ++it) {
      QDomElement itemElement = ro_Doc.createElement("interface");
      itemElement.setAttribute("name", it.key());
      itemElement.setAttribute("connected", it.value());
      connectedElement.appendChild(itemElement);
   }
   element.appendChild(connectedElement);
   
   QDomElement namesElement = ro_Doc.createElement("other-accepted-names");
   for (const QString &name : this->c_OtherAcceptedNames) {
      QDomElement nameElement = ro_Doc.createElement("name");
      nameElement.setAttribute("value", name);
      namesElement.appendChild(nameElement);
   }
   element.appendChild(namesElement);
   
   element.setAttribute("programming-support", this->q_ProgrammingSupport);
   element.setAttribute("diagnostic-protocol-kefex", this->q_DiagnosticProtocolKefex);
   element.setAttribute("diagnostic-protocol-opensyde-can", this->q_DiagnosticProtocolOpenSydeCan);
   element.setAttribute("diagnostic-protocol-opensyde-ethernet", this->q_DiagnosticProtocolOpenSydeEthernet);
   element.setAttribute("flashloader-stw-can", this->q_FlashloaderStwCan);
   element.setAttribute("flashloader-opensyde-can", this->q_FlashloaderOpenSydeCan);
   element.setAttribute("flashloader-opensyde-ethernet", this->q_FlashloaderOpenSydeEthernet);
   element.setAttribute("flashloader-opensyde-file-based", this->q_FlashloaderOpenSydeIsFileBased);
   element.setAttribute("flashloader-reset-wait-time-no-changes-can", this->u32_FlashloaderResetWaitTimeNoChangesCan);
   element.setAttribute("flashloader-reset-wait-time-no-changes-ethernet", this->u32_FlashloaderResetWaitTimeNoChangesEthernet);
   element.setAttribute("flashloader-reset-wait-time-no-fundamental-changes-can", this->u32_FlashloaderResetWaitTimeNoFundamentalChangesCan);
   element.setAttribute("flashloader-reset-wait-time-no-fundamental-changes-ethernet", this->u32_FlashloaderResetWaitTimeNoFundamentalChangesEthernet);
   element.setAttribute("flashloader-reset-wait-time-fundamental-changes-can", this->u32_FlashloaderResetWaitTimeFundamentalChangesCan);
   element.setAttribute("flashloader-reset-wait-time-fundamental-changes-ethernet", this->u32_FlashloaderResetWaitTimeFundamentalChangesEthernet);
   element.setAttribute("flashloader-request-download-timeout", this->u32_FlashloaderOpenSydeRequestDownloadTimeout);
   element.setAttribute("flashloader-transfer-data-timeout", this->u32_FlashloaderOpenSydeTransferDataTimeout);
   element.setAttribute("user-eeprom-size-bytes", this->u32_UserEepromSizeBytes);
   
   return element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from XML
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSubDeviceDefinition::FromQDomDocument(const QDomElement &ro_Element) {
   this->c_SubDeviceName = ro_Element.attribute("sub-device-name");
   
   this->c_ConnectedInterfaces.clear();
   QDomElement connectedElement = ro_Element.firstChildElement("connected-interfaces");
   QDomNode node = connectedElement.firstChild();
   while (!node.isNull()) {
      QDomElement itemElement = node.toElement();
      if (!itemElement.isNull()) {
         this->c_ConnectedInterfaces.insert(itemElement.attribute("name"), itemElement.attribute("connected").toInt() != 0);
      }
      node = node.nextSibling();
   }
   
   this->c_OtherAcceptedNames.clear();
   QDomElement namesElement = ro_Element.firstChildElement("other-accepted-names");
   node = namesElement.firstChild();
   while (!node.isNull()) {
      QDomElement nameElement = node.toElement();
      if (!nameElement.isNull()) {
         this->c_OtherAcceptedNames.append(nameElement.attribute("value"));
      }
      node = node.nextSibling();
   }
   
   this->q_ProgrammingSupport = ro_Element.attribute("programming-support").toInt() != 0;
   this->q_DiagnosticProtocolKefex = ro_Element.attribute("diagnostic-protocol-kefex").toInt() != 0;
   this->q_DiagnosticProtocolOpenSydeCan = ro_Element.attribute("diagnostic-protocol-opensyde-can").toInt() != 0;
   this->q_DiagnosticProtocolOpenSydeEthernet = ro_Element.attribute("diagnostic-protocol-opensyde-ethernet").toInt() != 0;
   this->q_FlashloaderStwCan = ro_Element.attribute("flashloader-stw-can").toInt() != 0;
   this->q_FlashloaderOpenSydeCan = ro_Element.attribute("flashloader-opensyde-can").toInt() != 0;
   this->q_FlashloaderOpenSydeEthernet = ro_Element.attribute("flashloader-opensyde-ethernet").toInt() != 0;
   this->q_FlashloaderOpenSydeIsFileBased = ro_Element.attribute("flashloader-opensyde-file-based").toInt() != 0;
   this->u32_FlashloaderResetWaitTimeNoChangesCan = static_cast<uint32_t>(ro_Element.attribute("flashloader-reset-wait-time-no-changes-can").toUInt());
   this->u32_FlashloaderResetWaitTimeNoChangesEthernet = static_cast<uint32_t>(ro_Element.attribute("flashloader-reset-wait-time-no-changes-ethernet").toUInt());
   this->u32_FlashloaderResetWaitTimeNoFundamentalChangesCan = static_cast<uint32_t>(ro_Element.attribute("flashloader-reset-wait-time-no-fundamental-changes-can").toUInt());
   this->u32_FlashloaderResetWaitTimeNoFundamentalChangesEthernet = static_cast<uint32_t>(ro_Element.attribute("flashloader-reset-wait-time-no-fundamental-changes-ethernet").toUInt());
   this->u32_FlashloaderResetWaitTimeFundamentalChangesCan = static_cast<uint32_t>(ro_Element.attribute("flashloader-reset-wait-time-fundamental-changes-can").toUInt());
   this->u32_FlashloaderResetWaitTimeFundamentalChangesEthernet = static_cast<uint32_t>(ro_Element.attribute("flashloader-reset-wait-time-fundamental-changes-ethernet").toUInt());
   this->u32_FlashloaderOpenSydeRequestDownloadTimeout = static_cast<uint32_t>(ro_Element.attribute("flashloader-request-download-timeout").toUInt());
   this->u32_FlashloaderOpenSydeTransferDataTimeout = static_cast<uint32_t>(ro_Element.attribute("flashloader-transfer-data-timeout").toUInt());
   this->u32_UserEepromSizeBytes = static_cast<uint32_t>(ro_Element.attribute("user-eeprom-size-bytes").toUInt());
}
