//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Group channel values
   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFCHANNELVALUES_HPP
#define C_OSCHALCDEFCHANNELVALUES_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscHalcDefStruct.hpp"
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

class C_OscHalcDefChannelValues {
public:
  C_OscHalcDefChannelValues(void);
  virtual ~C_OscHalcDefChannelValues();

  virtual void CalcHash(uint32_t &oru32_HashValue) const;

  QList<C_OscHalcDefStruct>
      c_Parameters; ///< All available parameters for this domain
  QList<C_OscHalcDefStruct>
      c_InputValues; ///< All available inputs for this domain
  QList<C_OscHalcDefStruct>
      c_OutputValues; ///< All available outputs for this domain
   QList<C_OscHalcDefStruct>
       c_StatusValues; ///< All available status values for this domain

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
