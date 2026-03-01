//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE target support package data handling class
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCTARGETSUPPORTPACKAGE_HPP
#define C_OSCTARGETSUPPORTPACKAGE_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QString>
#include <QDomDocument>

/* -- Namespace
 * -----------------------------------------------------------------------------------------------------
 */
namespace stw {
namespace opensyde_core {
/* -- Global Constants
 * -----------------------------------------------------------
 * -----------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

class C_OscTargetSupportPackage {
public:
  C_OscTargetSupportPackage();

  // --------------------------------------------------------------------------
  // Binary Serialization (QDataStream)
  // --------------------------------------------------------------------------
  int32_t ToQDataStream(QDataStream& orc_Stream) const;
  int32_t FromQDataStream(QDataStream& orc_Stream);

  // --------------------------------------------------------------------------
  // JSON Serialization (QJsonObject)
  // --------------------------------------------------------------------------
  QJsonObject ToJsonObject() const;
  int32_t FromJsonObject(const QJsonObject& orc_Object);

  // --------------------------------------------------------------------------
  // XML Serialization (QDomDocument)
  // --------------------------------------------------------------------------
  QDomElement ToQDomDocument(QDomDocument& orc_Doc, 
                             const QString& orc_RootElementName = "target-support-package") const;
  int32_t FromQDomElement(const QDomElement& orc_Element);

  QString c_DeviceName; ///< name of device this package it intended for
  QString c_Comment;    ///< comment about this package
  QString c_TemplatePath; ///< path to template package archive

  void Clear(void);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
