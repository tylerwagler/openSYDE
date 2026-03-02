//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       The squad of nodes. In case of multiple CPUs it has all indexes
   of the sub nodes and has its base name

   \copyright   Copyright 2021 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "stwerrors.hpp"

#include "C_OscNodeSquad.hpp"
#include <QJsonArray>
#include <QJsonValue>
#include <QDomDocument>
#include <QDomElement>

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */
const QString C_OscNodeSquad::hc_SEPARATOR = "::";

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
/*! \brief    Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscNodeSquad::C_OscNodeSquad(void) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscNodeSquad::~C_OscNodeSquad(void) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Combine names

   \param[in]  orc_MainDeviceName   Main device name
   \param[in]  orc_SubDeviceName    Sub device name

   \return
   Combined name, if possible
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscNodeSquad::h_CombineNames(const QString &orc_MainDeviceName,
                                       const QString &orc_SubDeviceName) {
  QString c_Retval;
  if (orc_MainDeviceName.isEmpty()) {
    c_Retval = orc_SubDeviceName;
  } else if (orc_SubDeviceName.isEmpty()) {
    c_Retval = orc_MainDeviceName;
  } else {
    c_Retval =
        orc_MainDeviceName + C_OscNodeSquad::hc_SEPARATOR + orc_SubDeviceName;
  }
  return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Sets the base name for the squad and updates the names of all sub
   nodes

   \param[in,out]  orc_Nodes           Vector of all available nodes. Sub nodes
   of node squad must be part of it \param[in]      orc_NodeBaseName    New name
   of squad node

   \retval   C_NO_ERR   Base name and names of all sub nodes updated
   \retval   C_RANGE    At least one sub node index does not exist in orc_Nodes
   \retval   C_CONFIG   At least one sub node device definition is not valid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeSquad::SetBaseName(QList<C_OscNode> &orc_Nodes,
                                    const QString &orc_NodeBaseName) {
  // Check the indexes first
  uint32_t u32_NodeIndexCounter;
  int32_t s32_Return = C_NO_ERR;

  this->c_BaseName = orc_NodeBaseName;

  for (u32_NodeIndexCounter = 0U;
       u32_NodeIndexCounter < this->c_SubNodeIndexes.size();
       ++u32_NodeIndexCounter) {
    if (this->c_SubNodeIndexes[u32_NodeIndexCounter] >= orc_Nodes.size()) {
      // At least one index is not valid
      s32_Return = C_RANGE;
      break;
    }
  }

  if (s32_Return == C_NO_ERR) {
    for (u32_NodeIndexCounter = 0U;
         u32_NodeIndexCounter < this->c_SubNodeIndexes.size();
         ++u32_NodeIndexCounter) {
      const uint32_t u32_NodeIndex =
          this->c_SubNodeIndexes[u32_NodeIndexCounter];

      if (u32_NodeIndex < orc_Nodes.size()) {
        C_OscNode &rc_Node = orc_Nodes[u32_NodeIndex];

        if ((rc_Node.pc_DeviceDefinition != NULL) &&
            (rc_Node.u32_SubDeviceIndex <
             rc_Node.pc_DeviceDefinition->c_SubDevices.size())) {
          // Setting the new name for the sub node
          rc_Node.c_Properties.c_Name = C_OscNodeSquad::h_CombineNames(
              orc_NodeBaseName, rc_Node.pc_DeviceDefinition
                                    ->c_SubDevices[rc_Node.u32_SubDeviceIndex]
                                    .c_SubDeviceName);
        } else {
          // Should not happen
          s32_Return = C_CONFIG;
          break;
        }
      }
    }
  }

  return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check is multi device

   \param[in]   ou32_NodeIndex         Node index
   \param[in]   orc_AvailableGroups    Available groups
   \param[out]  opu32_GroupIndex       Group index

   \return
   Is multi device
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscNodeSquad::h_CheckIsMultiDevice(
    const uint32_t ou32_NodeIndex,
    const QList<C_OscNodeSquad> &orc_AvailableGroups,
    uint32_t *const opu32_GroupIndex) {
  bool q_Retval = false;

  if (opu32_GroupIndex != NULL) {
    *opu32_GroupIndex = 0UL;
  }

  for (uint32_t u32_ItGroup = 0; u32_ItGroup < orc_AvailableGroups.size();
       ++u32_ItGroup) {
    const C_OscNodeSquad &rc_Group = orc_AvailableGroups[u32_ItGroup];

    for (uint32_t u32_ItSubDevice = 0UL;
         u32_ItSubDevice < rc_Group.c_SubNodeIndexes.size();
         ++u32_ItSubDevice) {
      if (ou32_NodeIndex == rc_Group.c_SubNodeIndexes[u32_ItSubDevice]) {
        q_Retval = true;
        if (opu32_GroupIndex != NULL) {
          *opu32_GroupIndex = u32_ItGroup;
        }
        break;
      }
    }
  }
  return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream (binary format)
   
   \param[in,out]  orc_Stream    Data stream to write to
   
   \return Error code
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeSquad::ToQDataStream(QDataStream& orc_Stream) const {
   int32_t s32_Retval = stw::errors::C_NO_ERR;
   
   orc_Stream << this->c_BaseName;
   uint32_t u32_Count = static_cast<uint32_t>(this->c_SubNodeIndexes.size());
   orc_Stream << u32_Count;
   for (uint32_t u32_I = 0; u32_I < u32_Count; ++u32_I) {
      orc_Stream << this->c_SubNodeIndexes[u32_I];
   }
   
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream (binary format)
   
   \param[in,out]  orc_Stream    Data stream to read from
   
   \return Error code
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeSquad::FromQDataStream(QDataStream& orc_Stream) {
   int32_t s32_Retval = stw::errors::C_NO_ERR;
   uint32_t u32_Count = 0;
   
   orc_Stream >> this->c_BaseName;
   orc_Stream >> u32_Count;
   this->c_SubNodeIndexes.clear();
   this->c_SubNodeIndexes.reserve(u32_Count);
   for (uint32_t u32_I = 0; u32_I < u32_Count; ++u32_I) {
      uint32_t u32_Index = 0;
      orc_Stream >> u32_Index;
      this->c_SubNodeIndexes.append(u32_Index);
   }
   
   return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QJsonObject (JSON format)
   
   \return JSON object containing serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscNodeSquad::ToJsonObject() const {
   QJsonObject c_Object;
   
   c_Object["baseName"] = this->c_BaseName;
   QJsonArray c_Array;
   for (uint32_t u32_I = 0; u32_I < static_cast<uint32_t>(this->c_SubNodeIndexes.size()); ++u32_I) {
      c_Array.append(static_cast<qint64>(this->c_SubNodeIndexes[u32_I]));
   }
   c_Object["subNodeIndexes"] = c_Array;
   
   return c_Object;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QJsonObject (JSON format)
   
   \param[in]  orc_Object    JSON object containing serialized data
   
   \return Error code
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeSquad::FromJsonObject(const QJsonObject& orc_Object) {
   int32_t s32_Retval = stw::errors::C_NO_ERR;
   
   this->c_BaseName = orc_Object["baseName"].toString();
   QJsonArray c_Array = orc_Object["subNodeIndexes"].toArray();
   this->c_SubNodeIndexes.clear();
   this->c_SubNodeIndexes.reserve(c_Array.size());
   for (const QJsonValue& rc_Value : c_Array) {
      this->c_SubNodeIndexes.append(static_cast<uint32_t>(rc_Value.toVariant().toUInt()));
   }
   
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
QDomElement C_OscNodeSquad::ToQDomDocument(QDomDocument& orc_Doc, 
                                           const QString& orc_RootElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
   
   c_Element.setAttribute("baseName", this->c_BaseName);
   QDomElement c_IndicesElement = orc_Doc.createElement("sub-node-indices");
   for (uint32_t u32_I = 0; u32_I < static_cast<uint32_t>(this->c_SubNodeIndexes.size()); ++u32_I) {
      QDomElement c_IndexElement = orc_Doc.createElement("index");
      c_IndexElement.setAttribute("value", QString::number(this->c_SubNodeIndexes[u32_I]));
      c_IndicesElement.appendChild(c_IndexElement);
   }
   c_Element.appendChild(c_IndicesElement);
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDomElement (XML format)
   
   \param[in]  orc_Element    XML element containing serialized data
   
   \return Error code
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeSquad::FromQDomElement(const QDomElement& orc_Element) {
   int32_t s32_Retval = stw::errors::C_NO_ERR;
   
   this->c_BaseName = orc_Element.attribute("baseName");
   QDomElement c_IndicesElement = orc_Element.firstChildElement("sub-node-indices");
   this->c_SubNodeIndexes.clear();
   
   QDomNode c_Node = c_IndicesElement.firstChild();
   while (!c_Node.isNull()) {
      QDomElement c_IndexElement = c_Node.toElement();
      if (!c_IndexElement.isNull()) {
         this->c_SubNodeIndexes.append(static_cast<uint32_t>(c_IndexElement.attribute("value").toUInt()));
      }
      c_Node = c_Node.nextSibling();
   }
   
   return s32_Retval;
}
