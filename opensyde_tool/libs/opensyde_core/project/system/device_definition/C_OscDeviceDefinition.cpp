//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE device definition container class

   Serves as data class to hold device definition data.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
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

#include "C_OscDeviceDefinition.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
using namespace stw::opensyde_core;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */
const QString C_OscDeviceDefinition::hc_DEFAULT_COMPANY_NAME =
    "Sensor-Technik Wiedemann GmbH";

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
/*! \brief   Constructor

   Initialize all class elements with default values
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscDeviceDefinition::C_OscDeviceDefinition(void) { this->Clear(); }

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Clear elements

   Sets all numeric elements to zero and all strings to ""
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDeviceDefinition::Clear(void) {
  c_DeviceName = "";
  c_DeviceNameAlias = "";
  c_DeviceDescription = "";
  c_ImagePath = "";
  c_FilePath = "";

  u8_NumCanBusses = 0U;
  u8_NumEthernetBusses = 0U;

  c_SupportedBitrates.clear();
  c_SupportedCanFdDataBitrates.clear();
  c_SupportedCanFeatures.clear();

  u8_ManufacturerId = 0;
  c_ManufacturerDisplayValue = C_OscDeviceDefinition::hc_DEFAULT_COMPANY_NAME;
  c_CompanyLogoLink = "";
  c_ProductPageLink = "";
  c_ToolboxIcon = "";

  c_SubDevices.clear();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get string to display for this device

   \return
   String to display
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscDeviceDefinition::GetDisplayName(void) const {
  QString c_Retval;
  if (this->c_DeviceNameAlias == "") {
    c_Retval = this->c_DeviceName;
  } else {
    c_Retval = this->c_DeviceNameAlias;
  }
  return c_Retval;
}
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDeviceDefinition::ToQDataStream(QDataStream &ro_DataStream) const {
   ro_DataStream << this->c_DeviceName;
   ro_DataStream << this->c_DeviceNameAlias;
   ro_DataStream << this->c_DeviceDescription;
   ro_DataStream << this->c_ImagePath;
   ro_DataStream << this->c_FilePath;
   ro_DataStream << this->u8_NumCanBusses;
   ro_DataStream << this->u8_NumEthernetBusses;
   
   ro_DataStream << static_cast<int32_t>(this->c_SupportedBitrates.size());
   for (uint16_t bitrate : this->c_SupportedBitrates) {
      ro_DataStream << bitrate;
   }
   
   ro_DataStream << static_cast<int32_t>(this->c_SupportedCanFdDataBitrates.size());
   for (uint16_t bitrate : this->c_SupportedCanFdDataBitrates) {
      ro_DataStream << bitrate;
   }
   
   ro_DataStream << static_cast<int32_t>(this->c_SupportedCanFeatures.size());
   for (const auto &rc_Feature : this->c_SupportedCanFeatures) {
      rc_Feature.ToQDataStream(ro_DataStream);
   }
   
   ro_DataStream << this->u8_ManufacturerId;
   ro_DataStream << this->c_ManufacturerDisplayValue;
   ro_DataStream << this->c_CompanyLogoLink;
   ro_DataStream << this->c_ProductPageLink;
   ro_DataStream << this->c_ToolboxIcon;
   
   ro_DataStream << static_cast<int32_t>(this->c_SubDevices.size());
   for (const auto &rc_SubDevice : this->c_SubDevices) {
      rc_SubDevice.ToQDataStream(ro_DataStream);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDeviceDefinition::FromQDataStream(QDataStream &ro_DataStream) {
   int32_t s32_Count;
   
   ro_DataStream >> this->c_DeviceName;
   ro_DataStream >> this->c_DeviceNameAlias;
   ro_DataStream >> this->c_DeviceDescription;
   ro_DataStream >> this->c_ImagePath;
   ro_DataStream >> this->c_FilePath;
   ro_DataStream >> this->u8_NumCanBusses;
   ro_DataStream >> this->u8_NumEthernetBusses;
   
   ro_DataStream >> s32_Count;
   this->c_SupportedBitrates.clear();
   for (int32_t i = 0; i < s32_Count; ++i) {
      uint16_t u16_Bitrate;
      ro_DataStream >> u16_Bitrate;
      this->c_SupportedBitrates.append(u16_Bitrate);
   }
   
   ro_DataStream >> s32_Count;
   this->c_SupportedCanFdDataBitrates.clear();
   for (int32_t i = 0; i < s32_Count; ++i) {
      uint16_t u16_Bitrate;
      ro_DataStream >> u16_Bitrate;
      this->c_SupportedCanFdDataBitrates.append(u16_Bitrate);
   }
   
   ro_DataStream >> s32_Count;
   this->c_SupportedCanFeatures.clear();
   for (int32_t i = 0; i < s32_Count; ++i) {
      C_OscSupportedCanInterfaceFeatures c_Feature;
      c_Feature.FromQDataStream(ro_DataStream);
      this->c_SupportedCanFeatures.append(c_Feature);
   }
   
   ro_DataStream >> this->u8_ManufacturerId;
   ro_DataStream >> this->c_ManufacturerDisplayValue;
   ro_DataStream >> this->c_CompanyLogoLink;
   ro_DataStream >> this->c_ProductPageLink;
   ro_DataStream >> this->c_ToolboxIcon;
   
   ro_DataStream >> s32_Count;
   this->c_SubDevices.clear();
   for (int32_t i = 0; i < s32_Count; ++i) {
      C_OscSubDeviceDefinition c_SubDevice;
      c_SubDevice.FromQDataStream(ro_DataStream);
      this->c_SubDevices.append(c_SubDevice);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to JSON
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscDeviceDefinition::ToJsonObject() const {
   QJsonObject obj;
   QJsonArray bitratesArray;
   QJsonArray canFdBitratesArray;
   QJsonArray featuresArray;
   QJsonArray subDevicesArray;
   
   obj["device-name"] = this->c_DeviceName;
   obj["device-name-alias"] = this->c_DeviceNameAlias;
   obj["device-description"] = this->c_DeviceDescription;
   obj["image-path"] = this->c_ImagePath;
   obj["file-path"] = this->c_FilePath;
   obj["num-can-busses"] = this->u8_NumCanBusses;
   obj["num-ethernet-busses"] = this->u8_NumEthernetBusses;
   
   for (uint16_t bitrate : this->c_SupportedBitrates) {
      bitratesArray.append(static_cast<qint64>(bitrate));
   }
   obj["supported-bitrates"] = bitratesArray;
   
   for (uint16_t bitrate : this->c_SupportedCanFdDataBitrates) {
      canFdBitratesArray.append(static_cast<qint64>(bitrate));
   }
   obj["supported-canfd-bitrates"] = canFdBitratesArray;
   
   for (const auto &rc_Feature : this->c_SupportedCanFeatures) {
      featuresArray.append(rc_Feature.ToJsonObject());
   }
   obj["supported-can-features"] = featuresArray;
   
   obj["manufacturer-id"] = static_cast<qint64>(this->u8_ManufacturerId);
   obj["manufacturer-display"] = this->c_ManufacturerDisplayValue;
   obj["company-logo-link"] = this->c_CompanyLogoLink;
   obj["product-page-link"] = this->c_ProductPageLink;
   obj["toolbox-icon"] = this->c_ToolboxIcon;
   
   for (const auto &rc_SubDevice : this->c_SubDevices) {
      subDevicesArray.append(rc_SubDevice.ToJsonObject());
   }
   obj["sub-devices"] = subDevicesArray;
   
   return obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from JSON
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDeviceDefinition::FromJsonObject(const QJsonObject &ro_Json) {
   this->c_DeviceName = ro_Json["device-name"].toString();
   this->c_DeviceNameAlias = ro_Json["device-name-alias"].toString();
   this->c_DeviceDescription = ro_Json["device-description"].toString();
   this->c_ImagePath = ro_Json["image-path"].toString();
   this->c_FilePath = ro_Json["file-path"].toString();
   this->u8_NumCanBusses = static_cast<uint8_t>(ro_Json["num-can-busses"].toInt());
   this->u8_NumEthernetBusses = static_cast<uint8_t>(ro_Json["num-ethernet-busses"].toInt());
   
   this->c_SupportedBitrates.clear();
   QJsonArray bitratesArray = ro_Json["supported-bitrates"].toArray();
   for (const auto &value : bitratesArray) {
      this->c_SupportedBitrates.append(static_cast<uint16_t>(value.toInt()));
   }
   
   this->c_SupportedCanFdDataBitrates.clear();
   QJsonArray canFdBitratesArray = ro_Json["supported-canfd-bitrates"].toArray();
   for (const auto &value : canFdBitratesArray) {
      this->c_SupportedCanFdDataBitrates.append(static_cast<uint16_t>(value.toInt()));
   }
   
   this->c_SupportedCanFeatures.clear();
   QJsonArray featuresArray = ro_Json["supported-can-features"].toArray();
   for (const auto &value : featuresArray) {
      C_OscSupportedCanInterfaceFeatures c_Feature;
      c_Feature.FromJsonObject(value.toObject());
      this->c_SupportedCanFeatures.append(c_Feature);
   }
   
   this->u8_ManufacturerId = static_cast<uint8_t>(ro_Json["manufacturer-id"].toInt());
   this->c_ManufacturerDisplayValue = ro_Json["manufacturer-display"].toString();
   this->c_CompanyLogoLink = ro_Json["company-logo-link"].toString();
   this->c_ProductPageLink = ro_Json["product-page-link"].toString();
   this->c_ToolboxIcon = ro_Json["toolbox-icon"].toString();
   
   this->c_SubDevices.clear();
   QJsonArray subDevicesArray = ro_Json["sub-devices"].toArray();
   for (const auto &value : subDevicesArray) {
      C_OscSubDeviceDefinition c_SubDevice;
      c_SubDevice.FromJsonObject(value.toObject());
      this->c_SubDevices.append(c_SubDevice);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to XML
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscDeviceDefinition::ToQDomDocument(QDomDocument &ro_Doc,
                                                   const QString &orc_ElementName) const {
   QDomElement element = ro_Doc.createElement(orc_ElementName);
   
   element.setAttribute("device-name", this->c_DeviceName);
   element.setAttribute("device-name-alias", this->c_DeviceNameAlias);
   element.setAttribute("device-description", this->c_DeviceDescription);
   element.setAttribute("image-path", this->c_ImagePath);
   element.setAttribute("file-path", this->c_FilePath);
   element.setAttribute("num-can-busses", this->u8_NumCanBusses);
   element.setAttribute("num-ethernet-busses", this->u8_NumEthernetBusses);
   
   QDomElement bitratesElement = ro_Doc.createElement("supported-bitrates");
   for (uint16_t bitrate : this->c_SupportedBitrates) {
      QDomElement itemElement = ro_Doc.createElement("bitrate");
      itemElement.setAttribute("value", bitrate);
      bitratesElement.appendChild(itemElement);
   }
   element.appendChild(bitratesElement);
   
   QDomElement canFdBitratesElement = ro_Doc.createElement("supported-canfd-bitrates");
   for (uint16_t bitrate : this->c_SupportedCanFdDataBitrates) {
      QDomElement itemElement = ro_Doc.createElement("bitrate");
      itemElement.setAttribute("value", bitrate);
      canFdBitratesElement.appendChild(itemElement);
   }
   element.appendChild(canFdBitratesElement);
   
   QDomElement featuresElement = ro_Doc.createElement("supported-can-features");
   for (const auto &rc_Feature : this->c_SupportedCanFeatures) {
      QDomElement featureElement = rc_Feature.ToQDomDocument(ro_Doc, "feature");
      featuresElement.appendChild(featureElement);
   }
   element.appendChild(featuresElement);
   
   element.setAttribute("manufacturer-id", this->u8_ManufacturerId);
   element.setAttribute("manufacturer-display", this->c_ManufacturerDisplayValue);
   element.setAttribute("company-logo-link", this->c_CompanyLogoLink);
   element.setAttribute("product-page-link", this->c_ProductPageLink);
   element.setAttribute("toolbox-icon", this->c_ToolboxIcon);
   
   QDomElement subDevicesElement = ro_Doc.createElement("sub-devices");
   for (const auto &rc_SubDevice : this->c_SubDevices) {
      QDomElement subDeviceElement = rc_SubDevice.ToQDomDocument(ro_Doc, "sub-device");
      subDevicesElement.appendChild(subDeviceElement);
   }
   element.appendChild(subDevicesElement);
   
   return element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from XML
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscDeviceDefinition::FromQDomDocument(const QDomElement &ro_Element) {
   this->c_DeviceName = ro_Element.attribute("device-name");
   this->c_DeviceNameAlias = ro_Element.attribute("device-name-alias");
   this->c_DeviceDescription = ro_Element.attribute("device-description");
   this->c_ImagePath = ro_Element.attribute("image-path");
   this->c_FilePath = ro_Element.attribute("file-path");
   this->u8_NumCanBusses = static_cast<uint8_t>(ro_Element.attribute("num-can-busses").toUInt());
   this->u8_NumEthernetBusses = static_cast<uint8_t>(ro_Element.attribute("num-ethernet-busses").toUInt());
   
   this->c_SupportedBitrates.clear();
   QDomElement bitratesElement = ro_Element.firstChildElement("supported-bitrates");
   QDomNode node = bitratesElement.firstChild();
   while (!node.isNull()) {
      QDomElement itemElement = node.toElement();
      if (!itemElement.isNull()) {
         this->c_SupportedBitrates.append(static_cast<uint16_t>(itemElement.attribute("value").toUInt()));
      }
      node = node.nextSibling();
   }
   
   this->c_SupportedCanFdDataBitrates.clear();
   QDomElement canFdBitratesElement = ro_Element.firstChildElement("supported-canfd-bitrates");
   node = canFdBitratesElement.firstChild();
   while (!node.isNull()) {
      QDomElement itemElement = node.toElement();
      if (!itemElement.isNull()) {
         this->c_SupportedCanFdDataBitrates.append(static_cast<uint16_t>(itemElement.attribute("value").toUInt()));
      }
      node = node.nextSibling();
   }
   
   this->c_SupportedCanFeatures.clear();
   QDomElement featuresElement = ro_Element.firstChildElement("supported-can-features");
   node = featuresElement.firstChild();
   while (!node.isNull()) {
      QDomElement featureElement = node.toElement();
      if (!featureElement.isNull()) {
         C_OscSupportedCanInterfaceFeatures c_Feature;
         c_Feature.FromQDomDocument(featureElement);
         this->c_SupportedCanFeatures.append(c_Feature);
      }
      node = node.nextSibling();
   }
   
   this->u8_ManufacturerId = static_cast<uint8_t>(ro_Element.attribute("manufacturer-id").toUInt());
   this->c_ManufacturerDisplayValue = ro_Element.attribute("manufacturer-display");
   this->c_CompanyLogoLink = ro_Element.attribute("company-logo-link");
   this->c_ProductPageLink = ro_Element.attribute("product-page-link");
   this->c_ToolboxIcon = ro_Element.attribute("toolbox-icon");
   
   this->c_SubDevices.clear();
   QDomElement subDevicesElement = ro_Element.firstChildElement("sub-devices");
   node = subDevicesElement.firstChild();
   while (!node.isNull()) {
      QDomElement subDeviceElement = node.toElement();
      if (!subDeviceElement.isNull()) {
         C_OscSubDeviceDefinition c_SubDevice;
         c_SubDevice.FromQDomDocument(subDeviceElement);
         this->c_SubDevices.append(c_SubDevice);
      }
      node = node.nextSibling();
   }
}
