//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base combobox with optional delegate support (header)
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGECBXBASE_HPP
#define C_OGECBXBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QComboBox>
#include "stwtypes.hpp"
#include "C_OgeCbxToolTipBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeCbxBase :
   public C_OgeCbxToolTipBase
{
   Q_OBJECT

public:
   explicit C_OgeCbxBase(QWidget * const opc_Parent = NULL);
   ~C_OgeCbxBase(void) override;

   /// Set a QStyledItemDelegate (for dark theme comboboxes)
   void SetStyledDelegate(void);

   /// Set a C_OgeCbxIconDelegate (for table comboboxes with icons)
   void SetIconDelegate(void);

   /// Set a C_OgeCbxIconDelegate with custom padding
   void SetIconDelegateWithPadding(const int16_t os16_PaddingLeft);

private:
   //Avoid call
   C_OgeCbxBase(const C_OgeCbxBase &);                //not implemented -> prevent copying
   C_OgeCbxBase & operator =(const C_OgeCbxBase &) &; //not implemented -> prevent assignment
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
