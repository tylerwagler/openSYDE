//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Data class for a complete data pool (implementation)

   Data class for a complete data pool

   \copyright   Copyright 2016 Sensor-Technik Wiedemann GmbH. All rights
   reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes
 * ------------------------------------------------------------------------------------------------------
 */
#include "precomp_headers.hpp"

#include <QMap>
#include <QString>
#include "C_OscNodeDataPool.hpp"
#include <QJsonArray>
#include <QJsonValue>

#include "C_OscHashUtil.hpp"
#include "C_OscUtils.hpp"
#include "C_SclChecksums.hpp"
#include "stwerrors.hpp"

/* -- Used Namespaces
 * -----------------------------------------------------------------------------------------------
 */

using namespace stw::opensyde_core;
using namespace stw::errors;
using namespace stw::scl;

/* -- Module Global Constants
 * ---------------------------------------------------------------------------------------
 */
const uint32_t C_OscNodeDataPool::hu32_DEFAULT_NVM_SIZE = 1000UL;

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
/*! \brief  Default constructor
 */
//----------------------------------------------------------------------------------------------------------------------
C_OscNodeDataPool::C_OscNodeDataPool(void)
    : e_Type(eDIAG), c_Name("NewDataPool"), u16_DefinitionCrcVersion(2),
      c_Comment(""), s32_RelatedDataBlockIndex(-1), q_IsSafety(false),
      q_ScopeIsPrivate(true), u32_NvmStartAddress(0), u32_NvmSize(0),
      c_Lists() {
  this->au8_Version[0] = 0;
  this->au8_Version[1] = 0;
  this->au8_Version[2] = 0;
  c_Lists.resize(1); // one default list for DIAG DP
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over all data

   The hash value is a 32 bit CRC value.
   It is not endian-safe, so it should only be used on the same system it is
   created on.

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and
   result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPool::CalcHash(uint32_t &oru32_HashValue) const {
  // pc_RelatedApplication is dynamic
  hash_util::CalcHashMembers(oru32_HashValue,
                             this->e_Type, this->c_Name, this->au8_Version, this->c_Comment,
                             this->s32_RelatedDataBlockIndex, this->q_IsSafety, this->q_ScopeIsPrivate,
                             this->u32_NvmStartAddress, this->u32_NvmSize, this->u16_DefinitionCrcVersion,
                             this->c_Lists);
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over data pool definition

   The hash value is a 32 bit CRC value.
   Only essential data are covered.
   It is endian-safe, so it may be used on systems with different endianness.

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and
   result [out] value
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPool::CalcGeneratedDefinitionHash(
    uint32_t &oru32_HashValue) const {
  if (this->u16_DefinitionCrcVersion == 1) {
    this->CalcDefinitionHash(oru32_HashValue, eCT_COMPAT_V1);
  } else {
    if ((this->e_Type == eNVM) || (this->e_Type == eHALC_NVM)) {
      this->CalcDefinitionHash(oru32_HashValue, eCT_NVM);
    } else {
      this->CalcDefinitionHash(oru32_HashValue, eCT_NON_NVM);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Calculates the hash value over data pool definition

   The hash value is a 32 bit CRC value.
   Only essential data are covered.
   It is endian-safe, so it may be used on systems with different endianness.

   \param[in,out]  oru32_HashValue  Hash value with initial [in] value and
   result [out] value \param[in]      oe_CrcType       Crc type
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPool::CalcDefinitionHash(uint32_t &oru32_HashValue,
                                           const E_CrcType oe_CrcType) const {
  uint8_t au8_Data[4];

  stw::scl::C_SclChecksums::CalcCRC32(&this->e_Type, 1U, oru32_HashValue);

  if ((oe_CrcType == eCT_COMPAT_V1) || (oe_CrcType == eCT_NVM) ||
      (oe_CrcType == eCT_NON_NVM_DEFAULT_COMPAT_V1)) {
    uint32_t u32_DpNvmStartAddress;
    uint32_t u32_DpNvmSize;
    if (oe_CrcType == eCT_NON_NVM_DEFAULT_COMPAT_V1) {
      u32_DpNvmStartAddress = 0UL;
      u32_DpNvmSize = C_OscNodeDataPool::hu32_DEFAULT_NVM_SIZE;
    } else {
      u32_DpNvmStartAddress = this->u32_NvmStartAddress;
      u32_DpNvmSize = this->u32_NvmSize;
    }
    au8_Data[0] = static_cast<uint8_t>(u32_DpNvmStartAddress);
    au8_Data[1] = static_cast<uint8_t>(u32_DpNvmStartAddress >> 8U);
    au8_Data[2] = static_cast<uint8_t>(u32_DpNvmStartAddress >> 16U);
    au8_Data[3] = static_cast<uint8_t>(u32_DpNvmStartAddress >> 24U);
    stw::scl::C_SclChecksums::CalcCRC32(
        &au8_Data[0], sizeof(u32_DpNvmStartAddress), oru32_HashValue);

    au8_Data[0] = static_cast<uint8_t>(u32_DpNvmSize);
    au8_Data[1] = static_cast<uint8_t>(u32_DpNvmSize >> 8U);
    au8_Data[2] = static_cast<uint8_t>(u32_DpNvmSize >> 16U);
    au8_Data[3] = static_cast<uint8_t>(u32_DpNvmSize >> 24U);
    stw::scl::C_SclChecksums::CalcCRC32(&au8_Data[0], sizeof(u32_DpNvmSize),
                                        oru32_HashValue);
  }

  for (uint32_t u32_ListCount = 0U; u32_ListCount < this->c_Lists.size();
       u32_ListCount++) {
    if ((oe_CrcType == eCT_COMPAT_V1) || (oe_CrcType == eCT_NVM) ||
        (oe_CrcType == eCT_NON_NVM_DEFAULT_COMPAT_V1)) {
      bool q_ListNvmCrcActive;
      if (oe_CrcType == eCT_NON_NVM_DEFAULT_COMPAT_V1) {
        q_ListNvmCrcActive = false;
      } else {
        q_ListNvmCrcActive = this->c_Lists[u32_ListCount].q_NvmCrcActive;
      }
      stw::scl::C_SclChecksums::CalcCRC32(&q_ListNvmCrcActive, 1U,
                                          oru32_HashValue);
    }

    if (oe_CrcType == eCT_NVM) {
      au8_Data[0] = static_cast<uint8_t>(
          this->c_Lists[u32_ListCount].u32_NvmStartAddress);
      au8_Data[1] = static_cast<uint8_t>(
          this->c_Lists[u32_ListCount].u32_NvmStartAddress >> 8U);
      au8_Data[2] = static_cast<uint8_t>(
          this->c_Lists[u32_ListCount].u32_NvmStartAddress >> 16U);
      au8_Data[3] = static_cast<uint8_t>(
          this->c_Lists[u32_ListCount].u32_NvmStartAddress >> 24U);
      stw::scl::C_SclChecksums::CalcCRC32(
          &au8_Data[0],
          sizeof(this->c_Lists[u32_ListCount].u32_NvmStartAddress),
          oru32_HashValue);

      au8_Data[0] =
          static_cast<uint8_t>(this->c_Lists[u32_ListCount].u32_NvmSize);
      au8_Data[1] =
          static_cast<uint8_t>(this->c_Lists[u32_ListCount].u32_NvmSize >> 8U);
      au8_Data[2] =
          static_cast<uint8_t>(this->c_Lists[u32_ListCount].u32_NvmSize >> 16U);
      au8_Data[3] =
          static_cast<uint8_t>(this->c_Lists[u32_ListCount].u32_NvmSize >> 24U);
      stw::scl::C_SclChecksums::CalcCRC32(
          &au8_Data[0], sizeof(this->c_Lists[u32_ListCount].u32_NvmSize),
          oru32_HashValue);
    }

    for (uint32_t u32_ElementCount = 0U;
         u32_ElementCount < this->c_Lists[u32_ListCount].c_Elements.size();
         u32_ElementCount++) {
      const uint32_t u32_Size = this->c_Lists[u32_ListCount]
                                    .c_Elements[u32_ElementCount]
                                    .GetSizeByte();
      const uint8_t u8_Type = static_cast<uint8_t>(
          this->c_Lists[u32_ListCount].c_Elements[u32_ElementCount].GetType());
      au8_Data[0] = static_cast<uint8_t>(u32_Size);
      au8_Data[1] = static_cast<uint8_t>(u32_Size >> 8U);
      au8_Data[2] = static_cast<uint8_t>(u32_Size >> 16U);
      au8_Data[3] = static_cast<uint8_t>(u32_Size >> 24U);
      stw::scl::C_SclChecksums::CalcCRC32(&au8_Data[0], sizeof(u32_Size),
                                          oru32_HashValue);
      stw::scl::C_SclChecksums::CalcCRC32(&u8_Type, sizeof(u8_Type),
                                          oru32_HashValue);

      if (oe_CrcType == eCT_NVM) {
        au8_Data[0] = static_cast<uint8_t>(this->c_Lists[u32_ListCount]
                                               .c_Elements[u32_ElementCount]
                                               .u32_NvmStartAddress);
        au8_Data[1] = static_cast<uint8_t>(this->c_Lists[u32_ListCount]
                                               .c_Elements[u32_ElementCount]
                                               .u32_NvmStartAddress >>
                                           8U);
        au8_Data[2] = static_cast<uint8_t>(this->c_Lists[u32_ListCount]
                                               .c_Elements[u32_ElementCount]
                                               .u32_NvmStartAddress >>
                                           16U);
        au8_Data[3] = static_cast<uint8_t>(this->c_Lists[u32_ListCount]
                                               .c_Elements[u32_ElementCount]
                                               .u32_NvmStartAddress >>
                                           24U);
        stw::scl::C_SclChecksums::CalcCRC32(
            &au8_Data[0],
            sizeof(this->c_Lists[u32_ListCount]
                       .c_Elements[u32_ElementCount]
                       .u32_NvmStartAddress),
            oru32_HashValue);
      }
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Move list in data pool

   \param[in]  oru32_Start    Start index
   \param[in]  oru32_Target   Target index
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPool::MoveList(const uint32_t &oru32_Start,
                                 const uint32_t &oru32_Target) {
  if ((oru32_Start < this->c_Lists.size()) &&
      (oru32_Target < this->c_Lists.size())) {
    // Copy physically
    const C_OscNodeDataPoolList c_ListData = this->c_Lists[oru32_Start];
    // Erase
    this->c_Lists.erase(this->c_Lists.begin() + oru32_Start);
    // Insert
    this->c_Lists.insert(this->c_Lists.begin() + oru32_Target, c_ListData);
    if ((this->e_Type == C_OscNodeDataPool::eNVM) ||
        (this->e_Type == C_OscNodeDataPool::eHALC_NVM)) {
      RecalculateAddress();
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Recalculate data pool list addresses
 */
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPool::RecalculateAddress(void) {
  if ((this->e_Type == C_OscNodeDataPool::eNVM) ||
      (this->e_Type == C_OscNodeDataPool::eHALC_NVM)) {
    uint32_t u32_Offset = this->u32_NvmStartAddress;

    for (uint32_t u32_ItList = 0; u32_ItList < this->c_Lists.size();
         ++u32_ItList) {
      C_OscNodeDataPoolList &rc_List = this->c_Lists[u32_ItList];
      rc_List.u32_NvmStartAddress = u32_Offset;
      rc_List.RecalculateAddress();
      u32_Offset += rc_List.u32_NvmSize;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get number of bytes occupied by variables

   \return
   Number of bytes occupied by variables
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscNodeDataPool::GetNumBytesUsed(void) const {
  uint32_t u32_Retval = 0;

  for (uint32_t u32_ItList = 0; u32_ItList < this->c_Lists.size();
       ++u32_ItList) {
    const C_OscNodeDataPoolList &rc_NodeDataPoolList =
        this->c_Lists[u32_ItList];
    u32_Retval += rc_NodeDataPoolList.GetNumBytesUsed();
  }
  return u32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get number of bytes not occupied by variables

   \return
   Number of bytes not occupied by variables
*/
//----------------------------------------------------------------------------------------------------------------------
int32_t C_OscNodeDataPool::GetFreeBytes(void) const {
  return static_cast<int32_t>(static_cast<int64_t>(u32_NvmSize) -
                              static_cast<int64_t>(GetNumBytesUsed()));
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get number of bytes occupied by lists

   \return
   Number of bytes occupied by lists
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t C_OscNodeDataPool::GetListsSize(void) const {
  uint32_t u32_Retval = 0;

  for (uint32_t u32_ItList = 0; u32_ItList < this->c_Lists.size();
       ++u32_ItList) {
    const C_OscNodeDataPoolList &rc_NodeDataPoolList =
        this->c_Lists[u32_ItList];
    u32_Retval += rc_NodeDataPoolList.u32_NvmSize;
  }
  return u32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Check error for specified list

   \param[in]   oru32_ListIndex              Node data pool list index
   \param[out]  opq_NameConflict             Name conflict
   \param[out]  opq_NameInvalid              Name not usable as variable
   \param[out]  opq_UsageInvalid             Usage over 100.0%
   \param[out]  opq_OutOfDataPool            List out of data pool borders
   \param[out]  opq_DataSetsInvalid          One or more elements are invalid
   \param[out]  opq_ElementsInvalid          One or more elements are invalid
   \param[out]  opc_InvalidDataSetIndices    Indices of invalid data sets
   \param[out]  opc_InvalidElementIndices    Indices of invalid elements
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPool::CheckErrorList(
    const uint32_t &oru32_ListIndex, bool *const opq_NameConflict,
    bool *const opq_NameInvalid, bool *const opq_UsageInvalid,
    bool *const opq_OutOfDataPool, bool *const opq_DataSetsInvalid,
    bool *const opq_ElementsInvalid,
    QList<uint32_t> *const opc_InvalidDataSetIndices,
    QList<uint32_t> *const opc_InvalidElementIndices) const {
  // Init
  if (oru32_ListIndex < this->c_Lists.size()) {
    const C_OscNodeDataPoolList &rc_CheckedList =
        this->c_Lists[oru32_ListIndex];
    // Check variable name
    if (opq_NameInvalid != NULL) {
      if (C_OscUtils::h_CheckValidCeName(rc_CheckedList.c_Name) == false) {
        *opq_NameInvalid = true;
      } else {
        *opq_NameInvalid = false;
      }
    }
    // Name conflict
    if (opq_NameConflict != NULL) {
      *opq_NameConflict = false;
      for (uint32_t u32_ItElement = 0; u32_ItElement < this->c_Lists.size();
           ++u32_ItElement) {
        if (u32_ItElement != oru32_ListIndex) {
          const C_OscNodeDataPoolList &rc_List = this->c_Lists[u32_ItElement];
          if (rc_CheckedList.c_Name.toLower() == rc_List.c_Name.toLower()) {
            *opq_NameConflict = true;
          }
        }
      }
    }

    // Size
    if (opq_UsageInvalid != NULL) {
      // Usage
      if (rc_CheckedList.u32_NvmSize < rc_CheckedList.GetNumBytesUsed()) {
        *opq_UsageInvalid = true;
      } else {
        *opq_UsageInvalid = false;
      }
    }
    if (opq_OutOfDataPool != NULL) {
      uint32_t u32_NvmSizeReserved = 0;
      // Check reserved size
      for (uint32_t u32_ItList = 0; u32_ItList <= oru32_ListIndex;
           ++u32_ItList) {
        const C_OscNodeDataPoolList &rc_CurrentList = this->c_Lists[u32_ItList];
        u32_NvmSizeReserved += rc_CurrentList.u32_NvmSize;
      }
      if (u32_NvmSizeReserved <= this->u32_NvmSize) {
        *opq_OutOfDataPool = false;
      } else {
        *opq_OutOfDataPool = true;
      }
    }
    // Check data sets
    if (opq_DataSetsInvalid != NULL) {
      bool q_NameConflict;
      bool q_NameInvalid;
      *opq_DataSetsInvalid = false;
      for (uint32_t u32_ItDataSet = 0;
           (u32_ItDataSet < rc_CheckedList.c_DataSets.size()) &&
           ((*opq_DataSetsInvalid == false) ||
            (opc_InvalidDataSetIndices != NULL));
           ++u32_ItDataSet) {
        q_NameConflict = false;
        q_NameInvalid = false;
        rc_CheckedList.CheckErrorDataSet(u32_ItDataSet, &q_NameConflict,
                                         &q_NameInvalid);
        if ((q_NameConflict == true) || (q_NameInvalid == true)) {
          *opq_DataSetsInvalid = true;
          if (opc_InvalidDataSetIndices != NULL) {
            opc_InvalidDataSetIndices->push_back(u32_ItDataSet);
          }
        }
      }
    }
    // Check elements
    if (opq_ElementsInvalid != NULL) {
      QMap<QString, uint32_t> c_PreviousNames;
      static QMap<uint32_t, bool> hc_PreviousResults;
      bool q_NameInvalid;
      bool q_MinOverMax;
      bool q_DataSetInvalid;
      *opq_ElementsInvalid = false;
      for (uint32_t u32_ItElement = 0;
           (u32_ItElement < rc_CheckedList.c_Elements.size()) &&
           ((*opq_ElementsInvalid == false) ||
            (opc_InvalidElementIndices != NULL));
           ++u32_ItElement) {
        // Overarching checks
        const C_OscNodeDataPoolListElement &rc_Element =
            rc_CheckedList.c_Elements[u32_ItElement];
        const QMap<QString, uint32_t>::const_iterator c_ItElement =
            c_PreviousNames.constFind(rc_Element.c_Name.toLower());
        if (c_ItElement != c_PreviousNames.constEnd()) {
          *opq_ElementsInvalid = true;
          if (opc_InvalidElementIndices != NULL) {
            bool q_Added = false;
            // Only add element once!
            for (uint32_t u32_It = 0UL;
                 u32_It < opc_InvalidElementIndices->size(); ++u32_It) {
              if ((*opc_InvalidElementIndices)[u32_It] == c_ItElement.value()) {
                q_Added = true;
                break;
              }
            }
            // Add conflicting other item
            if (q_Added == false) {
              opc_InvalidElementIndices->push_back(c_ItElement.value());
            }
            // Add itself
            opc_InvalidElementIndices->push_back(u32_ItElement);
          }
        } else {
          // Get Hash for all relevant data
          const uint32_t u32_Hash =
              m_GetElementHash(oru32_ListIndex, u32_ItElement);
          // Check if check was already performed in the past
          const QMap<uint32_t, bool>::const_iterator c_ItErr =
              hc_PreviousResults.constFind(u32_Hash);
          // Append new name
          c_PreviousNames[rc_Element.c_Name.toLower()] = u32_ItElement;
          // Element specific checks
          if (c_ItErr == hc_PreviousResults.constEnd()) {
            q_NameInvalid = false;
            q_MinOverMax = false;
            q_DataSetInvalid = false;
            rc_CheckedList.CheckErrorElement(u32_ItElement, NULL,
                                             &q_NameInvalid, &q_MinOverMax,
                                             &q_DataSetInvalid, NULL);
            if (((q_NameInvalid == true) || (q_MinOverMax == true)) ||
                (q_DataSetInvalid == true)) {
              *opq_ElementsInvalid = true;
              if (opc_InvalidElementIndices != NULL) {
                opc_InvalidElementIndices->push_back(u32_ItElement);
              }
              // Append for possible reusing this result
              hc_PreviousResults[u32_Hash] = true;
            } else {
              // Append for possible reusing this result
              hc_PreviousResults[u32_Hash] = false;
            }
          } else {
            // Do not reset error
            *opq_ElementsInvalid = (*opq_ElementsInvalid) || c_ItErr.value();
            if ((opc_InvalidElementIndices != NULL) && (c_ItErr.value())) {
              opc_InvalidElementIndices->push_back(u32_ItElement);
            }
          }
        }
      }
    }
  } else {
    if (opq_NameConflict != NULL) {
      *opq_NameConflict = false;
    }
    if (opq_NameInvalid != NULL) {
      *opq_NameInvalid = false;
    }
    if (opq_UsageInvalid != NULL) {
      *opq_UsageInvalid = false;
    }
    if (opq_DataSetsInvalid != NULL) {
      *opq_DataSetsInvalid = false;
    }
    if (opq_ElementsInvalid != NULL) {
      *opq_ElementsInvalid = false;
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Handle name max char limit

   \param[in]      ou32_NameMaxCharLimit  Name max char limit
   \param[in,out]  opc_ChangedItems       Changed items
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPool::HandleNameMaxCharLimit(
    const uint32_t ou32_NameMaxCharLimit,
    QList<C_OscSystemNameMaxCharLimitChangeReportItem>
        *const opc_ChangedItems) {
  if ((this->e_Type != C_OscNodeDataPool::eHALC) &&
      (this->e_Type != C_OscNodeDataPool::eHALC_NVM)) {
    C_OscSystemNameMaxCharLimitChangeReportItem::h_HandleNameMaxCharLimitItem(
        ou32_NameMaxCharLimit, "node-datapool-name", this->c_Name,
        opc_ChangedItems);
    for (uint32_t u32_ItList = 0UL; u32_ItList < this->c_Lists.size();
         ++u32_ItList) {
      C_OscNodeDataPoolList &rc_List = this->c_Lists[u32_ItList];
      rc_List.HandleNameMaxCharLimit(ou32_NameMaxCharLimit, opc_ChangedItems);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Get hash for element

   \param[in]  ou32_ListIndex       List index
   \param[in]  ou32_ElementIndex    Element index

   \return
   Hash for element
*/
//----------------------------------------------------------------------------------------------------------------------
uint32_t
C_OscNodeDataPool::m_GetElementHash(const uint32_t ou32_ListIndex,
                                    const uint32_t ou32_ElementIndex) const {
  uint32_t u32_Retval = 0xFFFFFFFFUL;

  if (ou32_ListIndex < this->c_Lists.size()) {
    const C_OscNodeDataPoolList &rc_List = this->c_Lists[ou32_ListIndex];
    if (ou32_ElementIndex < rc_List.c_Elements.size()) {
      const C_OscNodeDataPoolListElement &rc_Element =
          rc_List.c_Elements[ou32_ElementIndex];
      rc_Element.CalcHash(u32_Retval);
    }
  }
  return u32_Retval;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDataStream
   \param   ro_DataStream  Output stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPool::ToQDataStream(QDataStream &ro_DataStream) const {
  ro_DataStream << static_cast<int32_t>(e_Type);
  ro_DataStream << c_Name;
  ro_DataStream << au8_Version[0] << au8_Version[1] << au8_Version[2];
  ro_DataStream << u16_DefinitionCrcVersion;
  ro_DataStream << c_Comment;
  ro_DataStream << s32_RelatedDataBlockIndex;
  ro_DataStream << q_IsSafety;
  ro_DataStream << q_ScopeIsPrivate;
  ro_DataStream << u32_NvmStartAddress;
  ro_DataStream << u32_NvmSize;
  
  // Serialize lists
  ro_DataStream << static_cast<qint32>(c_Lists.size());
  for (const auto &c_List : c_Lists) {
    c_List.ToQDataStream(ro_DataStream);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDataStream
   \param   ro_DataStream  Input stream
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPool::FromQDataStream(QDataStream &ro_DataStream) {
  int32_t s_Type;
  ro_DataStream >> s_Type;
  e_Type = static_cast<E_Type>(s_Type);
  ro_DataStream >> c_Name;
  ro_DataStream >> au8_Version[0] >> au8_Version[1] >> au8_Version[2];
  ro_DataStream >> u16_DefinitionCrcVersion;
  ro_DataStream >> c_Comment;
  ro_DataStream >> s32_RelatedDataBlockIndex;
  ro_DataStream >> q_IsSafety;
  ro_DataStream >> q_ScopeIsPrivate;
  ro_DataStream >> u32_NvmStartAddress;
  ro_DataStream >> u32_NvmSize;
  
  // Deserialize lists
  qint32 s_ListsSize;
  ro_DataStream >> s_ListsSize;
  c_Lists.clear();
  for (qint32 s_I = 0; s_I < s_ListsSize; ++s_I) {
    C_OscNodeDataPoolList c_List;
    c_List.FromQDataStream(ro_DataStream);
    c_Lists.append(c_List);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QJsonObject
   \return  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
QJsonObject C_OscNodeDataPool::ToJsonObject() const {
  QJsonObject c_Obj;
  
  // Convert enum to string
  QString c_TypeStr;
  switch (e_Type) {
    case eDIAG: c_TypeStr = "diag"; break;
    case eNVM: c_TypeStr = "nvm"; break;
    case eCOM: c_TypeStr = "com"; break;
    case eHALC: c_TypeStr = "halc"; break;
    case eHALC_NVM: c_TypeStr = "halc_nvm"; break;
    default: c_TypeStr = "unknown"; break;
  }
  c_Obj["type"] = c_TypeStr;
  
  c_Obj["name"] = c_Name;
  c_Obj["version"] = QString("%1.%2.%3")
    .arg(au8_Version[0]).arg(au8_Version[1]).arg(au8_Version[2]);
  c_Obj["definition-crc-version"] = static_cast<qint64>(u16_DefinitionCrcVersion);
  c_Obj["comment"] = c_Comment;
  c_Obj["related-data-block-index"] = s32_RelatedDataBlockIndex;
  c_Obj["is-safety"] = q_IsSafety;
  c_Obj["scope-is-private"] = q_ScopeIsPrivate;
  c_Obj["nvm-start-address"] = static_cast<qint64>(u32_NvmStartAddress);
  c_Obj["nvm-size"] = static_cast<qint64>(u32_NvmSize);
  
  // Serialize lists
  QJsonArray c_ListsArray;
  for (const auto &c_List : c_Lists) {
    c_ListsArray.append(c_List.ToJsonObject());
  }
  c_Obj["lists"] = c_ListsArray;
  
  return c_Obj;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QJsonObject
   \param   orc_Object  JSON object
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPool::FromJsonObject(const QJsonObject &orc_Object) {
  if (orc_Object.contains("type")) {
    QString c_TypeStr = orc_Object["type"].toString();
    if (c_TypeStr == "diag") e_Type = eDIAG;
    else if (c_TypeStr == "nvm") e_Type = eNVM;
    else if (c_TypeStr == "com") e_Type = eCOM;
    else if (c_TypeStr == "halc") e_Type = eHALC;
    else if (c_TypeStr == "halc_nvm") e_Type = eHALC_NVM;
    else e_Type = eDIAG; // Default
  }
  
  if (orc_Object.contains("name")) c_Name = orc_Object["name"].toString();
  if (orc_Object.contains("version")) {
    QString c_VersionStr = orc_Object["version"].toString();
    QStringList c_VersionParts = c_VersionStr.split('.');
    if (c_VersionParts.size() >= 3) {
      au8_Version[0] = c_VersionParts[0].toUInt();
      au8_Version[1] = c_VersionParts[1].toUInt();
      au8_Version[2] = c_VersionParts[2].toUInt();
    }
  }
  if (orc_Object.contains("definition-crc-version")) u16_DefinitionCrcVersion = static_cast<uint16_t>(orc_Object["definition-crc-version"].toInt());
  if (orc_Object.contains("comment")) c_Comment = orc_Object["comment"].toString();
  if (orc_Object.contains("related-data-block-index")) s32_RelatedDataBlockIndex = orc_Object["related-data-block-index"].toInt();
  if (orc_Object.contains("is-safety")) q_IsSafety = orc_Object["is-safety"].toBool();
  if (orc_Object.contains("scope-is-private")) q_ScopeIsPrivate = orc_Object["scope-is-private"].toBool();
  if (orc_Object.contains("nvm-start-address")) u32_NvmStartAddress = static_cast<uint32_t>(orc_Object["nvm-start-address"].toInt());
  if (orc_Object.contains("nvm-size")) u32_NvmSize = static_cast<uint32_t>(orc_Object["nvm-size"].toInt());
  
  // Deserialize lists
  if (orc_Object.contains("lists")) {
    QJsonArray c_ListsArray = orc_Object["lists"].toArray();
    c_Lists.clear();
    for (const auto &c_Value : c_ListsArray) {
      C_OscNodeDataPoolList c_List;
      c_List.FromJsonObject(c_Value.toObject());
      c_Lists.append(c_List);
    }
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Serialize to QDomElement
   \param   orc_Doc        XML document
   \param   orc_ElementName  Element name
   \return  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
QDomElement C_OscNodeDataPool::ToQDomDocument(QDomDocument &orc_Doc,
                                               const QString &orc_ElementName) const {
  QDomElement c_Element = orc_Doc.createElement(orc_ElementName);
  
  // Convert enum to string
  QString c_TypeStr;
  switch (e_Type) {
    case eDIAG: c_TypeStr = "diag"; break;
    case eNVM: c_TypeStr = "nvm"; break;
    case eCOM: c_TypeStr = "com"; break;
    case eHALC: c_TypeStr = "halc"; break;
    case eHALC_NVM: c_TypeStr = "halc_nvm"; break;
    default: c_TypeStr = "unknown"; break;
  }
  c_Element.setAttribute("type", c_TypeStr);
  
  c_Element.setAttribute("name", c_Name);
  c_Element.setAttribute("version", QString("%1.%2.%3")
    .arg(au8_Version[0]).arg(au8_Version[1]).arg(au8_Version[2]));
  c_Element.setAttribute("definition-crc-version", QString::number(u16_DefinitionCrcVersion));
  c_Element.setAttribute("comment", c_Comment);
  c_Element.setAttribute("related-data-block-index", QString::number(s32_RelatedDataBlockIndex));
  c_Element.setAttribute("is-safety", q_IsSafety ? "true" : "false");
  c_Element.setAttribute("scope-is-private", q_ScopeIsPrivate ? "true" : "false");
  c_Element.setAttribute("nvm-start-address", QString::number(u32_NvmStartAddress));
  c_Element.setAttribute("nvm-size", QString::number(u32_NvmSize));
  
  // Serialize lists
  for (const auto &c_List : c_Lists) {
    QDomElement c_ListElem = c_List.ToQDomDocument(orc_Doc, "list");
    c_Element.appendChild(c_ListElem);
  }
  
  return c_Element;
}

//----------------------------------------------------------------------------------------------------------------------
/*!
   \brief   Deserialize from QDomElement
   \param   orc_Element  XML element
*/
//----------------------------------------------------------------------------------------------------------------------
void C_OscNodeDataPool::FromQDomDocument(const QDomElement &orc_Element) {
  if (orc_Element.hasAttribute("type")) {
    QString c_TypeStr = orc_Element.attribute("type");
    if (c_TypeStr == "diag") e_Type = eDIAG;
    else if (c_TypeStr == "nvm") e_Type = eNVM;
    else if (c_TypeStr == "com") e_Type = eCOM;
    else if (c_TypeStr == "halc") e_Type = eHALC;
    else if (c_TypeStr == "halc_nvm") e_Type = eHALC_NVM;
    else e_Type = eDIAG; // Default
  }
  
  if (orc_Element.hasAttribute("name")) c_Name = orc_Element.attribute("name");
  if (orc_Element.hasAttribute("version")) {
    QString c_VersionStr = orc_Element.attribute("version");
    QStringList c_VersionParts = c_VersionStr.split('.');
    if (c_VersionParts.size() >= 3) {
      au8_Version[0] = c_VersionParts[0].toUInt();
      au8_Version[1] = c_VersionParts[1].toUInt();
      au8_Version[2] = c_VersionParts[2].toUInt();
    }
  }
  if (orc_Element.hasAttribute("definition-crc-version")) u16_DefinitionCrcVersion = static_cast<uint16_t>(orc_Element.attribute("definition-crc-version").toUInt());
  if (orc_Element.hasAttribute("comment")) c_Comment = orc_Element.attribute("comment");
  if (orc_Element.hasAttribute("related-data-block-index")) s32_RelatedDataBlockIndex = orc_Element.attribute("related-data-block-index").toInt();
  if (orc_Element.hasAttribute("is-safety")) q_IsSafety = (orc_Element.attribute("is-safety") == "true");
  if (orc_Element.hasAttribute("scope-is-private")) q_ScopeIsPrivate = (orc_Element.attribute("scope-is-private") == "true");
  if (orc_Element.hasAttribute("nvm-start-address")) u32_NvmStartAddress = orc_Element.attribute("nvm-start-address").toUInt();
  if (orc_Element.hasAttribute("nvm-size")) u32_NvmSize = orc_Element.attribute("nvm-size").toUInt();
  
  // Deserialize lists
  QDomNode c_Node = orc_Element.firstChild();
  while (!c_Node.isNull()) {
    QDomElement c_Elem = c_Node.toElement();
    if (!c_Elem.isNull() && c_Elem.tagName() == "list") {
      C_OscNodeDataPoolList c_List;
      c_List.FromQDomDocument(c_Elem);
      c_Lists.append(c_List);
    }
    c_Node = c_Node.nextSibling();
  }
}
