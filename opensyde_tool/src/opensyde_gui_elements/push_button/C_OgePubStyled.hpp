//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only push button

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGEPUBSTYLED_HPP
#define C_OGEPUBSTYLED_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QPushButton>
#include <QString>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgePubStyled :
   public QPushButton
{
   Q_OBJECT
   Q_PROPERTY(QString variant READ GetVariant WRITE SetVariant)

public:
   explicit C_OgePubStyled(QWidget * const opc_Parent = NULL);

   QString GetVariant(void) const;
   void SetVariant(const QString & orc_Variant);

private:
   QString mc_Variant;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
