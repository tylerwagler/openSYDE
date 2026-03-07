//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated style-only group box

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGEGBXSTYLED_HPP
#define C_OGEGBXSTYLED_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QGroupBox>
#include <QString>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeGbxStyled :
   public QGroupBox
{
   Q_OBJECT
   Q_PROPERTY(QString variant READ GetVariant WRITE SetVariant)

public:
   explicit C_OgeGbxStyled(QWidget * const opc_Parent = NULL);

   QString GetVariant(void) const;
   void SetVariant(const QString & orc_Variant);

private:
   QString mc_Variant;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
