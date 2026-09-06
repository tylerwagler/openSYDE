//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Label for dashboard value display with font adaptation
   \copyright   Copyright Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGE_LAB_DASHBOARD_LABEL_VALUE_HPP
#define C_OGE_LAB_DASHBOARD_LABEL_VALUE_HPP

#include <QLabel>
#include <QString>
#include <QFont>

namespace stw {
namespace opensyde_gui_elements {

class C_OgeLabDashboardLabelValue : public QLabel
{
    Q_OBJECT

public:
    explicit C_OgeLabDashboardLabelValue(QWidget * opc_Parent = nullptr);
    ~C_OgeLabDashboardLabelValue() override = default;

    void SetValue(const QString & orc_Value);
    QString GetValue(void) const;
    void SetUnit(const QString & orc_Unit);
    void SetPrecision(const int32_t os32_Precision);
    void SetWarningThresholds(const float64_t of64_Min, const float64_t of64_Max);
    void SetErrorThresholds(const float64_t of64_Min, const float64_t of64_Max);
    
    // Font adaptation methods
    void SetAllowAutomatedAdaptation(const bool oq_Allow);
    void ResetFont(void);
    void AdjustFontToSpecificSize(const uint32_t ou32_TargetWidth);
    bool GetAllowAutomatedAdaptation(void) const;

protected:
    void paintEvent(QPaintEvent * const opc_Event) override;

private:
    void m_UpdateColor(void);
    void m_AdjustFont(void);

    QString mc_Value;
    QString mc_Unit;
    int32_t ms32_Precision;
    float64_t mf64_WarningMin;
    float64_t mf64_WarningMax;
    float64_t mf64_ErrorMin;
    float64_t mf64_ErrorMax;
    bool mq_IsWarning;
    bool mq_IsError;
    bool mq_AllowAutomatedAdaptation;
    QFont mc_OriginalFont;
};

} // namespace opensyde_gui_elements
} // namespace stw

#endif
