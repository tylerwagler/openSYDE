//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       HALC definition domain channel use-case availability
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFCHANNELAVAILABILITY_HPP
#define C_OSCHALCDEFCHANNELAVAILABILITY_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "stwtypes.hpp"
#include <QDataStream>
#include <QDomElement>
#include <QJsonObject>
#include <QList>
#include <vector>

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

class C_OscHalcDefChannelAvailability {
public:
  C_OscHalcDefChannelAvailability(void);
  virtual ~C_OscHalcDefChannelAvailability();

  virtual void CalcHash(uint32_t &oru32_HashValue) const;

  uint32_t u32_ValueIndex; ///< Channel index corresponding to this availability
   QList<uint32_t> c_DependentValues; ///< Optional other channel indices which
                                      ///< are also linked to this
                                      // setting

   // --------------------------------------------------------------------------
   // Qt Native Serialization
   // --------------------------------------------------------------------------
   void ToQDataStream(QDataStream &ro_DataStream) const;
   void FromQDataStream(QDataStream &ro_DataStream);

   QJsonObject ToJsonObject() const;
   int32_t FromJsonObject(const QJsonObject &orc_Object);

   QDomElement ToQDomElement(QDomDocument &orc_Doc,
                              const QString &orc_ElementName) const;
   int32_t FromQDomElement(const QDomElement &orc_Element);
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
