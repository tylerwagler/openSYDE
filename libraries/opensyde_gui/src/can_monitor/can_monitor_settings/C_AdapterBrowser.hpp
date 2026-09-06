//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       CAN adapter picker (header)

   See cpp file for detailed description

   \copyright   Copyright 2026 Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_ADAPTERBROWSERHPP
#define C_ADAPTERBROWSERHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>

#include "C_OscCanAdapterConfig.hpp"
#include "C_OgeWiOnlyBackground.hpp"
#include "can/i_can_backend.h"

class QFrame;
class QGridLayout;
class QWidget;

namespace stw
{
namespace opensyde_gui_elements
{
class C_CamOgeCbxDark;
class C_CamOgeWiSettingSubSection;
class C_OgeLabGenericNoPaddingNoMargins;
class C_OgePubToolTipBase;
}
}

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
///Inline picker for CAN backend + adapter + bitrate, fed by libcan's ICanBackend::enumerateAdapters().
///Styled to match the other settings subsections (header strip + content card on dark background).
///Adapted from Elytron Defense's Qt_Template/AdapterBrowser; signal-driven so embedding settings
///widgets can save selections on change.
class C_AdapterBrowser :
   public stw::opensyde_gui_elements::C_OgeWiOnlyBackground
{
   Q_OBJECT

public:
   explicit C_AdapterBrowser(QWidget * const opc_Parent = nullptr);

   stw::opensyde_core::C_OscCanAdapterConfig GetAdapterConfig(void) const;
   void SetAdapterConfig(const stw::opensyde_core::C_OscCanAdapterConfig & orc_Config);

   //Drop-in compatibility with the legacy C_CamMosDllWidget interface so existing parent widgets
   //(C_CamMosWidget / C_FlaSetWidget) embed the new picker without their popup/expand plumbing
   //needing changes.
   void LoadUserSettings(void) const;
   void PrepareForExpanded(const bool oq_Expand) const;
   void OnCommunicationStarted(const bool oq_Online) const;

   //The signals keyword is necessary for Qt signal slot functionality
   //lint -save -e1736
Q_SIGNALS:
   //lint -restore
   ///Emitted whenever the user changes any of backend / adapter / bitrate.
   void SigConfigChanged(const stw::opensyde_core::C_OscCanAdapterConfig & orc_Config);
   ///Compatibility no-op: legacy widget emitted SigHide when its close button was clicked. The
   ///browser has no close button so this signal is never emitted; the slot stays connectable
   ///so parents don't need to delete the connection.
   void SigHide(void);
   ///Compatibility no-op: legacy widget emitted SigCanDllConfigured after the user clicked
   ///"Configure DLL". The new picker saves on change, so this is never emitted.
   void SigCanDllConfigured(void);

private:
   //Avoid call
   C_AdapterBrowser(const C_AdapterBrowser &);
   C_AdapterBrowser & operator =(const C_AdapterBrowser &) &;

   void m_BuildUi(void);
   void m_RefreshAdapters(void);
   void m_DisplaySelected(void);
   void m_EmitChanged(void);
   void m_OnExpand(const bool oq_Expand);

   stw::opensyde_gui_elements::C_CamOgeWiSettingSubSection * mpc_Header;
   QWidget * mpc_Content;
   stw::opensyde_gui_elements::C_CamOgeCbxDark * mpc_BackendCombo;
   stw::opensyde_gui_elements::C_CamOgeCbxDark * mpc_AdapterCombo;
   stw::opensyde_gui_elements::C_CamOgeCbxDark * mpc_BitrateCombo;
   stw::opensyde_gui_elements::C_OgePubToolTipBase * mpc_RefreshBtn;
   QWidget * mpc_DetailsContainer;
   QGridLayout * mpc_DetailsGrid;

   std::vector< ::can::AdapterInfo> mc_CurrentAdapters;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
