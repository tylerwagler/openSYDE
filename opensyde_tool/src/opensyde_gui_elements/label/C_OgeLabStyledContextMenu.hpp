#ifndef C_OGE_LAB_STYLED_CONTEXT_MENU_HPP
#define C_OGE_LAB_STYLED_CONTEXT_MENU_HPP
#include <QLabel>
#include <QFont>
#include <QString>
#include <QSize>
namespace stw { namespace opensyde_gui_elements {
class C_OgeLabStyledContextMenu : public QLabel {
public:
    explicit C_OgeLabStyledContextMenu(QWidget * opc_Parent = nullptr) : QLabel(opc_Parent) {}
    ~C_OgeLabStyledContextMenu() override = default;
    void setMinimumSize(int, int) {}
    void setMinimumSize(const QSize&) {}
    void setMaximumSize(int, int) {}
    void setMaximumSize(const QSize&) {}
    void setFont(const QFont&) {}
    void setText(const QString&) {}
    QString text() const { return QLabel::text(); }
    void SetToolTipInformation(const QString&, const QString&) {}
};
}}
#endif
