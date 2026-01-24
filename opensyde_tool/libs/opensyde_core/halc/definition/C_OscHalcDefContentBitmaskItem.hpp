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
#include <QString>
#include "stwtypes.hpp"

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

   int32_t SetValueByString(const QString & orc_Item);

   virtual void CalcHash(uint32_t & oru32_HashValue) const;
   virtual void CalcHashStructure(uint32_t & oru32_HashValue) const;

   QString c_Display; ///< Name to display to user
   QString c_Comment; ///< Description to display to user
   bool q_ApplyValueSetting;        ///< Current setting to apply this bitmask value
   uint64_t u64_Value;              ///< Bitmask value

private:
   static int32_t mh_ParseUintFromString(const QString & orc_Item, uint64_t & oru64_Value);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
