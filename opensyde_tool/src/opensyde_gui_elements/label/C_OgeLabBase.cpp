//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base class for dashboard labels with font adaptation
   \copyright   Copyright Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "C_OgeLabBase.hpp"
#include <QPainter>
#include <QFontMetrics>

namespace stw {
namespace opensyde_gui_elements {

C_OgeLabBase::C_OgeLabBase(QWidget * opc_Parent)
    : QLabel(opc_Parent)
    , mq_AllowAutomatedAdaptation(true)
    , mu32_TargetWidth(0)
{
    mc_OriginalFont = this->font();
}

void C_OgeLabBase::SetAllowAutomatedAdaptation(const bool oq_Allow)
{
    mq_AllowAutomatedAdaptation = oq_Allow;
}

bool C_OgeLabBase::GetAllowAutomatedAdaptation(void) const
{
    return mq_AllowAutomatedAdaptation;
}

void C_OgeLabBase::ResetFont(void)
{
    setFont(mc_OriginalFont);
}

void C_OgeLabBase::AdjustFontToSpecificSize(const uint32_t ou32_TargetWidth)
{
    QFont font = this->font();
    QFontMetrics metrics(font);
    int32_t s32_CurrentWidth = metrics.horizontalAdvance(text());
    
    while ((s32_CurrentWidth > static_cast<int32_t>(ou32_TargetWidth)) && (font.pointSize() > 6)) {
        font.setPointSize(font.pointSize() - 1);
        metrics = QFontMetrics(font);
        s32_CurrentWidth = metrics.horizontalAdvance(text());
    }
    setFont(font);
}

void C_OgeLabBase::m_AdjustFont(void)
{
    if (mq_AllowAutomatedAdaptation && mu32_TargetWidth > 0) {
        AdjustFontToSpecificSize(mu32_TargetWidth);
    }
}

void C_OgeLabBase::m_UpdateTextWidth(void)
{
    mu32_TargetWidth = static_cast<uint32_t>(this->width());
    m_AdjustFont();
}

void C_OgeLabBase::SetToolTipInformation(const QString & orc_Heading, const QString & orc_Content)
{
    this->setToolTip(orc_Heading + ": " + orc_Content);
}

void C_OgeLabBase::paintEvent(QPaintEvent * const opc_Event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), Qt::white);
    painter.setPen(Qt::black);
    painter.drawText(rect(), Qt::AlignCenter, text());
}

} // namespace opensyde_gui_elements
} // namespace stw
