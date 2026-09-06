//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for toolbox of system definition

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDTOPOLOGYTOOLBOX_HPP
#define C_SDTOPOLOGYTOOLBOX_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include <QWidget>
#include "C_SdTopologyListWidget.hpp"
#include "C_OscDeviceDefinition.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

namespace Ui
{
class C_SdTopologyToolbox;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdTopologyToolbox :
   public QWidget
{
   Q_OBJECT

public:
   explicit C_SdTopologyToolbox(QWidget * const opc_Parent = nullptr);
   ~C_SdTopologyToolbox() override;

   // slot
   void SearchChanged(const QString & orc_Text);
   void InitStaticNames(void) const;

protected:
   void enterEvent(QEnterEvent * const opc_Event) override;
   void leaveEvent(QEvent * const opc_Event) override;

private:
   //Avoid call
   C_SdTopologyToolbox(const C_SdTopologyToolbox &);
   C_SdTopologyToolbox & operator =(const C_SdTopologyToolbox &) &;

   Ui::C_SdTopologyToolbox * mpc_Ui;
   QVector<C_SdTopologyListWidget *> mc_ListWidgets;

   QIcon mc_Icon;
   C_SdTopologyListWidget * mpc_List;

   void m_FillToolboxDynamic(void);
   void m_FillToolboxStatic(void);
   void m_FillToolboxWithDynamicNodes(const stw::opensyde_core::C_OscDeviceDefinition & orc_Device);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
