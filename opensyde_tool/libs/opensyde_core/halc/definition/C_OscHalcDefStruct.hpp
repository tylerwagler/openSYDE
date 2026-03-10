//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       HALC definition generic struct part
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFSTRUCT_HPP
#define C_OSCHALCDEFSTRUCT_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscHalcDefElement.hpp"
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

class C_OscHalcDefStruct : public C_OscHalcDefElement {
public:
  C_OscHalcDefStruct(void);
  virtual ~C_OscHalcDefStruct(void);

  virtual void CalcHash(uint32_t &oru32_HashValue) const;

  QList<C_OscHalcDefElement>
      c_StructElements; ///< Optional data elements of this struct
                        // If these are zero: assume plain type
   // If these are non-zero: assume struct type (don't use initial, min and max
   // value in this case)

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
