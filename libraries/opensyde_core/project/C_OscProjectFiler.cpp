//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handle project save and load

   Handle project save and load

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <cstdio>
#include <system_error>
#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_SclDateTime.hpp"
#include "TglFile.hpp"
#include "TglUtils.hpp"
#include "C_OscProjectFiler.hpp"
#include "C_OscXmlParser.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_SclStringUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::opensyde_core;
using namespace stw::scl;

using namespace stw::errors;
using namespace stw::tgl;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save project

   Save project data to XML file.
   The target path must exists.
   A potentially pre-existing file will be erased.

   Before saving the:
   - Author will be overwritten with the name of the user logged into the PC system
   - ModificationTime will be overwritten with the current time

   \param[in,out]  orc_Project         Project data to save (Author and ModificationTime will be updated)
   \param[in]      orc_Path            Path of project
   \param[in]      orc_OpenSydeVersion Current openSYDE version

   \return
   Errc::success    data saved
   Errc::rd_wr      problems accessing file system (could not erase pre-existing file before saving;
                    no write access to file)
   Errc::range      orc_Path is empty
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProjectFiler::h_Save(C_OscProject & orc_Project, const std::string & orc_Path,
                                          const std::string & orc_OpenSydeVersion)
{
   std::error_code c_Retval = Errc::success;

   //Check if file was specified
   if (orc_Path == "")
   {
      c_Retval = Errc::range;
   }
   else
   {
      bool q_NewFile = true;
      //erase file if it already exists:
      if (TglFileExists(orc_Path) == true)
      {
         //erase it:
         int x_Return; //lint !e970 !e8080  //using type to match library interface
         x_Return = std::remove(orc_Path.c_str());
         if (x_Return != 0)
         {
            osc_write_log_error("Saving project file", "Could not erase pre-existing file \"" + orc_Path + "\".");
            c_Retval = Errc::rd_wr;
         }
         q_NewFile = false;
      }

      //Normal XML
      if (!c_Retval)
      {
         c_Retval = mh_SaveInternal(orc_Project, orc_Path, orc_OpenSydeVersion, q_NewFile);
         if (c_Retval)
         {
            osc_write_log_error("Saving project file", "Could not write to file \"" + orc_Path + "\".");
            c_Retval = Errc::rd_wr;
         }
      }
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Load project

   Load project data from XML file.

   \param[in,out] orc_Project Project data
   \param[in]     orc_Path    Path of project

   \return
   Errc::success    data read
   Errc::range      specified file does not exist
   Errc::noact      specified file is present but structure is invalid (e.g. invalid XML file)
   Errc::config     content of file is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProjectFiler::h_Load(C_OscProject & orc_Project, const std::string & orc_Path)
{
   std::error_code c_Retval = Errc::success;

   if (TglFileExists(orc_Path) == true)
   {
      std::string c_Tmp;
      //Open file
      C_OscXmlParser c_Xml;
      c_Retval = c_Xml.LoadFromFile(orc_Path);
      if (!c_Retval)
      {
         //Check if file and root node exists
         if (c_Xml.SelectRoot() == "Project")
         {
            //Author & Editor
            c_Tmp = c_Xml.GetAttributeString("author");
            if (c_Tmp == "")
            {
               stw::tgl::TglGetSystemUserName(c_Tmp);
            }
            orc_Project.c_Author = c_Tmp;
            c_Tmp = c_Xml.GetAttributeString("editor");
            if (c_Tmp == "")
            {
               // use author if last editor is empty (reason: prior openSYDE versions handled author as editor)
               c_Tmp = orc_Project.c_Author;
            }
            orc_Project.c_Editor = c_Tmp;
            //Time
            {
               orc_Project.c_CreationTime =
                  C_OscProject::h_GetTimeOfString(c_Xml.GetAttributeString("creation_time"));
            }
            {
               orc_Project.c_ModificationTime =
                  C_OscProject::h_GetTimeOfString(c_Xml.GetAttributeString("modification_time"));
            }
            orc_Project.c_OpenSydeVersion = c_Xml.GetAttributeString("openSYDE_version");
            orc_Project.c_Template = c_Xml.GetAttributeString("template");
            //Check Version
            if (c_Xml.SelectNodeChild("Version") == "Version")
            {
               c_Retval = Errc::success;
               orc_Project.c_Version = c_Xml.GetNodeContent();
               c_Xml.SelectNodeParent();
            }
            else
            {
               osc_write_log_error("Loading project file", "XML node \"Version\" not found.");
               c_Retval = Errc::config;
            }
         }
         else
         {
            osc_write_log_error("Loading project file", "XML node \"Project\" not found.");
            c_Retval = Errc::config;
         }
      }
      else
      {
         c_Retval = Errc::noact;
      }
   }
   else
   {
      osc_write_log_error("Loading project file", "File does not exist \"" + orc_Path + "\".");
      c_Retval = Errc::range;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save project

   Warning: no error handling (job of caller)

   Before saving the:
   - Editor will be overwritten with the name of the user logged into the PC system
   - ModificationTime will be overwritten with the current time

   \param[in,out]  orc_Project         Project data to save (Author and ModificationTime will be updated)
   \param[in]      orc_Path            Path of project
   \param[in]      orc_OpenSydeVersion Current openSYDE version
   \param[in]      oq_New              Flag if file is new

   \return
   Errc::success    data was written
   Errc::noact      could not write to file
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscProjectFiler::mh_SaveInternal(C_OscProject & orc_Project, const std::string & orc_Path,
                                                   const std::string & orc_OpenSydeVersion, const bool oq_New)
{
   std::string c_Tmp;
   //Open file
   C_OscXmlParser c_Xml;
   std::error_code c_Return = Errc::success;

   //create root:
   c_Xml.CreateNodeChild("Project");
   c_Xml.SelectRoot();

   //author
   if (oq_New == true)
   {
      stw::tgl::TglGetSystemUserName(c_Tmp);
      orc_Project.c_Author = c_Tmp;
   }
   c_Xml.SetAttributeString("author", orc_Project.c_Author);

   //editor
   stw::tgl::TglGetSystemUserName(c_Tmp);
   orc_Project.c_Editor = c_Tmp;
   c_Xml.SetAttributeString("editor", orc_Project.c_Editor);

   //Creation
   if (oq_New == true)
   {
      orc_Project.c_CreationTime = C_SclDateTime::Now();
   }
   c_Xml.SetAttributeString("creation_time", C_OscProject::h_GetTimeFormatted(orc_Project.c_CreationTime));

   //modification
   orc_Project.c_ModificationTime = C_SclDateTime::Now();
   c_Xml.SetAttributeString("modification_time", C_OscProject::h_GetTimeFormatted(orc_Project.c_ModificationTime));

   //openSYDE version
   orc_Project.c_OpenSydeVersion = orc_OpenSydeVersion;
   c_Xml.SetAttributeString("openSYDE_version", orc_Project.c_OpenSydeVersion);

   //Template
   c_Xml.SetAttributeString("template", orc_Project.c_Template);

   //update version
   c_Xml.CreateAndSelectNodeChild("Version");
   c_Xml.SetNodeContent(orc_Project.c_Version);
   c_Xml.SelectNodeParent();

   c_Return = c_Xml.SaveToFile(orc_Path);
   return c_Return;
}
