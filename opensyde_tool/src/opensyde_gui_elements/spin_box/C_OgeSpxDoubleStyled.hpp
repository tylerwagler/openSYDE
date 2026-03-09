//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only double spin box with tooltip support

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGESPXDOUBLESTYLED_HPP
#define C_OGESPXDOUBLESTYLED_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include "C_OgeSpxDoubleToolTipBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeSpxDoubleStyled :
   public C_OgeSpxDoubleToolTipBase
{
   Q_OBJECT
   Q_PROPERTY(QString variant READ GetVariant WRITE SetVariant)

public:
   explicit C_OgeSpxDoubleStyled(QWidget * const opc_Parent = NULL);
   C_OgeSpxDoubleStyled(const QString & orc_Variant, QWidget * const opc_Parent);

   QString GetVariant(void) const;
   void SetVariant(const QString & orc_Variant);

private:
   QString mc_Variant;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
