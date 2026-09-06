#ifndef C_OGE_LAB_STYLED_TOOLTIP_HPP
#define C_OGE_LAB_STYLED_TOOLTIP_HPP
#include <QLabel>
#include <QString>
#include <Qt>
namespace stw { namespace opensyde_gui_elements {
class C_OgeLabStyledToolTip : public QLabel {
public:
    explicit C_OgeLabStyledToolTip(QWidget * opc_Parent = nullptr) : QLabel(opc_Parent) {}
    ~C_OgeLabStyledToolTip() override = default;
    void setText(const QString&) {}
    void SetToolTipInformation(const QString&, const QString&) {}
    void setAlignment(Qt::Alignment) {}
    void setTextInteractionFlags(Qt::TextInteractionFlags) {}
    void setOpenExternalLinks(bool) {}
    void setMargin(int) {}
};
}}
#endif
