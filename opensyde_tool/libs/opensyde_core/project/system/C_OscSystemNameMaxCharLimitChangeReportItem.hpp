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

#include <QString>

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
   C_OscSystemNameMaxCharLimitChangeReportItem(const QString oc_Type,
                                               const QString oc_OriginalName,
                                               const QString oc_NewName);

   QString c_Type;
   QString c_OriginalName;
   QString c_NewName;

   static void h_HandleNameMaxCharLimitItem(const uint32_t ou32_NameMaxCharLimit,
                                            const QString & orc_Type, const QString & orc_Name,
                                            std::list<C_OscSystemNameMaxCharLimitChangeReportItem> * const opc_ChangedItems);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
