//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       HALC definition generic element part
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFELEMENT_HPP
#define C_OSCHALCDEFELEMENT_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "C_OscHalcDefContent.hpp"
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

class C_OscHalcDefElement {
public:
  C_OscHalcDefElement(void);
  virtual ~C_OscHalcDefElement(void);

  void SetType(const C_OscNodeDataPoolContent::E_Type oe_Value);
  void SetArray(const bool oq_Value);
  void SetComplexType(const C_OscHalcDefContent::E_ComplexType oe_Type);
  int32_t AddEnumItem(const QString &orc_DisplayName,
                      const C_OscNodeDataPoolContent &orc_Value);
  void AddBitmaskItem(const C_OscHalcDefContentBitmaskItem &orc_Value);

  C_OscHalcDefContent::E_ComplexType GetComplexType(void) const;
  C_OscNodeDataPoolContent::E_Type GetType(void) const;
  bool GetArray(void) const;
  const QList<QPair<QString, C_OscNodeDataPoolContent>> &
  GetEnumItems(void) const;
  const QList<C_OscHalcDefContentBitmaskItem> &GetBitmaskItems(void) const;

  virtual void CalcHash(uint32_t &oru32_HashValue) const;

  QString c_Id;      ///< Unique ID for later reference of this element
  QString c_Display; ///< Name to display to user
  QString c_Comment; ///< Description to display to user
  C_OscHalcDefContent c_InitialValue;      ///< Initial value of this element
  C_OscHalcDefContent c_MinValue;          ///< Minimum value of this element
  C_OscHalcDefContent c_MaxValue;          ///< Maximum value of this element
   QList<uint32_t> c_UseCaseAvailabilities; ///< Availability for this element
                                            ///< (use-case index)

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
