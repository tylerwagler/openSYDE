//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Shared base for the CAN Monitor / SYDEflash title bars (header)

   See cpp file for detailed description.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGETITLEBARWIDGET_HPP
#define C_OGETITLEBARWIDGET_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include <QString>
#include <QLabel>
#include "C_OgeWiOnlyBackground.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeTitleBarWidget :
   public C_OgeWiOnlyBackground
{
   Q_OBJECT

public:
   explicit C_OgeTitleBarWidget(QWidget * const opc_Parent = nullptr);

protected:
   /// Subclasses identify themselves so the shared About slot can populate the dialog.
   virtual QString m_GetAppName(void) const = 0;
   virtual QString m_GetLogoPath(void) const = 0;
   virtual QString m_GetAboutExtraCredits(void) const;

   /// Load and scale the STW logo into the supplied label. Subclasses call this
   /// once from their ctor with their .ui's pc_LogoLabel.
   void m_LoadStwLogo(QLabel * const opc_Label) const;

   //lint -save -e1736
protected Q_SLOTS:
   //lint -restore
   void m_TriggerHelp(void);
   void m_ShowAbout(void);

private:
   //Avoid call
   C_OgeTitleBarWidget(const C_OgeTitleBarWidget &);
   C_OgeTitleBarWidget & operator =(const C_OgeTitleBarWidget &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
