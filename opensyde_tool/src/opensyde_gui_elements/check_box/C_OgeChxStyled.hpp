//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only check box with tooltip

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGECHXSTYLED_HPP
#define C_OGECHXSTYLED_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include "C_OgeChxToolTipBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeChxStyled :
   public C_OgeChxToolTipBase
{
   Q_OBJECT
   Q_PROPERTY(QString variant READ GetVariant WRITE SetVariant)

public:
   explicit C_OgeChxStyled(QWidget * const opc_Parent = NULL);

   QString GetVariant(void) const;
   void SetVariant(const QString & orc_Variant);

private:
   QString mc_Variant;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
