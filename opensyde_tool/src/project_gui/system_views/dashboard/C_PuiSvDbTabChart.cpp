//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       UI class for dashboard tab chart data (implementation)

   UI class for dashboard tab chart data

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwerrors.hpp"
#include "C_OscHashUtil.hpp"
#include "C_PuiSvDbTabChart.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::errors;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_PuiSvDbTabChart::C_PuiSvDbTabChart() :
   C_PuiSvDbWidgetBase(),
   s32_SplitterLeftWidth(300),
   e_SettingZoomMode(C_PuiSvDbTabChart::eSETTING_ZM_XY),
   e_SettingVerticalAxisMode(C_PuiSvDbTabChart::eSETTING_YA_ONE_VISIBLE),
   q_IsZoomModeActive(false),
   q_IsPaused(false),
   q_AreSamplePointsShown(true)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with init [in] value and result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_PuiSvDbTabChart::CalcHash(uint32_t & oru32_HashValue) const
{
   stw::opensyde_core::hash_util::CalcHashMembers(oru32_HashValue,
                                                  this->c_DataPoolElementsActive,
                                                  this->c_DataPoolElementsColorIndex);
   // c_VisibleScreen: intentional float64_t -> float32_t truncation preserved from original implementation
   for (const auto & orc_Inner : this->c_VisibleScreen)
   {
      for (const auto & of64_Val : orc_Inner)
      {
         //lint -e{736,9120} C++ interface
         const float32_t f32_Val = of64_Val;
         //lint -e{9110} Usual way
         stw::opensyde_core::hash_util::CalcHashMembers(oru32_HashValue, f32_Val);
      }
   }
   stw::opensyde_core::hash_util::CalcHashMembers(oru32_HashValue,
                                                  this->s32_SplitterLeftWidth,
                                                  this->e_SettingZoomMode,
                                                  this->q_IsZoomModeActive,
                                                  this->e_SettingVerticalAxisMode,
                                                  this->q_IsPaused,
                                                  this->q_AreSamplePointsShown);

   C_PuiSvDbWidgetBase::CalcHash(oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Check if item is a read element

   \return
   True  Read element
   False Write element
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_PuiSvDbTabChart::IsReadElement(void) const
{
   return true;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Remove element from widget

   \param[in]  oru32_Index    Internal index

   \retval   C_NO_ERR   Index found
   \retval   C_RANGE    Index not found
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_PuiSvDbTabChart::RemoveElement(const uint32_t & oru32_Index)
{
   int32_t s32_Retval = C_PuiSvDbWidgetBase::RemoveElement(oru32_Index);

   if (s32_Retval == C_NO_ERR)
   {
      if (oru32_Index < this->c_DataPoolElementsActive.size())
      {
         this->c_DataPoolElementsActive.erase(this->c_DataPoolElementsActive.begin() + oru32_Index);
      }
      else
      {
         s32_Retval = C_RANGE;
      }

      if (oru32_Index < this->c_DataPoolElementsColorIndex.size())
      {
         this->c_DataPoolElementsColorIndex.remove(oru32_Index, 1);
      }
      else
      {
         s32_Retval = C_RANGE;
      }
   }
   return s32_Retval;
}
