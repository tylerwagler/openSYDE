//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class, containing the interpreted entries for one element
   in a parameter set file (implementation)

   Data class, containing the interpreted entries for one element in a parameter
   set file

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscParamSetInterpretedElement.hpp"

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
C_OscParamSetInterpretedElement::C_OscParamSetInterpretedElement(void) {}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream
   \param   ro_DataStream  Output stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedElement::ToQDataStream(QDataStream &ro_DataStream) const {
  ro_DataStream << c_Name;
  c_NvmValue.ToQDataStream(ro_DataStream);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream
   \param   ro_DataStream  Input stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedElement::FromQDataStream(QDataStream &ro_DataStream) {
  ro_DataStream >> c_Name;
  c_NvmValue.FromQDataStream(ro_DataStream);
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QJsonObject
   \return  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscParamSetInterpretedElement::ToJsonObject() const {
  QJsonObject c_Obj;
  c_Obj["name"] = c_Name;
  c_Obj["nvm-value"] = c_NvmValue.ToJsonObject();
  return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QJsonObject
   \param   orc_Object  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedElement::FromJsonObject(const QJsonObject &orc_Object) {
  if (orc_Object.contains("name")) c_Name = orc_Object["name"].toString();
  if (orc_Object.contains("nvm-value")) c_NvmValue.FromJsonObject(orc_Object["nvm-value"].toObject());
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDomElement
   \param   orc_Doc        XML document
   \param   orc_ElementName  Element name
   \return  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscParamSetInterpretedElement::ToQDomDocument(QDomDocument &orc_Doc,
                                                            const QString &orc_ElementName) const {
  QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
  c_Element.setAttribute("name", c_Name);
  QDomElement c_ValueElem = c_NvmValue.ToQDomDocument(orc_Doc, "nvm-value");
  c_Element.appendChild(c_ValueElem);
  return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDomElement
   \param   orc_Element  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedElement::FromQDomDocument(const QDomElement &orc_Element) {
  if (orc_Element.hasAttribute("name")) c_Name = orc_Element.attribute("name");
  QDomNode c_Node = orc_Element.firstChild();
  while (!c_Node.isNull()) {
    QDomElement c_Elem = c_Node.toElement();
    if (!c_Elem.isNull() && c_Elem.tagName() == "nvm-value") {
      c_NvmValue.FromQDomDocument(c_Elem);
    }
    c_Node = c_Node.nextSibling();
  }
}
