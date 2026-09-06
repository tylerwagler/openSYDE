#ifndef C_OGE_LAB_DOUBLE_CLICK_HPP
#define C_OGE_LAB_DOUBLE_CLICK_HPP
#include "C_OgeLabUnified.hpp"
namespace stw { namespace opensyde_gui_elements {
class C_OgeLabDoubleClick : public C_OgeLabUnified {
public:
    explicit C_OgeLabDoubleClick(QWidget * opc_Parent = nullptr) : C_OgeLabUnified(opc_Parent) {}
    ~C_OgeLabDoubleClick() override = default;
    void SetBackgroundColor(int) {}
    void SetForegroundColor(int) {}
    void SetFontPixel(int, bool = false, bool = false) {}
    void SetToolTipInformation(const QString&, const QString&) {}
};
}}
#endif
