//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only label (inherits QLabel)

   Replaces numerous individual label subclasses that existed solely for
   QSS class-selector targeting. Uses a Q_PROPERTY "variant" so QSS can
   target C_OgeLabStyled[variant="HeadingWidget"] instead of needing a
   separate C++ class per visual style.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGELABSTYLED_HPP
#define C_OGELABSTYLED_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QLabel>
#include <QString>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeLabStyled :
   public QLabel
{
   Q_OBJECT
   Q_PROPERTY(QString variant READ GetVariant WRITE SetVariant)

public:
   explicit C_OgeLabStyled(QWidget * const opc_Parent = NULL);
   C_OgeLabStyled(const QString & orc_Variant, QWidget * const opc_Parent);

   QString GetVariant(void) const;
   void SetVariant(const QString & orc_Variant);

private:
   QString mc_Variant;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
