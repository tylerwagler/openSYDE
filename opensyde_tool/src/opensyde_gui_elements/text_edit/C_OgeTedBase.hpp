//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated text edit with objectName/variant-based styling (header)
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGETEDBASE_HPP
#define C_OGETEDBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include <QTextEdit>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeTedBase :
   public QTextEdit
{
   Q_OBJECT
   Q_PROPERTY(QString textVariant READ GetTextVariant WRITE SetTextVariant)

public:
   explicit C_OgeTedBase(QWidget * const opc_Parent = NULL);
   ~C_OgeTedBase(void) override;

   void SetTextVariant(const QString & orc_Variant);
   QString GetTextVariant(void) const;

private:
   QString mc_TextVariant;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
