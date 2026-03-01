//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Content including enum
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFCONTENT_HPP
#define C_OSCHALCDEFCONTENT_HPP

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include <QList>
#include <QString>
#include <QStringList>
#include <vector>

#include "C_OscHalcDefContentBitmaskItem.hpp"
#include "C_OscNodeDataPoolContent.hpp"

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

class C_OscHalcDefContent : public C_OscNodeDataPoolContent {
public:
  enum E_ComplexType { eCT_PLAIN, eCT_ENUM, eCT_BIT_MASK, eCT_STRING };

  C_OscHalcDefContent();

  void SetComplexType(const E_ComplexType oe_Type);
  E_ComplexType GetComplexType(void) const;
  int32_t AddEnumItem(const QString &orc_DisplayName,
                      const C_OscNodeDataPoolContent &orc_Value);
  int32_t SetEnumValue(const QString &orc_DisplayName);
  int32_t GetEnumValue(QString &orc_DisplayName);
  const C_OscNodeDataPoolContent *
  FindEnumItem(const QString &orc_DisplayName) const;
  const QList<QPair<QString, C_OscNodeDataPoolContent>> &
  GetEnumItems(void) const;

  void AddBitmaskItem(const C_OscHalcDefContentBitmaskItem &orc_Value);
  const QList<C_OscHalcDefContentBitmaskItem> &GetBitmaskItems(void) const;
  void GetBitmaskStatusValues(QStringList *const opc_Displays,
                              QList<bool> *const opc_Values) const;
  int32_t GetBitmask(const QString &orc_DisplayName, bool &orq_Value) const;
  int32_t SetBitmask(const QString &orc_DisplayName, const bool oq_Value);

  int32_t SetStringValue(const QString &orc_Value);
  int32_t GetStringValue(QString &orc_Value) const;

  virtual void CalcHash(uint32_t &oru32_HashValue) const;
  virtual void CalcHashElement(uint32_t &oru32_HashValue,
                               const uint32_t ou32_Index) const;
  virtual void CalcHashStructure(uint32_t &oru32_HashValue) const;

private:
  E_ComplexType me_ComplexType;
  QList<QPair<QString, C_OscNodeDataPoolContent>>
      mc_EnumItems; ///< All known enum values
  QList<C_OscHalcDefContentBitmaskItem>
      mc_BitmaskItems; ///< All known bitmask values
};

/* -- Extern Global Variables
 * ---------------------------------------------------------------------------------------
 */
} // namespace opensyde_core
} // namespace stw

#endif
