//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Stub for C_OgePubIconOnly
   \copyright   Copyright Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGE_PUB_ICON_ONLY_HPP
#define C_OGE_PUB_ICON_ONLY_HPP

#include <QWidget>

namespace stw {
namespace opensyde_gui_elements {

class C_OgePubIconOnly : public QWidget
{
public:
    explicit C_OgePubIconOnly(QWidget * opc_Parent = nullptr);
    ~C_OgePubIconOnly() override = default;
};

} // namespace opensyde_gui_elements
} // namespace stw

#endif
