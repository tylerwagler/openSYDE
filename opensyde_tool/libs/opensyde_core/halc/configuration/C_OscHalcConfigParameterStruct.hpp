//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Group domain parameter struct config data for HALC configuration
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCCONFIGPARAMETERSTRUCT_HPP
#define C_OSCHALCCONFIGPARAMETERSTRUCT_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscHalcConfigParameter.hpp"
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

class C_OscHalcConfigParameterStruct : public C_OscHalcConfigParameter {
public:
  C_OscHalcConfigParameterStruct();

  virtual void CalcHash(uint32_t &oru32_HashValue) const;

  QList<C_OscHalcConfigParameter>
      c_ParameterElements; ///< Parameter elements (synced with
                           ///< C_OscHalcDefBase)

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
