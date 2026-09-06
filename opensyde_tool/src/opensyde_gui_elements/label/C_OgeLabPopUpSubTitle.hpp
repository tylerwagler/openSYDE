#ifndef C_OGE_LAB_POP_UP_SUB_TITLE_HPP
#define C_OGE_LAB_POP_UP_SUB_TITLE_HPP
#include "C_OgeLabUnified.hpp"
namespace stw { namespace opensyde_gui_elements {
class C_OgeLabPopUpSubTitle : public C_OgeLabUnified {
public:
    explicit C_OgeLabPopUpSubTitle(QWidget * opc_Parent = nullptr) : C_OgeLabUnified(opc_Parent) {}
    ~C_OgeLabPopUpSubTitle() override = default;
};
}}
#endif
