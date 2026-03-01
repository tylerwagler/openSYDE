//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Node update data (implementation)

   Node update data

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include "stwerrors.hpp"

#include "C_OscViewNodeUpdate.hpp"
#include "C_SclChecksums.hpp"
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QDomDocument>

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */
using namespace stw::scl;
using namespace stw::errors;
using namespace stw::opensyde_core;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */

/* -- Types
 * ---------------------------------------------------------------------------------------------------------
 */

/* -- Global Variables
 * ----------------------------------------------------------------------------------------------
 */

/* -- Module Global Variables
 * ---------------------------------------------------------------------------------------
 */

/* -- Module Global Function Prototypes
 * -----------------------------------------------------------------------------
 */

/* -- Implementation
 * ------------------------------------------------------------------------------------------------
 */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscViewNodeUpdate::C_OscViewNodeUpdate(void)
    : u32_NodeUpdatePosition(0U), mq_SkipUpdateOfPemFile(false),
      me_StateSecurity(eST_SEC_NO_CHANGE), me_StateDebugger(eST_DEB_NO_CHANGE) {
  // For each type of file
  this->mc_SkipUpdateOfFiles.resize(3);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Calculates the hash value over all data

   The hash value is a 32 bit CRC value.

   \param[in,out]  oru32_HashValue  Hash value with init [in] value and result
   [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::CalcHash(uint32_t &oru32_HashValue) const {
  const uint32_t u32_Size =
      static_cast<int32_t>(this->mc_DataBlockPaths.size());

  stw::scl::C_SclChecksums::CalcCRC32(
      &this->me_StateDebugger, sizeof(this->me_StateDebugger), oru32_HashValue);
  stw::scl::C_SclChecksums::CalcCRC32(
      &this->me_StateSecurity, sizeof(this->me_StateSecurity), oru32_HashValue);
  stw::scl::C_SclChecksums::CalcCRC32(&u32_Size, sizeof(u32_Size),
                                      oru32_HashValue);
  for (uint32_t u32_It = 0; u32_It < this->mc_DataBlockPaths.size(); ++u32_It) {
    const QString &rc_QtData = this->mc_DataBlockPaths[u32_It];
    stw::scl::C_SclChecksums::CalcCRC32(rc_QtData.toUtf8().constData(),
                                        rc_QtData.length(), oru32_HashValue);
  }
  for (uint32_t u32_It = 0; u32_It < this->mc_FileBasedPaths.size(); ++u32_It) {
    const QString &rc_QtData = this->mc_FileBasedPaths[u32_It];
    stw::scl::C_SclChecksums::CalcCRC32(rc_QtData.toUtf8().constData(),
                                        rc_QtData.length(), oru32_HashValue);
  }
  for (uint32_t u32_It = 0; u32_It < this->mc_ParamSetPaths.size(); ++u32_It) {
    const C_OscViewNodeUpdateParamInfo &rc_Test =
        this->mc_ParamSetPaths[u32_It];
    rc_Test.CalcHash(oru32_HashValue);
  }
  for (uint32_t u32_It = 0; u32_It < this->mc_SkipUpdateOfFiles.size();
       ++u32_It) {
    for (uint32_t u32_SubIt = 0;
         u32_SubIt < this->mc_SkipUpdateOfFiles[u32_It].size(); ++u32_SubIt) {
      const bool q_Value = this->mc_SkipUpdateOfFiles[u32_It][u32_SubIt];
      stw::scl::C_SclChecksums::CalcCRC32(&q_Value, sizeof(q_Value),
                                          oru32_HashValue);
    }
  }
  stw::scl::C_SclChecksums::CalcCRC32(
      &u32_NodeUpdatePosition, sizeof(u32_NodeUpdatePosition), oru32_HashValue);

  stw::scl::C_SclChecksums::CalcCRC32(this->mc_PemFilePath.toUtf8().constData(),
                                      this->mc_PemFilePath.length(),
                                      oru32_HashValue);
  stw::scl::C_SclChecksums::CalcCRC32(&this->mq_SkipUpdateOfPemFile,
                                      sizeof(this->mq_SkipUpdateOfPemFile),
                                      oru32_HashValue);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Clear all parameter set paths for this node
 */
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::ClearParamPaths(void) {
  this->mc_ParamSetPaths.clear();
  this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].clear();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Clear all paths as appropriate for the type

   \param[in]  oe_Type  Selector for structure
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::ClearPathsAsAppropriate(
    const C_OscViewNodeUpdate::E_GenericFileType oe_Type) {
  const int32_t s32_Type = static_cast<int32_t>(oe_Type);

  if (oe_Type == eFTP_DATA_BLOCK) {
    for (uint32_t u32_It = 0UL; u32_It < this->mc_DataBlockPaths.size();
         ++u32_It) {
      this->mc_DataBlockPaths[u32_It] = "<Add File>";
      this->mc_SkipUpdateOfFiles[s32_Type][u32_It] = false;
    }
  } else {
    this->mc_FileBasedPaths.clear();
    this->mc_SkipUpdateOfFiles[s32_Type].clear();
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get parameter set paths

   \return
   Current parameter set paths
*/
//----------------------------------------------------------------------------------------------------------------------
const QList<C_OscViewNodeUpdateParamInfo> &
C_OscViewNodeUpdate::GetParamInfos(void) const {
  return this->mc_ParamSetPaths;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get application paths

   \param[in]  oe_Type  Selector for structure

   \return
   Current application paths
*/
//----------------------------------------------------------------------------------------------------------------------
const QStringList &
C_OscViewNodeUpdate::GetPaths(const E_GenericFileType oe_Type) const {
  return (oe_Type == eFTP_DATA_BLOCK) ? this->mc_DataBlockPaths
                                      : this->mc_FileBasedPaths;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get parameter set skip flags

   \return
   Current application skip flags
*/
//----------------------------------------------------------------------------------------------------------------------
const QList<bool> &
C_OscViewNodeUpdate::GetSkipUpdateOfParamInfosFlags(void) const {
  return this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX];
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get application skip flags

   \param[in]  oe_Type  Selector for structure

   \return
   Current application skip flags
*/
//----------------------------------------------------------------------------------------------------------------------
const QList<bool> &C_OscViewNodeUpdate::GetSkipUpdateOfPathsFlags(
    const E_GenericFileType oe_Type) const {
  return this->mc_SkipUpdateOfFiles[static_cast<int32_t>(oe_Type)];
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set parameter set paths

   \param[in]  orc_Value   New parameter set paths
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::SetParamInfos(
    const QList<C_OscViewNodeUpdateParamInfo> &orc_Value) {
  this->mc_ParamSetPaths = orc_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set application paths

   \param[in]  orc_Value   New application paths
   \param[in]  oe_Type     Selector for structure
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::SetPaths(const QStringList &orc_Value,
                                   const E_GenericFileType oe_Type) {
  if (oe_Type == eFTP_DATA_BLOCK) {
    this->mc_DataBlockPaths = orc_Value;
  } else {
    this->mc_FileBasedPaths = orc_Value;
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set parameter set skip flags

   \param[in]  orc_Value   New parameter set skip flags
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::SetSkipUpdateOfParamInfosFlags(
    const QList<bool> &orc_Value) {
  this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX] = orc_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set application skip flags

   \param[in]  orc_Value   New application skip flags
   \param[in]  oe_Type     Selector for structure
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::SetSkipUpdateOfPathsFlags(
    const QList<bool> &orc_Value, const E_GenericFileType oe_Type) {
  this->mc_SkipUpdateOfFiles[static_cast<int32_t>(oe_Type)] = orc_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set node update information parameter set information

   \param[in]  ou32_Index           Index to access
   \param[in]  orc_FilePath         New path
   \param[in]  ou32_LastKnownCrc    Last known CRC for this file

   \return
   C_NO_ERR Operation success
   C_RANGE  Operation failure: parameter invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t
C_OscViewNodeUpdate::SetParamInfoContent(const uint32_t ou32_Index,
                                         const QString &orc_FilePath,
                                         const uint32_t ou32_LastKnownCrc) {
  int32_t s32_Retval = C_NO_ERR;

  if (ou32_Index < this->mc_ParamSetPaths.size()) {
    C_OscViewNodeUpdateParamInfo &rc_ParamSet =
        this->mc_ParamSetPaths[ou32_Index];
    rc_ParamSet.SetContent(orc_FilePath, ou32_LastKnownCrc);
  } else {
    s32_Retval = C_RANGE;
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Add path to internal structure

   \param[in]  orc_Path    New path
   \param[in]  oe_Type     Selector for structure
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::AddPath(
    const QString &orc_Path,
    const C_OscViewNodeUpdate::E_GenericFileType oe_Type) {
  if (oe_Type == eFTP_DATA_BLOCK) {
    this->mc_DataBlockPaths.push_back(orc_Path);
  } else {
    this->mc_FileBasedPaths.push_back(orc_Path);
  }
  this->mc_SkipUpdateOfFiles[static_cast<int32_t>(oe_Type)].push_back(false);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Add node update information for a parameter set

   \param[in]  orc_Value   New path
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::AddParamInfo(
    const C_OscViewNodeUpdateParamInfo &orc_Value) {
  this->mc_ParamSetPaths.push_back(orc_Value);
  this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].push_back(false);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set node update information path

   \param[in]  ou32_Index  Index to access
   \param[in]  orc_Value   New path
   \param[in]  oe_Type     Selector for structure

   \return
   C_NO_ERR Operation success
   C_RANGE  Operation failure: parameter invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewNodeUpdate::SetPath(
    const uint32_t ou32_Index, const QString &orc_Value,
    const C_OscViewNodeUpdate::E_GenericFileType oe_Type) {
  int32_t s32_Retval = C_NO_ERR;

  if (oe_Type == eFTP_DATA_BLOCK) {
    if (ou32_Index < this->mc_DataBlockPaths.size()) {
      this->mc_DataBlockPaths[ou32_Index] = orc_Value;
    } else {
      s32_Retval = C_RANGE;
    }
  } else {
    if (ou32_Index < this->mc_FileBasedPaths.size()) {
      this->mc_FileBasedPaths[ou32_Index] = orc_Value;
    } else {
      s32_Retval = C_RANGE;
    }
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set node update information parameter set information

   \param[in]  ou32_Index  Index to access
   \param[in]  orc_Value   New path

   \return
   C_NO_ERR Operation success
   C_RANGE  Operation failure: parameter invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewNodeUpdate::SetParamInfo(
    const uint32_t ou32_Index, const C_OscViewNodeUpdateParamInfo &orc_Value) {
  int32_t s32_Retval = C_NO_ERR;

  if (ou32_Index < this->mc_ParamSetPaths.size()) {
    this->mc_ParamSetPaths[ou32_Index] = orc_Value;
  } else {
    s32_Retval = C_RANGE;
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set node update information path

   \param[in]  ou32_Index  Index to access
   \param[in]  oq_SkipFile Flag if file will be skipped when updating
   \param[in]  oe_Type     Selector for structure

   \return
   C_NO_ERR Operation success
   C_RANGE  Operation failure: parameter invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewNodeUpdate::SetSkipUpdateOfPath(
    const uint32_t ou32_Index, const bool oq_SkipFile,
    const C_OscViewNodeUpdate::E_GenericFileType oe_Type) {
  int32_t s32_Retval = C_NO_ERR;
  const int32_t s32_Type = static_cast<int32_t>(oe_Type);

  if (ou32_Index < this->mc_SkipUpdateOfFiles[s32_Type].size()) {
    this->mc_SkipUpdateOfFiles[s32_Type][ou32_Index] = oq_SkipFile;
  } else {
    s32_Retval = C_RANGE;
  }

  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set node update information parameter set information

   \param[in]  ou32_Index  Index to access
   \param[in]  oq_SkipFile Flag if file will be skipped when updating

   \return
   C_NO_ERR Operation success
   C_RANGE  Operation failure: parameter invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewNodeUpdate::SetSkipUpdateOfParamInfo(const uint32_t ou32_Index,
                                                      const bool oq_SkipFile) {
  int32_t s32_Retval = C_NO_ERR;

  if (ou32_Index <
      this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].size()) {
    this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX][ou32_Index] =
        oq_SkipFile;
  } else {
    s32_Retval = C_RANGE;
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Remove node update information path

   \param[in]  ou32_Index  Index to remove
   \param[in]  oe_Type     Selector for structure

   \return
   C_NO_ERR Operation success
   C_RANGE  Operation failure: parameter invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewNodeUpdate::RemovePath(
    const uint32_t ou32_Index,
    const C_OscViewNodeUpdate::E_GenericFileType oe_Type) {
  int32_t s32_Retval = C_NO_ERR;

  if (oe_Type == eFTP_DATA_BLOCK) {
    if (ou32_Index < this->mc_DataBlockPaths.size()) {
      this->mc_DataBlockPaths.erase(this->mc_DataBlockPaths.begin() +
                                    ou32_Index);
    } else {
      s32_Retval = C_RANGE;
    }
  } else {
    if (ou32_Index < this->mc_FileBasedPaths.size()) {
      this->mc_FileBasedPaths.erase(this->mc_FileBasedPaths.begin() +
                                    ou32_Index);
    } else {
      s32_Retval = C_RANGE;
    }
  }

  if (s32_Retval == C_NO_ERR) {
    const int32_t s32_Type = static_cast<int32_t>(oe_Type);
    if (ou32_Index < this->mc_SkipUpdateOfFiles[s32_Type].size()) {
      this->mc_SkipUpdateOfFiles[s32_Type].erase(
          this->mc_SkipUpdateOfFiles[s32_Type].begin() + ou32_Index);
    } else {
      s32_Retval = C_RANGE;
    }
  }

  return s32_Retval;
}

//------------------------------------------------------------------------------------------------0----------------------
/*! \brief   Remove node update information parameter set information

   \param[in]  ou32_Index  Index to remove

   \return
   C_NO_ERR Operation success
   C_RANGE  Operation failure: parameter invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewNodeUpdate::RemoveParamInfo(const uint32_t ou32_Index) {
  int32_t s32_Retval = C_NO_ERR;

  if (ou32_Index < this->mc_ParamSetPaths.size()) {
    this->mc_ParamSetPaths.erase(this->mc_ParamSetPaths.begin() + ou32_Index);
  } else {
    s32_Retval = C_RANGE;
  }

  if (s32_Retval == C_NO_ERR) {
    if (ou32_Index <
        this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].size()) {
      this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].erase(
          this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].begin() +
          ou32_Index);
    } else {
      s32_Retval = C_RANGE;
    }
  }

  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set node update information PEM file path

   \param[in]  orc_Value   New path
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::SetPemFilePath(const QString &orc_Value) {
  this->mc_PemFilePath = orc_Value;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns node update information PEM file path

   \return
   - PEM file path
   - Empty string if no PEM file set
*/
//----------------------------------------------------------------------------------------------------------------------
QString C_OscViewNodeUpdate::GetPemFilePath(void) const {
  return this->mc_PemFilePath;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns node update information PEM file path

   Sets the state to do not change too
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::RemovePemFilePath(void) {
  this->mc_PemFilePath = "";
  this->mq_SkipUpdateOfPemFile = false;
  this->me_StateSecurity = eST_SEC_NO_CHANGE;
  this->me_StateDebugger = eST_DEB_NO_CHANGE;
  this->mq_SkipUpdateOfPemFile = false;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Set node update information PEM file path skip flag

   \param[in]  oq_Skip   New flag
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::SetSkipUpdateOfPemFile(const bool oq_Skip) {
  this->mq_SkipUpdateOfPemFile = oq_Skip;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Returns node update information PEM file path skip flag

   \retval   true       Flag for skipping the PEM file
   \retval   false      Flag for not skipping the PEM file
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscViewNodeUpdate::GetSkipUpdateOfPemFile(void) const {
  return this->mq_SkipUpdateOfPemFile;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Set node update information states

   \param[in]      oe_StateSecurity   Security state of node
   \param[in]      oe_StateDebugger   Debugger state of node
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::SetStates(
    const C_OscViewNodeUpdate::E_StateSecurity oe_StateSecurity,
    const C_OscViewNodeUpdate::E_StateDebugger oe_StateDebugger) {
  this->me_StateDebugger = oe_StateDebugger;
  this->me_StateSecurity = oe_StateSecurity;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Returns the update information states of the node

   \param[out]      ore_StateSecurity   Security state of node
   \param[out]      ore_StateDebugger   Debugger state of node
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::GetStates(
    C_OscViewNodeUpdate::E_StateSecurity &ore_StateSecurity,
    C_OscViewNodeUpdate::E_StateDebugger &ore_StateDebugger) const {
  ore_StateDebugger = this->me_StateDebugger;
  ore_StateSecurity = this->me_StateSecurity;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Adapt to system definition change

   \param[in]  ou32_ApplicationIndex   Application index
   \param[in]  oe_ApplicationType      Application type
   \param[in]  ou32_NumDataBlockPaths  Number of Data Block paths
   \param[in]  orc_AllApplications     All applications
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::OnSyncNodeApplicationAdded(
    const uint32_t ou32_ApplicationIndex,
    const C_OscNodeApplication::E_Type oe_ApplicationType,
    const uint32_t ou32_NumDataBlockPaths,
    const QList<C_OscNodeApplication> &orc_AllApplications) {
  if (oe_ApplicationType !=
      stw::opensyde_core::C_OscNodeApplication::ePARAMETER_SET_HALC) {
    uint32_t u32_UpdateApplicationIndex;
    // Scenario guaranteed can only exist once [0-1]
    if (C_OscViewNodeUpdate::mh_CheckApplicationsContainParamTypeBeforeIndex(
            orc_AllApplications, ou32_ApplicationIndex)) {
      // Skip one index as this would not have a datablock entry
      Q_ASSERT(ou32_ApplicationIndex > 0UL);
      u32_UpdateApplicationIndex = ou32_ApplicationIndex - 1UL;
    } else {
      u32_UpdateApplicationIndex = ou32_ApplicationIndex;
    }
    if (u32_UpdateApplicationIndex <= this->mc_DataBlockPaths.size()) {
      const int32_t s32_TYPE =
          static_cast<int32_t>(C_OscViewNodeUpdate::eFTP_DATA_BLOCK);
      this->mc_DataBlockPaths.insert(this->mc_DataBlockPaths.begin() +
                                         u32_UpdateApplicationIndex,
                                     QString());
      this->mc_SkipUpdateOfFiles[s32_TYPE].insert(
          this->mc_SkipUpdateOfFiles[s32_TYPE].begin() +
              u32_UpdateApplicationIndex,
          false);
    }
  } else {
    uint32_t u32_PathCounter;

    for (u32_PathCounter = 0U; u32_PathCounter < ou32_NumDataBlockPaths;
         ++u32_PathCounter) {
      this->mc_ParamSetPaths.insert(this->mc_ParamSetPaths.begin() +
                                        u32_PathCounter,
                                    C_OscViewNodeUpdateParamInfo());
      this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].insert(
          this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].begin() +
              u32_PathCounter,
          false);
    }
  }
}
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Adapt to system definition change

   \param[in]  ou32_ApplicationSourceIndex   Application source index
   \param[in]  ou32_ApplicationTargetIndex   Application target index
   \param[in]  oe_ApplicationSourceType      Application source type
   \param[in]  oe_ApplicationTargetType      Application target type
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::OnSyncNodeApplicationMoved(
    const uint32_t ou32_ApplicationSourceIndex,
    const uint32_t ou32_ApplicationTargetIndex,
    const C_OscNodeApplication::E_Type oe_ApplicationSourceType,
    const C_OscNodeApplication::E_Type oe_ApplicationTargetType) {
  if ((oe_ApplicationSourceType !=
       stw::opensyde_core::C_OscNodeApplication::ePARAMETER_SET_HALC) &&
      (oe_ApplicationTargetType !=
       stw::opensyde_core::C_OscNodeApplication::ePARAMETER_SET_HALC)) {
    QString c_Entry;

    bool q_Entry = false;
    const int32_t s32_TYPE =
        static_cast<int32_t>(C_OscViewNodeUpdate::eFTP_DATA_BLOCK);

    if (ou32_ApplicationSourceIndex < this->mc_DataBlockPaths.size()) {
      c_Entry = this->mc_DataBlockPaths[ou32_ApplicationSourceIndex];
      this->mc_DataBlockPaths.erase(this->mc_DataBlockPaths.begin() +
                                    ou32_ApplicationSourceIndex);

      q_Entry =
          this->mc_SkipUpdateOfFiles[s32_TYPE][ou32_ApplicationSourceIndex];
      this->mc_SkipUpdateOfFiles[s32_TYPE].erase(
          this->mc_SkipUpdateOfFiles[s32_TYPE].begin() +
          ou32_ApplicationSourceIndex);
    }
    if (ou32_ApplicationTargetIndex <= this->mc_DataBlockPaths.size()) {
      this->mc_DataBlockPaths.insert(this->mc_DataBlockPaths.begin() +
                                         ou32_ApplicationTargetIndex,
                                     c_Entry);
      this->mc_SkipUpdateOfFiles[s32_TYPE].insert(
          this->mc_SkipUpdateOfFiles[s32_TYPE].begin() +
              ou32_ApplicationTargetIndex,
          q_Entry);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Adapt to system definition change

   \param[in]  ou32_ApplicationIndex   Application index
   \param[in]  oe_ApplicationType      Application type
   \param[in]  ou32_NumDataBlockPaths  Number of Data Block paths
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::OnSyncNodeApplicationAboutToBeDeleted(
    const uint32_t ou32_ApplicationIndex,
    const C_OscNodeApplication::E_Type oe_ApplicationType,
    const uint32_t ou32_NumDataBlockPaths) {
  if (oe_ApplicationType !=
      stw::opensyde_core::C_OscNodeApplication::ePARAMETER_SET_HALC) {
    if (ou32_ApplicationIndex < this->mc_DataBlockPaths.size()) {
      const int32_t s32_TYPE =
          static_cast<int32_t>(C_OscViewNodeUpdate::eFTP_DATA_BLOCK);
      this->mc_DataBlockPaths.erase(this->mc_DataBlockPaths.begin() +
                                    ou32_ApplicationIndex);
      this->mc_SkipUpdateOfFiles[s32_TYPE].erase(
          this->mc_SkipUpdateOfFiles[s32_TYPE].begin() + ou32_ApplicationIndex);
    }
  } else {
    if (ou32_NumDataBlockPaths > 0U) {
      const uint32_t u32_EraseRange = ou32_NumDataBlockPaths;

      this->mc_ParamSetPaths.erase(this->mc_ParamSetPaths.begin(),
                                   this->mc_ParamSetPaths.begin() +
                                       u32_EraseRange);
      this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].erase(
          this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].begin(),
          this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].begin() +
              u32_EraseRange);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Adapt to system definition change

   \param[in]  ou32_ApplicationIndex   Application index
   \param[in]  oe_ApplicationType      Application type
   \param[in]  ou32_NumDataBlockPaths  Number of Data Block paths
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::OnSyncNodeApplicationAboutToBeChangedFromParamSetHalc(
    const uint32_t ou32_ApplicationIndex,
    const C_OscNodeApplication::E_Type oe_ApplicationType,
    const uint32_t ou32_NumDataBlockPaths) {
  // Must be still the same type
  Q_ASSERT(oe_ApplicationType ==
           stw::opensyde_core::C_OscNodeApplication::ePARAMETER_SET_HALC);
  if (oe_ApplicationType ==
      stw::opensyde_core::C_OscNodeApplication::ePARAMETER_SET_HALC) {
    // Delete the param set specific information about the datablock
    this->OnSyncNodeApplicationAboutToBeDeleted(
        ou32_ApplicationIndex, oe_ApplicationType, ou32_NumDataBlockPaths);

    // Add as "normal" datablock
    if (ou32_ApplicationIndex <= this->mc_DataBlockPaths.size()) {
      const int32_t s32_TYPE =
          static_cast<int32_t>(C_OscViewNodeUpdate::eFTP_DATA_BLOCK);
      this->mc_DataBlockPaths.insert(
          this->mc_DataBlockPaths.begin() + ou32_ApplicationIndex, QString());
      this->mc_SkipUpdateOfFiles[s32_TYPE].insert(
          this->mc_SkipUpdateOfFiles[s32_TYPE].begin() + ou32_ApplicationIndex,
          false);
    }
  }
}
//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Adapt to system definition change

   \param[in]  ou32_ApplicationIndex   Application index
   \param[in]  oe_ApplicationType      Application type
   \param[in]  ou32_NumDataBlockPaths  Number of Data Block paths
   \param[in]  orc_AllApplications     All applications
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::OnSyncNodeApplicationChangedToParamSetHalc(
    const uint32_t ou32_ApplicationIndex,
    const C_OscNodeApplication::E_Type oe_ApplicationType,
    const uint32_t ou32_NumDataBlockPaths,
    const QList<C_OscNodeApplication> &orc_AllApplications) {
  Q_ASSERT(oe_ApplicationType ==
           stw::opensyde_core::C_OscNodeApplication::ePARAMETER_SET_HALC);
  if (oe_ApplicationType ==
      stw::opensyde_core::C_OscNodeApplication::ePARAMETER_SET_HALC) {
    // Delete the "normal" datablock configuration
    if (ou32_ApplicationIndex < this->mc_DataBlockPaths.size()) {
      const int32_t s32_TYPE =
          static_cast<int32_t>(C_OscViewNodeUpdate::eFTP_DATA_BLOCK);
      this->mc_DataBlockPaths.erase(this->mc_DataBlockPaths.begin() +
                                    ou32_ApplicationIndex);
      this->mc_SkipUpdateOfFiles[s32_TYPE].erase(
          this->mc_SkipUpdateOfFiles[s32_TYPE].begin() + ou32_ApplicationIndex);
    }

    // Add as param set datablock
    this->OnSyncNodeApplicationAdded(ou32_ApplicationIndex, oe_ApplicationType,
                                     ou32_NumDataBlockPaths,
                                     orc_AllApplications);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Adapt to system definition change

   \param[in]  oe_ApplicationType   Application type
   \param[in]  ou32_OldSize         Size of application result path before
   change \param[in]  ou32_NewSize         Size of application result path after
   change
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscViewNodeUpdate::OnSyncNodeApplicationResultPathSizeChanged(
    const C_OscNodeApplication::E_Type oe_ApplicationType,
    const uint32_t ou32_OldSize, const uint32_t ou32_NewSize) {
  Q_ASSERT(oe_ApplicationType ==
           stw::opensyde_core::C_OscNodeApplication::ePARAMETER_SET_HALC);
  if (oe_ApplicationType ==
      stw::opensyde_core::C_OscNodeApplication::ePARAMETER_SET_HALC) {
    if (ou32_OldSize < ou32_NewSize) {
      // 1 to 2
      // Insert one
      Q_ASSERT((ou32_OldSize == 1U) && (ou32_NewSize == 2U));
      this->mc_ParamSetPaths.insert(this->mc_ParamSetPaths.begin() + 1,
                                    C_OscViewNodeUpdateParamInfo());
      this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].insert(
          this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].begin() + 1,
          false);
    } else if (ou32_NewSize < ou32_OldSize) {
      // 2 to 1
      // Delete the second entry
      Q_ASSERT((ou32_OldSize == 2U) && (ou32_NewSize == 1U));
      this->mc_ParamSetPaths.erase(this->mc_ParamSetPaths.begin() + 1);
      this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].erase(
          this->mc_SkipUpdateOfFiles[mhs32_PARAMETER_SET_INDEX].begin() + 1);
    } else {
      // Same size, nothing to do.
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check applications contain param type before index

   \param[in]  orc_AllApplications     All applications
   \param[in]  ou32_ApplicationIndex   Application index

   \return
   Flags

   \retval   True    Param type found before index
   \retval   False   Param type not found before index
*/
//----------------------------------------------------------------------------------------------------------------------
bool C_OscViewNodeUpdate::mh_CheckApplicationsContainParamTypeBeforeIndex(
    const QList<C_OscNodeApplication> &orc_AllApplications,
    const uint32_t ou32_ApplicationIndex) {
  bool q_Retval = false;

  for (uint32_t u32_ItAppl = 0UL; (u32_ItAppl < orc_AllApplications.size()) &&
                                  (u32_ItAppl < ou32_ApplicationIndex);
       ++u32_ItAppl) {
    const C_OscNodeApplication &rc_CurApplication =
        orc_AllApplications[u32_ItAppl];
    if (rc_CurApplication.e_Type == C_OscNodeApplication::ePARAMETER_SET_HALC) {
      q_Retval = true;
      break;
    }
  }
  return q_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDataStream (binary format)

   \param[out] orc_Stream    Output stream for serialization

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewNodeUpdate::ToQDataStream(QDataStream& orc_Stream) const {
   using namespace stw::errors;
   
   // Serialize enums
   orc_Stream << static_cast<int32_t>(me_StateSecurity);
   orc_Stream << static_cast<int32_t>(me_StateDebugger);
   
   // Serialize strings
   orc_Stream << mc_PemFilePath;
   orc_Stream << mq_SkipUpdateOfPemFile;
   
   // Serialize data block paths
   uint32_t u32_Count = mc_DataBlockPaths.size();
   orc_Stream << u32_Count;
   for (const QString& c_Path : mc_DataBlockPaths) {
      orc_Stream << c_Path;
   }
   
   // Serialize file based paths
   u32_Count = mc_FileBasedPaths.size();
   orc_Stream << u32_Count;
   for (const QString& c_Path : mc_FileBasedPaths) {
      orc_Stream << c_Path;
   }
   
   // Serialize param set paths
   u32_Count = mc_ParamSetPaths.size();
   orc_Stream << u32_Count;
   for (const C_OscViewNodeUpdateParamInfo& c_Info : mc_ParamSetPaths) {
      c_Info.ToQDataStream(orc_Stream);
   }
   
   // Serialize skip flags for data block paths
   u32_Count = mc_SkipUpdateOfFiles.size();
   orc_Stream << u32_Count;
   for (const QList<bool>& c_Flags : mc_SkipUpdateOfFiles) {
      uint32_t u32_FlagCount = c_Flags.size();
      orc_Stream << u32_FlagCount;
      for (bool q_Flag : c_Flags) {
         orc_Stream << q_Flag;
      }
   }
   
   if (orc_Stream.status() == QDataStream::Ok) {
      return C_NO_ERR;
   } else {
      return C_RD_WR;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QDataStream (binary format)

   \param[in,out] orc_Stream    Input stream for deserialization

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewNodeUpdate::FromQDataStream(QDataStream& orc_Stream) {
   using namespace stw::errors;
   
   // Deserialize enums
   int32_t s32_Enum = 0;
   orc_Stream >> s32_Enum;
   me_StateSecurity = static_cast<E_StateSecurity>(s32_Enum);
   
   orc_Stream >> s32_Enum;
   me_StateDebugger = static_cast<E_StateDebugger>(s32_Enum);
   
   // Deserialize strings
   orc_Stream >> mc_PemFilePath;
   orc_Stream >> mq_SkipUpdateOfPemFile;
   
   // Deserialize data block paths
   uint32_t u32_Count = 0;
   orc_Stream >> u32_Count;
   mc_DataBlockPaths.clear();
   for (uint32_t i = 0; i < u32_Count; ++i) {
      QString c_Path;
      orc_Stream >> c_Path;
      mc_DataBlockPaths.append(c_Path);
   }
   
   // Deserialize file based paths
   orc_Stream >> u32_Count;
   mc_FileBasedPaths.clear();
   for (uint32_t i = 0; i < u32_Count; ++i) {
      QString c_Path;
      orc_Stream >> c_Path;
      mc_FileBasedPaths.append(c_Path);
   }
   
   // Deserialize param set paths
   orc_Stream >> u32_Count;
   mc_ParamSetPaths.clear();
   for (uint32_t i = 0; i < u32_Count; ++i) {
      C_OscViewNodeUpdateParamInfo c_Info;
      c_Info.FromQDataStream(orc_Stream);
      mc_ParamSetPaths.append(c_Info);
   }
   
   // Deserialize skip flags
   orc_Stream >> u32_Count;
   mc_SkipUpdateOfFiles.clear();
   for (uint32_t i = 0; i < u32_Count; ++i) {
      uint32_t u32_FlagCount = 0;
      orc_Stream >> u32_FlagCount;
      QList<bool> c_Flags;
      for (uint32_t j = 0; j < u32_FlagCount; ++j) {
         bool q_Flag = false;
         orc_Stream >> q_Flag;
         c_Flags.append(q_Flag);
      }
      mc_SkipUpdateOfFiles.append(c_Flags);
   }
   
   if (orc_Stream.status() == QDataStream::Ok) {
      return C_NO_ERR;
   } else {
      return C_RD_WR;
   }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QJsonObject

   \return JSON object containing all data
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscViewNodeUpdate::ToJsonObject() const {
   QJsonObject c_Object;
   
   // Serialize enums as strings
   switch (me_StateSecurity) {
      case eST_SEC_NO_CHANGE: c_Object["state-security"] = "no-change"; break;
      case eST_SEC_ACTIVATE: c_Object["state-security"] = "activate"; break;
      case eST_SEC_DEACTIVATE: c_Object["state-security"] = "deactivate"; break;
   }
   
   switch (me_StateDebugger) {
      case eST_DEB_NO_CHANGE: c_Object["state-debugger"] = "no-change"; break;
      case eST_DEB_ACTIVATE: c_Object["state-debugger"] = "activate"; break;
      case eST_DEB_DEACTIVATE: c_Object["state-debugger"] = "deactivate"; break;
   }
   
   c_Object["pem-file-path"] = mc_PemFilePath;
   c_Object["skip-update-of-pem-file"] = mq_SkipUpdateOfPemFile;
   
   // Serialize data block paths
   QJsonArray c_DataBlockArray;
   for (const QString& c_Path : mc_DataBlockPaths) {
      c_DataBlockArray.append(c_Path);
   }
   c_Object["data-block-paths"] = c_DataBlockArray;
   
   // Serialize file based paths
   QJsonArray c_FileBasedArray;
   for (const QString& c_Path : mc_FileBasedPaths) {
      c_FileBasedArray.append(c_Path);
   }
   c_Object["file-based-paths"] = c_FileBasedArray;
   
   // Serialize param set paths
   QJsonArray c_ParamSetArray;
   for (const C_OscViewNodeUpdateParamInfo& c_Info : mc_ParamSetPaths) {
      c_ParamSetArray.append(c_Info.ToJsonObject());
   }
   c_Object["param-set-paths"] = c_ParamSetArray;
   
   // Serialize skip flags
   QJsonArray c_SkipFlagsArray;
   for (const QList<bool>& c_Flags : mc_SkipUpdateOfFiles) {
      QJsonArray c_FlagArray;
      for (bool q_Flag : c_Flags) {
         c_FlagArray.append(q_Flag);
      }
      c_SkipFlagsArray.append(c_FlagArray);
   }
   c_Object["skip-update-of-files"] = c_SkipFlagsArray;
   
   return c_Object;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QJsonObject

   \param[in] orc_Object    JSON object to deserialize from

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewNodeUpdate::FromJsonObject(const QJsonObject& orc_Object) {
   using namespace stw::errors;
   
   // Deserialize enums
   if (orc_Object.contains("state-security")) {
      QString c_Str = orc_Object["state-security"].toString();
      if (c_Str == "activate") me_StateSecurity = eST_SEC_ACTIVATE;
      else if (c_Str == "deactivate") me_StateSecurity = eST_SEC_DEACTIVATE;
      else me_StateSecurity = eST_SEC_NO_CHANGE;
   }
   
   if (orc_Object.contains("state-debugger")) {
      QString c_Str = orc_Object["state-debugger"].toString();
      if (c_Str == "activate") me_StateDebugger = eST_DEB_ACTIVATE;
      else if (c_Str == "deactivate") me_StateDebugger = eST_DEB_DEACTIVATE;
      else me_StateDebugger = eST_DEB_NO_CHANGE;
   }
   
   if (orc_Object.contains("pem-file-path")) {
      mc_PemFilePath = orc_Object["pem-file-path"].toString();
   }
   
   if (orc_Object.contains("skip-update-of-pem-file")) {
      mq_SkipUpdateOfPemFile = orc_Object["skip-update-of-pem-file"].toBool();
   }
   
   // Deserialize data block paths
   if (orc_Object.contains("data-block-paths")) {
      QJsonArray c_Array = orc_Object["data-block-paths"].toArray();
      mc_DataBlockPaths.clear();
      for (const QJsonValue& c_Value : c_Array) {
         mc_DataBlockPaths.append(c_Value.toString());
      }
   }
   
   // Deserialize file based paths
   if (orc_Object.contains("file-based-paths")) {
      QJsonArray c_Array = orc_Object["file-based-paths"].toArray();
      mc_FileBasedPaths.clear();
      for (const QJsonValue& c_Value : c_Array) {
         mc_FileBasedPaths.append(c_Value.toString());
      }
   }
   
   // Deserialize param set paths
   if (orc_Object.contains("param-set-paths")) {
      QJsonArray c_Array = orc_Object["param-set-paths"].toArray();
      mc_ParamSetPaths.clear();
      for (const QJsonValue& c_Value : c_Array) {
         C_OscViewNodeUpdateParamInfo c_Info;
         c_Info.FromJsonObject(c_Value.toObject());
         mc_ParamSetPaths.append(c_Info);
      }
   }
   
   // Deserialize skip flags
   if (orc_Object.contains("skip-update-of-files")) {
      QJsonArray c_Array = orc_Object["skip-update-of-files"].toArray();
      mc_SkipUpdateOfFiles.clear();
      for (const QJsonValue& c_Value : c_Array) {
         QJsonArray c_FlagArray = c_Value.toArray();
         QList<bool> c_Flags;
         for (const QJsonValue& c_Flag : c_FlagArray) {
            c_Flags.append(c_Flag.toBool());
         }
         mc_SkipUpdateOfFiles.append(c_Flags);
      }
   }
   
   return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Serialize to QDomDocument

   \param[in] orc_Doc    XML document to append to
   \param[in] orc_RootElementName    Name of the root element

   \return QDomElement representing the serialized data
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscViewNodeUpdate::ToQDomDocument(QDomDocument& orc_Doc, 
                                                const QString& orc_RootElementName) const {
   QDomElement c_Element = orc_Doc.createElement(orc_RootElementName);
   
   // Serialize enums as text
   QDomElement c_StateSecurityElement = orc_Doc.createElement("state-security");
   QString c_StateSecurityStr;
   switch (me_StateSecurity) {
      case eST_SEC_ACTIVATE: c_StateSecurityStr = "activate"; break;
      case eST_SEC_DEACTIVATE: c_StateSecurityStr = "deactivate"; break;
      default: c_StateSecurityStr = "no-change"; break;
   }
   c_StateSecurityElement.appendChild(orc_Doc.createTextNode(c_StateSecurityStr));
   c_Element.appendChild(c_StateSecurityElement);
   
   QDomElement c_StateDebuggerElement = orc_Doc.createElement("state-debugger");
   QString c_StateDebuggerStr;
   switch (me_StateDebugger) {
      case eST_DEB_ACTIVATE: c_StateDebuggerStr = "activate"; break;
      case eST_DEB_DEACTIVATE: c_StateDebuggerStr = "deactivate"; break;
      default: c_StateDebuggerStr = "no-change"; break;
   }
   c_StateDebuggerElement.appendChild(orc_Doc.createTextNode(c_StateDebuggerStr));
   c_Element.appendChild(c_StateDebuggerElement);
   
   // Serialize strings
   QDomElement c_PemFilePathElement = orc_Doc.createElement("pem-file-path");
   c_PemFilePathElement.appendChild(orc_Doc.createTextNode(mc_PemFilePath));
   c_Element.appendChild(c_PemFilePathElement);
   
   QDomElement c_SkipPemElement = orc_Doc.createElement("skip-update-of-pem-file");
   c_SkipPemElement.appendChild(orc_Doc.createTextNode(mq_SkipUpdateOfPemFile ? "1" : "0"));
   c_Element.appendChild(c_SkipPemElement);
   
   // Serialize data block paths
   QDomElement c_DataBlockElement = orc_Doc.createElement("data-block-paths");
   for (const QString& c_Path : mc_DataBlockPaths) {
      QDomElement c_PathElement = orc_Doc.createElement("path");
      c_PathElement.appendChild(orc_Doc.createTextNode(c_Path));
      c_DataBlockElement.appendChild(c_PathElement);
   }
   c_Element.appendChild(c_DataBlockElement);
   
   // Serialize file based paths
   QDomElement c_FileBasedElement = orc_Doc.createElement("file-based-paths");
   for (const QString& c_Path : mc_FileBasedPaths) {
      QDomElement c_PathElement = orc_Doc.createElement("path");
      c_PathElement.appendChild(orc_Doc.createTextNode(c_Path));
      c_FileBasedElement.appendChild(c_PathElement);
   }
   c_Element.appendChild(c_FileBasedElement);
   
   // Serialize param set paths
   QDomElement c_ParamSetElement = orc_Doc.createElement("param-set-paths");
   for (const C_OscViewNodeUpdateParamInfo& c_Info : mc_ParamSetPaths) {
      c_ParamSetElement.appendChild(c_Info.ToQDomDocument(orc_Doc, "param-info"));
   }
   c_Element.appendChild(c_ParamSetElement);
   
   // Serialize skip flags
   QDomElement c_SkipFlagsElement = orc_Doc.createElement("skip-update-of-files");
   for (const QList<bool>& c_Flags : mc_SkipUpdateOfFiles) {
      QDomElement c_FlagGroupElement = orc_Doc.createElement("flag-group");
      for (bool q_Flag : c_Flags) {
         QDomElement c_FlagElement = orc_Doc.createElement("flag");
         c_FlagElement.appendChild(orc_Doc.createTextNode(q_Flag ? "1" : "0"));
         c_FlagGroupElement.appendChild(c_FlagElement);
      }
      c_SkipFlagsElement.appendChild(c_FlagGroupElement);
   }
   c_Element.appendChild(c_SkipFlagsElement);
   
   return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief Deserialize from QDomElement

   \param[in] orc_Element    XML element to deserialize from

   \return C_NO_ERR on success
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscViewNodeUpdate::FromQDomElement(const QDomElement& orc_Element) {
   using namespace stw::errors;
   
   // Deserialize enums
   QDomNode c_StateSecurityNode = orc_Element.namedItem("state-security");
   if (!c_StateSecurityNode.isNull()) {
      QString c_Str = c_StateSecurityNode.toElement().text();
      if (c_Str == "activate") me_StateSecurity = eST_SEC_ACTIVATE;
      else if (c_Str == "deactivate") me_StateSecurity = eST_SEC_DEACTIVATE;
      else me_StateSecurity = eST_SEC_NO_CHANGE;
   }
   
   QDomNode c_StateDebuggerNode = orc_Element.namedItem("state-debugger");
   if (!c_StateDebuggerNode.isNull()) {
      QString c_Str = c_StateDebuggerNode.toElement().text();
      if (c_Str == "activate") me_StateDebugger = eST_DEB_ACTIVATE;
      else if (c_Str == "deactivate") me_StateDebugger = eST_DEB_DEACTIVATE;
      else me_StateDebugger = eST_DEB_NO_CHANGE;
   }
   
   // Deserialize strings
   QDomNode c_PemFilePathNode = orc_Element.namedItem("pem-file-path");
   if (!c_PemFilePathNode.isNull()) {
      mc_PemFilePath = c_PemFilePathNode.toElement().text();
   }
   
   QDomNode c_SkipPemNode = orc_Element.namedItem("skip-update-of-pem-file");
   if (!c_SkipPemNode.isNull()) {
      mq_SkipUpdateOfPemFile = (c_SkipPemNode.toElement().text() == "1");
   }
   
   // Deserialize data block paths
   QDomNode c_DataBlockNode = orc_Element.namedItem("data-block-paths");
   if (!c_DataBlockNode.isNull()) {
      QDomElement c_DataBlockElement = c_DataBlockNode.toElement();
      mc_DataBlockPaths.clear();
      QDomNode c_PathNode = c_DataBlockElement.firstChild();
      while (!c_PathNode.isNull()) {
         if (c_PathNode.isElement()) {
            mc_DataBlockPaths.append(c_PathNode.toElement().text());
         }
         c_PathNode = c_PathNode.nextSibling();
      }
   }
   
   // Deserialize file based paths
   QDomNode c_FileBasedNode = orc_Element.namedItem("file-based-paths");
   if (!c_FileBasedNode.isNull()) {
      QDomElement c_FileBasedElement = c_FileBasedNode.toElement();
      mc_FileBasedPaths.clear();
      QDomNode c_PathNode = c_FileBasedElement.firstChild();
      while (!c_PathNode.isNull()) {
         if (c_PathNode.isElement()) {
            mc_FileBasedPaths.append(c_PathNode.toElement().text());
         }
         c_PathNode = c_PathNode.nextSibling();
      }
   }
   
   // Deserialize param set paths
   QDomNode c_ParamSetNode = orc_Element.namedItem("param-set-paths");
   if (!c_ParamSetNode.isNull()) {
      QDomElement c_ParamSetElement = c_ParamSetNode.toElement();
      mc_ParamSetPaths.clear();
      QDomNode c_InfoNode = c_ParamSetElement.firstChild();
      while (!c_InfoNode.isNull()) {
         if (c_InfoNode.isElement()) {
            C_OscViewNodeUpdateParamInfo c_Info;
            c_Info.FromQDomElement(c_InfoNode.toElement());
            mc_ParamSetPaths.append(c_Info);
         }
         c_InfoNode = c_InfoNode.nextSibling();
      }
   }
   
   // Deserialize skip flags
   QDomNode c_SkipFlagsNode = orc_Element.namedItem("skip-update-of-files");
   if (!c_SkipFlagsNode.isNull()) {
      QDomElement c_SkipFlagsElement = c_SkipFlagsNode.toElement();
      mc_SkipUpdateOfFiles.clear();
      QDomNode c_GroupNode = c_SkipFlagsElement.firstChild();
      while (!c_GroupNode.isNull()) {
         if (c_GroupNode.isElement()) {
            QDomElement c_GroupElement = c_GroupNode.toElement();
            QList<bool> c_Flags;
            QDomNode c_FlagNode = c_GroupElement.firstChild();
            while (!c_FlagNode.isNull()) {
               if (c_FlagNode.isElement()) {
                  c_Flags.append(c_FlagNode.toElement().text() == "1");
               }
               c_FlagNode = c_FlagNode.nextSibling();
            }
            mc_SkipUpdateOfFiles.append(c_Flags);
         }
         c_GroupNode = c_GroupNode.nextSibling();
      }
   }
   
   return C_NO_ERR;
}
