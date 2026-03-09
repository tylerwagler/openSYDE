//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class, containing all interpreted entries for one node in a
   parameter set file (implementation)

   Data class, containing all interpreted entries for one node in a parameter
   set file

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscParamSetInterpretedNode.hpp"

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
C_OscParamSetInterpretedNode::C_OscParamSetInterpretedNode(void) {}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream
   \param   ro_DataStream  Output stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedNode::ToQDataStream(QDataStream &ro_DataStream) const {
  ro_DataStream << c_Name;
  ro_DataStream << static_cast<qint32>(c_DataPools.size());
  for (const auto &c_DataPool : c_DataPools) {
    c_DataPool.ToQDataStream(ro_DataStream);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream
   \param   ro_DataStream  Input stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedNode::FromQDataStream(QDataStream &ro_DataStream) {
  ro_DataStream >> c_Name;
  qint32 s_Size;
  ro_DataStream >> s_Size;
  c_DataPools.clear();
  for (qint32 s_I = 0; s_I < s_Size; ++s_I) {
    C_OscParamSetInterpretedDataPool c_DataPool;
    c_DataPool.FromQDataStream(ro_DataStream);
    c_DataPools.append(c_DataPool);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QJsonObject
   \return  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscParamSetInterpretedNode::ToJsonObject() const {
  QJsonObject c_Obj;
  c_Obj["name"] = c_Name;
  QJsonArray c_DataPoolsArray;
  for (const auto &c_DataPool : c_DataPools) {
    c_DataPoolsArray.append(c_DataPool.ToJsonObject());
  }
  c_Obj["data-pools"] = c_DataPoolsArray;
  return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QJsonObject
   \param   orc_Object  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedNode::FromJsonObject(const QJsonObject &orc_Object) {
  if (orc_Object.contains("name")) c_Name = orc_Object["name"].toString();
  if (orc_Object.contains("data-pools")) {
    QJsonArray c_DataPoolsArray = orc_Object["data-pools"].toArray();
    c_DataPools.clear();
    for (const auto &c_Value : c_DataPoolsArray) {
      C_OscParamSetInterpretedDataPool c_DataPool;
      c_DataPool.FromJsonObject(c_Value.toObject());
      c_DataPools.append(c_DataPool);
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
QDomElement C_OscParamSetInterpretedNode::ToQDomDocument(QDomDocument &orc_Doc,
                                                         const QString &orc_ElementName) const {
  QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
  c_Element.setAttribute("name", c_Name);
  for (const auto &c_DataPool : c_DataPools) {
    QDomElement c_DataPoolElem = c_DataPool.ToQDomDocument(orc_Doc, "data-pool");
    c_Element.appendChild(c_DataPoolElem);
  }
  return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDomElement
   \param   orc_Element  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedNode::FromQDomDocument(const QDomElement &orc_Element) {
  if (orc_Element.hasAttribute("name")) c_Name = orc_Element.attribute("name");
  QDomNode c_Node = orc_Element.firstChild();
  while (!c_Node.isNull()) {
    QDomElement c_Elem = c_Node.toElement();
    if (!c_Elem.isNull() && c_Elem.tagName() == "data-pool") {
      C_OscParamSetInterpretedDataPool c_DataPool;
      c_DataPool.FromQDomDocument(c_Elem);
      c_DataPools.append(c_DataPool);
    }
    c_Node = c_Node.nextSibling();
  }
}
