//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Dialog for file generation
   \copyright   Copyright 2020 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SDCODEGENERATIONDIALOG_HPP
#define C_SDCODEGENERATIONDIALOG_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */

#include <cstdint>
#include "C_OgePopUpContentBase.hpp"
#include "C_OscNode.hpp"
#include "C_SdCodeGenerationModel.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_SdCodeGenerationDialog;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SdCodeGenerationDialog :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_SdCodeGenerationDialog(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent);
   ~C_SdCodeGenerationDialog(void) override;

   void InitStaticNames(void) const;
   void PrepareDialog(const std::vector<uint32_t> & orc_NodesIndices);
   void GetCheckedItems(std::vector<uint32_t> & orc_NodeIndices,
                        std::vector<std::vector<uint32_t> > & orc_AppIndicesPerNode) const;


private:
   Ui::C_SdCodeGenerationDialog * mpc_Ui;
   stw::opensyde_gui_logic::C_SdCodeGenerationModel mc_Model;

   void m_OkClicked(void);
   void m_OnCancel(void);
   void m_UpdateSelection(const int32_t os32_SelectionCount) const;

   //Avoid call
   C_SdCodeGenerationDialog(const C_SdCodeGenerationDialog &);
   C_SdCodeGenerationDialog & operator =(const C_SdCodeGenerationDialog &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace
#endif
