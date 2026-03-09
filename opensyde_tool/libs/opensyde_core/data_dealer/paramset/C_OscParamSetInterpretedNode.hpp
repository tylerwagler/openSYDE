//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class, containing all interpreted entries for one node in a
   parameter set file (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPARAMSETINTERPRETEDNODE_HPP
#define C_OSCPARAMSETINTERPRETEDNODE_HPP

#include "C_OscParamSetInterpretedDataPool.hpp"
#include <QDataStream>
#include <QDomElement>
#include <QJsonObject>
#include <QList>
#include <QString>

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

/// Data class, containing all interpreted entries for one node in a parameter
/// set file
class C_OscParamSetInterpretedNode {
public:
  C_OscParamSetInterpretedNode(void);

  QString c_Name; ///< Node name
  QList<C_OscParamSetInterpretedDataPool>
      c_DataPools; ///< Interpreted data pools, used by this parameter set

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
