//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Dialog for RTF file export (header)

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_RTFEXPORTWIDGET_HPP
#define C_RTFEXPORTWIDGET_HPP

#include "stwtypes.hpp"
#include "C_OgePopUpContentBase.hpp"
#include "C_SclString.hpp"
#include "C_SclStringList.hpp"
#include "C_SdTopologyWidget.hpp"

namespace Ui
{
class C_RtfExportWidget;
}
/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
class C_RtfExportWidget :
   public stw::opensyde_gui_elements::C_OgePopUpContentBase
{
   Q_OBJECT

public:
   explicit C_RtfExportWidget(stw::opensyde_gui_elements::C_OgePopUpDialog & orc_Parent);
   ~C_RtfExportWidget(void) noexcept override;

   int32_t GetRtfPath(stw::scl::C_SclString & orc_RtfPath) const;
   int32_t GetCompanyName(stw::scl::C_SclString & orc_CompanyName) const;
   int32_t GetCompanyLogoPath(stw::scl::C_SclString & orc_CompanyLogoPath) const;

   void SetRtfPath(const stw::scl::C_SclString & orc_RtfPath) const;
   void SetCompanyName(const stw::scl::C_SclString & orc_CompanyName) const;
   void SetCompanyLogoPath(const stw::scl::C_SclString & orc_CompanyLogoPath) const;

   int32_t ExportToRtf(const stw::scl::C_SclString & orc_RtfPath, const stw::scl::C_SclString & orc_CompanyName,
                       const stw::scl::C_SclString & orc_CompanyLogoPath, C_SdTopologyWidget * const opc_Widget,
                       stw::scl::C_SclStringList & orc_WarningMessages, stw::scl::C_SclString & orc_ErrorMessage);

   void InitStaticNames(void) const;

protected:

   class C_ExportXmlStructure
   {
   public:
      // Project
      stw::scl::C_SclString c_Title;
      stw::scl::C_SclString c_Name;
      stw::scl::C_SclString c_Version;
      stw::scl::C_SclString c_Created;
      stw::scl::C_SclString c_Author;
      stw::scl::C_SclString c_SysDefPath;
      stw::scl::C_SclString c_DevicesIniPath;
      stw::scl::C_SclString c_OutputPath;
      stw::scl::C_SclString c_NetworkTopologyImage;
      // openSYDE
      stw::scl::C_SclString c_OpenSydeVersion;
      // Company
      stw::scl::C_SclString c_CompanyName;
      stw::scl::C_SclString c_CompanyLogoPath;
   };

   int32_t m_CheckSettings(void) const;

private:
   Ui::C_RtfExportWidget * mpc_Ui;
   std::map<stw::scl::C_SclString, stw::scl::C_SclString> mc_NodeMapping;
   stw::scl::C_SclStringList mc_Warnings;
   stw::scl::C_SclString mc_Error;

   void m_OkClicked(void);
   void m_CancelClicked(void);
   void m_RtfPathClicked(void);
   void m_LogoPathClicked(void) const;

   int32_t m_CreateConfigXml(const stw::scl::C_SclString & orc_Path,
                             const C_ExportXmlStructure & orc_ExportXmlStructure) const;

   //Avoid call
   C_RtfExportWidget(const C_RtfExportWidget &);
   C_RtfExportWidget & operator =(const C_RtfExportWidget &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
