//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Utility class for C code export.
   \copyright   Copyright 2019 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_OSCEXPORTUTI_HPP
#define C_OSCEXPORTUTI_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QStringList>
#include "C_OscNodeDataPool.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_core
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

class C_OscExportUti
{
public:
   static QString h_GetSectionSeparator(const QString & orc_SectionName);
   static QString h_GetHeaderSeparator(void);
   static QString h_GetCreationToolInfo(const QString & orc_ExportToolInfo);
   static void h_AddExternCeStart(QStringList & orc_Data);
   static void h_AddExternCeEnd(QStringList & orc_Data);
   static void h_AddProjectIdDef(QStringList & orc_Data, const QString & orc_MagicName,
                                 const bool oq_HeaderFile);
   static void h_AddProjIdFunctionPrototype(QStringList & orc_Data,
                                            const QString & orc_MagicName);
   static int32_t h_SaveToFile(QStringList & orc_Data, const QString & orc_Path,
                               const QString & orc_FileName, const bool oq_HeaderFile);
   static void h_CollectFilePaths(QStringList & orc_FilePaths,
                                  const QString & orc_Path, const QString & orc_FileName,
                                  const bool oq_SourceCode);

   static QString h_GetTypePrefix(const C_OscNodeDataPoolContent::E_Type oe_Type, const bool oq_IsArray);
   static QString h_GetElementTypeAsString(const C_OscNodeDataPoolContent::E_Type oe_Type);
   static QString h_GetElementCeName(const QString & orc_Name, const bool oq_IsArray,
                                                   const C_OscNodeDataPoolContent::E_Type oe_Type,
                                                   const QString & orc_ArrayPos = "0");

   static QString h_FloatToStrGe(const float32_t of32_Value,  bool * const opq_InfOrNan = NULL);
   static QString h_FloatToStrGe(const float64_t of64_Value, bool * const opq_InfOrNan = NULL);
   static bool h_CheckInfOrNan(const QString & orc_String);
   static void h_AddDecimalPointIfNone(QString & orc_FloatString);
};

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
