//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Unified push button with configurable appearance and behavior
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGEPUBUNIFIED_HPP
#define C_OGEPUBUNIFIED_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QPushButton>
#include <QPixmap>
#include <QIcon>
#include "C_OgePubToolTipBase.hpp"
#include "stwtypes.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgePubUnified :
    public C_OgePubToolTipBase
{
    Q_OBJECT
    Q_ENUMS(E_ButtonType)
    Q_ENUMS(E_IconType)
    Q_PROPERTY(E_ButtonType ButtonType READ GetButtonType WRITE SetButtonType)
    Q_PROPERTY(E_IconType IconType READ GetIconType WRITE SetIconType)

public:
    /// Button display types
    enum E_ButtonType
    {
        eSTANDARD = 0,       ///< Standard material design button
        eICON_ONLY,           ///< Icon-only button (no text)
        eICON_TEXT,           ///< Button with icon and text
        eSVG_ICON,            ///< SVG icon-only button
        eSVG_ICON_TEXT,       ///< SVG icon with text
        eCOLORED_BACKGROUND,  ///< Button with colored background
        eNAVIGATION,          ///< Navigation button with hover effects
        eTAB_CLOSE            ///< Tab close button
    };

    /// Icon source types
    enum E_IconType
    {
        eNO_ICON = 0,    ///< No icon
        ePIXMAP,         ///< QPixmap icon
        eICON,           ///< QIcon (with states)
        eSVG             ///< SVG icon
    };

    explicit C_OgePubUnified(QWidget * const opc_Parent = nullptr,
                             const E_ButtonType e_Type = eSTANDARD);
    ~C_OgePubUnified(void) override;

    // Configuration methods
    void SetButtonType(const E_ButtonType e_Type);
    E_ButtonType GetButtonType(void) const;

    void SetIconType(const E_IconType e_Type);
    E_IconType GetIconType(void) const;

    // Icon configuration
    void SetPixmapIcon(const QPixmap & orc_IconNormal,
                       const QPixmap & orc_IconHovered = QPixmap(),
                       const QPixmap & orc_IconClicked = QPixmap(),
                       const QPixmap & orc_IconDisabled = QPixmap());

    void SetIcon(const QIcon & orc_Icon,
                 const QString & orc_IconPathNormal = QString(),
                 const QString & orc_IconPathHovered = QString(),
                 const QString & orc_IconPathClicked = QString(),
                 const QString & orc_IconPathDisabled = QString());

    void SetSvgIcon(const QString & orc_PathEnable, const QString & orc_PathDisable = QString());
    void SetCustomIcons(const QString & orc_IconPathNormal,
                        const QString & orc_IconPathHovered = QString(),
                        const QString & orc_IconPathClicked = QString(),
                        const QString & orc_IconPathDisabled = QString());

    void SetIconSize(const int32_t os32_IconSize);
    int32_t GetIconSize(void) const;

    // Text configuration
    void SetTextPosition(const Qt::Alignment e_Alignment);
    Qt::Alignment GetTextPosition(void) const;

    // Color configuration
    void SetForegroundColor(const QColor & orc_Normal,
                            const QColor & orc_Hover = QColor(),
                            const QColor & orc_Pressed = QColor(),
                            const QColor & orc_Disabled = QColor());

    void SetBackgroundColor(const QColor & orc_Normal,
                            const QColor & orc_Hover = QColor(),
                            const QColor & orc_Pressed = QColor(),
                            const QColor & orc_Disabled = QColor());

    void SetBorderColor(const QColor & orc_Normal,
                        const QColor & orc_Disabled = QColor());

    void SetDarkMode(const bool oq_Active);

    // Styling
    void SetVariant(const QString & orc_Variant);
    QString GetVariant(void) const;

    // Special configuration for tab close button
    void SetDarkModeForTabClose(const bool oq_Active);

protected:
    bool event(QEvent * const opc_Event) override;
    void paintEvent(QPaintEvent * const opc_Event) override;
    void enterEvent(QEnterEvent * const opc_Event) override;
    void leaveEvent(QEvent * const opc_Event) override;
    void mousePressEvent(QMouseEvent * const opc_Event) override;
    void mouseReleaseEvent(QMouseEvent * const opc_Event) override;

private:
    E_ButtonType me_ButtonType;
    E_IconType me_IconType;
    int32_t ms32_IconSize;
    Qt::Alignment me_TextAlignment;

    // Icon storage
    QPixmap mc_PixmapNormal;
    QPixmap mc_PixmapHovered;
    QPixmap mc_PixmapClicked;
    QPixmap mc_PixmapDisabled;

    QString mc_SvgIconEnable;
    QString mc_SvgIconDisable;

    // Colors
    QColor mc_ForegroundColorNormal;
    QColor mc_ForegroundColorHover;
    QColor mc_ForegroundColorPressed;
    QColor mc_ForegroundColorDisabled;

    QColor mc_BackgroundColorNormal;
    QColor mc_BackgroundColorHover;
    QColor mc_BackgroundColorPressed;
    QColor mc_BackgroundColorDisabled;

    QColor mc_BorderColorNormal;
    QColor mc_BorderColorDisabled;

    bool mq_DarkMode;
    bool mq_Hovered;
    bool mq_HoveredIconOnly;  // Special flag for icon-only hover handling

    QString mc_Variant;

    void m_UpdateStylesheet(void);
    void m_HandleHover(const bool oq_Enter);
    void m_PaintStandardButton(QPainter & orc_Painter);
    void m_PaintIconOnlyButton(QPainter & orc_Painter);
    void m_PaintIconTextButton(QPainter & orc_Painter);
    void m_PaintSvgButton(QPainter & orc_Painter);
    void m_PaintColoredButton(QPainter & orc_Painter);
    void m_PaintTabCloseButton(QPainter & orc_Painter);

    static const int32_t mhs32_DEFAULT_ICON_SIZE;
    static const int32_t mhs32_ICON_TEXT_SPACING;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
