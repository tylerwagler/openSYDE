//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class, containing the interpreted entries for one datapool
   in a parameter set file (implementation)

   Data class, containing the interpreted entries for one datapool in a
   parameter set file

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "C_OscParamSetInterpretedDataPool.hpp"
#include <QJsonArray>
#include <QJsonValue>

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
C_OscParamSetInterpretedDataPool::C_OscParamSetInterpretedDataPool(void) {}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream
   \param   ro_DataStream  Output stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedDataPool::ToQDataStream(QDataStream &ro_DataStream) const {
  c_DataPoolInfo.ToQDataStream(ro_DataStream);
  ro_DataStream << static_cast<qint32>(c_Lists.size());
  for (const auto &c_List : c_Lists) {
    c_List.ToQDataStream(ro_DataStream);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream
   \param   ro_DataStream  Input stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedDataPool::FromQDataStream(QDataStream &ro_DataStream) {
  c_DataPoolInfo.FromQDataStream(ro_DataStream);
  qint32 s_Size;
  ro_DataStream >> s_Size;
  c_Lists.clear();
  for (qint32 s_I = 0; s_I < s_Size; ++s_I) {
    C_OscParamSetInterpretedList c_List;
    c_List.FromQDataStream(ro_DataStream);
    c_Lists.append(c_List);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QJsonObject
   \return  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscParamSetInterpretedDataPool::ToJsonObject() const {
  QJsonObject c_Obj;
  c_Obj["data-pool-info"] = c_DataPoolInfo.ToJsonObject();
  QJsonArray c_ListsArray;
  for (const auto &c_List : c_Lists) {
    c_ListsArray.append(c_List.ToJsonObject());
  }
  c_Obj["lists"] = c_ListsArray;
  return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QJsonObject
   \param   orc_Object  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedDataPool::FromJsonObject(const QJsonObject &orc_Object) {
  if (orc_Object.contains("data-pool-info")) {
    c_DataPoolInfo.FromJsonObject(orc_Object["data-pool-info"].toObject());
  }
  if (orc_Object.contains("lists")) {
    QJsonArray c_ListsArray = orc_Object["lists"].toArray();
    c_Lists.clear();
    for (const auto &c_Value : c_ListsArray) {
      C_OscParamSetInterpretedList c_List;
      c_List.FromJsonObject(c_Value.toObject());
      c_Lists.append(c_List);
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
QDomElement C_OscParamSetInterpretedDataPool::ToQDomDocument(QDomDocument &orc_Doc,
                                                             const QString &orc_ElementName) const {
  QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
  QDomElement c_InfoElem = c_DataPoolInfo.ToQDomDocument(orc_Doc, "data-pool-info");
  c_Element.appendChild(c_InfoElem);
  for (const auto &c_List : c_Lists) {
    QDomElement c_ListElem = c_List.ToQDomDocument(orc_Doc, "list");
    c_Element.appendChild(c_ListElem);
  }
  return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDomElement
   \param   orc_Element  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetInterpretedDataPool::FromQDomDocument(const QDomElement &orc_Element) {
  QDomNode c_Node = orc_Element.firstChild();
  while (!c_Node.isNull()) {
    QDomElement c_Elem = c_Node.toElement();
    if (!c_Elem.isNull()) {
      const QString c_TagName = c_Elem.tagName();
      if (c_TagName == "data-pool-info") {
        c_DataPoolInfo.FromQDomElement(c_Elem);
      } else if (c_TagName == "list") {
        C_OscParamSetInterpretedList c_List;
        c_List.FromQDomDocument(c_Elem);
        c_Lists.append(c_List);
      }
    }
    c_Node = c_Node.nextSibling();
  }
}
