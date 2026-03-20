//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated line edit with unified functionality and mode-based behavior
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGELEUNIFIED_HPP
#define C_OGELEUNIFIED_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include <QLineEdit>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include "stwtypes.hpp"
#include "C_OgeLeToolTipBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeLeUnified :
   public C_OgeLeToolTipBase
{
    Q_OBJECT
    Q_PROPERTY(QString lineEditVariant READ GetLineEditVariant WRITE SetLineEditVariant)
    Q_PROPERTY(int32_t backgroundColor READ GetBackgroundColor WRITE SetBackgroundColor)
    Q_PROPERTY(E_Mode lineEditMode READ GetLineEditMode WRITE SetLineEditMode)

public:
    enum class E_Mode
    {
        STANDARD,           // Default behavior
        NAVIGATION,         // ESC key signal support
        SEARCH,             // Theme switching support
        IP_ADDRESS,         // IP address formatting
        PLACEHOLDER_VAR,    // Variable insertion support
        FILE_PATH,          // Path handling
        COMBO_BOX,          // Combo box integration
        LIST_HEADER         // Counter/name display
    };

    explicit C_OgeLeUnified(QWidget * const opc_Parent = NULL);
    explicit C_OgeLeUnified(const E_Mode e_Mode, QWidget * const opc_Parent = NULL);
    ~C_OgeLeUnified(void) override;

    // Variant styling (from C_OgeLeBase)
    void SetLineEditVariant(const QString & orc_Variant);
    QString GetLineEditVariant(void) const;
    void SetBackgroundColor(const int32_t os32_Value);
    int32_t GetBackgroundColor(void) const;

    // Mode management
    void SetLineEditMode(const E_Mode e_Mode);
    E_Mode GetLineEditMode(void) const;

    // Mode-specific configuration
    void SetSearchTheme(const bool oq_Dark);  // For SEARCH mode
    void SetEscapeSignalEnabled(const bool oq_Enable);  // For NAVIGATION mode
    void SetIPFormattingEnabled(const bool oq_Enable);  // For IP_ADDRESS mode

    // The signals keyword is necessary for Qt signal slot functionality
    //lint -save -e1736

Q_SIGNALS:
    //lint -restore
    void SigEscape(void);  // For NAVIGATION mode
    void SignalFocusIn(const QLineEdit * const opc_LineEdit);  // For IP_ADDRESS mode
    void SignalFocusOut(const QLineEdit * const opc_LineEdit);  // For IP_ADDRESS mode
    void SignalTabKey(const QLineEdit * const opc_LineEdit);  // For IP_ADDRESS mode

protected:
    void keyPressEvent(QKeyEvent * const opc_Event) override;
    void focusInEvent(QFocusEvent * const opc_Event) override;
    void focusOutEvent(QFocusEvent * const opc_Event) override;
    void mousePressEvent(QMouseEvent * const opc_Event) override;

private:
    QString mc_LineEditVariant;
    int32_t ms32_BackgroundColor;
    E_Mode me_Mode;
    bool mq_EscapeSignalEnabled;
    bool mq_IPFormattingEnabled;
    bool mq_DarkTheme;

    void m_OnSearchThemeChanged(const bool oq_Dark);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
