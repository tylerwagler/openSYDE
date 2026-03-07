//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only line edit with context menu

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGELESTYLED_HPP
#define C_OGELESTYLED_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include "C_OgeLeContextMenuBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeLeStyled :
   public C_OgeLeContextMenuBase
{
   Q_OBJECT
   Q_PROPERTY(QString variant READ GetVariant WRITE SetVariant)

public:
   explicit C_OgeLeStyled(QWidget * const opc_Parent = NULL);
   C_OgeLeStyled(const QString & orc_Variant, QWidget * const opc_Parent);

   QString GetVariant(void) const;
   void SetVariant(const QString & orc_Variant);

private:
   QString mc_Variant;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
