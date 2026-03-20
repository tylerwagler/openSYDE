//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Stub for C_OgeLabStyledToolTip
   \copyright   Copyright Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGE_LAB_STYLED_TOOL_TIP_HPP
#define C_OGE_LAB_STYLED_TOOL_TIP_HPP

#include "C_OgeLabToolTipBase.hpp"

namespace stw {
namespace opensyde_gui_elements {

class C_OgeLabStyledToolTip : public C_OgeLabToolTipBase
{
public:
    explicit C_OgeLabStyledToolTip(QWidget * opc_Parent = nullptr);
    ~C_OgeLabStyledToolTip() override = default;
};

} // namespace opensyde_gui_elements
} // namespace stw

#endif
