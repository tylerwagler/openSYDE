//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Store HALC channel information
   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFCHANNEL_HPP
#define C_OSCHALCDEFCHANNEL_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
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

class C_OscHalcDefChannelDef {
public:
  C_OscHalcDefChannelDef(void);

  QString c_Name; ///< Name of this channel

   void CalcHash(uint32_t &oru32_HashValue) const;

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
