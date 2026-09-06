#ifndef C_OGE_LAB_UNIFIED_HPP
#define C_OGE_LAB_UNIFIED_HPP
#include <QLabel>
#include <QString>
#include <QMouseEvent>
namespace stw { namespace opensyde_gui_elements {
class C_OgeLabUnified : public QLabel {
    Q_OBJECT
public:
    explicit C_OgeLabUnified(QWidget * opc_Parent = nullptr) : QLabel(opc_Parent) {}
    C_OgeLabUnified(const QString& orc_Text, QWidget * opc_Parent = nullptr) : QLabel(opc_Parent) { setText(orc_Text); }
    ~C_OgeLabUnified() override = default;
    
Q_SIGNALS:
    void SigMousePress(const QMouseEvent*);
    void SigMouseMove(const QMouseEvent*);
    void SigMouseRelease(const QMouseEvent*);
    void SigDoubleClicked();
};
}}
#endif
