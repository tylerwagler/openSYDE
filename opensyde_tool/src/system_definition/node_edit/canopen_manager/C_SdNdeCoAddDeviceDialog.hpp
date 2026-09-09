//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Pop up dialog widget for adding a device in can open manager.
   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDNDECOADDDEVICEDIALOG_HPP
#define C_SDNDECOADDDEVICEDIALOG_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QFileInfo>
#include "C_OgePopUpContentBase.hpp"
#include <cstdint>
#include "TglUtils.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_SdNdeCoAddDeviceDialog;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdNdeCoAddDeviceDialog :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_SdNdeCoAddDeviceDialog(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
                                     const uint32_t ou32_NodeIndex, const uint8_t ou8_InterfaceId);
   ~C_SdNdeCoAddDeviceDialog(void) override;

   void InitStaticNames(void) const;
   int32_t GetNodeSelection(uint32_t & oru32_NodeIndex, uint32_t & oru32_InterfaceIndex, QString & orc_EdsPath) const;
   std::string GetEdsFile(void);
   static QString h_AskForCanOpenPath(QWidget * const opc_BaseWidget);


private:
   Ui::C_SdNdeCoAddDeviceDialog * mpc_Ui;

   uint32_t mu32_BusIndex;
   uint32_t mu32_NodeIndex;
   uint8_t mu8_InterfaceId;
   std::vector<uint32_t> mc_NodeIndexes;
   std::vector<uint32_t> mc_InterfaceIndexes;

   static const QStringList mhc_SUFFIX;

   void m_OkClicked(void);
   void m_CancelClicked(void);
   void m_EdsPathButtonClicked(void);
   void m_SetEdsPath(const QString & orc_New);
   void m_LoadPicture(const QFileInfo oc_FileInfo);

   void m_FillUpComboBox(const uint32_t ou32_BusIndex, const QString oc_NodeName);
   bool m_CheckIfNodeHasCanOpenManager(const uint32_t ou32_NodeIndex);
   void m_OnLoadEds(void);

   //Avoid call
   C_SdNdeCoAddDeviceDialog(const C_SdNdeCoAddDeviceDialog &);
   C_SdNdeCoAddDeviceDialog & operator =(const C_SdNdeCoAddDeviceDialog &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
