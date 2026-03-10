//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class, containing all raw entries for one node in a
   parameter set file (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPARAMSETRAWNODE_HPP
#define C_OSCPARAMSETRAWNODE_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscParamSetDataPoolInfo.hpp"
#include "C_OscParamSetRawEntry.hpp"
#include <QList>
#include <QString>
#include <QDataStream>
#include <QJsonObject>
#include <QDomElement>

/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_core {
/* -- Global Constants
 * ----------------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_OscParamSetRawNode {
public:
  C_OscParamSetRawNode(void);

  QString c_Name;                         ///< Node name
  QList<C_OscParamSetRawEntry> c_Entries; ///< All entries to write to this node
  QList<C_OscParamSetDataPoolInfo>
      c_DataPools; ///< Information about used data pools

  // --------------------------------------------------------------------------
  // Binary Serialization (QDataStream)
  // --------------------------------------------------------------------------
  
  //----------------------------------------------------------------------------------------------------------------------
  /*!
     \brief Serialize to QDataStream (binary format)
     
     \param[out] orc_Stream    Output stream for serialization
     
     \return C_NO_ERR on success
  */
  //----------------------------------------------------------------------------------------------------------------------
  int32_t ToQDataStream(QDataStream& orc_Stream) const;
  
  //----------------------------------------------------------------------------------------------------------------------
  /*!
     \brief Deserialize from QDataStream (binary format)
     
     \param[in,out] orc_Stream    Input stream for deserialization
     
     \return C_NO_ERR on success
  */
  //----------------------------------------------------------------------------------------------------------------------
  int32_t FromQDataStream(QDataStream& orc_Stream);
  
  // --------------------------------------------------------------------------
  // JSON Serialization (QJsonObject)
  // --------------------------------------------------------------------------
  
  //----------------------------------------------------------------------------------------------------------------------
  /*!
     \brief Serialize to QJsonObject
     
     \return JSON object containing all data
  */
  //----------------------------------------------------------------------------------------------------------------------
  QJsonObject ToJsonObject() const;
  
  //----------------------------------------------------------------------------------------------------------------------
  /*!
     \brief Deserialize from QJsonObject
     
     \param[in] orc_Object    JSON object to deserialize from
     
     \return C_NO_ERR on success
  */
  //----------------------------------------------------------------------------------------------------------------------
  int32_t FromJsonObject(const QJsonObject& orc_Object);
  
  // --------------------------------------------------------------------------
  // XML Serialization (QDomElement)
  // --------------------------------------------------------------------------
  
 //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Serialize to QDomElement (XML format) - wrapper for C_OscFilerUtil compatibility
      
      \param[in] orc_Doc            DOM document to create element in
      \param[in] orc_RootElementName    Name of the root element to create
      
      \return QDomElement representing the serialized data
   */
   //----------------------------------------------------------------------------------------------------------------------
   QDomElement ToQDomElement(QDomDocument& orc_Doc, const QString& orc_RootElementName = "param-set-raw-node") const;
   
   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Serialize to QDomDocument
     
     \param[in] orc_Doc    XML document to append to
     \param[in] orc_RootElementName    Name of the root element to create
     
     \return QDomElement representing the serialized data
  */
  //----------------------------------------------------------------------------------------------------------------------
  QDomElement ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName = "param-set-raw-node") const;
  
  //----------------------------------------------------------------------------------------------------------------------
  /*!
     \brief Deserialize from QDomElement
     
     \param[in] orc_Element    XML element to deserialize from
     
     \return C_NO_ERR on success
  */
  //----------------------------------------------------------------------------------------------------------------------
  int32_t FromQDomElement(const QDomElement& orc_Element);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
