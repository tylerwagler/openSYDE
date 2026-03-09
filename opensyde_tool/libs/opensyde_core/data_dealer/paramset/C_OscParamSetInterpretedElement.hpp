//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class, containing the interpreted entries for one element
   in a parameter set file (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPARAMSETINTERPRETEDELEMENT_HPP
#define C_OSCPARAMSETINTERPRETEDELEMENT_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscNodeDataPoolContent.hpp"
#include <QDataStream>
#include <QDomElement>
#include <QJsonObject>
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

/// Data class, containing the interpreted entries for one element in a
/// parameter set file
class C_OscParamSetInterpretedElement {
public:
  C_OscParamSetInterpretedElement(void);

  QString c_Name;                      ///< Element name
  C_OscNodeDataPoolContent c_NvmValue; ///< Element parameter value

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
