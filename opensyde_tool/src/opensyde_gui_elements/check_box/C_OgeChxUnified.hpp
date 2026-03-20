//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated check box with unified functionality and variant-based styling
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGECHXUNIFIED_HPP
#define C_OGECHXUNIFIED_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include <QCheckBox>
#include "stwtypes.hpp"
#include "C_OgeChxToolTipBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeChxUnified :
   public C_OgeChxToolTipBase
{
    Q_OBJECT
    Q_PROPERTY(QString checkBoxVariant READ GetCheckBoxVariant WRITE SetCheckBoxVariant)
    Q_PROPERTY(bool tristate READ IsTristate WRITE SetTristate)

public:
    explicit C_OgeChxUnified(QWidget * const opc_Parent = NULL);
    explicit C_OgeChxUnified(const bool oq_Tristate, QWidget * const opc_Parent = NULL);
    ~C_OgeChxUnified(void) override;

    // Variant styling
    void SetCheckBoxVariant(const QString & orc_Variant);
    QString GetCheckBoxVariant(void) const;

    // Tristate support
    void SetTristate(const bool oq_Tristate);
    bool IsTristate(void) const;

    // Index support (for compatibility with C_OgeChxDefaultSmall)
    void SetIndexes(const uint32_t ou32_Index, const uint32_t ou32_SubIndex);
    void GetIndexes(uint32_t & oru32_Index, uint32_t & oru32_SubIndex) const;

    // The signals keyword is necessary for Qt signal slot functionality
    //lint -save -e1736

Q_SIGNALS:
    //lint -restore
    void SigCheckBoxToggled(const uint32_t ou32_Index, const uint32_t ou32_SubIndex, const bool oq_Checked);

protected:
    void resizeEvent(QResizeEvent * const opc_Event) override;
    void toggleChanged(const bool oq_Checked) override;

private:
    QString mc_CheckBoxVariant;
    bool mq_Tristate;
    uint32_t mu32_Index;
    uint32_t mu32_SubIndex;

    void m_CheckBoxToggled(const bool oq_Checked);
    void m_UpdateIndicatorSize(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
