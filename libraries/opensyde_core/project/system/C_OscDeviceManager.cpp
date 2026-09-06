//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Manager for all device descriptions (implementation)

   Discovers device definitions by recursively walking one or more root directories on
   the filesystem. A device folder is any folder containing a "device.syd" manifest;
   the folder hierarchy from each root becomes the group hierarchy in the toolbox
   (encoded as forward-slash path strings on the group name). Duplicate device names
   across roots resolve as first-root-wins, with a warning logged.

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <filesystem>
#include <map>
#include <set>
#include <system_error>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscDeviceManager.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscDeviceDefinitionFiler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

namespace
{
const char * const mhc_MANIFEST_FILENAME = "device.syd";
}

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscDeviceManager::C_OscDeviceManager(void) :
   mq_WasLoaded(false)
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Search for device with specified name

   The returned pointer points to one of the device instances owned by this class.
   So the caller has to consider the lifetime of the used instance of this class when using
   the returned pointer.

   \param[in]   orc_Name               Searched name
   \param[in]   orc_MainDeviceName     Main device name (empty if none exists)
   \param[out]  oru32_SubDeviceIndex   Sub device index

   \return
   != NULL:  pointer to device definition
   NULL:     device definition not found
*/
//----------------------------------------------------------------------------------------------------------------------
const C_OscDeviceDefinition * C_OscDeviceManager::LookForDevice(const std::string & orc_Name,
                                                                const std::string & orc_MainDeviceName,
                                                                uint32_t & oru32_SubDeviceIndex) const
{
   const C_OscDeviceDefinition * pc_Device = nullptr;

   for (uint32_t u32_ItDevice = 0U; u32_ItDevice < this->mc_DeviceGroups.size(); ++u32_ItDevice)
   {
      pc_Device = this->mc_DeviceGroups[u32_ItDevice].LookForDevice(orc_Name, orc_MainDeviceName, oru32_SubDeviceIndex);
      if (pc_Device != nullptr)
      {
         break;
      }
   }
   return pc_Device;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get all device groups

   \return
   copy of all device groups owned by this class
*/
//----------------------------------------------------------------------------------------------------------------------
std::vector<C_OscDeviceGroup> C_OscDeviceManager::GetDeviceGroups(void) const
{
   return this->mc_DeviceGroups;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get "WasLoaded" flag

   Set after LoadFromPaths runs.

   \return
   status of flag
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscDeviceManager::WasLoaded(void) const
{
   return mq_WasLoaded;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Discover devices by recursively scanning the given root paths

   For each root, walks the directory tree (depth-unlimited) looking for files literally
   named "device.syd". Each such file is parsed as a device-definition manifest. The
   folder hierarchy from the scan root to the device folder's parent (joined by "/")
   becomes the device's group name; top-level device folders end up in an unnamed group.

   On a duplicate device name (same c_DeviceName already registered from an earlier root
   or earlier folder), the first-seen device wins and a warning is logged.

   Per-device parse failures and per-root access failures are logged but do not stop the
   overall scan.

   \param[in]  orc_RootPaths   Root directories to scan, in priority order

   \return
   C_NO_ERR  Scan completed (with or without devices found)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscDeviceManager::LoadFromPaths(const std::vector<std::string> & orc_RootPaths)
{
   namespace fs = std::filesystem;

   this->mc_DeviceGroups.clear();
   this->mq_WasLoaded = false;

   std::set<std::string> c_KnownDeviceNames;
   // std::map gives a deterministic alphabetical group ordering at flatten time.
   std::map<std::string, C_OscDeviceGroup> c_GroupsByName;

   for (uint32_t u32_ItRoot = 0U; u32_ItRoot < orc_RootPaths.size(); ++u32_ItRoot)
   {
      const std::string & rc_RootStr = orc_RootPaths[u32_ItRoot];
      const fs::path c_RootPath(rc_RootStr.c_str());

      std::error_code c_Ec;
      if (fs::is_directory(c_RootPath, c_Ec) == false)
      {
         osc_write_log_warning("Loading device definitions",
                               "Configured device root \"" + rc_RootStr +
                               "\" does not exist or is not a directory; skipping.");
         continue;
      }

      fs::recursive_directory_iterator c_It(c_RootPath, fs::directory_options::follow_directory_symlink, c_Ec);
      if (c_Ec)
      {
         osc_write_log_warning("Loading device definitions",
                               "Failed to open device root \"" + rc_RootStr + "\": " +
                               std::string(c_Ec.message().c_str()));
         continue;
      }

      const fs::recursive_directory_iterator c_End;
      for (; c_It != c_End; c_It.increment(c_Ec))
      {
         if (c_Ec)
         {
            osc_write_log_warning("Loading device definitions",
                                  "Filesystem walk error under \"" + rc_RootStr + "\": " +
                                  std::string(c_Ec.message().c_str()));
            c_Ec.clear();
            continue;
         }

         std::error_code c_FileEc;
         if ((c_It->is_regular_file(c_FileEc) == false) ||
             (c_It->path().filename() != mhc_MANIFEST_FILENAME))
         {
            continue;
         }

         const fs::path c_ManifestPath = c_It->path();
         const fs::path c_DeviceFolder = c_ManifestPath.parent_path();

         // Group name = path from root to the device folder's parent, "/"-separated.
         // Top-level device folders (parent == root) yield an empty group name.
         std::error_code c_RelEc;
         const fs::path c_GroupRel = fs::relative(c_DeviceFolder.parent_path(), c_RootPath, c_RelEc);
         std::string c_GroupName;
         if (c_RelEc || c_GroupRel.empty() || (c_GroupRel == fs::path(".")))
         {
            c_GroupName = "";
         }
         else
         {
            c_GroupName = c_GroupRel.generic_string().c_str();
         }

         C_OscDeviceDefinition c_Device;
         const std::string c_ManifestStr(c_ManifestPath.string().c_str());
         const int32_t s32_LoadResult = C_OscDeviceDefinitionFiler::h_Load(c_Device, c_ManifestStr);
         if (s32_LoadResult != C_NO_ERR)
         {
            osc_write_log_error("Loading device definitions",
                                "Failed to parse manifest \"" + c_ManifestStr + "\".");
            continue;
         }

         if (c_KnownDeviceNames.count(c_Device.c_DeviceName) > 0U)
         {
            osc_write_log_warning("Loading device definitions",
                                  "Duplicate device name \"" + c_Device.c_DeviceName +
                                  "\" found at \"" + c_ManifestStr + "\"; first occurrence wins.");
            continue;
         }

         c_KnownDeviceNames.insert(c_Device.c_DeviceName);

         C_OscDeviceGroup & rc_Group = c_GroupsByName[c_GroupName];
         if (rc_Group.GetGroupName().empty() && (c_GroupName.empty() == false))
         {
            rc_Group.SetGroupName(c_GroupName);
         }
         rc_Group.AddDevice(c_Device);
      }
   }

   this->mc_DeviceGroups.reserve(c_GroupsByName.size());
   for (std::map<std::string, C_OscDeviceGroup>::iterator c_ItGroup = c_GroupsByName.begin();
        c_ItGroup != c_GroupsByName.end(); ++c_ItGroup)
   {
      this->mc_DeviceGroups.push_back(c_ItGroup->second);
   }

   this->mq_WasLoaded = true;
   return C_NO_ERR;
}
