//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Unified label with configurable appearance and behavior
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGELABUNIFIED_HPP
#define C_OGELABUNIFIED_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QLabel>
#include "stwtypes.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeLabUnified :
    public QLabel
{
    Q_OBJECT
    Q_ENUMS(E_LabelType)

public:
    /// Label display types
    enum E_LabelType
    {
        eSTANDARD = 0,           ///< Standard label
        eDASHBOARD_VALUE,         ///< Dashboard value label
        eDASHBOARD_PROGRESS_MAX,  ///< Dashboard progress maximum
        eDOUBLE_CLICK,            ///< Label with double-click support
        eELIDED,                  ///< Label with text eliding
        eEXTERNAL_LINK,           ///< Label with external link
        eFRAME_ERROR,             ///< Label with error frame
        eGENERIC_BUBBLE,          ///< Generic bubble label
        eGENERIC_NO_PADDING,      ///< Generic label without padding/margins
        eGENERIC_CONTEXT_MENU,    ///< Generic label with context menu
        ePOPUP_TITLE,             ///< Popup title label
        ePOPUP_SUBTITLE,          ///< Popup subtitle label
        eSTYLED,                  ///< Styled label
        eSTYLED_CONTEXT_MENU,     ///< Styled label with context menu
        eSTYLED_TOOLTIP,          ///< Styled label with tooltip
        eSVG_ONLY                 ///< SVG-only label
    };

    explicit C_OgeLabUnified(QWidget * const opc_Parent = NULL,
                             const E_LabelType e_Type = eSTANDARD);
    ~C_OgeLabUnified(void) override;

    void SetType(const E_LabelType e_Type);
    E_LabelType GetType(void) const;

    // Feature flags
    void SetElided(const bool oq_Elided);
    void SetExternalLink(const QString & orc_Href);
    void SetErrorFrame(const bool oq_Error);
    void SetBubbleStyle(const bool oq_Bubble);
    void SetNoPaddingNoMargins(const bool oq_NoPadding);
    void SetContextMenuEnabled(const bool oq_Enabled);
    void SetDoubleClickEnabled(const bool oq_Enabled);
    void SetSvgContent(const QString & orc_SvgPath);

    // Styling
    void SetVariant(const QString & orc_Variant);
    QString GetVariant(void) const;
    void SetDarkMode(const bool oq_Active);

    // Text eliding
    void SetElidedText(const QString & orc_Text, const Qt::TextElideMode e_Mode = Qt::ElideRight);

    // Popup-specific
    void SetPopupTitle(const bool oq_Title);

protected:
    void mouseDoubleClickEvent(QMouseEvent * const opc_Event) override;
    void contextMenuEvent(QContextMenuEvent * const opc_Event) override;
    void enterEvent(QEnterEvent * const opc_Event) override;
    void leaveEvent(QEvent * const opc_Event) override;

private:
    E_LabelType me_Type;
    QString mc_Variant;
    bool mq_DarkMode;
    bool mq_Elided;
    bool mq_ErrorFrame;
    bool mq_BubbleStyle;
    bool mq_NoPaddingNoMargins;
    bool mq_ContextMenuEnabled;
    bool mq_DoubleClickEnabled;
    QString mc_Href;
    QString mc_SvgPath;

    void m_ApplyTypeStyle(void);
    void m_UpdateVariant(void);
    void m_UpdateElidedText(void);
    void m_HandleLink(const QString & orc_Href);

    // Avoid calls
    C_OgeLabUnified(const C_OgeLabUnified &);
    C_OgeLabUnified & operator =(const C_OgeLabUnified &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
