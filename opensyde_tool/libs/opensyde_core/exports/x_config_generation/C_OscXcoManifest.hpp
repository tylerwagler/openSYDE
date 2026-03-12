//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Manifest data
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXCOMANIFEST_HPP
#define C_OSCXCOMANIFEST_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QString>
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

class C_OscXcoManifest {
public:
   C_OscXcoManifest();

   // Data members
   QString c_NodeName;

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
   QDomElement ToQDomElement(QDomDocument& orc_Doc, const QString& orc_RootElementName = "xco-manifest") const;
   
   //----------------------------------------------------------------------------------------------------------------------
   /*!
      \brief Serialize to QDomDocument (XML format) - wrapper for C_OscFilerUtil compatibility
      
      \param[in] orc_Doc            DOM document to create element in
      \param[in] orc_RootElementName    Name of the root element to create
      
      \return QDomElement representing the serialized data
   */
   //----------------------------------------------------------------------------------------------------------------------
   QDomElement ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName = "xco-manifest") const;
   
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
