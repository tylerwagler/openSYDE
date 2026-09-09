//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Simple data class for max char limit change report information

   Simple data class for max char limit change report information

   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OscSystemNameMaxCharLimitChangeReportItem.hpp"
#include "C_SclStringUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::scl;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscSystemNameMaxCharLimitChangeReportItem::C_OscSystemNameMaxCharLimitChangeReportItem()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Init constructor

   \param[in]  oc_Type           Type
   \param[in]  oc_OriginalName   Original name
   \param[in]  oc_NewName        New name
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscSystemNameMaxCharLimitChangeReportItem::C_OscSystemNameMaxCharLimitChangeReportItem(
   const std::string oc_Type, const std::string oc_OriginalName,
   const std::string oc_NewName) :
   c_Type(oc_Type),
   c_OriginalName(oc_OriginalName),
   c_NewName(oc_NewName)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle name max char limit item

   \param[in]      ou32_NameMaxCharLimit  Name max char limit
   \param[in,out]  orc_Type               Type
   \param[in,out]  orc_Name               Name
   \param[in,out]  opc_ChangedItems       Changed items
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscSystemNameMaxCharLimitChangeReportItem::h_HandleNameMaxCharLimitItem(const uint32_t ou32_NameMaxCharLimit,
                                                                               const std::string & orc_Type,
                                                                               std::string & orc_Name,
                                                                               std::list<C_OscSystemNameMaxCharLimitChangeReportItem> * const opc_ChangedItems)
{
   if (orc_Name.length() > ou32_NameMaxCharLimit)
   {
      const std::string c_NewNameTmp = SubStringCompat(orc_Name, 1UL, ou32_NameMaxCharLimit);
      if (opc_ChangedItems != nullptr)
      {
         const C_OscSystemNameMaxCharLimitChangeReportItem c_Entry(orc_Type, orc_Name, c_NewNameTmp);
         opc_ChangedItems->push_back(c_Entry);
      }
      else
      {
         orc_Name = c_NewNameTmp;
      }
   }
}
