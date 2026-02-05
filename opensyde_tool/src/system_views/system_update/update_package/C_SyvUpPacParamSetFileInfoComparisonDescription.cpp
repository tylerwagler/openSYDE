//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Parameter set file comparison description storage
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"


#include "C_SyvUpPacParamSetFileInfoComparisonDescription.hpp"
using namespace stw::opensyde_gui_logic;

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

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
C_SyvUpPacParamSetFileInfoComparisonDescription::C_SyvUpPacParamSetFileInfoComparisonDescription(void) :
   u32_LayerNum(0UL),
   e_ResultType(eRT_NOT_FOUND)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get display text for result

   \return
   Display text for result
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_SyvUpPacParamSetFileInfoComparisonDescription::GetResultText(void) const
{
   QString c_Retval;

   switch (this->e_ResultType)
   {
   case eRT_MATCH:
      c_Retval = "match";
      break;
   case eRT_NO_MATCH:
      c_Retval = "don't match";
      break;
   case eRT_NOT_FOUND:
      c_Retval = "-";
      break;
   default:
      Q_ASSERT(false);
      break;
   }

   return c_Retval;
}
