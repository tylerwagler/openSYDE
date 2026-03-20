//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated line edit with objectName/variant-based styling (header)
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGELEBASE_HPP
#define C_OGELEBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include "C_OgeLeToolTipBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeLeBase :
   public C_OgeLeToolTipBase
{
   Q_OBJECT
   Q_PROPERTY(QString lineEditVariant READ GetLineEditVariant WRITE SetLineEditVariant)

public:
   explicit C_OgeLeBase(QWidget * const opc_Parent = NULL);
   ~C_OgeLeBase(void) override;

   void SetLineEditVariant(const QString & orc_Variant);
   QString GetLineEditVariant(void) const;

   void SetBackgroundColor(const int32_t os32_Value);

private:
   QString mc_LineEditVariant;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
