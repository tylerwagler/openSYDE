//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Export Datapool settings of an openSYDE node.

   See cpp file for detailed description

   \copyright   Copyright 2017 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCEXPORTDATAPOOL_HPP
#define C_OSCEXPORTDATAPOOL_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <vector>
#include <QStringList>

#include "stwtypes.hpp"
#include "C_OscNodeDataPool.hpp"
#include "C_OscNodeCodeExportSettings.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscExportDataPool
{
public:
   enum E_Linkage ///< Flag for Datapool-application-relation when generating source code files
   {
      eLOCAL = 0,   ///< Generate code for Datapool owner
      eREMOTE,      ///< Generate code for DPD owner
      eREMOTEPUBLIC ///< Generate code for public Datapool that is owned by another application
   };

   static QString h_GetFileName(const C_OscNodeDataPool & orc_DataPool);
   static uint16_t h_ConvertOverallCodeVersion(const uint16_t ou16_GenCodeVersion);
   static int32_t h_CreateSourceCode(const QString & orc_Path, const uint16_t ou16_GenCodeVersion,
                                     const C_OscNodeCodeExportSettings::E_Scaling oe_ScalingSupport,
                                     const C_OscNodeDataPool & orc_DataPool, const uint8_t ou8_DataPoolIndex,
                                     const E_Linkage oe_Linkage, const uint8_t ou8_DataPoolIndexRemote,
                                     const uint8_t ou8_ProcessId,
                                     const QString & orc_ExportToolInfo = "");

protected:
   static const bool mhq_IS_HEADER_FILE = false;
   static const bool mhq_IS_IMPLEMENTATION_FILE = true;

   static int32_t mh_CreateImplementationFile(const QString & orc_ExportToolInfo,
                                              const QString & orc_Path,
                                              const C_OscNodeDataPool & orc_DataPool,
                                              const QString & orc_ProjectId,
                                              const uint16_t ou16_GenCodeVersion, const uint8_t ou8_DataPoolIndexRemote,
                                              const uint8_t ou8_ProcessId, const E_Linkage oe_Linkage);
   static int32_t mh_CreateHeaderFile(const QString & orc_ExportToolInfo,
                                      const QString & orc_Path, const C_OscNodeDataPool & orc_DataPool,
                                      const uint8_t ou8_DataPoolIndex, const QString & orc_ProjectId,
                                      const uint16_t ou16_GenCodeVersion, const E_Linkage oe_Linkage,
                                      const C_OscNodeCodeExportSettings::E_Scaling oe_ScalingSupport);

   static void mh_AddHeader(const QString & orc_ExportToolInfo, QStringList & orc_Data,
                            const C_OscNodeDataPool & orc_DataPool, const bool oq_FileType);
   static void mh_AddIncludes(QStringList & orc_Data, const C_OscNodeDataPool & orc_DataPool,
                              const bool oq_FileType);
   static int32_t mh_AddDefinesHeader(QStringList & orc_Data, const C_OscNodeDataPool & orc_DataPool,
                                      const uint8_t ou8_DataPoolIndex, const QString & orc_ProjectId,
                                      const uint16_t ou16_GenCodeVersion, const E_Linkage oe_Linkage,
                                      const C_OscNodeCodeExportSettings::E_Scaling oe_ScalingSupport);
   static void mh_AddDefinesImpl(QStringList & orc_Data, const C_OscNodeDataPool & orc_DataPool,
                                 const QString & orc_ProjectId, const uint16_t ou16_GenCodeVersion);
   static void mh_AddTypes(QStringList & orc_Data, const C_OscNodeDataPool & orc_DataPool,
                           const bool oq_FileType, const E_Linkage oe_Linkage);
   static void mh_AddGlobalVariables(QStringList & orc_Data, const C_OscNodeDataPool & orc_DataPool,
                                     const uint16_t ou16_GenCodeVersion, const bool oq_FileType,
                                     const E_Linkage oe_Linkage);
   static void mh_AddModuleGlobal(QStringList & orc_Data, const C_OscNodeDataPool & orc_DataPool,
                                  const uint16_t ou16_GenCodeVersion, const uint8_t ou8_ProcessId,
                                  const uint8_t ou8_DataPoolIndexRemote, const E_Linkage oe_Linkage);
   static void mh_AddImplementation(QStringList & orc_Data, const bool oq_FileType);
   static QString mh_GetType(const C_OscNodeDataPoolContent::E_Type oe_Type);
   static QString mh_GetElementValueString(const C_OscNodeDataPoolContent & orc_Value,
                                                         const C_OscNodeDataPoolContent::E_Type oe_Type,
                                                         const bool oq_IsArray);
   static QString mh_GetElementSize(const C_OscNodeDataPoolContent::E_Type oe_Type,
                                                  const uint32_t ou32_ArraySize, const bool oq_IsArray);
   static QString mh_ConvertLinkageToString(const E_Linkage oe_Linkage);
   static QString mh_GetMagicName(const QString & orc_ProjectId,
                                                const C_OscNodeDataPool & orc_DataPool);
   static QString mh_GetElementScaleDefine(const QString & orc_DataPoolName,
                                                         const QString & orc_ListName,
                                                         const QString & orc_ElementName,
                                                         const bool oq_Factor);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
