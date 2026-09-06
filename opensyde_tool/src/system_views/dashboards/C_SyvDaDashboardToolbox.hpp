//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for toolbox of dashboard

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYVDADASHBOARDTOOLBOX_HPP
#define C_SYVDADASHBOARDTOOLBOX_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QWidget>
#include "stwtypes.hpp"
#include "C_SdTopologyListWidget.hpp"
#include <QFrame>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

namespace Ui
{
class C_SyvDaDashboardToolbox;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SyvDaDashboardToolbox :
   public QWidget
{
   Q_OBJECT

public:
   explicit C_SyvDaDashboardToolbox(QWidget * const opc_Parent = nullptr);
   ~C_SyvDaDashboardToolbox() override;

   void InitStaticNames(void) const;

protected:
   void enterEvent(QEnterEvent * const opc_Event) override;
   void leaveEvent(QEvent * const opc_Event) override;

private:
   //Avoid call
   C_SyvDaDashboardToolbox(const C_SyvDaDashboardToolbox &);
   C_SyvDaDashboardToolbox & operator =(const C_SyvDaDashboardToolbox &) &;

   Ui::C_SyvDaDashboardToolbox * mpc_Ui;
   QVector<C_SdTopologyListWidget *> mc_ListWidgets;
   QFrame * mpc_FrameSeparatorReadingWidgets;
   QFrame * mpc_FrameSeparatorWritingWidgets;

   void m_FillToolboxStatic(void);
   void m_AddWidgetItems(void);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
