//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Shared base for the dashboard widget-property panels (header)

   See cpp file for detailed description.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYVDAPEPANELBASE_HPP
#define C_SYVDAPEPANELBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include <QWidget>
#include <QGraphicsItem>
#include "C_SyvDaPeBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SyvDaPePanelBase :
   public QWidget
{
   Q_OBJECT

public:
   explicit C_SyvDaPePanelBase(C_SyvDaPeBase & orc_Parent);

protected:
   //lint -e{1725} Reference member intentionally retained as the parent dialog outlives the panel.
   C_SyvDaPeBase & mrc_ParentDialog;

   /// Helper used by every subclass at the end of its m_UpdatePreview: replace
   /// whatever is currently in the parent dialog's preview scene with opc_Item.
   /// Subclass owns item construction, sizing, theming and per-type config — the
   /// base just publishes it.
   void m_PublishPreviewItem(QGraphicsItem * const opc_Item) const;

private:
   //Avoid call
   C_SyvDaPePanelBase(const C_SyvDaPePanelBase &);
   C_SyvDaPePanelBase & operator =(const C_SyvDaPePanelBase &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
