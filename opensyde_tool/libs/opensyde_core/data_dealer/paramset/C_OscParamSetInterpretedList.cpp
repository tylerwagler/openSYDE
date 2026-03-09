//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class, containing the interpreted entries for one list in a
   parameter set file (implementation)

   Data class, containing the interpreted entries for one list in a parameter
   set file

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscParamSetInterpretedList.hpp"

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
C_OscParamSetInterpretedList::C_OscParamSetInterpretedList(void) {}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream
   \param   ro_DataStream  Output stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedList::ToQDataStream(QDataStream &ro_DataStream) const {
  ro_DataStream << c_Name;
  ro_DataStream << static_cast<qint32>(c_Elements.size());
  for (const auto &c_Element : c_Elements) {
    c_Element.ToQDataStream(ro_DataStream);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream
   \param   ro_DataStream  Input stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedList::FromQDataStream(QDataStream &ro_DataStream) {
  ro_DataStream >> c_Name;
  qint32 s_Size;
  ro_DataStream >> s_Size;
  c_Elements.clear();
  for (qint32 s_I = 0; s_I < s_Size; ++s_I) {
    C_OscParamSetInterpretedElement c_Element;
    c_Element.FromQDataStream(ro_DataStream);
    c_Elements.append(c_Element);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QJsonObject
   \return  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscParamSetInterpretedList::ToJsonObject() const {
  QJsonObject c_Obj;
  c_Obj["name"] = c_Name;
  QJsonArray c_ElementsArray;
  for (const auto &c_Element : c_Elements) {
    c_ElementsArray.append(c_Element.ToJsonObject());
  }
  c_Obj["elements"] = c_ElementsArray;
  return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QJsonObject
   \param   orc_Object  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedList::FromJsonObject(const QJsonObject &orc_Object) {
  if (orc_Object.contains("name")) c_Name = orc_Object["name"].toString();
  if (orc_Object.contains("elements")) {
    QJsonArray c_ElementsArray = orc_Object["elements"].toArray();
    c_Elements.clear();
    for (const auto &c_Value : c_ElementsArray) {
      C_OscParamSetInterpretedElement c_Element;
      c_Element.FromJsonObject(c_Value.toObject());
      c_Elements.append(c_Element);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDomElement
   \param   orc_Doc        XML document
   \param   orc_ElementName  Element name
   \return  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscParamSetInterpretedList::ToQDomDocument(QDomDocument &orc_Doc,
                                                         const QString &orc_ElementName) const {
  QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
  c_Element.setAttribute("name", c_Name);
  for (const auto &c_ListElement : c_Elements) {
    QDomElement c_ValueElem = c_ListElement.ToQDomDocument(orc_Doc, "element");
    c_Element.appendChild(c_ValueElem);
  }
  return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDomElement
   \param   orc_Element  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedList::FromQDomDocument(const QDomElement &orc_Element) {
  if (orc_Element.hasAttribute("name")) c_Name = orc_Element.attribute("name");
  QDomNode c_Node = orc_Element.firstChild();
  while (!c_Node.isNull()) {
    QDomElement c_Elem = c_Node.toElement();
    if (!c_Elem.isNull() && c_Elem.tagName() == "element") {
      C_OscParamSetInterpretedElement c_Element;
      c_Element.FromQDomDocument(c_Elem);
      c_Elements.append(c_Element);
    }
    c_Node = c_Node.nextSibling();
  }
}
