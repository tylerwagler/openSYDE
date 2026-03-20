//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Unified combo box with configurable appearance and behavior
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGECBXUNIFIED_HPP
#define C_OGECBXUNIFIED_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QComboBox>
#include "C_OgeCbxToolTipBase.hpp"
#include "stwtypes.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeCbxUnified :
    public C_OgeCbxToolTipBase
{
    Q_OBJECT
    Q_ENUMS(E_ComboBoxType)

public:
    /// Combo box display types
    enum E_ComboBoxType
    {
        eSTANDARD = 0,       ///< Standard combo box
        eICON_ONLY,           ///< Icon-only items
        eTEXT,                ///< Text items
        ePARAM,               ///< Parameter combo box
        eTABLE,               ///< Table combo box
        eDASHBOARD,           ///< Dashboard combo box
        eFONT_PROPERTIES,     ///< Font properties combo box
        eMULTI_SELECT         ///< Multi-select combo box
    };

    explicit C_OgeCbxUnified(QWidget * const opc_Parent = NULL,
                             const E_ComboBoxType e_Type = eSTANDARD);
    ~C_OgeCbxUnified(void) override;

    void SetType(const E_ComboBoxType e_Type);
    E_ComboBoxType GetType(void) const;

    // Delegate support
    void SetStyledDelegate(void);
    void SetIconDelegate(void);
    void SetIconDelegateWithPadding(const int16_t os16_PaddingLeft);

    // Initialization
    void InitFromStringList(const QStringList & orc_Strings,
                            const QStringList & orc_Values = QStringList());
    void InitMinMaxAndScaling(const stw::opensyde_core::C_OscNodeDataPoolContent & orc_Min,
                              const stw::opensyde_core::C_OscNodeDataPoolContent & orc_Max,
                              const float64_t of64_Factor, const float64_t of64_Offset);

    // Value retrieval
    int32_t GetValue(int64_t & ors64_Value, QString & orc_ErrorDescription) const;

    // Special features
    void SetVariant(const QString & orc_Variant);
    QString GetVariant(void) const;
    void SetDarkMode(const bool oq_Active);
    void ResizeViewToContents(void) const;

protected:
    void showEvent(QShowEvent * const opc_Event) override;
    void resizeEvent(QResizeEvent * const opc_Event) override;

private:
    E_ComboBoxType me_Type;
    QString mc_Variant;
    bool mq_DarkMode;
    int16_t ms16_IconPaddingLeft;

    void m_ApplyTypeStyle(void);
    void m_UpdateVariant(void);

    // Avoid calls
    C_OgeCbxUnified(const C_OgeCbxUnified &);
    C_OgeCbxUnified & operator =(const C_OgeCbxUnified &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
