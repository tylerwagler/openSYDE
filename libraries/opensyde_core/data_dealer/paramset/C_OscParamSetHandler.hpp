//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handler class for parameter set file operations (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCPARAMSETHANDLER_HPP
#define C_OSCPARAMSETHANDLER_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>
#include <system_error>

#include <cstdint>
#include <string>
#include "C_OscErrorCategory.hpp"
#include "C_OscXmlParser.hpp"
#include "C_OscParamSetInterpretedData.hpp"
#include "C_OscParamSetRawNode.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

///utility class providing access to parameter set files
class C_OscParamSetHandler
{
public:
   C_OscParamSetHandler(void);

   //File
   std::error_code CreateCleanFileWithoutCrc(const std::string & orc_FilePath,
                                             const bool oq_InterpretedDataOnly = false) const;
   std::error_code ReadFile(const std::string & orc_FilePath, const bool oq_IgnoreCrc,
                            const bool oq_InterpretedDataOnly = false, uint16_t * const opu16_FileCrc = nullptr,
                            bool * const opq_MissingOptionalContent = nullptr);
   static std::error_code h_UpdateCrcForFile(const std::string & orc_FilePath);

   //Data
   void ClearContent(void);
   std::error_code AddRawDataForNode(const C_OscParamSetRawNode & orc_Content);
   void AddInterpretedFileData(const C_OscParamSetInterpretedFileInfoData & orc_FileInfo);
   std::error_code AddInterpretedDataForNode(const C_OscParamSetInterpretedNode & orc_Content);
   const C_OscParamSetRawNode * GetRawDataForNode(const std::string & orc_NodeName) const;
   const C_OscParamSetInterpretedData & GetInterpretedData(void) const;

   uint32_t GetNumberOfNodes(void) const;
   const C_OscParamSetRawNode * GetRawDataForNode(const uint32_t ou32_NodeIndex) const;

private:
   C_OscParamSetInterpretedData mc_Data;
   std::vector<C_OscParamSetRawNode> mc_RawNodes;

   std::error_code m_LoadNodes(C_OscXmlParser & orc_XmlParser, const bool oq_InterpretedDataOnly,
                               bool & orq_MissingOptionalContent);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
