//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Base class for openSYDE X-config generation command line tools.

   See cpp file for detailed description

   \copyright   Copyright 2025 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_XCONFIGGENEXPORT_HPP
#define C_XCONFIGGENEXPORT_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>

#include "stwtypes.hpp"
#include <string>
#include "C_OscSystemDefinition.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */

/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_XconfigGenExportBase
{
public:
   enum E_ResultCode
   {
      eRESULT_OK = 0,
      eRESULT_HELPING = 10,                         //command line switch "--help" detected (no operation performed)
      eRESULT_ERASE_FILE_LIST_ERROR = 11,           //could not remove pre-existing file list file
      eRESULT_WRITE_FILE_LIST_ERROR = 12,           //could not write file list file
      eRESULT_ERASE_TARGET_FOLDER_ERROR = 13,       //could not erase pre-existing target folder
      eRESULT_CREATE_TARGET_FOLDER_ERROR = 14,      //could not create target folder
      eRESULT_INVALID_CLI_PARAMETERS = 20,          //invalid command line parameters
      eRESULT_SYSTEM_DEFINITION_OPEN_ERROR = 30,    //specified system definition could not be loaded
      eRESULT_X_CONFIG_GENERATION_ERROR = 40,       //could not generate code for at least one application
      eRESULT_DEVICE_NOT_FOUND = 41,                //device specified on command line does not exist
      eRESULT_DEVICE_NOT_COMPATIBLE = 42,           //device specified on command line does not support openSYDE
      eRESULT_APPLICATION_NOT_FOUND = 43,           //application specified on command line does not exist
      eRESULT_APPLICATION_NOT_PROGRAMMABLE = 44,    //application specified on command line is not defined as
                                                    // "programmable"
      eRESULT_APPLICATION_UNKNOWN_CODE_VERSION = 45 //application has unknown code format version
                                                    // 9009 is reserved for batch files (executable wrapped in batch
                                                    // script not found)
   };

   static std::string h_GetApplicationVersion(const std::string & orc_FileName);

   virtual ~C_XconfigGenExportBase(void);

   E_ResultCode Init(const int32_t os32_Argc = 0, char_t * const * const oppcn_Argv = NULL);
   E_ResultCode ParseCommandLine(const int32_t os32_Argc, char_t * const opacn_Argv[]);
   E_ResultCode LoadSystemDefinition(void);
   E_ResultCode GenerateSourceCode(void);
   E_ResultCode Exit(const E_ResultCode oe_ResultCode);

private:
   void m_PrintCommandLineParameters(void) const;
   E_ResultCode m_CreateNodeCode(const stw::opensyde_core::C_OscNode & orc_Node,
                                 const std::string & orc_OutputPath);
   void m_PrintCodeCreationInformation(const std::string & orc_NodeName,
                                       const stw::opensyde_core::C_OscNodeApplication & orc_Application,
                                       const bool oq_GenerationSuccessful,
                                       std::vector<std::string> & orc_CreatedFiles);
   void m_PrintCodeFormatUnknownInfo(const std::string & orc_NodeName,
                                     const stw::opensyde_core::C_OscNodeApplication & orc_Application);

protected:
   //parameters from command line:
   std::string mc_SystemDefinitionFilePath;
   std::string mc_OutputPath;
   std::string mc_DeviceDefinitions;
   std::string mc_DeviceName;      //name of device for which to create code ("" -> all)
   std::string mc_ApplicationName; //name of application for which to create code ("" -> all)
   bool mq_EraseTargetFolder;

   //parsed system definition
   stw::opensyde_core::C_OscSystemDefinition mc_SystemDefinition;

   //application executable information (set by ::Init)
   std::string mc_ExeName;
   std::string mc_ExeVersion; //Format: V<M>.<mm>r<r>
   std::string mc_LogFileName;
   std::string mc_ListOfFilesFileName;
   std::string mc_BinaryHash;

   //list of created source code files; to be filled by GenerateSourceCode
   std::vector<std::string> mc_CreatedFiles;

   //-------------------------------------------------------------------------------------------------------------------
   /*!
      \brief   Generate source code for one application of one node

      The caller guarantees ou16_ApplicationIndex to be within valid range and the application to be a
       "programmable application".

      \param[in]     orc_Node               node to generate code for
      \param[in]     ou16_ApplicationIndex  application to generate code for
      \param[in]     orc_OutputPath         code generation output path
      \param[out]    orc_CreatedFiles       list of created files (with paths)

      \return
      eRESULT_OK                        code created for all programmable application
      eRESULT_CODE_GENERATION_ERROR     problems creating code
      Do not return
   */
   //-------------------------------------------------------------------------------------------------------------------
   virtual E_ResultCode m_CreateApplicationCode(const stw::opensyde_core::C_OscNode & orc_Node,
                                                const stw::opensyde_core::C_OscSystemDefinition & orc_SystemDefinition,
                                                const std::string & orc_OutputPath,
                                                std::vector<std::string> & orc_CreatedFiles) = 0;

   //-------------------------------------------------------------------------------------------------------------------
   /*!
      \brief   Print banner and application name to console
   */
   //-------------------------------------------------------------------------------------------------------------------
   virtual void m_PrintBanner(void) = 0;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */

#endif
