//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Simple data class for max char limit change report information
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCSYSTEMNAMEMAXCHARLIMITCHANGEREPORTITEM_HPP
#define C_OSCSYSTEMNAMEMAXCHARLIMITCHANGEREPORTITEM_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <list>

#include <string>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscSystemNameMaxCharLimitChangeReportItem
{
public:
   C_OscSystemNameMaxCharLimitChangeReportItem();
   C_OscSystemNameMaxCharLimitChangeReportItem(const std::string oc_Type,
                                               const std::string oc_OriginalName,
                                               const std::string oc_NewName);

   std::string c_Type;
   std::string c_OriginalName;
   std::string c_NewName;

   static void h_HandleNameMaxCharLimitItem(const uint32_t ou32_NameMaxCharLimit,
                                            const std::string & orc_Type, std::string & orc_Name,
                                            std::list<C_OscSystemNameMaxCharLimitChangeReportItem> * const opc_ChangedItems);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
