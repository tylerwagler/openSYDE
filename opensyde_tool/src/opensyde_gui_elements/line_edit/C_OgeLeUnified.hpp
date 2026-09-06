#ifndef C_OGE_LE_UNIFIED_HPP
#define C_OGE_LE_UNIFIED_HPP
#include <QLineEdit>
#include <QString>
namespace stw { namespace opensyde_gui_elements {
class C_OgeLeUnified : public QLineEdit {
    Q_OBJECT
public:
    explicit C_OgeLeUnified(QWidget * opc_Parent = nullptr) : QLineEdit(opc_Parent) {}
    C_OgeLeUnified(const QString& orc_Text, QWidget * opc_Parent = nullptr) : QLineEdit(opc_Parent) { setText(orc_Text); }
    ~C_OgeLeUnified() override = default;

Q_SIGNALS:
    void SignalFocusIn();
    void SignalFocusOut();
    void SignalTabKey();
    void SigEscape();
};
}}
#endif
