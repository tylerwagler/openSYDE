//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated radio button with objectName/variant-based styling (header)
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGERABBASE_HPP
#define C_OGERABBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include "C_OgeRabToolTipBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeRabBase :
   public C_OgeRabToolTipBase
{
   Q_OBJECT
   Q_PROPERTY(QString radioVariant READ GetRadioVariant WRITE SetRadioVariant)

public:
   explicit C_OgeRabBase(QWidget * const opc_Parent = NULL);
   ~C_OgeRabBase(void) override;

   void SetRadioVariant(const QString & orc_Variant);
   QString GetRadioVariant(void) const;

   void SetStyleVariantReceivers(void);

private:
   QString mc_RadioVariant;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
