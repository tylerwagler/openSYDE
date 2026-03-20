//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base combobox with optional delegate support (implementation)
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QStyledItemDelegate>

#include "C_OgeCbxBase.hpp"
#include "C_OgeCbxIconDelegate.hpp"

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
   \param[in,out] opc_Parent Optional pointer to parent
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeCbxBase::C_OgeCbxBase(QWidget * const opc_Parent) :
   C_OgeCbxToolTipBase(opc_Parent)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeCbxBase::~C_OgeCbxBase(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set a QStyledItemDelegate (for dark theme comboboxes)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxBase::SetStyledDelegate(void)
{
   QStyledItemDelegate * const pc_ItemDelegate = new QStyledItemDelegate();
   this->setItemDelegate(pc_ItemDelegate);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set a C_OgeCbxIconDelegate (for table comboboxes with icons)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxBase::SetIconDelegate(void)
{
   C_OgeCbxIconDelegate * const pc_ItemDelegate = new C_OgeCbxIconDelegate();
   this->setItemDelegate(pc_ItemDelegate);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set a C_OgeCbxIconDelegate with custom padding
   \param[in] os16_PaddingLeft Left padding in pixels
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxBase::SetIconDelegateWithPadding(const int16_t os16_PaddingLeft)
{
   C_OgeCbxIconDelegate * const pc_ItemDelegate = new C_OgeCbxIconDelegate();
   pc_ItemDelegate->SetPaddingLeft(os16_PaddingLeft);
   this->setItemDelegate(pc_ItemDelegate);
}
