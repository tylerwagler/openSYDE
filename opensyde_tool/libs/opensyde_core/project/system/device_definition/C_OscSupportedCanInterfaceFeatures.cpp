//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Dataclass for supported can interface  features

   Dataclass for supported can interface features

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
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

#include "C_OscSupportedCanInterfaceFeatures.hpp"

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
C_OscSupportedCanInterfaceFeatures::C_OscSupportedCanInterfaceFeatures(void)
    : q_SupportsCanFd(false) {}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupportedCanInterfaceFeatures::ToQDataStream(QDataStream &ro_DataStream) const {
   ro_DataStream << this->c_Interface;
   ro_DataStream << static_cast<int32_t>(this->q_SupportsCanFd);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupportedCanInterfaceFeatures::FromQDataStream(QDataStream &ro_DataStream) {
   int32_t s32_SupportsCanFd;
   ro_DataStream >> this->c_Interface;
   ro_DataStream >> s32_SupportsCanFd;
   this->q_SupportsCanFd = (s32_SupportsCanFd != 0);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to JSON
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscSupportedCanInterfaceFeatures::ToJsonObject() const {
   QJsonObject obj;
   obj["interface"] = this->c_Interface;
   obj["supports-canfd"] = this->q_SupportsCanFd;
   return obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from JSON
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupportedCanInterfaceFeatures::FromJsonObject(const QJsonObject &ro_Json) {
   this->c_Interface = ro_Json["interface"].toString();
   this->q_SupportsCanFd = ro_Json["supports-canfd"].toBool();
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to XML
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscSupportedCanInterfaceFeatures::ToQDomDocument(QDomDocument &ro_Doc,
                                                                const QString &orc_ElementName) const {
   QDomElement element = ro_Doc.createElement(orc_ElementName);
   element.setAttribute("interface", this->c_Interface);
   element.setAttribute("supports-canfd", this->q_SupportsCanFd);
   return element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from XML
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSupportedCanInterfaceFeatures::FromQDomDocument(const QDomElement &ro_Element) {
   this->c_Interface = ro_Element.attribute("interface");
   this->q_SupportsCanFd = ro_Element.attribute("supports-canfd").toInt() != 0;
}
