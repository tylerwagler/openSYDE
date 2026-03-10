//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data logger job information
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDATALOGGERJOB_HPP
#define C_OSCDATALOGGERJOB_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscDataLoggerDataElementReference.hpp"
#include "C_OscDataLoggerJobProperties.hpp"
#include <QList>
#include <QDataStream>
#include <QJsonObject>
#include <QDomDocument>

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

class C_OscDataLoggerJob {
public:
  C_OscDataLoggerJob();

  void CalcHash(uint32_t &oru32_HashValue) const;

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
  // XML Serialization (QDomDocument)
  // --------------------------------------------------------------------------
  
  //----------------------------------------------------------------------------------------------------------------------
  /*!
     \brief Serialize to QDomDocument
     
     \param[in] orc_Doc    XML document to append to
     \param[in] orc_RootElementName    Name of the root element to create
     
     \return QDomElement representing the serialized data
  */
  //----------------------------------------------------------------------------------------------------------------------
  QDomElement ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName = "job") const;
  
 //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Serialize to QDomElement (XML format) - wrapper for C_OscFilerUtil compatibility
      
      \param[in] orc_Doc            DOM document to create element in
      \param[in] orc_RootElementName    Name of the root element to create
      
      \return QDomElement representing the serialized data
   */
   //----------------------------------------------------------------------------------------------------------------------
   QDomElement ToQDomElement(QDomDocument& orc_Doc, const QString& orc_RootElementName = "job") const;
   
   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Deserialize from QDomElement
     
     \param[in] orc_Element    XML element to deserialize from
     
     \return C_NO_ERR on success
  */
  //----------------------------------------------------------------------------------------------------------------------
  int32_t FromQDomElement(const QDomElement& orc_Element);

  static bool hq_AllowDataloggerFeature; ///< Flag to enable/disable data logger
                                         // feature
  bool q_IsEnabled;                      ///< Flag to enable or disable logging
                                         // configuration
  C_OscDataLoggerJobProperties c_Properties; ///< all configured properties
  QList<C_OscDataLoggerDataElementReference>
      c_ConfiguredDataElements; ///< selected data elements to log
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
