//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       HALC definition generic element part
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFELEMENT_HPP
#define C_OSCHALCDEFELEMENT_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>
#include <system_error>
#include "C_OscHalcDefContent.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefElement
{
public:
   C_OscHalcDefElement(void);
   virtual ~C_OscHalcDefElement(void);

   void SetType(const C_OscNodeDataPoolContent::E_Type oe_Value);
   void SetArray(const bool oq_Value);
   void SetComplexType(const C_OscHalcDefContent::E_ComplexType oe_Type);
   std::error_code AddEnumItem(const std::string & orc_DisplayName, const C_OscNodeDataPoolContent & orc_Value);
   void AddBitmaskItem(const C_OscHalcDefContentBitmaskItem & orc_Value);

   C_OscHalcDefContent::E_ComplexType GetComplexType(void) const;
   C_OscNodeDataPoolContent::E_Type GetType(void) const;
   bool GetArray(void) const;
   const std::vector<std::pair<std::string, C_OscNodeDataPoolContent> > & GetEnumItems(void) const;
   const std::vector<C_OscHalcDefContentBitmaskItem> & GetBitmaskItems(void) const;

   virtual void CalcHash(uint32_t & oru32_HashValue) const;

   std::string c_Id;                    ///< Unique ID for later reference of this element
   std::string c_Display;               ///< Name to display to user
   std::string c_Comment;               ///< Description to display to user
   C_OscHalcDefContent c_InitialValue;            ///< Initial value of this element
   C_OscHalcDefContent c_MinValue;                ///< Minimum value of this element
   C_OscHalcDefContent c_MaxValue;                ///< Maximum value of this element
   std::vector<uint32_t> c_UseCaseAvailabilities; ///< Availability for this element (use-case index)
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
