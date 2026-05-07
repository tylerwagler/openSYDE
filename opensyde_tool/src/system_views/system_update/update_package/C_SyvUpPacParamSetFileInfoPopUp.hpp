//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Screen for parameter set file information (header)

   See cpp file for detailed description

   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYVUPPACPARAMSETFILEINFOPOPUP_HPP
#define C_SYVUPPACPARAMSETFILEINFOPOPUP_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include "C_OgePopUpContentBase.hpp"
#include "C_SyvUpPacParamSetFileInfo.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_SyvUpPacParamSetFileInfoPopUp;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SyvUpPacParamSetFileInfoPopUp :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_SyvUpPacParamSetFileInfoPopUp(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent,
                                            const QString & orc_Path, const uint32_t ou32_NodeIndex);
   ~C_SyvUpPacParamSetFileInfoPopUp(void) noexcept override;

   void InitStaticNames(void) const;

private:
   Ui::C_SyvUpPacParamSetFileInfoPopUp * mpc_Ui;
   stw::opensyde_gui_logic::C_SyvUpPacParamSetFileInfo mc_FileInfo;

   int32_t m_ReadFile(void);
   void m_OkClicked(void);

   //Avoid call
   C_SyvUpPacParamSetFileInfoPopUp(const C_SyvUpPacParamSetFileInfoPopUp &);
   C_SyvUpPacParamSetFileInfoPopUp & operator =(const C_SyvUpPacParamSetFileInfoPopUp &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
