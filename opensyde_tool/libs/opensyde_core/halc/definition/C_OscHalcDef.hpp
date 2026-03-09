//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       HALC definition
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEF_HPP
#define C_OSCHALCDEF_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscHalcDefBase.hpp"
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

class C_OscHalcDef : public C_OscHalcDefBase {
public:
  C_OscHalcDef(void);

  virtual uint32_t GetDomainSize(void) const;
  virtual const C_OscHalcDefDomain *
  GetDomainDefDataConst(const uint32_t ou32_Index) const;
   virtual void AddDomain(const C_OscHalcDefDomain &orc_Domain);
   virtual void HandleFileLoadPostProcessing(void);

   // --------------------------------------------------------------------------
   // Qt Native Serialization
   // --------------------------------------------------------------------------
   int32_t ToQDataStream(QDataStream &ro_DataStream) const;
   int32_t FromQDataStream(QDataStream &ro_DataStream);

   QJsonObject ToJsonObject() const;
   int32_t FromJsonObject(const QJsonObject &orc_Object);

   QDomElement ToQDomDocument(QDomDocument &orc_Doc,
                              const QString &orc_ElementName) const;
   int32_t FromQDomDocument(const QDomElement &orc_Element);

private:
  QList<C_OscHalcDefDomain> mc_Domains; ///< Domains of HALC definition
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
