//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class, containing the interpreted entries for one list in a
   parameter set file (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPARAMSETINTERPRETEDLIST_HPP
#define C_OSCPARAMSETINTERPRETEDLIST_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscParamSetInterpretedElement.hpp"
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

/// Data class, containing the interpreted entries for one list in a parameter
/// set file
class C_OscParamSetInterpretedList {
public:
  C_OscParamSetInterpretedList(void);

  QString c_Name; ///< List name
  QList<C_OscParamSetInterpretedElement>
      c_Elements; ///< Interpreted elements of this list

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
