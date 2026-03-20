//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Consolidated scroll area with objectName/variant-based styling (header)
   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OGESABASE_HPP
#define C_OGESABASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QString>
#include <QScrollArea>

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OgeSaBase :
   public QScrollArea
{
   Q_OBJECT
   Q_PROPERTY(QString scrollVariant READ GetScrollVariant WRITE SetScrollVariant)

public:
   explicit C_OgeSaBase(QWidget * const opc_Parent = NULL);
   ~C_OgeSaBase(void) override;

   void SetScrollVariant(const QString & orc_Variant);
   QString GetScrollVariant(void) const;

private:
   QString mc_ScrollVariant;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
