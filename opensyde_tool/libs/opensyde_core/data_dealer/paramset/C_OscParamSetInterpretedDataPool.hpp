//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class, containing the interpreted entries for one datapool
   in a parameter set file (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPARAMSETINTERPRETEDDATAPOOL_HPP
#define C_OSCPARAMSETINTERPRETEDDATAPOOL_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscParamSetDataPoolInfo.hpp"
#include "C_OscParamSetInterpretedList.hpp"
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

/// container class for handling interpreted parameter set file content
class C_OscParamSetInterpretedDataPool {
public:
  C_OscParamSetInterpretedDataPool(void);

  C_OscParamSetDataPoolInfo c_DataPoolInfo;
  QList<C_OscParamSetInterpretedList>
      c_Lists; ///< Interpreted lists of this datapool

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
