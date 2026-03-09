//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Standalone HALC domain config
   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCCONFIGSTANDALONEDOMAIN_HPP
#define C_OSCHALCCONFIGSTANDALONEDOMAIN_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscHalcConfigDomain.hpp"
#include "C_OscHalcConfigStandaloneChannel.hpp"
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

class C_OscHalcConfigStandaloneDomain : public C_OscHalcConfigDomain {
public:
  C_OscHalcConfigStandaloneDomain(void);
  C_OscHalcConfigStandaloneDomain(
      const C_OscHalcConfigDomain &orc_ConfigDomain,
      const QList<C_OscHalcConfigStandaloneChannel> &orc_StandaloneChannels);

  QList<C_OscHalcConfigStandaloneChannel> c_StandaloneChannels;

  // --------------------------------------------------------------------------
  // Qt Native Serialization
  // --------------------------------------------------------------------------
  void ToQDataStream(QDataStream &ro_DataStream) const;
  void FromQDataStream(QDataStream &ro_DataStream);

  QJsonObject ToJsonObject() const;
  void FromJsonObject(const QJsonObject &orc_Object);

  QDomElement ToQDomElement(QDomDocument &orc_Doc,
                             const QString &orc_ElementName) const;
  void FromQDomElement(const QDomElement &orc_Element);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
