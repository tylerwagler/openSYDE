//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Stub for C_OgeLabStyledContextMenu
   \copyright   Copyright Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGE_LAB_STYLED_CONTEXT_MENU_HPP
#define C_OGE_LAB_STYLED_CONTEXT_MENU_HPP

#include "C_OgeLabBase.hpp"
#include <QLabel>

namespace stw {
namespace opensyde_gui_elements {

class C_OgeLabStyledContextMenu : public C_OgeLabBase
{
public:
    explicit C_OgeLabStyledContextMenu(QWidget * opc_Parent = nullptr);
    ~C_OgeLabStyledContextMenu() override = default;
};

} // namespace opensyde_gui_elements
} // namespace stw

#endif
