//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Xapp certificates package data
   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCXCEMANIFEST_HPP
#define C_OSCXCEMANIFEST_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QList>
#include <QString>
#include <QByteArray>
#include <QDataStream>
#include <QJsonObject>
#include <QDomDocument>
#include <QDomElement>

#include "C_OscXceUpdatePackageParameters.hpp"

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

class C_OscXceManifest {
public:
   C_OscXceManifest();

   // Data members
   QString c_CertificatesPath;
   QList<C_OscXceUpdatePackageParameters> c_UpdatePackageParameters;

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
   QDomElement ToQDomDocument(QDomDocument& orc_Doc, const QString& orc_RootElementName = "xce-manifest") const;
   
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
