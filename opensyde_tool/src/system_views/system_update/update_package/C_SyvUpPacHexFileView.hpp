//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Widget for Data Block file details (header)

   See cpp file for detailed description

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_SYVUPPACHEXFILEVIEW_HPP
#define C_SYVUPPACHEXFILEVIEW_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "C_OscHexFile.hpp"
#include "C_OgePopUpContentBase.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace Ui
{
class C_SyvUpPacHexFileView;
}

namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_SyvUpPacHexFileView :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_SyvUpPacHexFileView(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent, const QString & orc_File);
   ~C_SyvUpPacHexFileView(void) override;

   void InitStaticNames(void) const;

private:
   Ui::C_SyvUpPacHexFileView * mpc_Ui;
   const QString mc_AbsoluteFilePath;
   static const QString mhc_START_TD;
   static const QString mhc_CONTINUE_TD;

   void m_LoadInfo(void) const;
   void m_OkClicked(void);
   static void mh_AddFileSection(const QString & orc_Path, QString & orc_Content);
   static QString mh_GetMd5Hex(const QString & orc_Path);
   static void mh_AddDataInformation(stw::opensyde_core::C_OscHexFile & orc_HexFile, QString & orc_Content);
   static void mh_AddApplicationInformation(stw::opensyde_core::C_OscHexFile & orc_HexFile, QString & orc_Content);

   //Avoid call
   C_SyvUpPacHexFileView(const C_SyvUpPacHexFileView &);
   C_SyvUpPacHexFileView & operator =(const C_SyvUpPacHexFileView &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
