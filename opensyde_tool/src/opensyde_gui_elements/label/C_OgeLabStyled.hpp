//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Stub for C_OgeLabStyled
   \copyright   Copyright Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGE_LAB_STYLED_HPP
#define C_OGE_LAB_STYLED_HPP

#include "C_OgeLabBase.hpp"
#include <QLabel>

namespace stw {
namespace opensyde_gui_elements {

class C_OgeLabStyled : public C_OgeLabBase
{
public:
    explicit C_OgeLabStyled(QWidget * opc_Parent = nullptr);
    ~C_OgeLabStyled() override = default;
};

} // namespace opensyde_gui_elements
} // namespace stw

#endif
