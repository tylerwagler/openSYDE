//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Custom scroll area with dark mode support (implementation)

   Custom scroll area with dark mode support

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QScrollBar>
#include "C_OgeDarkScrollArea.hpp"
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
C_OgeDarkScrollArea::C_OgeDarkScrollArea(QWidget * const opc_Parent) :
   QScrollArea(opc_Parent)
{
   // configure the scrollbar to stop resizing the widget when showing or hiding the scrollbar
   this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
   this->horizontalScrollBar()->hide();
   // configure the scrollbar to stop resizing the widget when showing or hiding the scrollbar
   this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
   this->verticalScrollBar()->hide();
   //Connects
   connect(this->verticalScrollBar(), &QScrollBar::rangeChanged, this,
           &C_OgeDarkScrollArea::m_ScrollBarRangeChangedVer);
   connect(this->horizontalScrollBar(), &QScrollBar::rangeChanged, this,
           &C_OgeDarkScrollArea::m_ScrollBarRangeChangedHor);

   // Deactivate custom context menu of scroll bar
   this->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
   this->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
}
//----------------------------------------------------------------------------------------------------------------------
void C_OgeDarkScrollArea::m_ScrollBarRangeChangedVer(const int32_t os32_Min, const int32_t os32_Max) const
{
   stw::opensyde_gui_logic::C_OgeWiUtil::h_ShowHideScrollBar(this->verticalScrollBar(), os32_Min, os32_Max);
}
//----------------------------------------------------------------------------------------------------------------------
void C_OgeDarkScrollArea::m_ScrollBarRangeChangedHor(const int32_t os32_Min, const int32_t os32_Max) const
{
   stw::opensyde_gui_logic::C_OgeWiUtil::h_ShowHideScrollBar(this->horizontalScrollBar(), os32_Min, os32_Max);
}
