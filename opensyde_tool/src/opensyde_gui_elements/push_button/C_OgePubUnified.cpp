//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Unified push button with configurable appearance and behavior
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <QPainter>
#include <QEvent>
#include <QMouseEvent>
#include <QStyle>
#include <QStyleOption>

#include "C_OgePubUnified.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_gui_elements;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const int32_t C_OgePubUnified::mhs32_DEFAULT_ICON_SIZE = 36;
const int32_t C_OgePubUnified::mhs32_ICON_TEXT_SPACING = 20;

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor

   \param[in,out]  opc_Parent    Optional pointer to parent
   \param[in]      e_Type        Initial button type
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgePubUnified::C_OgePubUnified(QWidget * const opc_Parent, const E_ButtonType e_Type) :
   C_OgePubToolTipBase(opc_Parent),
   me_ButtonType(e_Type),
   me_IconType(eNO_ICON),
   ms32_IconSize(mhs32_DEFAULT_ICON_SIZE),
   me_TextAlignment(Qt::AlignVCenter | Qt::AlignLeft),
   mq_DarkMode(false),
   mq_Hovered(false),
   mq_HoveredIconOnly(false)
{
   // Set default object name for stylesheet targeting
   this->setObjectName("C_OgePubUnified");

   // Enable hover events for icon-only buttons
   if ((e_Type == eICON_ONLY) || (e_Type == eNAVIGATION))
   {
      this->setAttribute(Qt::WA_Hover, true);
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgePubUnified::~C_OgePubUnified(void)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set button type

   \param[in]  e_Type   Button type to set
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::SetButtonType(const E_ButtonType e_Type)
{
   this->me_ButtonType = e_Type;

   // Enable/disable hover events based on button type
   if ((e_Type == eICON_ONLY) || (e_Type == eNAVIGATION))
   {
      this->setAttribute(Qt::WA_Hover, true);
   }
   else
   {
      this->setAttribute(Qt::WA_Hover, false);
   }

   this->m_UpdateStylesheet();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current button type

   \return   Current button type
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgePubUnified::E_ButtonType C_OgePubUnified::GetButtonType(void) const
{
   return this->me_ButtonType;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set icon type

   \param[in]  e_Type   Icon type to set
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::SetIconType(const E_IconType e_Type)
{
   this->me_IconType = e_Type;
   this->m_UpdateStylesheet();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current icon type

   \return   Current icon type
*/
//----------------------------------------------------------------------------------------------------------------------
C_OgePubUnified::E_IconType C_OgePubUnified::GetIconType(void) const
{
   return this->me_IconType;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set pixmap-based icons

   \param[in]  orc_IconNormal     Normal state icon
   \param[in]  orc_IconHovered    Hover state icon (optional)
   \param[in]  orc_IconClicked    Clicked state icon (optional)
   \param[in]  orc_IconDisabled   Disabled state icon (optional)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::SetPixmapIcon(const QPixmap & orc_IconNormal,
                                    const QPixmap & orc_IconHovered,
                                    const QPixmap & orc_IconClicked,
                                    const QPixmap & orc_IconDisabled)
{
   this->mc_PixmapNormal = orc_IconNormal;
   this->mc_PixmapHovered = orc_IconHovered;
   this->mc_PixmapClicked = orc_IconClicked;
   this->mc_PixmapDisabled = orc_IconDisabled;
   this->me_IconType = ePIXMAP;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set QIcon-based icons

   \param[in]  orc_Icon              QIcon with states
   \param[in]  orc_IconPathNormal    Normal state icon path (optional)
   \param[in]  orc_IconPathHovered   Hover state icon path (optional)
   \param[in]  orc_IconPathClicked   Clicked state icon path (optional)
   \param[in]  orc_IconPathDisabled  Disabled state icon path (optional)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::SetIcon(const QIcon & orc_Icon,
                              const QString & orc_IconPathNormal,
                              const QString & orc_IconPathHovered,
                              const QString & orc_IconPathClicked,
                              const QString & orc_IconPathDisabled)
{
   Q_UNUSED(orc_Icon)
   Q_UNUSED(orc_IconPathNormal)
   Q_UNUSED(orc_IconPathHovered)
   Q_UNUSED(orc_IconPathClicked)
   Q_UNUSED(orc_IconPathDisabled)

   this->me_IconType = eICON;
   // QIcon handling can be added if needed
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set SVG icon

   \param[in]  orc_PathEnable   SVG path for enabled state
   \param[in]  orc_PathDisable  SVG path for disabled state (optional)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::SetSvgIcon(const QString & orc_PathEnable, const QString & orc_PathDisable)
{
   this->mc_SvgIconEnable = orc_PathEnable;
   this->mc_SvgIconDisable = orc_PathDisable;
   this->me_IconType = eSVG;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set custom icons by path (convenience wrapper for SetIcon)

   \param[in]  orc_IconPathNormal    Path for normal state icon
   \param[in]  orc_IconPathHovered   Path for hovered state icon
   \param[in]  orc_IconPathClicked   Path for clicked state icon
   \param[in]  orc_IconPathDisabled  Path for disabled state icon
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::SetCustomIcons(const QString & orc_IconPathNormal,
                                     const QString & orc_IconPathHovered,
                                     const QString & orc_IconPathClicked,
                                     const QString & orc_IconPathDisabled)
{
   this->SetIcon(QIcon(orc_IconPathNormal), orc_IconPathNormal,
                 orc_IconPathHovered, orc_IconPathClicked, orc_IconPathDisabled);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set icon size

   \param[in]  os32_IconSize   Icon size in pixels
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::SetIconSize(const int32_t os32_IconSize)
{
   this->ms32_IconSize = os32_IconSize;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current icon size

   \return   Icon size in pixels
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OgePubUnified::GetIconSize(void) const
{
   return this->ms32_IconSize;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set text alignment

   \param[in]  e_Alignment   Text alignment
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::SetTextPosition(const Qt::Alignment e_Alignment)
{
   this->me_TextAlignment = e_Alignment;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current text alignment

   \return   Text alignment
*/
//----------------------------------------------------------------------------------------------------------------------
Qt::Alignment C_OgePubUnified::GetTextPosition(void) const
{
   return this->me_TextAlignment;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set foreground colors

   \param[in]  orc_Normal    Normal state color
   \param[in]  orc_Hover     Hover state color (optional)
   \param[in]  orc_Pressed   Pressed state color (optional)
   \param[in]  orc_Disabled  Disabled state color (optional)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::SetForegroundColor(const QColor & orc_Normal,
                                         const QColor & orc_Hover,
                                         const QColor & orc_Pressed,
                                         const QColor & orc_Disabled)
{
   this->mc_ForegroundColorNormal = orc_Normal;
   this->mc_ForegroundColorHover = (orc_Hover.isValid()) ? orc_Hover : orc_Normal;
   this->mc_ForegroundColorPressed = (orc_Pressed.isValid()) ? orc_Pressed : orc_Normal;
   this->mc_ForegroundColorDisabled = (orc_Disabled.isValid()) ? orc_Disabled : orc_Normal;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set background colors

   \param[in]  orc_Normal    Normal state color
   \param[in]  orc_Hover     Hover state color (optional)
   \param[in]  orc_Pressed   Pressed state color (optional)
   \param[in]  orc_Disabled  Disabled state color (optional)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::SetBackgroundColor(const QColor & orc_Normal,
                                         const QColor & orc_Hover,
                                         const QColor & orc_Pressed,
                                         const QColor & orc_Disabled)
{
   this->mc_BackgroundColorNormal = orc_Normal;
   this->mc_BackgroundColorHover = (orc_Hover.isValid()) ? orc_Hover : orc_Normal;
   this->mc_BackgroundColorPressed = (orc_Pressed.isValid()) ? orc_Pressed : orc_Normal;
   this->mc_BackgroundColorDisabled = (orc_Disabled.isValid()) ? orc_Disabled : orc_Normal;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set border colors

   \param[in]  orc_Normal    Normal state color
   \param[in]  orc_Disabled  Disabled state color (optional)
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::SetBorderColor(const QColor & orc_Normal, const QColor & orc_Disabled)
{
   this->mc_BorderColorNormal = orc_Normal;
   this->mc_BorderColorDisabled = (orc_Disabled.isValid()) ? orc_Disabled : orc_Normal;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Enable/disable dark mode

   \param[in]  oq_Active   Dark mode flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::SetDarkMode(const bool oq_Active)
{
   this->mq_DarkMode = oq_Active;
   this->m_UpdateStylesheet();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set variant for stylesheet targeting

   \param[in]  orc_Variant   Variant name
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::SetVariant(const QString & orc_Variant)
{
   this->mc_Variant = orc_Variant;
   this->m_UpdateStylesheet();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get current variant

   \return   Variant name
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OgePubUnified::GetVariant(void) const
{
   return this->mc_Variant;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set dark mode for tab close button

   \param[in]  oq_Active   Dark mode flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::SetDarkModeForTabClose(const bool oq_Active)
{
   this->mq_DarkMode = oq_Active;
   this->update();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle event filtering

   \param[in]  opc_Event   Event to handle

   \return   Event processing result
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OgePubUnified::event(QEvent * const opc_Event)
{
   const bool q_Retval = C_OgePubToolTipBase::event(opc_Event);

   if ((opc_Event != nullptr) && ((opc_Event->type() == QEvent::HoverLeave) ||
                               (opc_Event->type() == QEvent::Hide) ||
                               (opc_Event->type() == QEvent::Show)))
   {
      this->m_HandleHover(false);
   }

   return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Paint event handler

   \param[in,out]  opc_Event   Paint event
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::paintEvent(QPaintEvent * const opc_Event)
{
   Q_UNUSED(opc_Event)

   QPainter c_Painter(this);
   c_Painter.setRenderHint(QPainter::Antialiasing, true);

   switch (this->me_ButtonType)
   {
      case eSTANDARD:
      case eICON_TEXT:
         this->m_PaintIconTextButton(c_Painter);
         break;
      case eICON_ONLY:
         this->m_PaintIconOnlyButton(c_Painter);
         break;
      case eSVG_ICON:
      case eSVG_ICON_TEXT:
         this->m_PaintSvgButton(c_Painter);
         break;
      case eCOLORED_BACKGROUND:
         this->m_PaintColoredButton(c_Painter);
         break;
      case eTAB_CLOSE:
         this->m_PaintTabCloseButton(c_Painter);
         break;
      case eNAVIGATION:
         this->m_PaintStandardButton(c_Painter);
         break;
      default:
         this->m_PaintStandardButton(c_Painter);
         break;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Enter event handler

   \param[in,out]  opc_Event   Enter event
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::enterEvent(QEnterEvent * const opc_Event)
{
   C_OgePubToolTipBase::enterEvent(opc_Event);
   this->m_HandleHover(true);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Leave event handler

   \param[in,out]  opc_Event   Leave event
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::leaveEvent(QEvent * const opc_Event)
{
   C_OgePubToolTipBase::leaveEvent(opc_Event);
   this->m_HandleHover(false);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Mouse press event handler

   \param[in,out]  opc_Event   Mouse press event
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::mousePressEvent(QMouseEvent * const opc_Event)
{
   this->update();
   C_OgePubToolTipBase::mousePressEvent(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Mouse release event handler

   \param[in,out]  opc_Event   Mouse release event
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::mouseReleaseEvent(QMouseEvent * const opc_Event)
{
   this->update();
   C_OgePubToolTipBase::mouseReleaseEvent(opc_Event);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update stylesheet based on current configuration
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::m_UpdateStylesheet(void)
{
   QString c_Style = QString("C_OgePubUnified");

   if (this->mc_Variant.isEmpty() == false)
   {
      c_Style += QString("[%1]").arg(this->mc_Variant);
   }

   this->setObjectName(c_Style);
   this->style()->unpolish(this);
   this->style()->polish(this);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Handle hover state changes

   \param[in]  oq_Enter   True if entering, false if leaving
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::m_HandleHover(const bool oq_Enter)
{
   if (oq_Enter)
   {
      this->mq_Hovered = true;
      if (this->me_ButtonType == eICON_ONLY)
      {
         this->mq_HoveredIconOnly = true;
      }
   }
   else
   {
      this->mq_Hovered = false;
      this->mq_HoveredIconOnly = false;
   }
   this->update();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Paint standard button

   \param[in,out]  orc_Painter   QPainter instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::m_PaintStandardButton(QPainter & orc_Painter)
{
   // Default implementation - relies on QSS for styling
   orc_Painter.setPen(this->palette().buttonText().color());
   orc_Painter.drawText(this->rect(), Qt::AlignCenter, this->text());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Paint icon-only button

   \param[in,out]  orc_Painter   QPainter instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::m_PaintIconOnlyButton(QPainter & orc_Painter)
{
   const QRect c_Rect = this->rect();
   const int32_t s32_IconSize = this->ms32_IconSize;

   // Select appropriate icon based on state
   QPixmap c_Icon;
   if (this->isEnabled() == false)
   {
      c_Icon = this->mc_PixmapDisabled;
   }
   else if (this->isDown() == true)
   {
      c_Icon = this->mc_PixmapClicked;
   }
   else if (this->mq_HoveredIconOnly == true)
   {
      c_Icon = this->mc_PixmapHovered;
   }
   else
   {
      c_Icon = this->mc_PixmapNormal;
    }

    if (c_Icon.isNull() == false)
    {
       const QRect c_IconRect(c_Rect.center().x() - s32_IconSize / 2, c_Rect.center().y() - s32_IconSize / 2,
                              static_cast<int>(s32_IconSize), static_cast<int>(s32_IconSize));
       orc_Painter.drawPixmap(c_IconRect, c_Icon);
    }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Paint icon-text button

   \param[in,out]  orc_Painter   QPainter instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::m_PaintIconTextButton(QPainter & orc_Painter)
{
   const QRect c_Rect = this->rect();
   const int32_t s32_IconSize = this->ms32_IconSize;

   // Define text rectangle (excluding icon area)
   QRect c_TextRect = c_Rect;
   if (this->me_IconType != eNO_ICON)
   {
      c_TextRect.setLeft(c_TextRect.left() + s32_IconSize + mhs32_ICON_TEXT_SPACING);
   }

   // Draw icon if present
   if (this->me_IconType != eNO_ICON)
   {
      QPixmap c_Icon;
      if (this->isEnabled() == false)
      {
         c_Icon = this->mc_PixmapDisabled;
      }
      else if (this->isDown() == true)
      {
         c_Icon = this->mc_PixmapClicked;
      }
      else if (this->mq_Hovered == true)
      {
         c_Icon = this->mc_PixmapHovered;
      }
      else
      {
         c_Icon = this->mc_PixmapNormal;
      }

      if (c_Icon.isNull() == false)
      {
         const QRect c_IconRect(c_Rect.left(), c_Rect.center().y() - s32_IconSize / 2, s32_IconSize, s32_IconSize);
         orc_Painter.drawPixmap(c_IconRect, c_Icon);
      }
   }

   // Draw text
   orc_Painter.setPen(this->palette().buttonText().color());
   orc_Painter.drawText(c_TextRect, this->me_TextAlignment, this->text());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Paint SVG button

   \param[in,out]  orc_Painter   QPainter instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::m_PaintSvgButton(QPainter & orc_Painter)
{
   // SVG rendering would be implemented here
   // For now, fall back to standard painting
   this->m_PaintStandardButton(orc_Painter);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Paint colored background button

   \param[in,out]  orc_Painter   QPainter instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::m_PaintColoredButton(QPainter & orc_Painter)
{
   // Colored background rendering would be implemented here
   // For now, fall back to standard painting
   this->m_PaintStandardButton(orc_Painter);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Paint tab close button

   \param[in,out]  orc_Painter   QPainter instance
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OgePubUnified::m_PaintTabCloseButton(QPainter & orc_Painter)
{
   // Tab close button rendering would be implemented here
   // For now, fall back to standard painting
   this->m_PaintStandardButton(orc_Painter);
}
