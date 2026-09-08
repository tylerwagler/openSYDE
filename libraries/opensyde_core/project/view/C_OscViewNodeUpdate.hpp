//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node update data (header)

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCVIEWNODEUPDATE_HPP
#define C_OSCVIEWNODEUPDATE_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <string>
#include <system_error>
#include <vector>
#include "stwtypes.hpp"
#include "C_OscViewNodeUpdateParamInfo.hpp"
#include "C_OscNodeApplication.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscViewNodeUpdate
{
public:
   C_OscViewNodeUpdate(void);

   enum E_GenericFileType
   {
      eFTP_DATA_BLOCK,
      eFTP_FILE_BASED
   };

   enum E_StateSecureAuthentication
   {
      eST_SEC_NO_CHANGE,
      eST_SEC_ACTIVATE,
      eST_SEC_DEACTIVATE
   };

   enum E_StateDebugger
   {
      eST_DEB_NO_CHANGE,
      eST_DEB_ACTIVATE,
      eST_DEB_DEACTIVATE
   };

   enum E_StateTrafficEncryption
   {
      eST_TEN_NO_CHANGE,
      eST_TEN_ACTIVATE,
      eST_TEN_DEACTIVATE
   };

   void CalcHash(uint32_t & oru32_HashValue) const;

   //Vectors
   void ClearParamPaths(void);
   void ClearPathsAsAppropriate(const E_GenericFileType oe_Type);
   const std::vector<C_OscViewNodeUpdateParamInfo> & GetParamInfos(void) const;
   const std::vector<std::string> & GetPaths(const E_GenericFileType oe_Type) const;
   const std::vector<bool> & GetSkipUpdateOfParamInfosFlags(void) const;
   const std::vector<bool> & GetSkipUpdateOfPathsFlags(const E_GenericFileType oe_Type) const;
   void SetParamInfos(const std::vector<C_OscViewNodeUpdateParamInfo> & orc_Value);
   void SetPaths(const std::vector<std::string> & orc_Value, const E_GenericFileType oe_Type);
   void SetSkipUpdateOfParamInfosFlags(const std::vector<bool> & orc_Value);
   void SetSkipUpdateOfPathsFlags(const std::vector<bool> & orc_Value, const E_GenericFileType oe_Type);
   std::error_code SetParamInfoContent(const uint32_t ou32_Index, const std::string & orc_FilePath,
                                       const uint32_t ou32_LastKnownCrc);

   //Elements
   void AddPath(const std::string & orc_Path, const E_GenericFileType oe_Type);
   void AddParamInfo(const C_OscViewNodeUpdateParamInfo & orc_Value);
   std::error_code SetPath(const uint32_t ou32_Index, const std::string & orc_Value, const E_GenericFileType oe_Type);
   std::error_code SetParamInfo(const uint32_t ou32_Index, const C_OscViewNodeUpdateParamInfo & orc_Value);
   std::error_code SetSkipUpdateOfPath(const uint32_t ou32_Index, const bool oq_SkipFile,
                                       const E_GenericFileType oe_Type);
   std::error_code SetSkipUpdateOfParamInfo(const uint32_t ou32_Index, const bool oq_SkipFile);
   std::error_code RemovePath(const uint32_t ou32_Index, const E_GenericFileType oe_Type);
   std::error_code RemoveParamInfo(const uint32_t ou32_Index);

   // PEM file
   void SetPemFilePath(const std::string & orc_Value);
   std::string GetPemFilePath(void) const;
   void RemovePemFilePath(void);
   void SetSkipUpdateOfPemFile(const bool oq_Skip);
   bool GetSkipUpdateOfPemFile(void) const;

   //State elements
   void SetStates(const E_StateSecureAuthentication oe_StateSecureAuthentication,
                  const E_StateDebugger oe_StateDebugger, const E_StateTrafficEncryption oe_StateTrafficEncryption);
   void GetStates(E_StateSecureAuthentication & ore_StateSecureAuthentication, E_StateDebugger & ore_StateDebugger,
                  E_StateTrafficEncryption & ore_StateTrafficEncryption) const;

   void OnSyncNodeApplicationAdded(const uint32_t ou32_ApplicationIndex,
                                   const C_OscNodeApplication::E_Type oe_ApplicationType,
                                   const uint32_t ou32_NumDataBlockPaths,
                                   const std::vector<C_OscNodeApplication> & orc_AllApplications);
   void OnSyncNodeApplicationMoved(const uint32_t ou32_ApplicationSourceIndex,
                                   const uint32_t ou32_ApplicationTargetIndex,
                                   const C_OscNodeApplication::E_Type oe_ApplicationSourceType,
                                   const C_OscNodeApplication::E_Type oe_ApplicationTargetType);
   void OnSyncNodeApplicationAboutToBeDeleted(const uint32_t ou32_ApplicationIndex,
                                              const C_OscNodeApplication::E_Type oe_ApplicationType,
                                              const uint32_t ou32_NumDataBlockPaths);
   void OnSyncNodeApplicationAboutToBeChangedFromParamSetHalc(const uint32_t ou32_ApplicationIndex,
                                                              const C_OscNodeApplication::E_Type oe_ApplicationType,
                                                              const uint32_t ou32_NumDataBlockPaths);
   void OnSyncNodeApplicationChangedToParamSetHalc(const uint32_t ou32_ApplicationIndex,
                                                   const C_OscNodeApplication::E_Type oe_ApplicationType,
                                                   const uint32_t ou32_NumDataBlockPaths,
                                                   const std::vector<C_OscNodeApplication> & orc_AllApplications);
   void OnSyncNodeApplicationResultPathSizeChanged(const C_OscNodeApplication::E_Type oe_ApplicationType,
                                                   const uint32_t ou32_OldSize, const uint32_t ou32_NewSize);

   uint32_t u32_NodeUpdatePosition; // Position of node in update sequence

private:
   std::vector<std::string> mc_DataBlockPaths;
   std::vector<std::string> mc_FileBasedPaths;
   std::vector<C_OscViewNodeUpdateParamInfo> mc_ParamSetPaths;
   std::vector<std::vector<bool> > mc_SkipUpdateOfFiles; // Flags for skipping an update of the associated file
   // First level has three "layers":
   // 1: flags for datablock paths
   // 2: flags for file based paths
   // 3: flags for parameter set paths

   std::string mc_PemFilePath;
   bool mq_SkipUpdateOfPemFile;

   E_StateSecureAuthentication me_StateSecureAuthentication;
   E_StateDebugger me_StateDebugger;
   E_StateTrafficEncryption me_StateTrafficEncryption;

   static const int32_t mhs32_PARAMETER_SET_INDEX = 2;

   static bool mh_CheckApplicationsContainParamTypeBeforeIndex(
      const std::vector<C_OscNodeApplication> & orc_AllApplications, const uint32_t ou32_ApplicationIndex);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
