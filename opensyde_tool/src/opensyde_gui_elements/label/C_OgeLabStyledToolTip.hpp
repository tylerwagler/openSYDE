//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only label with tooltip support

   Replaces individual label subclasses of C_OgeLabToolTipBase that existed
   solely for QSS targeting. Uses Q_PROPERTY "variant" for style selection.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGELABSTYLEDTOOLTIP_HPP
#define C_OGELABSTYLEDTOOLTIP_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include "C_OgeLabToolTipBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeLabStyledToolTip :
   public C_OgeLabToolTipBase
{
   Q_OBJECT
   Q_PROPERTY(QString variant READ GetVariant WRITE SetVariant)

public:
   explicit C_OgeLabStyledToolTip(QWidget * const opc_Parent = NULL);
   C_OgeLabStyledToolTip(const QString & orc_Variant, QWidget * const opc_Parent);

   QString GetVariant(void) const;
   void SetVariant(const QString & orc_Variant);

private:
   QString mc_Variant;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
