//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base class for pop-up dialog content widgets (implementation)

   Holds the C_OgePopUpDialog reference shared by every popup-content widget so subclasses don't
   each have to declare and initialize their own.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgePopUpContentBase.hpp"

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

   \param[in,out] orc_Parent  Reference to parent pop-up dialog
   \param[in,out] opc_Parent  Optional Qt parent widget
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgePopUpContentBase::C_OgePopUpContentBase(C_OgePopUpDialog & orc_Parent, QWidget * const opc_Parent) :
   QWidget(opc_Parent),
   mrc_ParentDialog(orc_Parent)
{
}
