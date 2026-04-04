//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for configuring CAN bitrate in SYDEflash (header)

   See cpp file for detailed description

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_FLABITRATEWIDGET_HPP
#define C_FLABITRATEWIDGET_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OgeWiOnlyBackground.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_FlaBitrateWidget;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_FlaBitrateWidget :
   public stw::opensyde_gui_elements::C_OgeWiOnlyBackground
{
   Q_OBJECT

public:
   explicit C_FlaBitrateWidget(QWidget * const opc_Parent = nullptr);
   ~C_FlaBitrateWidget(void) override;

   void LoadUserSettings(void);
   int32_t GetSelectedBitrate(void) const;
   void OnCommunicationStarted(const bool oq_Online);
   void PrepareForExpanded(const bool oq_Expand) const;

   //The signals keyword is necessary for Qt signal slot functionality
   //lint -save -e1736
Q_SIGNALS:
   //lint -restore
   void SigBitrateChanged(const int32_t os32_Bitrate);
   void SigCanDllConfigured(void); // Keep for compatibility with C_FlaSetWidget
   void SigHide(void);

private:
   Ui::C_FlaBitrateWidget * mpc_Ui;

   void m_InitUi(void);
   void m_SetBitrateComboBox(const int32_t os32_Bitrate);
   void m_OnBitrateChanged(void);
   void m_OnExpand(const bool oq_Expand);

   //Avoid call
   C_FlaBitrateWidget(const C_FlaBitrateWidget &);
   C_FlaBitrateWidget & operator =(const C_FlaBitrateWidget &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
