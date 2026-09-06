//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       openSYDE .conf file handler

   For using an derived class is necessary which is using the concrete conent

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <system_error>

#include "stwtypes.hpp"
#include "stwerrors.hpp"

#include "C_OscConfFileHandler.hpp"
#include "C_OscErrorCategory.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_SclStringCompat.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::scl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default destructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscConfFileHandler::~C_OscConfFileHandler()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load settings from file

   \param[in]   orc_Path path to .conf file

   \retval Errc::success  setting loaded
   \retval Errc::noact    failed to load settings file
   \retval Errc::config   required setting content not found
*/
//---------------------------------------------------------------------------------------------------------------------/
std::error_code C_OscConfFileHandler::LoadSettings(const std::string & orc_Path)
{
   C_SclStringList c_StringListSource;
   C_SclStringList c_StringListWithoutComments;
   std::error_code c_Result = Errc::success;

   mc_ConfigFilePath = orc_Path;

   try
   {
      c_StringListSource.LoadFromFile(orc_Path);
   }
   catch (...)
   {
      c_Result = Errc::noact;
   }

   if (!c_Result)
   {
      //extract only lines without comments:
      for (uint32_t u32_Line = 0U; u32_Line < c_StringListSource.GetCount(); u32_Line++)
      {
          const std::string c_Line = TrimCompat(c_StringListSource.Strings[u32_Line]);

         if ((c_Line != "") && (PosCompat(c_Line, "#") == 0))
         {
            c_StringListWithoutComments.Add(c_StringListSource.Strings[u32_Line]);
         }
      }

      c_Result = m_LoadSettings(c_StringListWithoutComments);
   }
   return c_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Replace config settings in existing file

   Strategy:
   * load existing file into a string list
   * replace existing entries at their original positions by searching with the first element of the pair as key
      The second element is the set value
   ** Other entries, comments, blank lines will not be touched
   * if a previous setting is missing we will add it

   Usage:
   * Set new values for supported parameters in class instance.
   * Call function

   \param[in]   orc_Path     path to .conf file
   \param[in]   orc_Configs  All key (first) value (second) pairs for replacing with new settings

   \retval Errc::success  settings updated
   \retval Errc::noact    failed to load previous settings file or
                     failed to save settings file
*/
//---------------------------------------------------------------------------------------------------------------------/
std::error_code C_OscConfFileHandler::mh_ReplaceSettings(const std::string & orc_Path,
                                                  const std::vector<std::pair<std::string, std::string> > & orc_Configs)
{
   C_SclStringList c_StringList;
   std::error_code c_Result = Errc::success;

   try
   {
      c_StringList.LoadFromFile(orc_Path);
   }
   catch (...)
   {
      c_Result = Errc::noact;
   }

   if (!c_Result)
   {
      uint32_t u32_Counter;

      // Replacing or adding all entries
      for (u32_Counter = 0U; u32_Counter < orc_Configs.size(); u32_Counter++)
      {
          const std::pair<std::string, std::string> & rc_KeyValuePair = orc_Configs[u32_Counter];
          const std::string c_NewEntry = rc_KeyValuePair.first + "=" + rc_KeyValuePair.second;
         const int32_t s32_Index = c_StringList.IndexOfName(rc_KeyValuePair.first);

         if (s32_Index == -1)
         {
            c_StringList.Append(c_NewEntry);
         }
         else
         {
            c_StringList.Strings[s32_Index] = c_NewEntry;
         }
      }

      try
      {
         c_StringList.SaveToFile(orc_Path);
      }
      catch (...)
      {
         c_Result = Errc::noact;
      }
   }
   return c_Result;
}
