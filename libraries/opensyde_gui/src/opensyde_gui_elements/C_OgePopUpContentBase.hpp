//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base class for pop-up dialog content widgets (header)

   Base class for pop-up dialog content widgets (note: main module description should be in .cpp file)

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGEPOPUPCONTENTBASE_HPP
#define C_OGEPOPUPCONTENTBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include <QWidget>
#include "C_OgePopUpDialog.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgePopUpContentBase :
   public QWidget
{
   Q_OBJECT

public:
   explicit C_OgePopUpContentBase(C_OgePopUpDialog & orc_Parent, QWidget * const opc_Parent = nullptr);

protected:
   //lint -e{1725} Reference member intentionally retained as the parent dialog outlives the content widget.
   C_OgePopUpDialog & mrc_ParentDialog;

private:
   //Avoid call
   C_OgePopUpContentBase(const C_OgePopUpContentBase &);
   C_OgePopUpContentBase & operator =(const C_OgePopUpContentBase &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
