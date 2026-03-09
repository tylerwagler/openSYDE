//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for code export settings (implementation)

   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscNodeCodeExportSettings.hpp"
#include "C_OscHashUtil.hpp"
#include "stwtypes.hpp"

#include <QDataStream>
#include <QDomDocument>
#include <QJsonObject>

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
/*! \brief  Default constructor/destructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscNodeCodeExportSettings::C_OscNodeCodeExportSettings(void) {
  this->Initialize();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Initialize
 */
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeCodeExportSettings::Initialize() { e_ScalingSupport = eFLOAT32; }

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out] oru32_HashValue    Hash value with initial [in] value and
   result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeCodeExportSettings::CalcHash(uint32_t & oru32_HashValue) const
{
   hash_util::CalcHashMembers(oru32_HashValue, this->e_ScalingSupport);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream (binary format)
   \param   ro_DataStream  Output stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeCodeExportSettings::ToQDataStream(QDataStream &ro_DataStream) const {
  ro_DataStream << static_cast<int32_t>(this->e_ScalingSupport);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream (binary format)
   \param   ro_DataStream  Input stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeCodeExportSettings::FromQDataStream(QDataStream &ro_DataStream) {
  int32_t s32_Scaling;
  ro_DataStream >> s32_Scaling;
  this->e_ScalingSupport = static_cast<E_Scaling>(s32_Scaling);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QJsonObject (JSON format)
   \return  JSON object containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscNodeCodeExportSettings::ToJsonObject() const {
  QJsonObject c_Obj;
  c_Obj["scaling-support"] = static_cast<int32_t>(this->e_ScalingSupport);
  return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QJsonObject (JSON format)
   \param   orc_Object  JSON object containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeCodeExportSettings::FromJsonObject(const QJsonObject &orc_Object) {
  if (orc_Object.contains("scaling-support")) {
    this->e_ScalingSupport =
        static_cast<E_Scaling>(orc_Object["scaling-support"].toInt());
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDomElement (XML format)
   \param   orc_Doc        XML document
   \param   orc_ElementName  Element name
   \return  XML element containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscNodeCodeExportSettings::ToQDomDocument(
    QDomDocument &orc_Doc, const QString &orc_ElementName) const {
  QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
  c_Element.setAttribute("scaling-support",
                         static_cast<int32_t>(this->e_ScalingSupport));
  return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDomElement (XML format)
   \param   orc_Element  XML element containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeCodeExportSettings::FromQDomDocument(const QDomElement &orc_Element) {
  if (orc_Element.hasAttribute("scaling-support")) {
    this->e_ScalingSupport = static_cast<E_Scaling>(
        orc_Element.attribute("scaling-support").toInt());
  }
}
