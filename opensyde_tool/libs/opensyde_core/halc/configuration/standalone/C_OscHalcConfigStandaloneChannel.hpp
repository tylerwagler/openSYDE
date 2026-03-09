//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Standalone HALC channel config with parameter Ids
   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCCONFIGSTANDALONECHANNEL_HPP
#define C_OSCHALCCONFIGSTANDALONECHANNEL_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QDataStream>
#include <QDomElement>
#include <QJsonObject>
#include <QString>
#include <QStringList>

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

class C_OscHalcConfigStandaloneChannel {
public:
  C_OscHalcConfigStandaloneChannel(void);

  QStringList c_ParameterIds;

  // --------------------------------------------------------------------------
  // Qt Native Serialization
  // --------------------------------------------------------------------------
  void ToQDataStream(QDataStream &ro_DataStream) const;
  void FromQDataStream(QDataStream &ro_DataStream);

  QJsonObject ToJsonObject() const;
  void FromJsonObject(const QJsonObject &orc_Object);

  int32_t ToQDomElement(QDomDocument &orc_Doc,
                             const QString &orc_ElementName) const;
  int32_t FromQDomElement(const QDomElement &orc_Element);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
