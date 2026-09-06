#ifndef C_OGE_LAB_POP_UP_TITLE_HPP
#define C_OGE_LAB_POP_UP_TITLE_HPP
#include "C_OgeLabUnified.hpp"
namespace stw { namespace opensyde_gui_elements {
class C_OgeLabPopUpTitle : public C_OgeLabUnified {
public:
    explicit C_OgeLabPopUpTitle(QWidget * opc_Parent = nullptr) : C_OgeLabUnified(opc_Parent) {}
    ~C_OgeLabPopUpTitle() override = default;
};
}}
#endif
