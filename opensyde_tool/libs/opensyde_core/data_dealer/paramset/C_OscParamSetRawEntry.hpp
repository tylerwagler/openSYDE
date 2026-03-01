//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for one raw entry in a parameter set file (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPARAMSETRAWENTRY_HPP
#define C_OSCPARAMSETRAWENTRY_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "stwtypes.hpp"
#include <QByteArray>
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
/// Describes one raw (black box) entry in a parameter set file
class C_OscParamSetRawEntry {
public:
  C_OscParamSetRawEntry(void);

  uint32_t u32_StartAddress; ///< Start address to write to
  QByteArray c_Bytes;        ///< Bytes to write at address

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
     \brief Serialize to QDomDocument
     
     \param[in] orc_Doc    XML document to append to
     \param[in] orc_RootElementName    Name of the root element to create
     
     \return QDomElement representing the serialized data
  */
  //----------------------------------------------------------------------------------------------------------------------
  QDomElement ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName = "raw-entry") const;
  
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
