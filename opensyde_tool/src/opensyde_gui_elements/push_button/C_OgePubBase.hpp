//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated push button with objectName-based styling (header)
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGEPUBBASE_HPP
#define C_OGEPUBBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QPushButton>
#include "C_OgePubToolTipBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgePubBase :
   public C_OgePubToolTipBase
{
   Q_OBJECT
   Q_PROPERTY(QString buttonVariant READ GetButtonVariant WRITE SetButtonVariant)

public:
   explicit C_OgePubBase(QWidget * const opc_Parent = NULL);
   ~C_OgePubBase(void) override;

   void SetButtonVariant(const QString & orc_Variant);
   QString GetButtonVariant(void) const;

private:
   QString mc_ButtonVariant;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
