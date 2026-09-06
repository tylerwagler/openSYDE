#ifndef C_OGE_LAB_STYLED_HPP
#define C_OGE_LAB_STYLED_HPP
#include <QLabel>
namespace stw { namespace opensyde_gui_elements {
class C_OgeLabStyled : public QLabel {
public:
    explicit C_OgeLabStyled(QWidget * opc_Parent = nullptr) : QLabel(opc_Parent) {}
    ~C_OgeLabStyled() override = default;
    void SetForegroundColor(int) {}
    void SetBackgroundColor(int) {}
    void SetFontPixel(int) {}
};
}}
#endif
