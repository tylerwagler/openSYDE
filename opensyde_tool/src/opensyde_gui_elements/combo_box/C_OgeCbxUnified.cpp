//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Unified combo box with configurable appearance and behavior
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "C_OgeCbxUnified.hpp"
#include "C_OgeCbxIconDelegate.hpp"
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
   \param[in,out] opc_Parent Optional pointer to parent
   \param[in] e_Type Combo box type
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeCbxUnified::C_OgeCbxUnified(QWidget * const opc_Parent, const E_ComboBoxType e_Type) :
   C_OgeCbxToolTipBase(opc_Parent),
   me_Type(e_Type),
   mq_DarkMode(false),
   ms16_IconPaddingLeft(0)
{
   // Apply type-specific configuration
   switch (me_Type)
   {
   case eICON_ONLY:
   case eTABLE:
   case ePARAM:
   {
      // Set icon delegate for these types
      C_OgeCbxIconDelegate * const pc_ItemDelegate = new C_OgeCbxIconDelegate();
      if (ms16_IconPaddingLeft > 0)
      {
         pc_ItemDelegate->SetPaddingLeft(ms16_IconPaddingLeft);
      }
      this->setItemDelegate(pc_ItemDelegate);  //lint !e429
      break;
   }

   case eFONT_PROPERTIES:
   {
      // Disable context menus for font combo
      this->view()->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
      this->view()->horizontalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
      break;
   }

   case eSTANDARD:
   case eTEXT:
   case eDASHBOARD:
   case eMULTI_SELECT:
   default:
      // No special setup needed
      break;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeCbxUnified::~C_OgeCbxUnified(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set combo box type
   \param[in] e_Type New type
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxUnified::SetType(const E_ComboBoxType e_Type)
{
   this->me_Type = e_Type;
   this->m_ApplyTypeStyle();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current combo box type
   \return Current type
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeCbxUnified::E_ComboBoxType C_OgeCbxUnified::GetType(void) const
{
   return this->me_Type;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set styled delegate (for dark theme)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxUnified::SetStyledDelegate(void)
{
   // Implementation would depend on specific styled delegate class
   // Placeholder for future implementation
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set icon delegate
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxUnified::SetIconDelegate(void)
{
   C_OgeCbxIconDelegate * const pc_ItemDelegate = new C_OgeCbxIconDelegate();
   this->setItemDelegate(pc_ItemDelegate);  //lint !e429
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set icon delegate with custom padding
   \param[in] os16_PaddingLeft Left padding in pixels
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxUnified::SetIconDelegateWithPadding(const int16_t os16_PaddingLeft)
{
   this->ms16_IconPaddingLeft = os16_PaddingLeft;
   C_OgeCbxIconDelegate * const pc_ItemDelegate = new C_OgeCbxIconDelegate();
   pc_ItemDelegate->SetPaddingLeft(os16_PaddingLeft);
   this->setItemDelegate(pc_ItemDelegate);  //lint !e429
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize from string list
   \param[in] orc_Strings Display strings
   \param[in] orc_Values Optional user values
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxUnified::InitFromStringList(const QStringList & orc_Strings, const QStringList & orc_Values)
{
   this->clear();
   for (int32_t i32_Index = 0; i32_Index < orc_Strings.size(); ++i32_Index)
   {
      if (i32_Index < orc_Values.size())
      {
         this->addItem(orc_Strings[i32_Index], orc_Values[i32_Index]);
      }
      else
      {
         this->addItem(orc_Strings[i32_Index]);
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Initialize with min/max and scaling
   \param[in] orc_Min Minimum value
   \param[in] orc_Max Maximum value
   \param[in] of64_Factor Scaling factor
   \param[in] of64_Offset Offset value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxUnified::InitMinMaxAndScaling(const stw::opensyde_core::C_OscNodeDataPoolContent & orc_Min,
   const stw::opensyde_core::C_OscNodeDataPoolContent & orc_Max, const float64_t of64_Factor,
   const float64_t of64_Offset)
{
   // Implementation would populate combo box based on range
   // Placeholder for future implementation
   Q_UNUSED(orc_Min);
   Q_UNUSED(orc_Max);
   Q_UNUSED(of64_Factor);
   Q_UNUSED(of64_Offset);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current value
   \param[out] ors64_Value Retrieved value
   \param[out] orc_ErrorDescription Error description if any
   \return Error code
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OgeCbxUnified::GetValue(int64_t & ors64_Value, QString & orc_ErrorDescription) const
{
   // Implementation would convert current index to scaled value
   // Placeholder for future implementation
   Q_UNUSED(ors64_Value);
   Q_UNUSED(orc_ErrorDescription);
   return 0;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set variant for styling
   \param[in] orc_Variant Variant name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxUnified::SetVariant(const QString & orc_Variant)
{
   this->mc_Variant = orc_Variant;
   this->m_UpdateVariant();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current variant
   \return Variant string
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeCbxUnified::GetVariant(void) const
{
   return this->mc_Variant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set dark mode
   \param[in] oq_Active True to enable dark mode
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxUnified::SetDarkMode(const bool oq_Active)
{
   this->mq_DarkMode = oq_Active;
   this->m_ApplyTypeStyle();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Resize view to contents
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxUnified::ResizeViewToContents(void) const
{
   // Implementation would adjust view size
   // Placeholder for future implementation
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Show event handler
   \param[in,out] opc_Event Show event
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxUnified::showEvent(QShowEvent * const opc_Event)
{
   C_OgeCbxToolTipBase::showEvent(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Resize event handler
   \param[in,out] opc_Event Resize event
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxUnified::resizeEvent(QResizeEvent * const opc_Event)
{
   C_OgeCbxToolTipBase::resizeEvent(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Apply type-specific styling
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxUnified::m_ApplyTypeStyle(void)
{
   // Apply styling based on type
   switch (this->me_Type)
   {
   case eSTANDARD:
   case eTEXT:
   case eICON_ONLY:
   case ePARAM:
   case eTABLE:
   case eDASHBOARD:
   case eFONT_PROPERTIES:
   case eMULTI_SELECT:
      default:
      // Default styling
      break;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update variant styling
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeCbxUnified::m_UpdateVariant(void)
{
   this->style()->unpolish(this);
   this->style()->polish(this);
}
