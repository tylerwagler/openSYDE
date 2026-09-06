//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Label for dashboard value display with font adaptation
   \copyright   Copyright Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "C_OgeLabDashboardLabelValue.hpp"
#include <QPainter>
#include <QFontMetrics>

namespace stw {
namespace opensyde_gui_elements {

C_OgeLabDashboardLabelValue::C_OgeLabDashboardLabelValue(QWidget * opc_Parent)
    : QLabel(opc_Parent)
    , ms32_Precision(2)
    , mf64_WarningMin(0.0)
    , mf64_WarningMax(0.0)
    , mf64_ErrorMin(0.0)
    , mf64_ErrorMax(0.0)
    , mq_IsWarning(false)
    , mq_IsError(false)
    , mq_AllowAutomatedAdaptation(true)
{
}

void C_OgeLabDashboardLabelValue::SetValue(const QString & orc_Value)
{
    mc_Value = orc_Value;
    m_UpdateColor();
    QLabel::setText(orc_Value);
    if (mq_AllowAutomatedAdaptation) {
        m_AdjustFont();
    }
}

QString C_OgeLabDashboardLabelValue::GetValue(void) const
{
    return mc_Value;
}

void C_OgeLabDashboardLabelValue::SetUnit(const QString & orc_Unit)
{
    mc_Unit = orc_Unit;
    update();
}

void C_OgeLabDashboardLabelValue::SetPrecision(const int32_t os32_Precision)
{
    ms32_Precision = os32_Precision;
}

void C_OgeLabDashboardLabelValue::SetWarningThresholds(const float64_t of64_Min, const float64_t of64_Max)
{
    mf64_WarningMin = of64_Min;
    mf64_WarningMax = of64_Max;
}

void C_OgeLabDashboardLabelValue::SetErrorThresholds(const float64_t of64_Min, const float64_t of64_Max)
{
    mf64_ErrorMin = of64_Min;
    mf64_ErrorMax = of64_Max;
}

void C_OgeLabDashboardLabelValue::SetAllowAutomatedAdaptation(const bool oq_Allow)
{
    mq_AllowAutomatedAdaptation = oq_Allow;
}

void C_OgeLabDashboardLabelValue::ResetFont(void)
{
    setFont(mc_OriginalFont);
}

void C_OgeLabDashboardLabelValue::AdjustFontToSpecificSize(const uint32_t ou32_TargetWidth)
{
    QFont font = this->font();
    QFontMetrics metrics(font);
    int32_t s32_CurrentWidth = metrics.horizontalAdvance(mc_Value);
    
    while ((s32_CurrentWidth > static_cast<int32_t>(ou32_TargetWidth)) && (font.pointSize() > 6)) {
        font.setPointSize(font.pointSize() - 1);
        metrics = QFontMetrics(font);
        s32_CurrentWidth = metrics.horizontalAdvance(mc_Value);
    }
    setFont(font);
}

bool C_OgeLabDashboardLabelValue::GetAllowAutomatedAdaptation(void) const
{
    return mq_AllowAutomatedAdaptation;
}

void C_OgeLabDashboardLabelValue::m_UpdateColor(void)
{
    // Check thresholds and update color flags
    mq_IsError = false;
    mq_IsWarning = false;
}

void C_OgeLabDashboardLabelValue::m_AdjustFont(void)
{
    if (mq_AllowAutomatedAdaptation) {
        AdjustFontToSpecificSize(this->width());
    }
}

void C_OgeLabDashboardLabelValue::paintEvent(QPaintEvent * const opc_Event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw background
    QColor bgColor = mq_IsError ? Qt::red : (mq_IsWarning ? Qt::yellow : Qt::white);
    painter.fillRect(rect(), bgColor);
    
    // Draw text
    QColor textColor = mq_IsError ? Qt::red : (mq_IsWarning ? Qt::darkYellow : Qt::black);
    painter.setPen(textColor);
    painter.drawText(rect(), Qt::AlignCenter, mc_Value);
}

} // namespace opensyde_gui_elements
} // namespace stw
