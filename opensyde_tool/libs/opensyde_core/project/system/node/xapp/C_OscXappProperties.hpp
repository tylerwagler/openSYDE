//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       X-app properties
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXAPPPROPERTIES_HPP
#define C_OSCXAPPPROPERTIES_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscSystemBus.hpp"
#include <QByteArray>
#include <QDataStream>
#include <QJsonObject>
#include <QDomDocument>
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

class C_OscXappProperties {
public:
   C_OscXappProperties();

   void Initialize();

   void CalcHash(uint32_t& oru32_HashValue) const;

   // Data members
   uint32_t u32_PollingIntervalMs;     ///< Data polling interval in milliseconds
   uint32_t u32_DataRequestIntervalMs; ///< Data request interval in milliseconds
   C_OscSystemBus::E_Type e_ConnectedInterfaceType; ///< e.g. CAN, ethernet
   uint8_t u8_ConnectedInterfaceNumber; ///< 0 = first interface for this type

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
   QDomElement ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName = "xapp-properties") const;
   
   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Serialize to QDomElement (XML format)
      
      \param[in] orc_Doc            DOM document to create element in
      \param[in] orc_RootElementName    Name of the root element to create
      
      \return QDomElement representing the serialized data
   */
   //----------------------------------------------------------------------------------------------------------------------
   QDomElement ToQDomElement(QDomDocument& orc_Doc, const QString& orc_RootElementName = "xappProperties") const;
   
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
