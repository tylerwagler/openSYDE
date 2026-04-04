//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for configuring CAN bitrate (header)

   See cpp file for detailed description

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CAMMOSBITRATEWIDGET_HPP
#define C_CAMMOSBITRATEWIDGET_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OgeWiOnlyBackground.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_CamMosBitrateWidget;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_CamMosBitrateWidget :
   public stw::opensyde_gui_elements::C_OgeWiOnlyBackground
{
   Q_OBJECT

public:
   explicit C_CamMosBitrateWidget(QWidget * const opc_Parent = nullptr);
   ~C_CamMosBitrateWidget(void) override;

   void LoadUserSettings(void);
   int32_t GetSelectedBitrate(void) const;
   void OnCommunicationStarted(const bool oq_Online);
   void PrepareForExpanded(const bool oq_Expand) const;

   //The signals keyword is necessary for Qt signal slot functionality
   //lint -save -e1736
Q_SIGNALS:
   //lint -restore
   void SigBitrateChanged(const int32_t os32_Bitrate);
   void SigCanBitrateConfigured(void);
   void SigHide(void);

private:
   Ui::C_CamMosBitrateWidget * mpc_Ui;

   void m_InitUi(void);
   void m_SetBitrateComboBox(const int32_t os32_Bitrate);
   void m_OnBitrateChanged(void);
   void m_OnExpand(const bool oq_Expand);

   //Avoid call
   C_CamMosBitrateWidget(const C_CamMosBitrateWidget &);
   C_CamMosBitrateWidget & operator =(const C_CamMosBitrateWidget &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
