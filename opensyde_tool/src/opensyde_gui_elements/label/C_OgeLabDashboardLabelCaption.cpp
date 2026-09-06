//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Label for dashboard caption display
   \copyright   Copyright Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

#include "C_OgeLabDashboardLabelCaption.hpp"
#include <QPainter>

namespace stw {
namespace opensyde_gui_elements {

C_OgeLabDashboardLabelCaption::C_OgeLabDashboardLabelCaption(QWidget * opc_Parent)
    : QLabel(opc_Parent)
{
}

void C_OgeLabDashboardLabelCaption::SetCaption(const QString & orc_Caption)
{
    mc_Caption = orc_Caption;
    QLabel::setText(orc_Caption);
}

QString C_OgeLabDashboardLabelCaption::GetCaption(void) const
{
    return mc_Caption;
}

void C_OgeLabDashboardLabelCaption::paintEvent(QPaintEvent * const opc_Event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw caption with appropriate styling
    painter.setPen(Qt::black);
    painter.drawText(rect(), Qt::AlignCenter, mc_Caption);
}

} // namespace opensyde_gui_elements
} // namespace stw
