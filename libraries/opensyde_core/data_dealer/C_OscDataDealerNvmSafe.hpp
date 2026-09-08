//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Encapsulates safety relevant sequences for NVM access. (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCDATADEALERNVMSAFE_HPP
#define C_OSCDATADEALERNVMSAFE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <set>
#include <system_error>

#include "C_OscErrorCategory.hpp"
#include "C_OscDataDealerNvm.hpp"
#include "C_OscParamSetHandler.hpp"
#include "C_OscParamSetRawNode.hpp"
#include "C_OscParamSetRawEntry.hpp"
#include "C_OscParamSetInterpretedList.hpp"
#include "C_OscParamSetInterpretedFileInfoData.hpp"
#include "C_OscNodeDataPoolListId.hpp"
#include "C_OscNodeDataPoolListElementId.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

///openSYDE safe NVM data dealer
class C_OscDataDealerNvmSafe :
   public C_OscDataDealerNvm
{
public:
   C_OscDataDealerNvmSafe(void);
   C_OscDataDealerNvmSafe(C_OscNode * const opc_Node, const uint32_t ou32_NodeIndex,
                          C_OscDiagProtocolBase * const opc_DiagProtocol);
   virtual ~C_OscDataDealerNvmSafe(void);

   std::error_code NvmSafeCheckCrcs(const C_OscNode & orc_Node) const;
   std::error_code NvmSafeWriteChangedValues(
      std::vector<C_OscNodeDataPoolListElementId> & orc_ChangedElements,
      const std::vector<C_OscNodeDataPoolListId> * const opc_AdditionalListsToUpdate = nullptr,
      uint8_t * const opu8_NrCode = nullptr);
   std::error_code NvmSafeReadValues(const C_OscNode * (&orpc_NodeCopy), uint8_t * const opu8_NrCode);
   std::error_code NvmSafeWriteCrcs(uint8_t * const opu8_NrCode);

   //Create file process
   void NvmSafeClearInternalContent(void);
   std::error_code NvmSafeReadParameterValues(const std::vector<C_OscNodeDataPoolListId> & orc_ListIds,
                                              uint8_t * const opu8_NrCode);
   std::error_code NvmSafeCreateCleanFileWithoutCrc(
      const std::string & orc_Path,
      const stw::opensyde_core::C_OscParamSetInterpretedFileInfoData & orc_FileInfo =
         stw::opensyde_core::C_OscParamSetInterpretedFileInfoData());
   std::error_code NvmSafeReadFileWithoutCrc(const std::string & orc_Path);
   std::error_code NvmSafeCheckParameterFileContents(const std::string & orc_Path,
                                                     std::vector<C_OscNodeDataPoolListId> & orc_DataPoolLists);
   std::error_code NvmSafeUpdateCrcForFile(const std::string & orc_Path);

   //Write file process
   std::error_code NvmSafeReadFileWithCrc(const std::string & orc_Path);
   std::error_code NvmSafeWriteParameterSetFile(const std::string & orc_Path, int32_t & ors32_ResultDetail);

private:
   static void mh_CreateInterpretedList(const C_OscNodeDataPoolList & orc_List,
                                        C_OscParamSetInterpretedList & orc_InterpretedList);
   std::error_code m_CheckParameterFileContent(const C_OscParamSetRawNode & orc_Node) const;
   std::error_code m_CreateRawEntryAndPrepareInterpretedData(C_OscNodeDataPoolList & orc_List,
                                                             C_OscParamSetRawEntry & orc_Entry,
                                                             uint8_t * const opu8_NrCode);

   enum E_CreateParameterSetFileState
   {
      eCPSFS_IDLE,
      eCPSFS_FILE_CREATED,
      eCPSFS_FILE_CHECKED
   };

   enum E_ParameterSetFileState
   {
      ePSFS_IDLE,
      ePSFS_DATA_RESET,
      ePSFS_FILE_CREATED,
      ePSFS_FILE_READ_WITHOUT_CRC,
      ePSFS_FILE_READ_WITH_CRC
   };

   E_CreateParameterSetFileState me_CreateParameterSetWorkflowState;
   E_ParameterSetFileState me_ParameterSetFileState;
   std::string mc_ParameterSetFilePath;
   C_OscNode mc_NodeCopy;
   C_OscParamSetHandler mc_ImageFileHandler;
   std::set<C_OscNodeDataPoolListId> mc_ChangedLists;
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
