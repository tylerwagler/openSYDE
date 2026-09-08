//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Content including enum
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFCONTENT_HPP
#define C_OSCHALCDEFCONTENT_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>

#include <string>
#include <system_error>
#include "C_OscNodeDataPoolContent.hpp"
#include "C_OscHalcDefContentBitmaskItem.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefContent :
   public C_OscNodeDataPoolContent
{
public:
   enum E_ComplexType
   {
      eCT_PLAIN,
      eCT_ENUM,
      eCT_BIT_MASK,
      eCT_STRING
   };

   C_OscHalcDefContent();

   void SetComplexType(const E_ComplexType oe_Type);
   E_ComplexType GetComplexType(void) const;
   std::error_code AddEnumItem(const std::string & orc_DisplayName, const C_OscNodeDataPoolContent & orc_Value);
   std::error_code SetEnumValue(const std::string & orc_DisplayName);
   std::error_code GetEnumValue(std::string & orc_DisplayName);
   const C_OscNodeDataPoolContent * FindEnumItem(const std::string & orc_DisplayName) const;
   const std::vector<std::pair<std::string, C_OscNodeDataPoolContent> > & GetEnumItems(void) const;

   void AddBitmaskItem(const C_OscHalcDefContentBitmaskItem & orc_Value);
   const std::vector<C_OscHalcDefContentBitmaskItem> & GetBitmaskItems(void) const;
   void GetBitmaskStatusValues(std::vector<std::string> * const opc_Displays,
                               std::vector<bool> * const opc_Values) const;
   std::error_code GetBitmask(const std::string & orc_DisplayName, bool & orq_Value) const;
   std::error_code SetBitmask(const std::string & orc_DisplayName, const bool oq_Value);

   std::error_code SetStringValue(const std::string & orc_Value);
   std::error_code GetStringValue(std::string & orc_Value) const;

   virtual void CalcHash(uint32_t & oru32_HashValue) const;
   virtual void CalcHashElement(uint32_t & oru32_HashValue, const uint32_t ou32_Index) const;
   virtual void CalcHashStructure(uint32_t & oru32_HashValue) const;

private:
   E_ComplexType me_ComplexType;
   std::vector<std::pair<std::string, C_OscNodeDataPoolContent> > mc_EnumItems; ///< All known enum values
   std::vector<C_OscHalcDefContentBitmaskItem> mc_BitmaskItems;                           ///< All known bitmask values
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
