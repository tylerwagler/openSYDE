//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Export initialization module for DPD and DPH.

   Create a .c and .h file providing initialization structures for the OSS DPD and DPH init functions.

   \copyright   Copyright 2018 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include <system_error>

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscErrorCategory.hpp"

#include "TglFile.hpp"
#include "TglUtils.hpp"
#include <string>
#include <string>
#include <vector>
#include "C_OscExportOsyInit.hpp"
#include "C_OscExportDataPool.hpp"
#include "C_OscExportCommunicationStack.hpp"
#include "C_OscProtocolDriverOsyTpBase.hpp"
#include "C_OscLoggingHandler.hpp"
#include "C_OscExportUti.hpp"
#include "C_SclStringUtil.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace stw::scl;
using namespace stw::tgl;
using namespace stw::opensyde_core;

/* -- Module Global Constants --------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Module Global Variables --------------------------------------------------------------------------------------- */

/* -- Module Global Function Prototypes ----------------------------------------------------------------------------- */

/* -- Implementation ------------------------------------------------------------------------------------------------ */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Return filename (without extension)

   \return filename
*/
//----------------------------------------------------------------------------------------------------------------------
std::string C_OscExportOsyInit::h_GetFileName(void)
{
   return "osy_init";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create source files

   Create .c and .h files with DPD / DPH initialization.
   The specified file path is used for the ".c" file to create.
   The name of the header file is composed by replacing the extension by ".h".
   Existing files will be replaced if possible.

   \param[in] orc_FilePath             path to file to create
   \param[in] orc_Node                 node definition
   \param[in] oq_RunsDpd               true: create DPD init code and init code for all local, public and remote DPs
                                       false: only create init code for local and public DPs
   \param[in] ou16_ApplicationIndex    index of application we create code for (to identify local DPs)
   \param[in] orc_ExportToolInfo       information about calling executable (name + version)

   \return
   Errc::success  Operation success
   Errc::rd_wr    Operation failure: cannot store files
*/
//----------------------------------------------------------------------------------------------------------------------
std::error_code C_OscExportOsyInit::h_CreateSourceCode(const std::string & orc_FilePath, const C_OscNode & orc_Node,
                                                       const bool oq_RunsDpd, const uint16_t ou16_ApplicationIndex,
                                                       const std::string & orc_ExportToolInfo)
{
   std::vector<std::string> c_Lines;
   std::error_code c_Return = Errc::success;
   uint8_t u8_DataPoolsKnownInThisApplication = 0U;
   uint8_t u8_CommDefinitionsKnownInThisApplication = 0U;
   uint8_t u8_NumCanChannels = 0U;
   uint8_t u8_NumEthChannels = 0U;
   uint32_t u32_CommProtocolCnt = 0U;

   //header file: quite simple (constant only as long as we always create DPD and DPH structures):
   c_Lines.push_back(C_OscExportUti::h_GetHeaderSeparator());
   c_Lines.push_back("/*!");
   c_Lines.push_back("   \\file");
   c_Lines.push_back("   \\brief       Application specific openSYDE initialization (Header file with interface)");
   c_Lines.push_back("");
   c_Lines.push_back(C_OscExportUti::h_GetCreationToolInfo(orc_ExportToolInfo));
   c_Lines.push_back("*/");
   c_Lines.push_back(C_OscExportUti::h_GetHeaderSeparator());
    c_Lines.push_back("#ifndef " + UpperCaseCompat(h_GetFileName()) + "H");
    c_Lines.push_back("#define " + UpperCaseCompat(h_GetFileName()) + "H");
   c_Lines.push_back("");
   c_Lines.push_back(C_OscExportUti::h_GetSectionSeparator("Includes"));
   c_Lines.push_back("#include \"stwtypes.h\"");
   if (oq_RunsDpd == true)
   {
      c_Lines.push_back("#include \"osy_dpd_driver.h\"");
   }
   c_Lines.push_back("#include \"osy_dpa_data_pool.h\"");
   //includes for the Datapool definitions
   //adding them to this central header allows the application to just include one central file and access all data
   // pools
   c_Lines.push_back("//Header files exporting application specific Datapools:");
   for (uint8_t u8_DataPool = 0U; u8_DataPool < orc_Node.c_DataPools.size(); u8_DataPool++)
   {
      if (mh_IsDpKnownToApp(u8_DataPool, ou16_ApplicationIndex, orc_Node, oq_RunsDpd) == true)
      {
         std::string c_HeaderName;
         c_HeaderName = C_OscExportDataPool::h_GetFileName(orc_Node.c_DataPools[u8_DataPool]);
         c_Lines.push_back("#include \"" + c_HeaderName + ".h\"");
         u8_DataPoolsKnownInThisApplication++;
      }
   }
   //count how many node protocols are NOT CANopen and how many ARE CANopen
   for (uint32_t u32_ProtIt = 0U; u32_ProtIt < orc_Node.c_ComProtocols.size(); ++u32_ProtIt)
   {
      if (orc_Node.c_ComProtocols[u32_ProtIt].e_Type != C_OscCanProtocol::eCAN_OPEN)
      {
         u32_CommProtocolCnt++;
      }
   }
   //includes for comm stack configuration
   //these are not needed by this module at all; they are only provided for application convenience
   //only add includes if there is at least one COMM Protocol which is NOT CANopen
   if ((orc_Node.c_ComProtocols.size() > 0U) && (u32_CommProtocolCnt > 0U))
   {
      c_Lines.push_back("//Header files exporting comm stack configuration and status:");

      for (uint32_t u32_ItProtocol = 0U; u32_ItProtocol < orc_Node.c_ComProtocols.size(); u32_ItProtocol++)
      {
         const C_OscCanProtocol & rc_Protocol = orc_Node.c_ComProtocols[u32_ItProtocol];

         //skip CANopen protocol
         if (rc_Protocol.e_Type != C_OscCanProtocol::eCAN_OPEN)
         {
            //logic: C_OscCanProtocol refers to a Datapool; if that Datapool is owned by the
            // C_OscNodeApplication then create it
            if (orc_Node.c_DataPools[rc_Protocol.u32_DataPoolIndex].s32_RelatedDataBlockIndex == ou16_ApplicationIndex)
            {
               for (uint32_t u32_ItInterface = 0U; u32_ItInterface < rc_Protocol.c_ComMessages.size();
                    u32_ItInterface++)
               {
                  //at least one message defined ?
                  if (rc_Protocol.c_ComMessages[u32_ItInterface].ContainsAtLeastOneMessage() == true)
                  {
                     const std::string c_HeaderName =
                        C_OscExportCommunicationStack::h_GetFileName(static_cast<uint8_t>(u32_ItInterface),
                                                                     rc_Protocol.e_Type);
                     c_Lines.push_back("#include \"" + c_HeaderName + ".h\"");
                     u8_CommDefinitionsKnownInThisApplication++;
                  }
               }
            }
         }
      }
   }

   c_Lines.push_back("");
   C_OscExportUti::h_AddExternCeStart(c_Lines);
   c_Lines.push_back(C_OscExportUti::h_GetSectionSeparator("Defines"));

   if (oq_RunsDpd == true)
   {
      uint32_t u32_BufferSize;

      //count number of active CAN and ETH channels:
      //CAN and Ethernet bus number
      for (uint8_t u8_Channel = 0U; u8_Channel < orc_Node.c_Properties.c_ComInterfaces.size(); u8_Channel++)
      {
         const C_OscNodeComInterfaceSettings & rc_ComIf = orc_Node.c_Properties.c_ComInterfaces[u8_Channel];

         if (C_OscExportOsyInit::mh_IsDpdInitRequired(rc_ComIf) == true)
         {
            switch (rc_ComIf.e_InterfaceType)
            {
            case C_OscSystemBus::eCAN:
               c_Lines.push_back("#define OSY_INIT_DPD_BUS_NUMBER_CAN_CHANNEL_" +
                           std::to_string(u8_NumCanChannels) + "       " +
                           std::to_string(rc_ComIf.u8_InterfaceNumber) + "U");
               u8_NumCanChannels++;
               break;
            case C_OscSystemBus::eETHERNET:
               c_Lines.push_back("#define OSY_INIT_DPD_BUS_NUMBER_ETHERNET_CHANNEL_" +
                           std::to_string(u8_NumEthChannels) + "  " +
                           std::to_string(rc_ComIf.u8_InterfaceNumber) + "U");
               u8_NumEthChannels++;
               break;
            default:
               tgl_assert(false); //oh for %$%$"§% sake
               break;
            }
         }
      }

      c_Lines.push_back("#define OSY_INIT_DPD_NUMBER_OF_CAN_CHANNELS         " + std::to_string(u8_NumCanChannels) +
                  "U");
      c_Lines.push_back("#define OSY_INIT_DPD_NUMBER_OF_ETHERNET_CHANNELS    " + std::to_string(u8_NumEthChannels) +
                  "U");
      c_Lines.push_back("");
      c_Lines.push_back("#define OSY_INIT_DPD_NUMBER_OF_PARALLEL_CONNECTIONS " +
                  std::to_string(orc_Node.c_Properties.c_OpenSydeServerSettings.u8_MaxClients) + "U");
      c_Lines.push_back("#define OSY_INIT_DPD_CAN_FIFO_SIZE_TX               " +
                  std::to_string(orc_Node.c_Properties.c_OpenSydeServerSettings.u16_MaxMessageBufferTx) + "U");
      c_Lines.push_back("#define OSY_INIT_DPD_CAN_ROUTING_FIFO_SIZE_RX       " +
                  std::to_string(orc_Node.c_Properties.c_OpenSydeServerSettings.u16_MaxRoutingMessageBufferRx) +
                  "U");

      u32_BufferSize = orc_Node.c_Properties.c_OpenSydeServerSettings.GetTransportBufferSizeInByte(
         orc_Node.c_DataPools,
         C_OscProtocolDriverOsyTpBase::hu16_OSY_MAXIMUM_SERVICE_SIZE);

      c_Lines.push_back("#define OSY_INIT_DPD_BUF_SIZE_INSTANCE              " + std::to_string(u32_BufferSize) + "U");
      c_Lines.push_back("#define OSY_INIT_DPD_MAX_NUM_CYCLIC_TRANSMISSIONS   " +
                  std::to_string(orc_Node.c_Properties.c_OpenSydeServerSettings.u8_MaxParallelTransmissions) +
                  "U");
      c_Lines.push_back("");
   }

   c_Lines.push_back("#define OSY_INIT_DPH_NUM_DATA_POOLS                 " +
               std::to_string(u8_DataPoolsKnownInThisApplication) + "U");
   c_Lines.push_back("");
   //add the next constant even if there are no COMM protocols; just placing the define does not create an external
   // dependency
   c_Lines.push_back("#define OSY_INIT_COM_NUM_PROTOCOL_CONFIGURATIONS    " +
               std::to_string(u8_CommDefinitionsKnownInThisApplication) + "U");
   c_Lines.push_back("");
   c_Lines.push_back(C_OscExportUti::h_GetSectionSeparator("Types"));
   c_Lines.push_back("");
   c_Lines.push_back(C_OscExportUti::h_GetSectionSeparator("Global Variables"));
   c_Lines.push_back("");
   c_Lines.push_back(C_OscExportUti::h_GetSectionSeparator("Function Prototypes"));
   if (oq_RunsDpd == true)
   {
      c_Lines.push_back("extern const T_osy_dpd_data * osy_dpd_get_init_config(void);");
   }
   c_Lines.push_back("extern const T_osy_dpa_data_pool * const * osy_dph_get_init_config(void);");
   c_Lines.push_back("extern uint8 osy_dph_get_num_data_pools(void);");
   c_Lines.push_back("");

   //prototypes for COMM utility functions; only place if there are COMM protocols to prevent external dependencies
   // (on type definitions)
   if ((u8_CommDefinitionsKnownInThisApplication > 0) && (u32_CommProtocolCnt > 0))
   {
      c_Lines.push_back("extern const T_osy_com_protocol_configuration * const * osy_com_get_protocol_configs(void);");
      c_Lines.push_back("extern uint8 osy_com_get_num_protocol_configs(void);");
      c_Lines.push_back("");
   }

   c_Lines.push_back(C_OscExportUti::h_GetSectionSeparator("Implementation"));
   c_Lines.push_back("");
   C_OscExportUti::h_AddExternCeEnd(c_Lines);
   c_Lines.push_back("#endif");

   // finally save all stuff into the file
   c_Return = C_OscExportUti::h_SaveToFile(c_Lines, orc_FilePath, h_GetFileName(), true);

   if (!c_Return)
   {
      //now for the c file:
      c_Lines.clear();

      //constant header part:
      c_Lines.push_back(C_OscExportUti::h_GetHeaderSeparator());
      c_Lines.push_back("/*!");
      c_Lines.push_back("   \\file");
      c_Lines.push_back("   \\brief       Application specific openSYDE initialization (Source file with implementation)");
      c_Lines.push_back("");
      c_Lines.push_back(C_OscExportUti::h_GetCreationToolInfo(orc_ExportToolInfo));
      c_Lines.push_back("*/");
      c_Lines.push_back(C_OscExportUti::h_GetHeaderSeparator());
      c_Lines.push_back("");
      c_Lines.push_back(C_OscExportUti::h_GetSectionSeparator("Includes"));
      c_Lines.push_back("#include <stddef.h> //for NULL");
      c_Lines.push_back("#include \"stwtypes.h\"");
      c_Lines.push_back("#include \"" + h_GetFileName() + ".h\"");
      c_Lines.push_back("#include \"osy_dpa_data_pool.h\"");
      c_Lines.push_back("");
      c_Lines.push_back(C_OscExportUti::h_GetSectionSeparator("Defines"));
      c_Lines.push_back("");
      c_Lines.push_back(C_OscExportUti::h_GetSectionSeparator("Types"));
      c_Lines.push_back("");
      c_Lines.push_back(C_OscExportUti::h_GetSectionSeparator("Global Variables"));
      c_Lines.push_back("");
      c_Lines.push_back(C_OscExportUti::h_GetSectionSeparator("Module Global Variables"));
      c_Lines.push_back("");
      c_Lines.push_back(C_OscExportUti::h_GetSectionSeparator("Module Global Function Prototypes"));
      c_Lines.push_back("");
      c_Lines.push_back(C_OscExportUti::h_GetSectionSeparator("Implementation"));

      if (oq_RunsDpd == true)
      {
         c_Lines.push_back("");
         c_Lines.push_back(C_OscExportUti::h_GetHeaderSeparator());
         c_Lines.push_back("/*! \\brief   Set up and provide openSYDE protocol driver configuration");
         c_Lines.push_back("");
         c_Lines.push_back("   Sets up:");
         c_Lines.push_back("   * CAN channel configuration");
         c_Lines.push_back("   * Ethernet channel configuration");
         c_Lines.push_back("   * Connection buffer definition");
         c_Lines.push_back("   * Main initialization structure");
         c_Lines.push_back("");
         c_Lines.push_back("   \\return");
         c_Lines.push_back(
            "   pointer to configuration structure (statically available; can be used for the DPD initialization function)");
         c_Lines.push_back("*/");
         c_Lines.push_back(C_OscExportUti::h_GetHeaderSeparator());
         c_Lines.push_back("const T_osy_dpd_data * osy_dpd_get_init_config(void)");
         c_Lines.push_back("{");

         //channel instances
         u8_NumCanChannels = 0U;
         u8_NumEthChannels = 0U;
         for (uint8_t u8_Channel = 0U; u8_Channel < orc_Node.c_Properties.c_ComInterfaces.size(); u8_Channel++)
         {
            const C_OscNodeComInterfaceSettings & rc_ComIf = orc_Node.c_Properties.c_ComInterfaces[u8_Channel];
            //create initialization if
            //* the bus is connected and
            //** routing is enabled (in this case we need to be able to route on application level as well)
            //** or: update is enabled (in this case we need to be able to perform the reset from application level
            //       as well)
            //** or: diagnostic is enabled
            if (C_OscExportOsyInit::mh_IsDpdInitRequired(rc_ComIf) == true)
            {
               if (rc_ComIf.e_InterfaceType == C_OscSystemBus::eCAN)
               {
                  c_Lines.push_back("   OSY_DPD_CAN_CHANNEL(ht_CanInitConfiguration" +
                              std::to_string(u8_NumCanChannels) +
                              ", OSY_INIT_DPD_BUS_NUMBER_CAN_CHANNEL_" + std::to_string(u8_NumCanChannels) +
                              ",");
                  c_Lines.push_back(
                     "                       OSY_INIT_DPD_NUMBER_OF_PARALLEL_CONNECTIONS, OSY_INIT_DPD_NUMBER_OF_PARALLEL_CONNECTIONS,");
                  c_Lines.push_back("                       OSY_INIT_DPD_BUF_SIZE_INSTANCE,");
                  c_Lines.push_back(
                     "                       OSY_INIT_DPD_CAN_ROUTING_FIFO_SIZE_RX, OSY_INIT_DPD_CAN_FIFO_SIZE_TX)");
                  u8_NumCanChannels++;
               }
               else
               {
                  c_Lines.push_back("   OSY_DPD_ETH_CHANNEL(ht_EthernetInitConfiguration" +
                              std::to_string(u8_NumEthChannels) + ", OSY_INIT_DPD_BUS_NUMBER_ETHERNET_CHANNEL_" +
                              std::to_string(u8_NumEthChannels) + ",");
                  c_Lines.push_back(
                     "                       OSY_INIT_DPD_NUMBER_OF_PARALLEL_CONNECTIONS, OSY_INIT_DPD_NUMBER_OF_PARALLEL_CONNECTIONS,");
                  c_Lines.push_back("                       OSY_INIT_DPD_BUF_SIZE_INSTANCE)");
                  u8_NumEthChannels++;
               }
            }
         }

         //channel lists
         c_Lines.push_back("");
         if (u8_NumCanChannels > 0)
         {
            c_Lines.push_back("   static const T_osy_udc_global_cantp_init_configuration * const");
            c_Lines.push_back("      hapt_CanInitConfigurations[OSY_INIT_DPD_NUMBER_OF_CAN_CHANNELS] =");
            c_Lines.push_back("   {");
            for (uint8_t u8_Channel = 0U; u8_Channel < u8_NumCanChannels; u8_Channel++)
            {
               std::string c_Text = "      &ht_CanInitConfiguration" + std::to_string(u8_Channel);
               if (u8_Channel != (u8_NumCanChannels - 1))
               {
                  c_Text += ",";
               }
               c_Lines.push_back(c_Text);
            }
            c_Lines.push_back("   };");
         }
         c_Lines.push_back("");
         if (u8_NumEthChannels > 0)
         {
            c_Lines.push_back("   static const T_osy_udc_global_ethertp_init_configuration * const");
            c_Lines.push_back("      hapt_EthernetInitConfigurations[OSY_INIT_DPD_NUMBER_OF_ETHERNET_CHANNELS] =");
            c_Lines.push_back("   {");
            for (uint8_t u8_Channel = 0U; u8_Channel < u8_NumEthChannels; u8_Channel++)
            {
               std::string c_Text = "      &ht_EthernetInitConfiguration" + std::to_string(u8_Channel);
               if (u8_Channel != (u8_NumEthChannels - 1))
               {
                  c_Text += ",";
               }
               c_Lines.push_back(c_Text);
            }
            c_Lines.push_back("   };");
         }
         c_Lines.push_back("");
         //connection instances
         for (uint8_t u8_Instance = 0U; u8_Instance < orc_Node.c_Properties.c_OpenSydeServerSettings.u8_MaxClients;
              u8_Instance++)
         {
            c_Lines.push_back("   OSY_DPD_CONNECTION_INSTANCE_INIT(ht_DpdConnectionInstance" +
                        std::to_string(u8_Instance) + ", " + std::to_string(u8_Instance) + "U, " +
                        "OSY_INIT_DPD_MAX_NUM_CYCLIC_TRANSMISSIONS)");
         }
         c_Lines.push_back("");
         c_Lines.push_back(
            "   static T_osy_dpd_connection_instance * const hapt_DpdConnections[OSY_INIT_DPD_NUMBER_OF_PARALLEL_CONNECTIONS] =");
         c_Lines.push_back("   {");
         for (uint8_t u8_Instance = 0U; u8_Instance < orc_Node.c_Properties.c_OpenSydeServerSettings.u8_MaxClients;
              u8_Instance++)
         {
            std::string c_Text = "      &ht_DpdConnectionInstance" + std::to_string(u8_Instance);
            if (u8_Instance != (orc_Node.c_Properties.c_OpenSydeServerSettings.u8_MaxClients - 1))
            {
               c_Text += ",";
            }
            c_Lines.push_back(c_Text);
         }
         c_Lines.push_back("   };");
         c_Lines.push_back("");

         c_Lines.push_back("   OSY_DPD_GLOBAL_DATA_INIT(ht_DpdDataInstance,");
         c_Lines.push_back("                            OSY_INIT_DPD_NUMBER_OF_CAN_CHANNELS,");
         c_Lines.push_back("                            OSY_INIT_DPD_NUMBER_OF_ETHERNET_CHANNELS,");
         c_Lines.push_back("                            OSY_INIT_DPD_NUMBER_OF_PARALLEL_CONNECTIONS,");
         c_Lines.push_back("                            &hapt_DpdConnections[0],");
         if (u8_NumCanChannels > 0)
         {
            c_Lines.push_back("                            &hapt_CanInitConfigurations[0],");
         }
         else
         {
            c_Lines.push_back("                            NULL,");
         }
         if (u8_NumEthChannels > 0)
         {
            c_Lines.push_back("                            &hapt_EthernetInitConfigurations[0])");
         }
         else
         {
            c_Lines.push_back("                            NULL)");
         }
         c_Lines.push_back("");
         c_Lines.push_back("   return &ht_DpdDataInstance;");
         c_Lines.push_back("}");
      }
      c_Lines.push_back("");
      c_Lines.push_back(C_OscExportUti::h_GetHeaderSeparator());
      c_Lines.push_back("/*! \\brief   Set up and provide openSYDE Datapool handler configuration");
      c_Lines.push_back("");
      c_Lines.push_back("   Sets up:");
      c_Lines.push_back("   * initialization structure listing all Datapools in correct sequence");
      c_Lines.push_back("");
      c_Lines.push_back("   \\return");
      c_Lines.push_back(
         "   pointer to initialization structure (statically available; can be used for the DPH initialization function)");
      c_Lines.push_back("   NULL: no Datapools defined");
      c_Lines.push_back("*/");
      c_Lines.push_back(C_OscExportUti::h_GetHeaderSeparator());
      c_Lines.push_back("const T_osy_dpa_data_pool * const * osy_dph_get_init_config(void)");
      c_Lines.push_back("{");

      //add table of Datapools
      if (u8_DataPoolsKnownInThisApplication == 0U)
      {
         c_Lines.push_back("   return NULL;");
      }
      else
      {
         c_Lines.push_back("   static const T_osy_dpa_data_pool * const hapt_Datapools[OSY_INIT_DPH_NUM_DATA_POOLS] =");
         c_Lines.push_back("   {");
         for (uint8_t u8_DataPool = 0U; u8_DataPool < orc_Node.c_DataPools.size(); u8_DataPool++)
         {
            if (mh_IsDpKnownToApp(u8_DataPool, ou16_ApplicationIndex, orc_Node, oq_RunsDpd) == true)
            {
               const std::string c_Text = "      &gt_" + orc_Node.c_DataPools[u8_DataPool].c_Name + "_DataPool,";
               c_Lines.push_back(c_Text);
            }
         }
         //remove final ",":
         DeleteCompat(c_Lines[static_cast<int32_t>(c_Lines.size()) - 1],
            c_Lines[static_cast<int32_t>(c_Lines.size()) - 1].length(), 1U);

         c_Lines.push_back("   };");
         c_Lines.push_back("");
         c_Lines.push_back("   return &hapt_Datapools[0];");
      }
      c_Lines.push_back("}");
      c_Lines.push_back("");
      c_Lines.push_back(C_OscExportUti::h_GetHeaderSeparator());
      c_Lines.push_back("/*! \\brief   Get number of defined openSYDE Datapools");
      c_Lines.push_back("");
      c_Lines.push_back("   \\return");
      c_Lines.push_back("   number of openSYDE Datapools");
      c_Lines.push_back("*/");
      c_Lines.push_back(C_OscExportUti::h_GetHeaderSeparator());
      c_Lines.push_back("uint8 osy_dph_get_num_data_pools(void)");
      c_Lines.push_back("{");
      c_Lines.push_back("   return OSY_INIT_DPH_NUM_DATA_POOLS;");
      c_Lines.push_back("}");

      //only add function implementations if there is at least one COMM Protocol which is NOT CANopen
      if ((u8_CommDefinitionsKnownInThisApplication > 0) && (u32_CommProtocolCnt > 0))
      {
         c_Lines.push_back("");
         c_Lines.push_back(C_OscExportUti::h_GetHeaderSeparator());
         c_Lines.push_back("/*! \\brief   Set up and provide a list of openSYDE COMM protocol configurations");
         c_Lines.push_back("");
         c_Lines.push_back("   Sets up a table with pointers to all defined COMM protocol configurations");
         c_Lines.push_back("");
         c_Lines.push_back("   \\return");
         c_Lines.push_back("   pointer to table of configurations (statically available)");
         c_Lines.push_back("*/");
         c_Lines.push_back(C_OscExportUti::h_GetHeaderSeparator());
         c_Lines.push_back("const T_osy_com_protocol_configuration * const * osy_com_get_protocol_configs(void)");
         c_Lines.push_back("{");
         c_Lines.push_back("   static const T_osy_com_protocol_configuration * const");
         c_Lines.push_back("      hapt_CommConfigurations[OSY_INIT_COM_NUM_PROTOCOL_CONFIGURATIONS] =");
         c_Lines.push_back("   {");

         for (uint32_t u32_Protocol = 0U; u32_Protocol < orc_Node.c_ComProtocols.size(); u32_Protocol++)
         {
            bool oq_Skip = false;
            const C_OscCanProtocol & rc_Protocol = orc_Node.c_ComProtocols[u32_Protocol];

            //skip any CANopen protocol
            if (rc_Protocol.e_Type == C_OscCanProtocol::eCAN_OPEN)
            {
               oq_Skip = true;
            }

            if (oq_Skip == false)
            {
               //logic: C_OscCanProtocol refers to a Datapool; if that Datapool is owned by the
               // C_OscNodeApplication then this node knows about the protocol
               if (orc_Node.c_DataPools[rc_Protocol.u32_DataPoolIndex].s32_RelatedDataBlockIndex ==
                   ou16_ApplicationIndex)
               {
                  for (uint8_t u8_Interface = 0U; u8_Interface < rc_Protocol.c_ComMessages.size(); u8_Interface++)
                  {
                     //at least one message defined ?
                     if (rc_Protocol.c_ComMessages[u8_Interface].ContainsAtLeastOneMessage() == true)
                     {
                        //finally we have a winner ...
                        const std::string c_Text = "      &" + C_OscExportCommunicationStack::h_GetConfigurationName(
                           u8_Interface, rc_Protocol.e_Type) + ",";

                        c_Lines.push_back(c_Text);
                     }
                  }
               }
            }
         }
         //remove final ",":
         DeleteCompat(c_Lines[static_cast<int32_t>(c_Lines.size()) - 1],
            c_Lines[static_cast<int32_t>(c_Lines.size()) - 1].length(), 1U);
         c_Lines.push_back("   };");
         c_Lines.push_back("");
         c_Lines.push_back("   return &hapt_CommConfigurations[0];");
         c_Lines.push_back("}");
         c_Lines.push_back("");
         c_Lines.push_back(C_OscExportUti::h_GetHeaderSeparator());
         c_Lines.push_back("/*! \\brief   Get number of defined openSYDE COMM protocol configurations");
         c_Lines.push_back("");
         c_Lines.push_back(
            "   The returned value matches the number of elements in the table returned by osy_com_get_protocol_configs.");
         c_Lines.push_back("");
         c_Lines.push_back("   \\return");
         c_Lines.push_back("   number of openSYDE Datapools");
         c_Lines.push_back("*/");
         c_Lines.push_back(C_OscExportUti::h_GetHeaderSeparator());
         c_Lines.push_back("uint8 osy_com_get_num_protocol_configs(void)");
         c_Lines.push_back("{");
         c_Lines.push_back("   return OSY_INIT_COM_NUM_PROTOCOL_CONFIGURATIONS;");
         c_Lines.push_back("}");
      }

      // finally save all stuff into the file
      c_Return = C_OscExportUti::h_SaveToFile(c_Lines, orc_FilePath, h_GetFileName(), false);
   }

   return c_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Utility: check whether DPD initialization needs to be performed

   Logic:
   Initialization is required if
   * the bus is connected and (
   ** routing is enabled (in this case we need to be able to route on application level as well)
   ** or: update is enabled (in this case we need to be able to perform the reset from application level as well)
   ** or: diagnostic is enabled )

   \param[in] orc_Settings            interface configuration

   \return
   true: DPD needs to be initialized on interface
   false: DPD does not need to be initialized on interface
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscExportOsyInit::mh_IsDpdInitRequired(const C_OscNodeComInterfaceSettings & orc_Settings)
{
   return ((orc_Settings.GetBusConnected() == true) &&
           ((orc_Settings.q_IsDiagnosisEnabled == true) ||
            (orc_Settings.q_IsRoutingEnabled == true) ||
            (orc_Settings.q_IsUpdateEnabled == true)));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Utility: Check if a Datapool is known by a specific application.

   A Datapool is "known" by a specific application if it is
      - not empty i.e. has at least one list containing at least one element and
      - Datapool is owned by Application ("local Datapool") or Datapool runs DPD ("remote Datapool") or
        Datapool is public ("public remote Datapool")

   \param[in]       ou8_DataPoolIndex        datapool index
   \param[in]       ou16_ApplicationIndex    application index
   \param[in]       orc_Node                 system definition node
   \param[in]       oq_RunsDpd               application runs DPD

   \retval  true     Datapool is known to the specified app
   \retval  false    Datapool is not known to the specified app (e.g. owned by another application)
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscExportOsyInit::mh_IsDpKnownToApp(const uint8_t ou8_DataPoolIndex, const uint16_t ou16_ApplicationIndex,
                                           const C_OscNode & orc_Node, const bool oq_RunsDpd)
{
   bool q_Return = false;

   if (ou8_DataPoolIndex < orc_Node.c_DataPools.size())
   {
      const C_OscNodeDataPool & rc_DataPool = orc_Node.c_DataPools[ou8_DataPoolIndex];
      bool q_AtLeastOneElement = false;

      // check if datapool is non-empty (files only get generated in this case)
      for (uint16_t u16_ListIndex = 0U; u16_ListIndex < rc_DataPool.c_Lists.size(); u16_ListIndex++)
      {
         const C_OscNodeDataPoolList & rc_List = rc_DataPool.c_Lists[u16_ListIndex];
         if (rc_List.c_Elements.size() != 0)
         {
            q_AtLeastOneElement = true;
            break;
         }
      }

      if (q_AtLeastOneElement == true)
      {
         // check if application runs DPD or application owns this Datapool or Datapool is public
         if ((oq_RunsDpd == true) || (rc_DataPool.s32_RelatedDataBlockIndex == ou16_ApplicationIndex) ||
             ((orc_Node.c_Applications[ou16_ApplicationIndex].u16_GenCodeVersion >= 4U) &&
              (rc_DataPool.q_ScopeIsPrivate == false)))
         {
            q_Return = true;
         }
      }
   }

   return q_Return;
}
