//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Transparent (except scroll bars) scroll area (implementation)

   Transparent (except scroll bars) scroll area.
   This class does not contain any functionality,
   but needs to exist, to have a unique group,
   to apply a specific stylesheet for.

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QScrollBar>

#include "C_OgeTransparentScrollArea.hpp"
#include "C_OgeWiUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_elements;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   Set up GUI with all elements.

   \param[in,out] opc_Parent Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeTransparentScrollArea::C_OgeTransparentScrollArea(QWidget * const opc_Parent) :
   QScrollArea(opc_Parent)
{
   // Deactivate custom context menu of scroll bar
   this->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
   this->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Deactivating the behaviour that the scrollarea will resize if the scroll bar will be showed or hided
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeTransparentScrollArea::DeactivateScrollbarResize(void)
{
   // configure the scrollbar to stop resizing the widget when showing or hiding the scrollbar
   this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
   this->verticalScrollBar()->hide();

   connect(this->verticalScrollBar(), &QScrollBar::rangeChanged, this,
           &C_OgeTransparentScrollArea::m_ScrollBarRangeChanged);
}

//----------------------------------------------------------------------------------------------------------------------
void C_OgeTransparentScrollArea::m_ScrollBarRangeChanged(const int32_t os32_Min, const int32_t os32_Max) const
{
   stw::opensyde_gui_logic::C_OgeWiUtil::h_ShowHideScrollBar(this->verticalScrollBar(), os32_Min, os32_Max);
}
