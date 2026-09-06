//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base class for common settings bar functionalities
   \copyright   Copyright 2024 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_CAMOGEWISETTINGSBASE_HPP
#define C_CAMOGEWISETTINGSBASE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>
#include <QPair>
#include "C_CamMosSectionPopup.hpp"
#include "C_OgeWiOnlyBackground.hpp"
#include "C_OgePubToolTipBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_elements
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_CamOgeWiSettingsBase :
   public stw::opensyde_gui_elements::C_OgeWiOnlyBackground
{
   Q_OBJECT

public:
   explicit C_CamOgeWiSettingsBase(QWidget * const opc_Parent = nullptr);

protected:
   void moveEvent(QMoveEvent * const opc_Event) override;

   static stw::opensyde_gui::C_CamMosSectionPopup * mh_GetPopUp(const QPair<stw::opensyde_gui::C_CamMosSectionPopup *,
                                                                            opensyde_gui_elements::C_OgePubToolTipBase *> & orc_Pair);
   static stw::opensyde_gui_elements::C_OgePubToolTipBase * mh_GetButton(
      const QPair<stw::opensyde_gui::C_CamMosSectionPopup *,
                  opensyde_gui_elements::C_OgePubToolTipBase *> & orc_Pair);
   void m_PrepareMove() const;
   void m_InitSettingsSection(stw::opensyde_gui::C_CamMosSectionPopup * const opc_PopUp,
                              opensyde_gui_elements::C_OgePubToolTipBase * const opc_Button,
                              const QWidget * const opc_Parent, const QString & orc_Icon);
   static void mh_InitSettingsButton(stw::opensyde_gui_elements::C_OgePubToolTipBase & orc_Button,
                                     const QString & orc_Icon);
   void m_ShowAnyPopup(const bool oq_Checked);

   void m_ShowPopup(stw::opensyde_gui::C_CamMosSectionPopup * const opc_Popup,
                    const stw::opensyde_gui_elements::C_OgePubToolTipBase * const opc_Button) const;
   QPoint m_GetPopupMovePoint(const stw::opensyde_gui_elements::C_OgePubToolTipBase * const opc_Button) const;

   std::vector<QPair<stw::opensyde_gui::C_CamMosSectionPopup *,
                     opensyde_gui_elements::C_OgePubToolTipBase *> > mc_Settings;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
