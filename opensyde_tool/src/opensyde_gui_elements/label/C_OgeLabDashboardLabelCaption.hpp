//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Label for dashboard caption display
   \copyright   Copyright Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGE_LAB_DASHBOARD_LABEL_CAPTION_HPP
#define C_OGE_LAB_DASHBOARD_LABEL_CAPTION_HPP

#include <QLabel>
#include <QString>

namespace stw {
namespace opensyde_gui_elements {

class C_OgeLabDashboardLabelCaption : public QLabel
{
    Q_OBJECT

public:
    explicit C_OgeLabDashboardLabelCaption(QWidget * opc_Parent = nullptr);
    ~C_OgeLabDashboardLabelCaption() override = default;

    void SetCaption(const QString & orc_Caption);
    QString GetCaption(void) const;

protected:
    void paintEvent(QPaintEvent * const opc_Event) override;

private:
    QString mc_Caption;
};

} // namespace opensyde_gui_elements
} // namespace stw

#endif
