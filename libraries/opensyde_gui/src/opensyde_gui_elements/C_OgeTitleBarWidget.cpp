//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Shared base for the CAN Monitor / SYDEflash title bars (implementation)

   The two app title bars host completely different action sets (project save/load vs node
   flash/search/configure), but they share the About-dialog launcher, the help-handler
   trigger, and the STW logo loading. This base class consolidates that shared piece so each
   subclass only has to declare its app metadata via the m_Get* hooks.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QPixmap>
#include <QPointer>

#include "C_OgeTitleBarWidget.hpp"
#include "C_OgePopUpDialog.hpp"
#include "C_NagAboutDialog.hpp"
#include "C_HeHandler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;
using namespace stw::opensyde_gui_elements;
using namespace stw::opensyde_gui_logic;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   \param[in,out]  opc_Parent    Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeTitleBarWidget::C_OgeTitleBarWidget(QWidget * const opc_Parent) :
   C_OgeWiOnlyBackground(opc_Parent)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default about-dialog credits string

   Subclasses can override to add app-specific third-party attributions. The default is empty,
   which matches what C_NagAboutDialog expects for "no extra credits beyond the standard list".
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeTitleBarWidget::m_GetAboutExtraCredits(void) const
{
   return QString();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load the STW logo, scale it down, and set it as the pixmap on the supplied label

   The 1/18 scaling matches what both apps were doing inline before consolidation.

   \param[in,out]  opc_Label    Label that displays the logo
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeTitleBarWidget::m_LoadStwLogo(QLabel * const opc_Label) const
{
   if (opc_Label != NULL)
   {
      QPixmap c_ImgLogo;
      c_ImgLogo.load("://images/STW_Logo_Dark.png");
      c_ImgLogo = c_ImgLogo.scaled((c_ImgLogo.width() / 18), (c_ImgLogo.height() / 18),
                                   Qt::KeepAspectRatio,
                                   Qt::SmoothTransformation);
      opc_Label->setPixmap(c_ImgLogo);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot: open the help page registered for this widget's class
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeTitleBarWidget::m_TriggerHelp(void)
{
   C_HeHandler::h_GetInstance().CallSpecificHelpPage(this->metaObject()->className());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Slot: launch the About dialog using app metadata supplied by the subclass
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeTitleBarWidget::m_ShowAbout(void)
{
   const QPointer<C_OgePopUpDialog> c_New = new C_OgePopUpDialog(this, this);

   new C_NagAboutDialog(*c_New, this->m_GetAppName(), this->m_GetLogoPath(), 20,
                        this->m_GetAboutExtraCredits());

   const QSize c_SIZE(650, 500);
   c_New->SetSize(c_SIZE);

   c_New->exec();

   if (c_New != NULL)
   {
      c_New->HideOverlay();
   }
} //lint !e429  no memory leak because of the parent of pc_Dialog and the Qt memory management
