//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Stub for C_OgeLabDashboardDefault
   \copyright   Copyright Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGE_LAB_DASHBOARD_DEFAULT_HPP
#define C_OGE_LAB_DASHBOARD_DEFAULT_HPP

#include "C_OgeLabBase.hpp"

namespace stw {
namespace opensyde_gui_elements {

class C_OgeLabDashboardDefault : public C_OgeLabBase
{
public:
    explicit C_OgeLabDashboardDefault(QWidget * opc_Parent = nullptr);
    ~C_OgeLabDashboardDefault() override = default;
};

} // namespace opensyde_gui_elements
} // namespace stw

#endif
