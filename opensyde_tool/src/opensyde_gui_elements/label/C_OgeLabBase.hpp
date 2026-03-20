//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated label with objectName/variant-based styling (header)
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGELABBASE_HPP
#define C_OGELABBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QLabel>
#include "C_OgeLabToolTipBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeLabBase :
   public C_OgeLabToolTipBase
{
   Q_OBJECT
   Q_PROPERTY(QString labelVariant READ GetLabelVariant WRITE SetLabelVariant)

public:
   explicit C_OgeLabBase(QWidget * const opc_Parent = NULL);
   ~C_OgeLabBase(void) override;

   void SetLabelVariant(const QString & orc_Variant);
   QString GetLabelVariant(void) const;

private:
   QString mc_LabelVariant;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
