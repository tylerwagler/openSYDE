//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Shared base for the dashboard widget-property panels (implementation)

   The six dashboard widget-property panels (Label, PieChart, ProgressBar, Slider, SpinBox,
   Toggle) all hold a reference to the same C_SyvDaPeBase host plus a dark-mode flag, and
   their m_UpdatePreview methods all end with the same scene clear / addItem / clearSelection
   block. This base captures that shared shell so each subclass only has to declare its
   per-type config getters/setters and the middle of m_UpdatePreview.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_SyvDaPePanelBase.hpp"
#include "C_SyvDaDashboardScene.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   \param[in,out]  orc_Parent    Parent host (owns the preview scene and theme/dark-mode state)
   \param[in]      oq_DarkMode   Flag for dark mode rendering
*/
//----------------------------------------------------------------------------------------------------------------------
C_SyvDaPePanelBase::C_SyvDaPePanelBase(C_SyvDaPeBase & orc_Parent, const bool oq_DarkMode) :
   QWidget(&orc_Parent),
   mrc_ParentDialog(orc_Parent),
   mq_DarkMode(oq_DarkMode)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Replace the preview scene contents with the supplied item

   \param[in,out]  opc_Item   Already-configured graphics item (subclass owns construction
                              and per-type setup); ownership transfers to the scene.
*/
//----------------------------------------------------------------------------------------------------------------------
void C_SyvDaPePanelBase::m_PublishPreviewItem(QGraphicsItem * const opc_Item) const
{
   this->mrc_ParentDialog.GetPreviewScene()->clear();
   this->mrc_ParentDialog.GetPreviewScene()->addItem(opc_Item);
   this->mrc_ParentDialog.GetPreviewScene()->clearSelection();
}
