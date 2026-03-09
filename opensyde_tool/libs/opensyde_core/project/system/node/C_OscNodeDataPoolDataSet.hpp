//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for data set related, additional information (header)

   See cpp file for detailed description

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCNODEDATAPOOLDATASET_HPP
#define C_OSCNODEDATAPOOLDATASET_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */

#include "stwtypes.hpp"
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

class C_OscNodeDataPoolDataSet {
public:
  C_OscNodeDataPoolDataSet(void);

  void CalcHash(uint32_t &oru32_HashValue) const;

  QString c_Name;    ///< User data set name
  QString c_Comment; ///< User data set comment

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
