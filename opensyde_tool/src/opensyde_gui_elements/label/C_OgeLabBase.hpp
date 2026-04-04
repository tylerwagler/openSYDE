//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base class for dashboard labels with font adaptation
   \copyright   Copyright Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGE_LAB_BASE_HPP
#define C_OGE_LAB_BASE_HPP

#include <QLabel>
#include <QString>
#include <QFont>

namespace stw {
namespace opensyde_gui_elements {

class C_OgeLabBase : public QLabel
{
    Q_OBJECT

public:
    explicit C_OgeLabBase(QWidget * opc_Parent = nullptr);
    ~C_OgeLabBase() override = default;

    void SetAllowAutomatedAdaptation(const bool oq_Allow);
    bool GetAllowAutomatedAdaptation(void) const;
    void ResetFont(void);
    void AdjustFontToSpecificSize(const uint32_t ou32_TargetWidth);
    void SetToolTipInformation(const QString & orc_Heading, const QString & orc_Content);

protected:
    void paintEvent(QPaintEvent * const opc_Event) override;

private:
    void m_AdjustFont(void);
    void m_UpdateTextWidth(void);

    bool mq_AllowAutomatedAdaptation;
    QFont mc_OriginalFont;
    uint32_t mu32_TargetWidth;
};

} // namespace opensyde_gui_elements
} // namespace stw

#endif
