#ifndef C_OGE_LAB_DASHBOARD_DEFAULT_HPP
#define C_OGE_LAB_DASHBOARD_DEFAULT_HPP
#include <QLabel>
namespace stw { namespace opensyde_gui_elements {
class C_OgeLabDashboardDefault : public QLabel {
public:
    explicit C_OgeLabDashboardDefault(QWidget * opc_Parent = nullptr) : QLabel(opc_Parent) {}
    ~C_OgeLabDashboardDefault() override = default;
};
}}
#endif
