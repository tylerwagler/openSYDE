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
#include <string>
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

   int32_t GetRtfPath(std::string & orc_RtfPath) const;
   int32_t GetCompanyName(std::string & orc_CompanyName) const;
   int32_t GetCompanyLogoPath(std::string & orc_CompanyLogoPath) const;

   void SetRtfPath(const std::string & orc_RtfPath) const;
   void SetCompanyName(const std::string & orc_CompanyName) const;
   void SetCompanyLogoPath(const std::string & orc_CompanyLogoPath) const;

   int32_t ExportToRtf(const std::string & orc_RtfPath, const std::string & orc_CompanyName,
                       const std::string & orc_CompanyLogoPath, C_SdTopologyWidget * const opc_Widget,
                       stw::scl::C_SclStringList & orc_WarningMessages, std::string & orc_ErrorMessage);

   void InitStaticNames(void) const;

protected:

   class C_ExportXmlStructure
   {
   public:
      // Project
      std::string c_Title;
      std::string c_Name;
      std::string c_Version;
      std::string c_Created;
      std::string c_Author;
      std::string c_SysDefPath;
      std::string c_DevicesIniPath;
      std::string c_OutputPath;
      std::string c_NetworkTopologyImage;
      // openSYDE
      std::string c_OpenSydeVersion;
      // Company
      std::string c_CompanyName;
      std::string c_CompanyLogoPath;
   };

   int32_t m_CheckSettings(void) const;

private:
   Ui::C_RtfExportWidget * mpc_Ui;
   std::map<std::string, std::string> mc_NodeMapping;
   stw::scl::C_SclStringList mc_Warnings;
   std::string mc_Error;

   void m_OkClicked(void);
   void m_CancelClicked(void);
   void m_RtfPathClicked(void);
   void m_LogoPathClicked(void) const;

   int32_t m_CreateConfigXml(const std::string & orc_Path,
                             const C_ExportXmlStructure & orc_ExportXmlStructure) const;

   //Avoid call
   C_RtfExportWidget(const C_RtfExportWidget &);
   C_RtfExportWidget & operator =(const C_RtfExportWidget &) &;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
