//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Stub for C_OgePubIconOnly
   \copyright   Copyright Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGE_PUB_ICON_ONLY_HPP
#define C_OGE_PUB_ICON_ONLY_HPP

#include <QPushButton>

namespace stw {
namespace opensyde_gui_elements {

class C_OgePubIconOnly : public QPushButton
{
public:
    explicit C_OgePubIconOnly(QWidget * opc_Parent = nullptr);
    ~C_OgePubIconOnly() override = default;
    
    void SetSvgIcon(const QString&) {}
    void SetIconSize(int32_t) {}
};

} // namespace opensyde_gui_elements
} // namespace stw

#endif
