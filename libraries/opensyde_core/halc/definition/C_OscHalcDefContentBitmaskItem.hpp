//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Bitmask item
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCHALCDEFCONTENTBITMASKITEM_HPP
#define C_OSCHALCDEFCONTENTBITMASKITEM_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include <string>
#include <system_error>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscHalcDefContentBitmaskItem
{
public:
   C_OscHalcDefContentBitmaskItem(void);
   virtual ~C_OscHalcDefContentBitmaskItem();

   std::error_code SetValueByString(const std::string & orc_Item);

   virtual void CalcHash(uint32_t & oru32_HashValue) const;
   virtual void CalcHashStructure(uint32_t & oru32_HashValue) const;

   std::string c_Display; ///< Name to display to user
   std::string c_Comment; ///< Description to display to user
   bool q_ApplyValueSetting;        ///< Current setting to apply this bitmask value
   uint64_t u64_Value;              ///< Bitmask value

private:
   static std::error_code mh_ParseUintFromString(const std::string & orc_Item, uint64_t & oru64_Value);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
