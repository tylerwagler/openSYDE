//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Unified label with configurable appearance and behavior
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QPainter>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QUrl>
#include <QDesktopServices>

#include "C_OgeLabUnified.hpp"
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

   \param[in,out]  opc_Parent  Optional pointer to parent
   \param[in]      e_Type      Initial label type
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeLabUnified::C_OgeLabUnified(QWidget * const opc_Parent, const E_LabelType e_Type) :
   QLabel(opc_Parent),
   me_Type(e_Type),
   mq_DarkMode(false),
   mq_Elided(false),
   mq_ErrorFrame(false),
   mq_BubbleStyle(false),
   mq_NoPaddingNoMargins(false),
   mq_ContextMenuEnabled(false),
   mq_DoubleClickEnabled(false)
{
   this->m_ApplyTypeStyle();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeLabUnified::~C_OgeLabUnified(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set label type

   \param[in]  e_Type   Label type to set
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::SetType(const E_LabelType e_Type)
{
   this->me_Type = e_Type;
   this->m_ApplyTypeStyle();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current label type

   \return   Current label type
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgeLabUnified::E_LabelType C_OgeLabUnified::GetType(void) const
{
   return this->me_Type;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Enable/disable elided text

   \param[in]  oq_Elided   Elided flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::SetElided(const bool oq_Elided)
{
   this->mq_Elided = oq_Elided;
   this->m_UpdateElidedText();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set external link

   \param[in]  orc_Href   Link URL
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::SetExternalLink(const QString & orc_Href)
{
   this->mc_Href = orc_Href;
   if (!orc_Href.isEmpty())
   {
      this->setOpenExternalLinks(true);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Enable/disable error frame

   \param[in]  oq_Error   Error frame flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::SetErrorFrame(const bool oq_Error)
{
   this->mq_ErrorFrame = oq_Error;
   this->m_ApplyTypeStyle();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Enable/disable bubble style

   \param[in]  oq_Bubble   Bubble style flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::SetBubbleStyle(const bool oq_Bubble)
{
   this->mq_BubbleStyle = oq_Bubble;
   this->m_ApplyTypeStyle();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Enable/disable no padding/margins

   \param[in]  oq_NoPadding   No padding flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::SetNoPaddingNoMargins(const bool oq_NoPadding)
{
   this->mq_NoPaddingNoMargins = oq_NoPadding;
   this->m_ApplyTypeStyle();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Enable/disable context menu

   \param[in]  oq_Enabled   Context menu enabled flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::SetContextMenuEnabled(const bool oq_Enabled)
{
   this->mq_ContextMenuEnabled = oq_Enabled;
   this->setContextMenuPolicy(oq_Enabled ? Qt::CustomContextMenu : Qt::NoContextMenu);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Enable/disable double-click

   \param[in]  oq_Enabled   Double-click enabled flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::SetDoubleClickEnabled(const bool oq_Enabled)
{
   this->mq_DoubleClickEnabled = oq_Enabled;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set SVG content

   \param[in]  orc_SvgPath   SVG file path
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::SetSvgContent(const QString & orc_SvgPath)
{
   this->mc_SvgPath = orc_SvgPath;
   if (!orc_SvgPath.isEmpty())
   {
      this->setPixmap(QPixmap(orc_SvgPath));
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set variant for stylesheet targeting

   \param[in]  orc_Variant   Variant name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::SetVariant(const QString & orc_Variant)
{
   this->mc_Variant = orc_Variant;
   this->m_UpdateVariant();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current variant

   \return   Variant name
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgeLabUnified::GetVariant(void) const
{
   return this->mc_Variant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Enable/disable dark mode

   \param[in]  oq_Active   Dark mode flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::SetDarkMode(const bool oq_Active)
{
   this->mq_DarkMode = oq_Active;
   this->m_ApplyTypeStyle();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set elided text

   \param[in]  orc_Text   Text to elide
   \param[in]  e_Mode     Elide mode
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::SetElidedText(const QString & orc_Text, const Qt::TextElideMode e_Mode)
{
   QFontMetrics c_FontMetrics(this->font());
   QString c_ElidedText = c_FontMetrics.elidedText(orc_Text, e_Mode, this->width());
   this->setText(c_ElidedText);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set popup title flag

   \param[in]  oq_Title   Popup title flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::SetPopupTitle(const bool oq_Title)
{
   Q_UNUSED(oq_Title)
   // Popup title styling handled in m_ApplyTypeStyle
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Mouse double-click event handler

   \param[in,out]  opc_Event   Mouse event
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::mouseDoubleClickEvent(QMouseEvent * const opc_Event)
{
   if (this->mq_DoubleClickEnabled == true)
   {
      Q_EMIT this->doubleClicked();
   }
   QLabel::mouseDoubleClickEvent(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Context menu event handler

   \param[in,out]  opc_Event   Context menu event
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::contextMenuEvent(QContextMenuEvent * const opc_Event)
{
   if (this->mq_ContextMenuEnabled == true)
   {
      Q_EMIT this->customContextMenuRequested(opc_Event->globalPos());
   }
   QLabel::contextMenuEvent(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Enter event handler

   \param[in,out]  opc_Event   Enter event
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::enterEvent(QEnterEvent * const opc_Event)
{
   if (this->mc_Href.isEmpty() == false)
   {
      this->setCursor(Qt::PointingHandCursor);
   }
   QLabel::enterEvent(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Leave event handler

   \param[in,out]  opc_Event   Leave event
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::leaveEvent(QEvent * const opc_Event)
{
   this->setCursor(Qt::ArrowCursor);
   QLabel::leaveEvent(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Apply type-specific styling
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::m_ApplyTypeStyle(void)
{
   QString c_Style = QString("C_OgeLabUnified");

   switch (this->me_Type)
   {
      case eDASHBOARD_VALUE:
         c_Style += "[DashboardValue]";
         break;
      case eDASHBOARD_PROGRESS_MAX:
         c_Style += "[DashboardProgressMax]";
         break;
      case eDOUBLE_CLICK:
         c_Style += "[DoubleClick]";
         break;
      case eELIDED:
         c_Style += "[Elided]";
         break;
      case eEXTERNAL_LINK:
         c_Style += "[ExternalLink]";
         break;
      case eFRAME_ERROR:
         c_Style += "[FrameError]";
         break;
      case eGENERIC_BUBBLE:
         c_Style += "[GenericBubble]";
         break;
      case eGENERIC_NO_PADDING:
         c_Style += "[GenericNoPadding]";
         break;
      case eGENERIC_CONTEXT_MENU:
         c_Style += "[GenericContextMenu]";
         break;
      case ePOPUP_TITLE:
         c_Style += "[PopupTitle]";
         break;
      case ePOPUP_SUBTITLE:
         c_Style += "[PopupSubtitle]";
         break;
      case eSTYLED:
         c_Style += "[Styled]";
         break;
      case eSTYLED_CONTEXT_MENU:
         c_Style += "[StyledContextMenu]";
         break;
      case eSTYLED_TOOLTIP:
         c_Style += "[StyledToolTip]";
         break;
      case eSVG_ONLY:
         c_Style += "[SvgOnly]";
         break;
      case eSTANDARD:
      default:
         c_Style += "[Standard]";
         break;
   }

   if (this->mc_Variant.isEmpty() == false)
   {
      c_Style += QString("[%1]").arg(this->mc_Variant);
   }

   this->setObjectName(c_Style);
   this->style()->unpolish(this);
   this->style()->polish(this);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update variant stylesheet
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::m_UpdateVariant(void)
{
   this->m_ApplyTypeStyle();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update elided text
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::m_UpdateElidedText(void)
{
   if (this->mq_Elided == true)
   {
      this->m_UpdateElidedText();
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle link click

   \param[in]  orc_Href   Link URL
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgeLabUnified::m_HandleLink(const QString & orc_Href)
{
   QUrl c_Url(orc_Href);
   QDesktopServices::openUrl(c_Url);
}
