//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Export initialization module for CANopen

   Create a .c and .h file providing initialization structures for OSS CANopen init functions

   \copyright   Copyright 2022 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "precomp_headers.hpp"

#include "stwtypes.hpp"
#include "stwerrors.hpp"
#include "C_OscExportCanOpenInit.hpp"
#include "C_OscExportUti.hpp"

/* -- Used Namespaces ----------------------------------------------------------------------------------------------- */

using namespace stw::errors;
using namespace std;
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
QString C_OscExportCanOpenInit::h_GetFileName()
{
   return "osco_man_config_init";
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Create source files

   Create .c and .h files with CANopen initialization.

   \param[in]       orc_FilePath              path to file which will be created
   \param[in]       orc_Node                  node definition
   \param[in]       orc_IfWithCanOpenMan      vector with all interface indices, which contain a CANopen manager
   \param[in]       orc_ExportToolInfo        information about calling executable (name + version)

   \return
   C_NO_ERR Operation success
   C_RD_WR  Operation failure: cannot store files
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscExportCanOpenInit::h_CreateSourceCode(const QString & orc_FilePath, const C_OscNode & orc_Node,
                                                   const std::vector<uint8_t> & orc_IfWithCanOpenMan,
                                                   const QString & orc_ExportToolInfo)
{
   int32_t s32_Return;

   QStringList c_Data;
   QString c_DefineValue;
   QString c_Subject;

   //header file:
   c_Data.append(C_OscExportUti::h_GetHeaderSeparator());
   c_Data.append("/*!");
   c_Data.append("   \\file");
   c_Data.append("   \\brief       Application specific openSYDE initialization (Header file with interface)");
   c_Data.append("");
   c_Data.append(C_OscExportUti::h_GetCreationToolInfo(orc_ExportToolInfo));
   c_Data.append("*/");
   c_Data.append(C_OscExportUti::h_GetHeaderSeparator());
   c_Data.append("#ifndef " + h_GetFileName().toUpper() + "H");
   c_Data.append("#define " + h_GetFileName().toUpper() + "H");
   c_Data.append("");
   c_Data.append(C_OscExportUti::h_GetSectionSeparator("Includes"));
   c_Data.append("#include \"stwtypes.h\"");
   c_Data.append("#include \"osco_configuration.h\"");
   c_Data.append("");
   c_Data.append("//Header files exporting application specific CANopen Manager interface configurations:");

   for (uint32_t u32_IfIt = 0; u32_IfIt < orc_IfWithCanOpenMan.size(); ++u32_IfIt)
   {
      const uint32_t u32_InterfaceIndex = static_cast<uint32_t>(orc_IfWithCanOpenMan[u32_IfIt]);
      c_Data.append("#include \"osco_man_config_can" + QString::number(u32_InterfaceIndex + 1U) + ".h\"");
   }

   c_Data.append("");
   C_OscExportUti::h_AddExternCeStart(c_Data);
   c_Data.append(C_OscExportUti::h_GetSectionSeparator("Defines"));
   c_Data.append("");

   c_Data.append("//Indexes of configurations");
   for (uint32_t u32_IfIt = 0; u32_IfIt < orc_IfWithCanOpenMan.size(); ++u32_IfIt)
   {
      const uint32_t u32_InterfaceIndex = static_cast<uint32_t>(orc_IfWithCanOpenMan[u32_IfIt]);
      c_Data.append("#define OSCO_MAN_CONFIG_INIT_CONFIG_INDEX_CAN" + QString::number(
                       u32_InterfaceIndex + 1U) + " (" + QString::number(u32_IfIt) + "U)");
   }

   c_Data.append("#define OSCO_MAN_CONFIG_INIT_NUM_CONFIGURATIONS (" +
                 QString::number(orc_Node.c_CanOpenManagers.size()) + "U)");
   c_Data.append("");

   //put together the string for define "total number of TX PDOs"
   c_Subject = "PDOS";
   c_DefineValue = "#define OSCO_MAN_CONFIG_INIT_NUM_TX_PDOS    (";
   mh_ComposeDefineNumTotal(c_DefineValue, orc_IfWithCanOpenMan, true, c_Subject);
   //add comment to line
   c_DefineValue += ") //total number of all TX PDOs";
   c_Data.append(c_DefineValue);

   //put together the string for define "total number of RX PDOs"
   c_DefineValue = "#define OSCO_MAN_CONFIG_INIT_NUM_RX_PDOS    (";
   mh_ComposeDefineNumTotal(c_DefineValue, orc_IfWithCanOpenMan, false, c_Subject);
   //add comment to line
   c_DefineValue += ") //total number of all RX PDOs";
   c_Data.append(c_DefineValue);

   c_Data.append(
      "#define OSCO_MAN_CONFIG_INIT_NUM_PDOS       (OSCO_MAN_CONFIG_INIT_NUM_TX_PDOS + OSCO_MAN_CONFIG_INIT_NUM_RX_PDOS)");

   //put together string for define "total number of mapping in all TX PDOs"
   c_Subject = "SIGNALS";
   c_DefineValue = "#define OSCO_MAN_CONFIG_INIT_NUM_TX_SIGNALS (";
   mh_ComposeDefineNumTotal(c_DefineValue, orc_IfWithCanOpenMan, true, c_Subject);
   //add comment to line
   c_DefineValue += ") //total number of mappings in all TX PDOs";
   c_Data.append(c_DefineValue);

   //put together string for define "total number of mapping in all RX PDOs"
   c_DefineValue = "#define OSCO_MAN_CONFIG_INIT_NUM_RX_SIGNALS (";
   mh_ComposeDefineNumTotal(c_DefineValue, orc_IfWithCanOpenMan, false, c_Subject);
   //add comment to line
   c_DefineValue += ") //total number of mappings in all RX PDOs";
   c_Data.append(c_DefineValue);

   c_Data.append(
      "#define OSCO_MAN_CONFIG_INIT_NUM_SIGNALS    (OSCO_MAN_CONFIG_INIT_NUM_TX_SIGNALS + OSCO_MAN_CONFIG_INIT_NUM_RX_SIGNALS)");

   //put together string for define "total number of devices"
   c_DefineValue = "#define OSCO_MAN_CONFIG_INIT_NUM_DEVICES    (";
   mh_ComposeDefineNumDevices(c_DefineValue, orc_IfWithCanOpenMan);
   //add comment to line
   c_DefineValue += ") //total number of devices in all configurations";
   c_Data.append(c_DefineValue);

   c_Data.append("");
   c_Data.append(C_OscExportUti::h_GetSectionSeparator("Types"));
   c_Data.append("");
   c_Data.append(C_OscExportUti::h_GetSectionSeparator("Global Variables"));
   c_Data.append("///Stack configuration");
   c_Data.append("extern const T_osco_man_manager_init_configuration gt_osco_man_InitConfiguration;");
   c_Data.append("");
   c_Data.append(C_OscExportUti::h_GetSectionSeparator("Function Prototypes"));
   c_Data.append("");
   c_Data.append(C_OscExportUti::h_GetSectionSeparator("Implementation"));
   c_Data.append("");
   C_OscExportUti::h_AddExternCeEnd(c_Data);
   c_Data.append("#endif");

   //finally save all to file
   s32_Return = C_OscExportUti::h_SaveToFile(c_Data, orc_FilePath, h_GetFileName(), true);

   if (s32_Return == C_NO_ERR)
   {
      //now for the c file:
      QString c_ProtocolConfig;

      c_Data.Clear();

      c_Data.append(C_OscExportUti::h_GetHeaderSeparator());
      c_Data.append("/*!");
      c_Data.append("   \\file");
      c_Data.append(
         "   \\brief       Application specific openSYDE CANopen Manager initialization (Source file with implementation)");
      c_Data.append("");
      c_Data.append(C_OscExportUti::h_GetCreationToolInfo(orc_ExportToolInfo));
      c_Data.append("*/");
      c_Data.append(C_OscExportUti::h_GetHeaderSeparator());
      c_Data.append("");
      c_Data.append(C_OscExportUti::h_GetSectionSeparator("Includes"));
      c_Data.append("#include \"stwtypes.h\"");
      c_Data.append("#include \"osco_man_config_init.h\"");
      c_Data.append("");
      c_Data.append(C_OscExportUti::h_GetSectionSeparator("Defines"));
      c_Data.append("");
      c_Data.append(C_OscExportUti::h_GetSectionSeparator("Types"));
      c_Data.append("");
      c_Data.append(C_OscExportUti::h_GetSectionSeparator("Global Variables"));
      c_Data.append("///Create Datapool instance data:");
      c_Data.append("OSCO_MAN_CREATE_STATIC_DEFINITION_INSTANCE_DATA(mt_OscoManInstanceData)");
      c_Data.append("");

      c_Data.append(
         "static const T_osco_man_manager_configuration * const mapt_OscoManConfigs[OSCO_MAN_CONFIG_INIT_NUM_CONFIGURATIONS] =");
      c_Data.append("{");
      //generate string with ProtocolConfiguration struct for each interface
      for (uint8_t u8_IfIt = 0; u8_IfIt < orc_IfWithCanOpenMan.size(); ++u8_IfIt)
      {
         const QString c_InterfaceIndex =
            QString::number(static_cast<uint32_t>(orc_IfWithCanOpenMan[u8_IfIt]) + 1U);
         c_ProtocolConfig = "   &gt_osco_man_can" + c_InterfaceIndex + "_ProtocolConfiguration";

         if (u8_IfIt < static_cast<uint32_t>(orc_IfWithCanOpenMan.size() - 1U))
         {
            c_ProtocolConfig += ",";
         }
         c_Data.append(c_ProtocolConfig);
      }
      c_Data.append("};");

      //add InitConfiguration struct
      c_Data.append("const T_osco_man_manager_init_configuration gt_osco_man_InitConfiguration =");
      c_Data.append("{");
      c_Data.append("   &mt_OscoManInstanceData,");
      c_Data.append("   OSCO_MAN_CONFIG_INIT_NUM_CONFIGURATIONS,");
      c_Data.append("   &mapt_OscoManConfigs[0]");
      c_Data.append("};");

      c_Data.append("");
      c_Data.append(C_OscExportUti::h_GetSectionSeparator("Module Global Variables"));
      c_Data.append("");
      c_Data.append(C_OscExportUti::h_GetSectionSeparator("Module Global Function Prototypes"));
      c_Data.append("");
      c_Data.append(C_OscExportUti::h_GetSectionSeparator("Implementation"));

      //save all this to file
      s32_Return = C_OscExportUti::h_SaveToFile(c_Data, orc_FilePath, h_GetFileName(), false);
   }

   return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Composes a string which represents the value for "#define OSCO_MAN_CONFIG_INIT_NUM_T/RX_PDOS" or
 *          for "#define OSCO_MAN_CONFIG_INIT_NUM_TX_SIGNALS" depending on last parameter

   e.g. "OSCO_MAN_CAN1_NUMBER_OF_TX_PDOS + OSCO_MAN_CAN2_NUMBER_OF_TX_PDOS"
   parenthesis and comment is added to string by caller

   \param[in,out]   orc_DefineValue        storage for string
   \param[in]       orc_IfWithCanOpenMan   vector with all interface indices, which contain a CANopen manager
   \param[in]       oq_IsTx                True: compose string for TX_PDOS
   \param[in]       orc_Subject            "subject" the string is composed for (can be "PDOS" or "SIGNALS")
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscExportCanOpenInit::mh_ComposeDefineNumTotal(QString & orc_DefineValue,
                                                      const std::vector<uint8_t> & orc_IfWithCanOpenMan,
                                                      const bool oq_IsTx, const QString & orc_Subject)
{
   const QString c_PdoType = oq_IsTx ? "TX" : "RX";

   for (uint32_t u32_IfIt = 0; u32_IfIt < orc_IfWithCanOpenMan.size(); ++u32_IfIt)
   {
      const QString c_InterfaceIndex = QString::number(
         static_cast<uint32_t>(orc_IfWithCanOpenMan[u32_IfIt]) + 1U);

      orc_DefineValue += "OSCO_MAN_CAN" + c_InterfaceIndex + "_NUMBER_OF_" + c_PdoType + "_" + orc_Subject;

      if (u32_IfIt < static_cast<uint32_t>(orc_IfWithCanOpenMan.size() - 1U))
      {
         orc_DefineValue += " + ";
      }
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Composes a string which represents the value for "#define OSCO_MAN_CONFIG_INIT_NUM_DEVICES"

   e.g. "OSCO_MAN_CAN1_NUMBER_OF_DEVICES + OSCO_MAN_CAN2_NUMBER_OF_DEVICES"
   parenthesis and comment is added to string by caller

   \param[in,out]   orc_DefineValue        storage for string
   \param[in]       orc_IfWithCanOpenMan   vector with all interface indices, which contain a CANopen manager
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscExportCanOpenInit::mh_ComposeDefineNumDevices(QString & orc_DefineValue,
                                                        const std::vector<uint8_t> & orc_IfWithCanOpenMan)
{
   for (uint32_t u32_IfIt = 0; u32_IfIt < orc_IfWithCanOpenMan.size(); ++u32_IfIt)
   {
      const QString c_InterfaceIndex = QString::number(
         static_cast<uint32_t>(orc_IfWithCanOpenMan[u32_IfIt]) + 1U);

      orc_DefineValue += "OSCO_MAN_CAN" + c_InterfaceIndex + "_NUMBER_OF_DEVICES";

      if (u32_IfIt < static_cast<uint32_t>(orc_IfWithCanOpenMan.size() - 1U))
      {
         orc_DefineValue += " + ";
      }
   }
}
