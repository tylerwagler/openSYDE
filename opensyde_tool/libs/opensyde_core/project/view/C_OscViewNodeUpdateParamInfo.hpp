//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node update information for parameter sets (header)

   See cpp file for detailed description

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCVIEWNODEUPDATEPARAMINFO_HPP
#define C_OSCVIEWNODEUPDATEPARAMINFO_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "stwtypes.hpp"
#include <QString>
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

class C_OscViewNodeUpdateParamInfo {
public:
  C_OscViewNodeUpdateParamInfo(void);

  void CalcHash(uint32_t &oru32_HashValue) const;

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
                             const QString& orc_RootElementName = "param-info") const;
  int32_t FromQDomElement(const QDomElement& orc_Element);

  // Set
  void SetContent(const QString &orc_FilePath,
                  const uint32_t ou32_LastKnownCrc);

  // Get
  const QString &GetPath(void) const;
  uint32_t GetLastKnownCrc(void) const;

private:
  QString mc_FilePath;
  uint32_t mu32_LastKnownCrc;
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
