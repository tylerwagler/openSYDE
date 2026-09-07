//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Filer for CANopen data classes

   Filer for CANopen data classes

   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"
#include "C_SclStringCompat.hpp"

#include "TglFile.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"
#include <string>
#include <system_error>
#include "C_OscUtils.hpp"
#include "C_OscXmlParserLog.hpp"
#include "C_OscNodeCommFiler.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscSystemFilerUtil.hpp"
#include "C_OscNodeDataPoolFiler.hpp"
#include "C_OscCanOpenManagerFiler.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */
using namespace stw::tgl;
using namespace stw::scl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */
const uint16_t C_OscCanOpenManagerFiler::mhu16_FILE_VERSION_1 = 1;

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Default constructor
*/
//----------------------------------------------------------------------------------------------------------------------
C_OscCanOpenManagerFiler::C_OscCanOpenManagerFiler()
{
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load CANopen manager configs

   \param[in,out]  orc_Config    Config
   \param[in]      orc_Path      Path
   \param[in]      orc_BasePath  Base path

   \return
   Errc::success    data read
   Errc::range      specified system definition file does not exist
   Errc::noact      specified file is present but structure is invalid (e.g. invalid XML file)
   Errc::config     system definition file content is invalid or incomplete
                    device definition file could not be loaded
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::h_LoadFile(std::map<uint8_t, C_OscCanOpenManagerInfo> & orc_Config,
                                                     const std::string & orc_Path, const std::string & orc_BasePath)
{
   std::error_code c_Retval = Errc::success;

   if (TglFileExists(orc_Path) == true)
   {
      C_OscXmlParserLog c_XmlParser;
      c_XmlParser.SetLogHeading("Loading CANopen manager data");
      c_Retval = make_error_code_from_stw(c_XmlParser.LoadFromFile(orc_Path));
      if (!c_Retval)
      {
         if (c_XmlParser.SelectRoot() == "opensyde-can-open-managers-config")
         {
            c_Retval = h_LoadData(orc_Config, c_XmlParser, orc_BasePath);
         }
         else
         {
            osc_write_log_error("Loading CANopen manager data",
                                "Could not find \"opensyde-can-open-managers-config\" node.");
            c_Retval = Errc::config;
         }
      }
      else
      {
         osc_write_log_error("Loading CANopen manager data", "File \"" + orc_Path + "\" could not be opened.");
         c_Retval = Errc::noact;
      }
   }
   else
   {
      osc_write_log_error("Loading CANopen manager data", "File \"" + orc_Path + "\" does not exist.");
      c_Retval = Errc::range;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save CANopen manager configs

   \param[in]      orc_Config                Config
   \param[in]      orc_Path                  Path
   \param[in]      orc_BasePath              Base path
   \param[in,out]  opc_CreatedFiles          Created files
   \param[in]      orc_NodeIndicesToNameMap  Node indices to name map

   \return
   Errc::success    data saved
   Errc::config     data invalid
   Errc::rd_wr      could not erase pre-existing file before saving
   Errc::rd_wr      could not write to file (e.g. missing write permissions; missing folder)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::h_SaveFile(const std::map<uint8_t, C_OscCanOpenManagerInfo> & orc_Config,
                                                     const std::string & orc_Path, const std::string & orc_BasePath,
                                                     std::vector<std::string> * const opc_CreatedFiles,
                                                     const std::map<uint32_t, std::string> & orc_NodeIndicesToNameMap)
{
   C_OscXmlParser c_XmlParser;
   std::error_code c_Retval = C_OscSystemFilerUtil::h_GetParserForNewFile(c_XmlParser, orc_Path,
                                                                          "opensyde-can-open-managers-config");

   if (!c_Retval)
   {
      //node
      c_Retval = C_OscCanOpenManagerFiler::h_SaveData(orc_Config, c_XmlParser, orc_BasePath, opc_CreatedFiles,
                                                      orc_NodeIndicesToNameMap);
      if (!c_Retval)
      {
         //Don't forget to save!
         if (c_XmlParser.SaveToFile(orc_Path) != C_NO_ERR)
         {
            osc_write_log_error("Saving CANopen manager data", "Could not create file for node.");
            c_Retval = Errc::config;
         }
      }
   }
   else
   {
      //More details are in log
      c_Retval = Errc::config;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load CANopen manager configs

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser
   \param[in]      orc_BasePath     Base path

   \return
   Errc::success    data read
   Errc::config     CANopen manager file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::h_LoadData(std::map<uint8_t, C_OscCanOpenManagerInfo> & orc_Config,
                                                     C_OscXmlParserBase & orc_XmlParser,
                                                     const std::string & orc_BasePath)
{
   std::error_code c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError("can-open-managers"));

   orc_Config.clear();
   if (!c_Retval)
   {
      uint32_t u32_ExpectedSize;
      c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error("length", u32_ExpectedSize));
      if (!c_Retval)
      {
         std::string c_NodeName = orc_XmlParser.SelectNodeChild("can-open-manager");
         if (c_NodeName == "can-open-manager")
         {
            do
            {
               uint32_t u32_Interface;
               c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error("interface", u32_Interface));
               if (!c_Retval)
               {
                  C_OscCanOpenManagerInfo c_CanOpenManager;
                  c_Retval = C_OscCanOpenManagerFiler::mh_LoadManagerData(c_CanOpenManager, orc_XmlParser,
                                                                          orc_BasePath);
                  if (!c_Retval)
                  {
                     orc_Config[static_cast<uint8_t>(u32_Interface)] = c_CanOpenManager;
                  }
               }
               c_NodeName = orc_XmlParser.SelectNodeNext("can-open-manager");
            }
            while ((c_NodeName == "can-open-manager") && (!c_Retval));
            tgl_assert(orc_XmlParser.SelectNodeParent() == "can-open-managers");
         }
         if (u32_ExpectedSize != orc_Config.size())
         {
            std::string c_Tmp;
            c_Tmp = PrintFormattedCompat("Unexpected CANopen manager count, expected: %u, got %u", u32_ExpectedSize,
                                 static_cast<uint32_t>(orc_Config.size()));
            orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("length", c_Tmp);
         }
      }
      tgl_assert(orc_XmlParser.SelectNodeParent() == "opensyde-can-open-managers-config");
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save CANopen manager configs

   \param[in]      orc_Config                Config
   \param[in,out]  orc_XmlParser             XML parser
   \param[in]      orc_BasePath              Base path
   \param[in,out]  opc_CreatedFiles          Created files
   \param[in]      orc_NodeIndicesToNameMap  Node indices to name map

   \return
   STW error codes

   \retval   Errc::success   data saved
   \retval   Errc::config    data invalid
   \retval   Errc::rd_wr     could not erase pre-existing file before saving
   \retval   Errc::rd_wr     could not write to file (e.g. missing write permissions; missing folder)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::h_SaveData(const std::map<uint8_t, C_OscCanOpenManagerInfo> & orc_Config,
                                                     C_OscXmlParserBase & orc_XmlParser,
                                                     const std::string & orc_BasePath,
                                                     std::vector<std::string> * const opc_CreatedFiles,
                                                     const std::map<uint32_t, std::string> & orc_NodeIndicesToNameMap)
{
   std::error_code c_Retval = Errc::success;

   //File version
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("file-version") == "file-version");
   orc_XmlParser.SetNodeContent(std::to_string(mhu16_FILE_VERSION_1));
   //Return
   orc_XmlParser.SelectNodeParent();
   tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("can-open-managers") == "can-open-managers");
   orc_XmlParser.SetAttributeUint32("length", static_cast<uint32_t>(orc_Config.size()));
   for (std::map<uint8_t,
                 C_OscCanOpenManagerInfo>::const_iterator c_It = orc_Config.begin();
        (c_It != orc_Config.end()) && (!c_Retval); ++c_It)
   {
      tgl_assert(orc_XmlParser.CreateAndSelectNodeChild("can-open-manager") == "can-open-manager");
      orc_XmlParser.SetAttributeUint32("interface", c_It->first);
      c_Retval = C_OscCanOpenManagerFiler::mh_SaveManagerData(c_It->second, orc_XmlParser, orc_BasePath,
                                                              opc_CreatedFiles, orc_NodeIndicesToNameMap);
      tgl_assert(orc_XmlParser.SelectNodeParent() == "can-open-managers");
   }
   //Return
   orc_XmlParser.SelectNodeParent();
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load manager data

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser
   \param[in]      orc_BasePath     Base path

   \return
   Errc::success    data read
   Errc::config     CANopen manager file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::mh_LoadManagerData(C_OscCanOpenManagerInfo & orc_Config,
                                                             C_OscXmlParserBase & orc_XmlParser,
                                                             const std::string & orc_BasePath)
{
   std::error_code c_Retval = C_OscCanOpenManagerFiler::mh_LoadManagerProperties(orc_Config, orc_XmlParser);

   if (!c_Retval)
   {
      c_Retval = C_OscCanOpenManagerFiler::mh_LoadManagerSubDevices(orc_Config.c_CanOpenDevices, orc_XmlParser,
                                                                    orc_BasePath);
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save manager data

   \param[in]      orc_Config                Config
   \param[in,out]  orc_XmlParser             XML parser
   \param[in]      orc_BasePath              Base path
   \param[in,out]  opc_CreatedFiles          Created files
   \param[in]      orc_NodeIndicesToNameMap  Node indices to name map

   \return
   STW error codes

   \retval   Errc::success   data saved
   \retval   Errc::config    data invalid
   \retval   Errc::rd_wr     could not erase pre-existing file before saving
   \retval   Errc::rd_wr     could not write to file (e.g. missing write permissions; missing folder)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::mh_SaveManagerData(const C_OscCanOpenManagerInfo & orc_Config,
                                                             C_OscXmlParserBase & orc_XmlParser,
                                                             const std::string & orc_BasePath,
                                                             std::vector<std::string> * const opc_CreatedFiles,
                                                             const std::map<uint32_t,
                                                             std::string> & orc_NodeIndicesToNameMap)
{
   C_OscCanOpenManagerFiler::mh_SaveManagerProperties(orc_Config, orc_XmlParser);
   return C_OscCanOpenManagerFiler::mh_SaveManagerSubDevices(orc_Config.c_CanOpenDevices, orc_XmlParser, orc_BasePath,
                                                             opc_CreatedFiles, orc_NodeIndicesToNameMap);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load manager properties

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser

   \return
   Errc::success    data read
   Errc::config     CANopen manager file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::mh_LoadManagerProperties(C_OscCanOpenManagerInfo & orc_Config,
                                                                   C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError("properties"));

   if (!c_Retval)
   {
      uint32_t u32_Value;
      c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError("use-opensyde-id",
                                                                              orc_Config.q_UseOpenSydeNodeId));

      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error("node-id-value", u32_Value));
         if (!c_Retval)
         {
            orc_Config.u8_NodeIdValue = static_cast<uint8_t>(u32_Value);
         }
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error("global-sdo-timeout-ms", u32_Value));
         if (!c_Retval)
         {
            orc_Config.u16_GlobalSdoTimeoutMs = static_cast<uint16_t>(u32_Value);
         }
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError("autostart-can-open-manager",
                                                                                 orc_Config.q_AutostartCanOpenManager));
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError("start-devices",
                                                                                 orc_Config.q_StartDevices));
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError("NMT-start-all",
                                                                                 orc_Config.q_NmtStartAll));
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError(
            "enable-heartbeat-producing", orc_Config.q_EnableHeartbeatProducing));
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error("heartbeat-producer-time-ms",
                                                                                   u32_Value));
         if (!c_Retval)
         {
            orc_Config.u16_HeartbeatProducerTimeMs = static_cast<uint16_t>(u32_Value);
         }
      }
      if (!c_Retval)
      {
         c_Retval = C_OscCanOpenManagerFiler::mh_LoadManagerSyncProperties(orc_Config, orc_XmlParser);
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError("nmt-error-behaviour"));
         if (!c_Retval)
         {
            const std::string c_Text = orc_XmlParser.GetNodeContent();
            c_Retval = C_OscCanOpenManagerFiler::mh_StringToCanOpenManagerInfoType(c_Text,
                                                                                   orc_Config.e_NmtErrorBehaviour);
            if (!c_Retval)
            {
               tgl_assert(orc_XmlParser.SelectNodeParent() == "properties");
            }
            else
            {
               orc_XmlParser.ReportErrorForNodeContentAppendXmlContext(
                  "Could not interpret \"" + c_Text + "\" as a valid NMT error behaviour type");
            }
         }
      }
      tgl_assert(orc_XmlParser.SelectNodeParent() == "can-open-manager");
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save manager properties

   \param[in]      orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerFiler::mh_SaveManagerProperties(const C_OscCanOpenManagerInfo & orc_Config,
                                                        C_OscXmlParserBase & orc_XmlParser)
{
   orc_XmlParser.CreateAndSelectNodeChild("properties");
   orc_XmlParser.SetAttributeBool("use-opensyde-id", orc_Config.q_UseOpenSydeNodeId);
   orc_XmlParser.SetAttributeUint32("node-id-value", static_cast<uint32_t>(orc_Config.u8_NodeIdValue));
   orc_XmlParser.SetAttributeUint32("global-sdo-timeout-ms", static_cast<uint32_t>(orc_Config.u16_GlobalSdoTimeoutMs));
   orc_XmlParser.SetAttributeBool("autostart-can-open-manager", orc_Config.q_AutostartCanOpenManager);
   orc_XmlParser.SetAttributeBool("start-devices", orc_Config.q_StartDevices);
   orc_XmlParser.SetAttributeBool("NMT-start-all", orc_Config.q_NmtStartAll);
   orc_XmlParser.SetAttributeBool("enable-heartbeat-producing", orc_Config.q_EnableHeartbeatProducing);
   orc_XmlParser.SetAttributeUint32("heartbeat-producer-time-ms",
                                    static_cast<uint32_t>(orc_Config.u16_HeartbeatProducerTimeMs));
   orc_XmlParser.CreateNodeChild("nmt-error-behaviour",
                                 C_OscCanOpenManagerFiler::mh_CanOpenManagerInfoTypeToString(orc_Config.
                                                                                             e_NmtErrorBehaviour));
   C_OscCanOpenManagerFiler::mh_SaveManagerSyncProperties(orc_Config, orc_XmlParser);
   tgl_assert(orc_XmlParser.SelectNodeParent() == "can-open-manager");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load manager sync properties

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser

   \return
   Errc::success    data read
   Errc::config     CANopen manager file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::mh_LoadManagerSyncProperties(C_OscCanOpenManagerInfo & orc_Config,
                                                                       C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   if (orc_XmlParser.SelectNodeChild("sync-message") == "sync-message")
   {
      c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError("produce",
                                                                              orc_Config.q_ProduceSyncMessage));
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error("cycle-period-us",
                                                                                   orc_Config.u32_SyncCyclePeriodUs));
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error("window-length-us",
                                                                                   orc_Config.u32_SyncWindowLengthUs));
      }
      if (!c_Retval)
      {
         tgl_assert(orc_XmlParser.SelectNodeParent() == "properties");
      }
   }
   else
   {
      orc_Config.q_ProduceSyncMessage = false;
      orc_Config.u32_SyncCyclePeriodUs = 20000UL;
      orc_Config.u32_SyncWindowLengthUs = 10000UL;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save manager sync properties

   \param[in]      orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerFiler::mh_SaveManagerSyncProperties(const C_OscCanOpenManagerInfo & orc_Config,
                                                            C_OscXmlParserBase & orc_XmlParser)
{
   orc_XmlParser.CreateAndSelectNodeChild("sync-message");
   orc_XmlParser.SetAttributeBool("produce", orc_Config.q_ProduceSyncMessage);
   orc_XmlParser.SetAttributeUint32("cycle-period-us", orc_Config.u32_SyncCyclePeriodUs);
   orc_XmlParser.SetAttributeUint32("window-length-us", orc_Config.u32_SyncWindowLengthUs);
   tgl_assert(orc_XmlParser.SelectNodeParent() == "properties");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load manager sub devices

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser
   \param[in]      orc_BasePath     Base path

   \return
   Errc::success    data read
   Errc::config     CANopen manager file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::mh_LoadManagerSubDevices(std::map<C_OscCanInterfaceId,
                                                                   C_OscCanOpenManagerDeviceInfo> & orc_Config,
                                                                   C_OscXmlParserBase & orc_XmlParser,
                                                                   const std::string & orc_BasePath)
{
   std::error_code c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError("can-open-devices"));

   orc_Config.clear();
   if (!c_Retval)
   {
      uint32_t u32_ExpectedSize;
      c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error("length", u32_ExpectedSize));
      if (!c_Retval)
      {
         std::string c_NodeName = orc_XmlParser.SelectNodeChild("can-open-device");
         if (c_NodeName == "can-open-device")
         {
            do
            {
               c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError("interface-id"));
               if (!c_Retval)
               {
                  uint32_t u32_Value;
                  c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error("node-index", u32_Value));
                  if (!c_Retval)
                  {
                     C_OscCanInterfaceId c_InterfaceId;
                     c_InterfaceId.u32_NodeIndex = u32_Value;
                     c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error("interface-id",
                                                                                               u32_Value));
                     if (!c_Retval)
                     {
                        C_OscCanOpenManagerDeviceInfo c_DeviceInfo;
                        c_InterfaceId.u8_InterfaceNumber = static_cast<uint8_t>(u32_Value);
                        tgl_assert(orc_XmlParser.SelectNodeParent() == "can-open-device");
                        c_Retval = C_OscCanOpenManagerFiler::mh_LoadManagerSubDevice(c_DeviceInfo, orc_XmlParser,
                                                                                     orc_BasePath);
                        if (!c_Retval)
                        {
                           orc_Config[c_InterfaceId] = c_DeviceInfo;
                        }
                     }
                  }
               }
               c_NodeName = orc_XmlParser.SelectNodeNext("can-open-device");
            }
            while ((c_NodeName == "can-open-device") && (!c_Retval));
            tgl_assert(orc_XmlParser.SelectNodeParent() == "can-open-devices");
         }
         if (u32_ExpectedSize != orc_Config.size())
         {
            std::string c_Tmp;
            c_Tmp = PrintFormattedCompat("Unexpected can open device count, expected: %u, got %u", u32_ExpectedSize,
                                 static_cast<uint32_t>(orc_Config.size()));
            orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("length", c_Tmp);
         }
      }
      tgl_assert(orc_XmlParser.SelectNodeParent() == "can-open-manager");
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save manager sub devices

   \param[in]      orc_Config                Config
   \param[in,out]  orc_XmlParser             XML parser
   \param[in]      orc_BasePath              Base path
   \param[in,out]  opc_CreatedFiles          Created files
   \param[in]      orc_NodeIndicesToNameMap  Node indices to name map

   \return
   STW error codes

   \retval   Errc::success   data saved
   \retval   Errc::config    data invalid
   \retval   Errc::rd_wr     could not erase pre-existing file before saving
   \retval   Errc::rd_wr     could not write to file (e.g. missing write permissions; missing folder)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::mh_SaveManagerSubDevices(const std::map<C_OscCanInterfaceId,
                                                                   C_OscCanOpenManagerDeviceInfo> & orc_Config,
                                                                   C_OscXmlParserBase & orc_XmlParser,
                                                                   const std::string & orc_BasePath,
                                                                   std::vector<std::string> * const opc_CreatedFiles,
                                                                   const std::map<uint32_t,
                                                                   std::string> & orc_NodeIndicesToNameMap)
{
   std::error_code c_Retval = Errc::success;

   orc_XmlParser.CreateAndSelectNodeChild("can-open-devices");
   orc_XmlParser.SetAttributeUint32("length", static_cast<uint32_t>(orc_Config.size()));
   for (std::map<C_OscCanInterfaceId,
                 C_OscCanOpenManagerDeviceInfo>::const_iterator c_It = orc_Config.begin();
        (c_It != orc_Config.end()) && (!c_Retval);
        ++c_It)
   {
      const std::map<uint32_t,
                     std::string>::const_iterator c_FoundName = orc_NodeIndicesToNameMap.find(
         c_It->first.u32_NodeIndex);
      orc_XmlParser.CreateAndSelectNodeChild("can-open-device");
      orc_XmlParser.CreateAndSelectNodeChild("interface-id");
      orc_XmlParser.SetAttributeUint32("node-index", c_It->first.u32_NodeIndex);
      orc_XmlParser.SetAttributeUint32("interface-id", static_cast<uint32_t>(c_It->first.u8_InterfaceNumber));
      tgl_assert(orc_XmlParser.SelectNodeParent() == "can-open-device");
      if (c_FoundName != orc_NodeIndicesToNameMap.end())
      {
         c_Retval = C_OscCanOpenManagerFiler::mh_SaveManagerSubDevice(c_It->second, orc_XmlParser, orc_BasePath,
                                                                      opc_CreatedFiles, c_FoundName->second,
                                                                      c_It->first.u8_InterfaceNumber);
      }
      else
      {
         c_Retval = Errc::config;
         osc_write_log_error("saving canopen manager",
                             "could not find index " + std::to_string(
                                c_It->first.u32_NodeIndex) + " in parameter orc_NodeIndicesToNameMap");
      }
      tgl_assert(orc_XmlParser.SelectNodeParent() == "can-open-devices");
   }
   tgl_assert(orc_XmlParser.SelectNodeParent() == "can-open-manager");
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load manager sub device

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser
   \param[in]      orc_BasePath     Base path

   \return
   Errc::success    data read
   Errc::config     CANopen manager file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::mh_LoadManagerSubDevice(C_OscCanOpenManagerDeviceInfo & orc_Config,
                                                                  C_OscXmlParserBase & orc_XmlParser,
                                                                  const std::string & orc_BasePath)
{
   std::error_code c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError("properties"));

   if (!c_Retval)
   {
      uint32_t u32_Value;
      c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError("device-optional",
                                                                              orc_Config.q_DeviceOptional));
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError("no-initialization",
                                                                                 orc_Config.q_NoInitialization));
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError("factory-settings-active",
                                                                                 orc_Config.q_FactorySettingsActive));
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error(
            "reset-node-object-dictionary-sub-index", u32_Value));
         orc_Config.u8_ResetNodeObjectDictionarySubIndex = static_cast<uint8_t>(u32_Value);
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError(
            "enable-heartbeat-producing", orc_Config.q_EnableHeartbeatProducing));
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error("heartbeat-producer-time-ms",
                                                                                   u32_Value));
         orc_Config.u16_HeartbeatProducerTimeMs = static_cast<uint16_t>(u32_Value);
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError("use-opensyde-id",
                                                                                 orc_Config.q_UseOpenSydeNodeId));
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error("node-id-value", u32_Value));
         orc_Config.u8_NodeIdValue = static_cast<uint8_t>(u32_Value);
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError(
            "enable-heartbeat-consuming", orc_Config.q_EnableHeartbeatConsuming));
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error("heartbeat-consumer-time-ms",
                                                                                   u32_Value));
         orc_Config.u16_HeartbeatConsumerTimeMs = static_cast<uint16_t>(u32_Value);
      }
      if (!c_Retval)
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError(
            "enable-heartbeat-consuming-auto-calculation", orc_Config.q_EnableHeartbeatConsumingAutoCalculation));
      }
      if (!c_Retval)
      {
         c_Retval = C_OscCanOpenManagerFiler::mh_LoadManagerSubDeviceEdsPart(orc_Config, orc_XmlParser, orc_BasePath);
      }
      if (!c_Retval)
      {
         c_Retval = C_OscCanOpenManagerFiler::mh_LoadManagerMappedSignals(orc_Config.c_EdsFileMappableSignals,
                                                                          orc_XmlParser);
      }
      tgl_assert(orc_XmlParser.SelectNodeParent() == "can-open-device");
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save manager sub device

   \param[in]      orc_Config             Config
   \param[in,out]  orc_XmlParser          XML parser
   \param[in]      orc_BasePath           Base path
   \param[in,out]  opc_CreatedFiles       Created files
   \param[in]      orc_NodeName           Node name
   \param[in]      ou8_InterfaceNumber    Interface number

   \return
   STW error codes

   \retval   Errc::success   data saved
   \retval   Errc::rd_wr     could not erase pre-existing file before saving
   \retval   Errc::rd_wr     could not write to file (e.g. missing write permissions; missing folder)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::mh_SaveManagerSubDevice(const C_OscCanOpenManagerDeviceInfo & orc_Config,
                                                                  C_OscXmlParserBase & orc_XmlParser,
                                                                  const std::string & orc_BasePath,
                                                                  std::vector<std::string> * const opc_CreatedFiles,
                                                                  const std::string & orc_NodeName,
                                                                  const uint8_t ou8_InterfaceNumber)
{
   std::error_code c_Retval = Errc::success;

   orc_XmlParser.CreateAndSelectNodeChild("properties");
   orc_XmlParser.SetAttributeBool("device-optional", orc_Config.q_DeviceOptional);
   orc_XmlParser.SetAttributeBool("no-initialization", orc_Config.q_NoInitialization);
   orc_XmlParser.SetAttributeBool("factory-settings-active", orc_Config.q_FactorySettingsActive);
   orc_XmlParser.SetAttributeUint32("reset-node-object-dictionary-sub-index",
                                    static_cast<uint32_t>(orc_Config.u8_ResetNodeObjectDictionarySubIndex));
   orc_XmlParser.SetAttributeBool("enable-heartbeat-producing", orc_Config.q_EnableHeartbeatProducing);
   orc_XmlParser.SetAttributeUint32("heartbeat-producer-time-ms",
                                    static_cast<uint32_t>(orc_Config.u16_HeartbeatProducerTimeMs));
   orc_XmlParser.SetAttributeBool("use-opensyde-id", orc_Config.q_UseOpenSydeNodeId);
   orc_XmlParser.SetAttributeUint32("node-id-value",
                                    static_cast<uint32_t>(orc_Config.u8_NodeIdValue));
   orc_XmlParser.SetAttributeBool("enable-heartbeat-consuming", orc_Config.q_EnableHeartbeatConsuming);
   orc_XmlParser.SetAttributeUint32("heartbeat-consumer-time-ms",
                                    static_cast<uint32_t>(orc_Config.u16_HeartbeatConsumerTimeMs));
   orc_XmlParser.SetAttributeBool("enable-heartbeat-consuming-auto-calculation",
                                  orc_Config.q_EnableHeartbeatConsumingAutoCalculation);
   c_Retval = C_OscCanOpenManagerFiler::mh_SaveManagerSubDeviceEdsPart(orc_Config, orc_XmlParser, orc_BasePath,
                                                                       opc_CreatedFiles, orc_NodeName,
                                                                       ou8_InterfaceNumber);
   C_OscCanOpenManagerFiler::mh_SaveManagerMappedSignals(orc_Config.c_EdsFileMappableSignals, orc_XmlParser);
   tgl_assert(orc_XmlParser.SelectNodeParent() == "can-open-device");
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load manager sub device EDS part

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser
   \param[in]      orc_BasePath     Base path

   \return
   Errc::success    data read
   Errc::config     CANopen manager file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::mh_LoadManagerSubDeviceEdsPart(C_OscCanOpenManagerDeviceInfo & orc_Config,
                                                                         C_OscXmlParserBase & orc_XmlParser,
                                                                         const std::string & orc_BasePath)
{
   std::error_code c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError("eds-file-name"));

   if (!c_Retval)
   {
      const std::string c_EdsFileName = orc_XmlParser.GetNodeContent();
      tgl_assert(orc_XmlParser.SelectNodeParent() == "properties");

      if (orc_BasePath.empty())
      {
         c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError("eds-file-content"));
         if (!c_Retval)
         {
            //Load EDS from string: not implemented
            tgl_assert(false);
            tgl_assert(orc_XmlParser.SelectNodeParent() == "properties");
         }
      }
      else
      {
         const std::string c_CompleteFileName = C_OscSystemFilerUtil::h_CombinePaths(orc_BasePath, c_EdsFileName);
         //EDS file is only loaded on demand, remember path:
         orc_Config.c_ProjectEdsFilePath = c_CompleteFileName;

         if (TglFileExists(c_CompleteFileName) == false)
         {
            //Issue a warning if file does not exist. Do not consider a hard error. As long as the content is not needed
            // having the file available is not strictly required.
            osc_write_log_info("Loading CANopen manager data",
                               "Non existing file \"" + c_CompleteFileName +
                               "\" is referenced by CANopen manager configuration.");
         }
      }
   }
   if (!c_Retval)
   {
      c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError("eds-original-file-name"));
      orc_Config.c_OriginalEdsFileName = orc_XmlParser.GetNodeContent();
      tgl_assert(orc_XmlParser.SelectNodeParent() == "properties");
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save manager sub device EDS part

   \param[in]      orc_Config             Config
   \param[in,out]  orc_XmlParser          XML parser
   \param[in]      orc_BasePath           Base path
   \param[in,out]  opc_CreatedFiles       Created files
   \param[in]      orc_NodeName           Node name
   \param[in]      ou8_InterfaceNumber    Interface number

   \return
   STW error codes

   \retval   Errc::success   data saved
   \retval   Errc::rd_wr     could not erase pre-existing file before saving
   \retval   Errc::rd_wr     could not write to file (e.g. missing write permissions; missing folder)
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::mh_SaveManagerSubDeviceEdsPart(
   const C_OscCanOpenManagerDeviceInfo & orc_Config, C_OscXmlParserBase & orc_XmlParser,
   const std::string & orc_BasePath, std::vector<std::string> * const opc_CreatedFiles,
   const std::string & orc_NodeName, const uint8_t ou8_InterfaceNumber)
{
   std::error_code c_Retval = Errc::success;
   const std::string c_ItemPrefixUnprepared = orc_NodeName + "_can_" + std::to_string(ou8_InterfaceNumber) + "_";
   const std::string c_ItemPrefixPrepared = C_OscSystemFilerUtil::h_PrepareItemNameForFileName(c_ItemPrefixUnprepared);
   const std::string c_FileNameWithPrefix = c_ItemPrefixPrepared + orc_Config.c_OriginalEdsFileName;

   orc_XmlParser.CreateNodeChild("eds-file-name", c_FileNameWithPrefix);
   orc_XmlParser.CreateNodeChild("eds-original-file-name", orc_Config.c_OriginalEdsFileName);
   if (orc_BasePath.empty())
   {
      const C_OscCanOpenObjectDictionary & rc_EdsFileContent = orc_Config.GetEdsFileContent();
      orc_XmlParser.CreateNodeChild("eds-file-content", rc_EdsFileContent.c_TextFileContent.GetText());
   }
   else
   {
      const std::string c_CompleteFileName = C_OscSystemFilerUtil::h_CombinePaths(orc_BasePath, c_FileNameWithPrefix);

      if (c_CompleteFileName == orc_Config.c_ProjectEdsFilePath)
      {
         //The source file is the same as the target file.
         //As EDS file content cannot be changed in the GUI there's no need to use "GetEdsFileContent"
         // (which will load the file from disk if not done so yet) and re-save.
         //Just leave the file system contents as they are.
         osc_write_log_info("CANopen Manager project saving",
                            "Location of EDS file \"" + c_CompleteFileName + "\" not changed. No need to re-save.");
      }
      else
      {
         //File path has changed. e.g.:
         //* "save as"
         //* file was newly added to configuration
         const C_OscCanOpenObjectDictionary & rc_EdsFileContent = orc_Config.GetEdsFileContent();
         //only use "\n" as separator; SaveStringToFile will add an \r anyways
         c_Retval = C_OscSystemFilerUtil::h_SaveStringToFile(rc_EdsFileContent.c_TextFileContent.GetText("\n"),
                                                             c_CompleteFileName, "Saving CANopen manager data");
      }
      if (opc_CreatedFiles != nullptr)
      {
         opc_CreatedFiles->push_back(c_FileNameWithPrefix);
      }
   }

   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load manager mapped signals

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser

   \return
   Errc::success    data read
   Errc::config     CANopen manager file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::mh_LoadManagerMappedSignals(
   std::vector<C_OscCanOpenManagerMappableSignal> & orc_Config, C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = Errc::success;

   orc_Config.clear();
   if (orc_XmlParser.SelectNodeChild("mappable-signals") == "mappable-signals")
   {
      uint32_t u32_ExpectedSize;
      c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeUint32Error("length", u32_ExpectedSize));
      if (!c_Retval)
      {
         std::string c_NodeName = orc_XmlParser.SelectNodeChild("mappable-signal");
         if (c_NodeName == "mappable-signal")
         {
            orc_Config.reserve(u32_ExpectedSize);
            do
            {
               C_OscCanOpenManagerMappableSignal c_Signal;
               c_Retval = C_OscCanOpenManagerFiler::mh_LoadManagerMappedSignal(c_Signal, orc_XmlParser);
               if (!c_Retval)
               {
                  orc_Config.push_back(c_Signal);
               }

               c_NodeName = orc_XmlParser.SelectNodeNext("mappable-signal");
            }
            while ((c_NodeName == "mappable-signal") && (!c_Retval));
            tgl_assert(orc_XmlParser.SelectNodeParent() == "mappable-signals");
         }
         if (u32_ExpectedSize != orc_Config.size())
         {
            std::string c_Tmp;
            c_Tmp = PrintFormattedCompat("Unexpected mappable signal count, expected: %u, got %u", u32_ExpectedSize,
                                 static_cast<uint32_t>(orc_Config.size()));
            orc_XmlParser.ReportErrorForAttributeContentAppendXmlContext("length", c_Tmp);
         }
      }
      tgl_assert(orc_XmlParser.SelectNodeParent() == "properties");
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save manager mapped signals

   \param[in]      orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerFiler::mh_SaveManagerMappedSignals(
   const std::vector<C_OscCanOpenManagerMappableSignal> & orc_Config, C_OscXmlParserBase & orc_XmlParser)
{
   orc_XmlParser.CreateAndSelectNodeChild("mappable-signals");
   orc_XmlParser.SetAttributeUint32("length", static_cast<uint32_t>(orc_Config.size()));
   for (std::vector<C_OscCanOpenManagerMappableSignal>::const_iterator c_It = orc_Config.begin();
        c_It != orc_Config.end();
        ++c_It)
   {
      orc_XmlParser.CreateAndSelectNodeChild("mappable-signal");
      C_OscCanOpenManagerFiler::mh_SaveManagerMappedSignal(*c_It, orc_XmlParser);
      tgl_assert(orc_XmlParser.SelectNodeParent() == "mappable-signals");
   }
   tgl_assert(orc_XmlParser.SelectNodeParent() == "properties");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load manager mapped signal

   \param[in,out]  orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser

   \return
   Errc::success    data read
   Errc::config     CANopen manager file content is invalid or incomplete
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::mh_LoadManagerMappedSignal(C_OscCanOpenManagerMappableSignal & orc_Config,
                                                                     C_OscXmlParserBase & orc_XmlParser)
{
   std::error_code c_Retval = make_error_code_from_stw(orc_XmlParser.GetAttributeBoolError(
      "is-auto-min-max-used", orc_Config.q_AutoMinMaxUsed));

   if (!c_Retval)
   {
      c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError("com-signal"));
      if (!c_Retval)
      {
         c_Retval = C_OscNodeCommFiler::h_LoadNodeComSignal(orc_Config.c_SignalData, orc_XmlParser, true);
      }
      tgl_assert(orc_XmlParser.SelectNodeParent() == "mappable-signal");
   }
   if (!c_Retval)
   {
      c_Retval = make_error_code_from_stw(orc_XmlParser.SelectNodeChildError("data-element"));
      if (!c_Retval)
      {
         c_Retval = C_OscNodeDataPoolFiler::h_LoadDataPoolElement(orc_Config.c_DatapoolData, orc_XmlParser);
      }
      tgl_assert(orc_XmlParser.SelectNodeParent() == "mappable-signal");
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Save manager mapped signal

   \param[in]      orc_Config       Config
   \param[in,out]  orc_XmlParser    XML parser
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscCanOpenManagerFiler::mh_SaveManagerMappedSignal(const C_OscCanOpenManagerMappableSignal & orc_Config,
                                                          C_OscXmlParserBase & orc_XmlParser)
{
   orc_XmlParser.SetAttributeBool("is-auto-min-max-used", orc_Config.q_AutoMinMaxUsed);
   orc_XmlParser.CreateAndSelectNodeChild("com-signal");
   C_OscNodeCommFiler::h_SaveNodeComSignal(orc_Config.c_SignalData, orc_XmlParser, C_OscCanProtocol::eCAN_OPEN);
   tgl_assert(orc_XmlParser.SelectNodeParent() == "mappable-signal");
   orc_XmlParser.CreateAndSelectNodeChild("data-element");
   C_OscNodeDataPoolFiler::h_SaveDataPoolElement(orc_Config.c_DatapoolData, orc_XmlParser, C_OscNodeDataPool::eCOM);
   tgl_assert(orc_XmlParser.SelectNodeParent() == "mappable-signal");
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Can open manager info type to string

   \param[in]  ore_Type    Type

   \return
   Stringified can open manager info type
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscCanOpenManagerFiler::mh_CanOpenManagerInfoTypeToString(
   const C_OscCanOpenManagerInfo::E_NmtErrorBehaviourType & ore_Type)
{
   std::string c_Retval;

   switch (ore_Type)
   {
   case C_OscCanOpenManagerInfo::eRESTART_ALL_DEVICES:
      c_Retval = "restart all devices";
      break;
   case C_OscCanOpenManagerInfo::eRESTART_FAILURE_DEVICE:
      c_Retval = "restart failure device";
      break;
   case C_OscCanOpenManagerInfo::eSTOP_ALL_DEVICES:
      c_Retval = "stop all devices";
      break;
   default:
      c_Retval = "invalid";
      break;
   }
   return c_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  String to can open manager info type

   \param[in]   orc_String    String
   \param[out]  ore_Type      Type

   \return
   Errc::success    no error
   Errc::range      String unknown
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscCanOpenManagerFiler::mh_StringToCanOpenManagerInfoType(
   const std::string & orc_String, C_OscCanOpenManagerInfo::E_NmtErrorBehaviourType & ore_Type)
{
   std::error_code c_Retval = Errc::success;

   if (orc_String == "restart all devices")
   {
      ore_Type = C_OscCanOpenManagerInfo::eRESTART_ALL_DEVICES;
   }
   else if (orc_String == "restart failure device")
   {
      ore_Type = C_OscCanOpenManagerInfo::eRESTART_FAILURE_DEVICE;
   }
   else if (orc_String == "stop all devices")
   {
      ore_Type = C_OscCanOpenManagerInfo::eSTOP_ALL_DEVICES;
   }
   else
   {
      c_Retval = Errc::range;
   }

   return c_Retval;
}
