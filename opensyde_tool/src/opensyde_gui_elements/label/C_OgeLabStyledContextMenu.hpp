//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only label with context menu support

   Replaces individual label subclasses of C_OgeLabContextMenuBase that
   existed solely for QSS targeting. Uses Q_PROPERTY "variant" for style selection.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGELABSTYLEDCONTEXTMENU_HPP
#define C_OGELABSTYLEDCONTEXTMENU_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include "C_OgeLabContextMenuBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeLabStyledContextMenu :
   public C_OgeLabContextMenuBase
{
   Q_OBJECT
   Q_PROPERTY(QString variant READ GetVariant WRITE SetVariant)

public:
   explicit C_OgeLabStyledContextMenu(QWidget * const opc_Parent = NULL);

   QString GetVariant(void) const;
   void SetVariant(const QString & orc_Variant);

private:
   QString mc_Variant;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
