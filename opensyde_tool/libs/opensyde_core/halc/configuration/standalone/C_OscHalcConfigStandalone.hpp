//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Complete standalone HALC config
   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCCONFIGSTANDALONE_HPP
#define C_OSCHALCCONFIGSTANDALONE_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscHalcConfigStandaloneDomain.hpp"
#include <QDataStream>
#include <QDomElement>
#include <QJsonObject>
#include <QList>

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

class C_OscHalcConfigStandalone {
public:
  C_OscHalcConfigStandalone(void);

  QString c_DeviceType;
  uint32_t u32_DefinitionContentVersion;
  QList<C_OscHalcConfigStandaloneDomain> c_Domains;

  // --------------------------------------------------------------------------
  // Qt Native Serialization
  // --------------------------------------------------------------------------
  void ToQDataStream(QDataStream &ro_DataStream) const;
  void FromQDataStream(QDataStream &ro_DataStream);

  QJsonObject ToJsonObject() const;
  void FromJsonObject(const QJsonObject &orc_Object);

  QDomElement ToQDomDocument(QDomDocument &orc_Doc,
                             const QString &orc_ElementName) const;
  void FromQDomDocument(const QDomElement &orc_Element);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
