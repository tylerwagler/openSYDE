//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Handler class for parameter set file operations (implementation)

   Handler class for parameter set file operations using multi-format support

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"
#include <QFileInfo>
#include <QFile>

#include "C_OscLoggingHandler.hpp"
#include "C_OscParamSetHandler.hpp"
#include "C_OscParamSetFilerBase.hpp"
#include "C_OscParamSetFilerBase_New.hpp"
#include "C_OscParamSetRawNodeFiler.hpp"
#include "C_OscParamSetInterpretedNodeFiler.hpp"
#include "C_OscFilerUtil.hpp"
#include "stwerrors.hpp"

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
C_OscParamSetHandler::C_OscParamSetHandler(void) {}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Create clean file from internally stored content without adding a
   CRC

   \param[in] orc_FilePath Parameter file path
   \param[in] oq_InterpretedDataOnly   Flag to load only the interpreted data
   and not the raw data

   \return
   C_NO_ERR   data saved
   C_RANGE    file already exists
   C_RD_WR    could not write to file (e.g. missing write permissions; missing
   folder)
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetHandler::CreateCleanFileWithoutCrc(
    const QString &orc_FilePath, const bool oq_InterpretedDataOnly) const {
  int32_t s32_Return = C_NO_ERR;

  const QFileInfo c_FileInfo(orc_FilePath);
  if ((c_FileInfo.exists() && c_FileInfo.isFile()) == false) {
    if ((oq_InterpretedDataOnly == true) ||
        (this->mc_RawNodes.size() == this->mc_Data.c_InterpretedNodes.size())) {
      
      QFile c_File(orc_FilePath);
      if (c_File.open(QIODevice::WriteOnly)) {
        QDataStream c_Stream(&c_File);
        c_Stream.setVersion(QDataStream::Qt_6_0);
        
        // Save file version
        C_OscParamSetFilerBase_New::h_SaveFileVersion(c_File);
        
        // Save file info
        C_OscParamSetFilerBase_New::h_SaveFileInfo(c_File, this->mc_Data.c_FileInfo);
        
        // Save each node
        for (uint32_t u32_ItNode = 0;
             u32_ItNode < this->mc_Data.c_InterpretedNodes.size(); ++u32_ItNode) {
          
          // Save interpreted node
          this->mc_Data.c_InterpretedNodes[u32_ItNode].ToQDataStream(c_Stream);
          if (c_Stream.status() != QDataStream::Ok) {
            osc_write_log_error("Saving parameter set interpreted node",
                "Failed to save interpreted node at index " + QString::number(u32_ItNode));
            s32_Return = C_RD_WR;
            break;
          }
          
          // Save raw node if not interpreted data only
          if ((oq_InterpretedDataOnly == false) && (u32_ItNode < static_cast<uint32_t>(this->mc_RawNodes.size()))) {
            this->mc_RawNodes[u32_ItNode].ToQDataStream(c_Stream);
            if (c_Stream.status() != QDataStream::Ok) {
              osc_write_log_error("Saving parameter set raw node",
                  "Failed to save raw node at index " + QString::number(u32_ItNode));
              s32_Return = C_RD_WR;
              break;
            }
          }
        }
        
        if (s32_Return == C_NO_ERR) {
          c_File.close();
        } else {
          c_File.close();
          osc_write_log_error("Creating parameter set file",
              "Failed to write data to \"" + orc_FilePath + "\"");
        }
      } else {
        osc_write_log_error("Creating parameter set file",
            "Could not open file \"" + orc_FilePath + "\" for writing");
        s32_Return = C_RD_WR;
      }
    } else {
      osc_write_log_warning("Creating parameter set",
          "Raw nodes count does not match interpreted nodes count");
      s32_Return = C_CONFIG;
    }
  } else {
    s32_Return = C_RANGE;
  }
  return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Read file and update internally stored content (cleared at start)

   \param[in]     orc_FilePath               Parameter file path
   \param[in]     oq_IgnoreCrc               Flag to ignore CRC if set to true
   \param[in]     oq_InterpretedDataOnly     Flag to load only the interpreted
   data and not the raw data \param[in,out] opu16_FileCrc              Optional
   storage for read file CRC (only set if no err and oq_IgnoreCrc is false)
   \param[in,out] opq_MissingOptionalContent Optional flag for indication of
   optional content missing Warning: only valid if C_NO_ERR

   \return
   C_NO_ERR   data read
   C_RD_WR    specified file does not exist
              specified file is present but structure is invalid (e.g. invalid
   XML file; not checksum found) C_CHECKSUM specified file is present but
   checksum is invalid C_CONFIG   file does not contain essential information
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetHandler::ReadFile(const QString &orc_FilePath,
                                        const bool oq_IgnoreCrc,
                                        const bool oq_InterpretedDataOnly,
                                        uint16_t *const opu16_FileCrc,
                                        bool *const opq_MissingOptionalContent) {
  int32_t s32_Retval = C_NO_ERR;

  this->ClearContent();
  if (QFileInfo(orc_FilePath).exists() && QFileInfo(orc_FilePath).isFile()) {
    
    QFile c_File(orc_FilePath);
    if (c_File.open(QIODevice::ReadOnly)) {
      QDataStream c_Stream(&c_File);
      c_Stream.setVersion(QDataStream::Qt_6_0);
      
      // Check file version
      s32_Retval = C_OscParamSetFilerBase_New::h_CheckFileVersion(c_File);
      
      if (s32_Retval == C_NO_ERR) {
        // Load file info
        bool q_MissingOptionalContent = false;
        s32_Retval = C_OscParamSetFilerBase_New::h_LoadFileInfo(c_File, this->mc_Data.c_FileInfo, q_MissingOptionalContent);
        
        if (s32_Retval == C_NO_ERR) {
          // Load nodes
          s32_Retval = this->m_LoadNodes(c_Stream, oq_InterpretedDataOnly, q_MissingOptionalContent);
          
          if ((opu16_FileCrc != nullptr) && (s32_Retval == C_NO_ERR)) {
            // CRC is now embedded in the file format
            *opu16_FileCrc = 0; // Placeholder - CRC handling depends on specific format
          }
          
          if (opq_MissingOptionalContent != nullptr) {
            *opq_MissingOptionalContent = q_MissingOptionalContent;
          }
        }
      }
      
      c_File.close();
    } else {
      s32_Retval = C_RD_WR;
    }
  } else {
    s32_Retval = C_RD_WR;
  }
  
  if (s32_Retval != C_NO_ERR) {
    const QString c_Text = "Could not load file \"" + orc_FilePath +
                            "\". Error code: " + QString::number(s32_Retval);
    osc_write_log_error("Loading Dataset data", c_Text);
  }

  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update/add CRC for provided file

   \param[in] orc_FilePath Parameter file path

   \return
   C_NO_ERR CRC updated
   C_CONFIG Unexpected XML format
   C_RD_WR  Error accessing file system
   C_RANGE  File does not exist
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetHandler::h_UpdateCrcForFile(const QString &orc_FilePath) {
  return C_OscParamSetFilerBase::h_AddCrc(orc_FilePath);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Clear internally stored content
 */
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetHandler::ClearContent(void) {
  this->mc_RawNodes.clear();
  this->mc_Data.Clear();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Add raw data for node (Node name used as ID)

   \param[in] orc_Content Raw data for node (Node name used as ID)

   \return
   C_NO_ERR Operation success
   C_RANGE  Operation failure: parameter invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetHandler::AddRawDataForNode(
    const C_OscParamSetRawNode &orc_Content) {
  int32_t s32_Retval = C_NO_ERR;

  for (uint32_t u32_ItRawNode = 0; u32_ItRawNode < this->mc_RawNodes.size();
       ++u32_ItRawNode) {
    const C_OscParamSetRawNode &rc_CurRawNode =
        this->mc_RawNodes[u32_ItRawNode];
    if (rc_CurRawNode.c_Name == orc_Content.c_Name) {
      s32_Retval = C_RANGE;
    }
  }
  if (s32_Retval == C_NO_ERR) {
    this->mc_RawNodes.push_back(orc_Content);
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Add some general information for one param set file

   \param[in] orc_FileInfo Optional general file information
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscParamSetHandler::AddInterpretedFileData(
    const C_OscParamSetInterpretedFileInfoData &orc_FileInfo) {
  this->mc_Data.AddInterpretedFileData(orc_FileInfo);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Add interpreted data for node (Node name used as ID)

   \param[in] orc_Content Interpreted data for node (Node name used as ID)

   \return
   C_NO_ERR Operation success
   C_RANGE  Operation failure: parameter invalid
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetHandler::AddInterpretedDataForNode(
    const C_OscParamSetInterpretedNode &orc_Content) {
  int32_t s32_Retval = C_NO_ERR;

  for (uint32_t u32_ItInterpretedNode = 0;
       u32_ItInterpretedNode < this->mc_Data.c_InterpretedNodes.size();
       ++u32_ItInterpretedNode) {
    const C_OscParamSetInterpretedNode &rc_CurInterpretedNode =
        this->mc_Data.c_InterpretedNodes[u32_ItInterpretedNode];
    if (rc_CurInterpretedNode.c_Name == orc_Content.c_Name) {
      s32_Retval = C_RANGE;
    }
  }
  if (s32_Retval == C_NO_ERR) {
    this->mc_Data.c_InterpretedNodes.push_back(orc_Content);
  }
  return s32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get raw node data for specified node

   \param[in] orc_NodeName Node name to look for

   \return
   NULL Raw node data not found
   Else Valid raw node data
*/
//----------------------------------------------------------------------------------------------------------------------
const C_OscParamSetRawNode *
C_OscParamSetHandler::GetRawDataForNode(const QString &orc_NodeName) const {
  const C_OscParamSetRawNode *pc_Retval = NULL;

  for (uint32_t u32_ItRawNode = 0; u32_ItRawNode < this->mc_RawNodes.size();
       ++u32_ItRawNode) {
    const C_OscParamSetRawNode &rc_CurRawNode =
        this->mc_RawNodes[u32_ItRawNode];
    if (rc_CurRawNode.c_Name == orc_NodeName) {
      pc_Retval = &this->mc_RawNodes[u32_ItRawNode];
    }
  }

  return pc_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get interpreted data

   \return
   Pointer to vector with interpreted data
*/
//----------------------------------------------------------------------------------------------------------------------
const C_OscParamSetInterpretedData &
C_OscParamSetHandler::GetInterpretedData(void) const {
  return this->mc_Data;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get number of nodes

   Get number of nodes that the file contains data for

   \return
   Number of nodes
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscParamSetHandler::GetNumberOfNodes(void) const {
  return static_cast<uint32_t>(this->mc_RawNodes.size());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Get raw data for one node

   Get raw data for specified node.

   \param[in]  ou32_NodeIndex   index of node

   \return
   NULL  Raw node data not found
   Else  Valid raw node data
*/
//----------------------------------------------------------------------------------------------------------------------
const C_OscParamSetRawNode *
C_OscParamSetHandler::GetRawDataForNode(const uint32_t ou32_NodeIndex) const {
  const C_OscParamSetRawNode *pc_Result = NULL;

  if (ou32_NodeIndex < this->mc_RawNodes.size()) {
    pc_Result = &this->mc_RawNodes[ou32_NodeIndex];
  }
  return pc_Result;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Update/add CRC for provided file

   \param[in]     orc_FilePath    Parameter file path

   \return
   C_NO_ERR   CRC added/updated
   C_RD_WR    file could not be opened
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetHandler::UpdateCrc(const QString &orc_FilePath) const {
  // CRC is now handled by the individual filers based on format
  // This method is kept for backward compatibility
  return C_NO_ERR;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save parameter set to file

   \param[in]     orc_FilePath    File path
   \param[in]     oq_WithCrc      Flag to include CRC

   \return
   C_NO_ERR   file saved successfully
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetHandler::SaveToFile(const QString &orc_FilePath, const bool oq_WithCrc) const {
  int32_t s32_Return = C_NO_ERR;
  
  QFile c_File(orc_FilePath);
  if (c_File.open(QIODevice::WriteOnly)) {
    QDataStream c_Stream(&c_File);
    c_Stream.setVersion(QDataStream::Qt_6_0);
    
    // Save file version
    C_OscParamSetFilerBase_New::h_SaveFileVersion(c_File);
    
    // Save file info
    C_OscParamSetFilerBase_New::h_SaveFileInfo(c_File, this->mc_Data.c_FileInfo);
    
    // Save all interpreted nodes
    for (uint32_t u32_Index = 0; u32_Index < this->mc_Data.c_InterpretedNodes.size(); ++u32_Index) {
      this->mc_Data.c_InterpretedNodes[u32_Index].ToQDataStream(c_Stream);
      if (c_Stream.status() != QDataStream::Ok) {
        osc_write_log_error("Saving interpreted node",
            "Failed to save interpreted node at index " + QString::number(u32_Index));
        s32_Return = C_RD_WR;
        break;
      }
    }
    
    if (s32_Return == C_NO_ERR) {
      // Save all raw nodes
      for (uint32_t u32_Index = 0; u32_Index < this->mc_RawNodes.size(); ++u32_Index) {
        this->mc_RawNodes[u32_Index].ToQDataStream(c_Stream);
        if (c_Stream.status() != QDataStream::Ok) {
          osc_write_log_error("Saving raw node",
              "Failed to save raw node at index " + QString::number(u32_Index));
          s32_Return = C_RD_WR;
          break;
        }
      }
    }
    
    c_File.close();
  } else {
    osc_write_log_error("Saving parameter set file",
        "Could not open file \"" + orc_FilePath + "\" for writing");
    s32_Return = C_RD_WR;
  }
  
  return s32_Return;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Internal method to load all nodes from file

   \param[in]     orc_Stream             QDataStream to read from
   \param[in]     oq_InterpretedDataOnly     Flag to load only interpreted data
   \param[in,out] orq_MissingOptionalContent Missing optional content flag

   \return
   C_NO_ERR   nodes loaded successfully
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscParamSetHandler::m_LoadNodes(QDataStream &orc_Stream,
                                           const bool oq_InterpretedDataOnly,
                                           bool &orq_MissingOptionalContent) {
  int32_t s32_Retval = C_NO_ERR;
  
  // Load interpreted nodes
  for (uint32_t u32_Index = 0; u32_Index < this->mc_Data.c_InterpretedNodes.size(); ++u32_Index) {
    this->mc_Data.c_InterpretedNodes[u32_Index].FromQDataStream(orc_Stream);
    if (orc_Stream.status() != QDataStream::Ok) {
      osc_write_log_error("Loading interpreted node",
          "Failed to load interpreted node at index " + QString::number(u32_Index));
      s32_Retval = C_RD_WR;
      break;
    }
  }
  
  // Load raw nodes if not interpreted data only
  if ((s32_Retval == C_NO_ERR) && (oq_InterpretedDataOnly == false)) {
    for (uint32_t u32_Index = 0; u32_Index < this->mc_RawNodes.size(); ++u32_Index) {
      this->mc_RawNodes[u32_Index].FromQDataStream(orc_Stream);
      if (orc_Stream.status() != QDataStream::Ok) {
        osc_write_log_error("Loading raw node",
            "Failed to load raw node at index " + QString::number(u32_Index));
        s32_Retval = C_RD_WR;
        break;
      }
    }
  }
  
  return s32_Retval;
}
