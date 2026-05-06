//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       UI boundary data: stores UI information (header)

   See cpp file for detailed description

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_PUIBSBOUNDARY_HPP
#define C_PUIBSBOUNDARY_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QColor>

#include "stwtypes.hpp"
#include "C_PuiBsBox.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_PuiBsBoundary :
   public C_PuiBsBox
{
public:
   C_PuiBsBoundary();

   void CalcHash(uint32_t & oru32_HashValue) const override;

   QColor c_UiBorderColor;     ///< color of border line
   QColor c_UiBackgroundColor; ///< color of background
   int32_t s32_UiBorderWidth;  ///< width of border line
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
