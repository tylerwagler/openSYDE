//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       KEFEX DEF Project handling

   Implements reading DEF-style KEFEX projects.

   \copyright   Copyright 2009 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef CKFXDEFPROJECTHPP
#define CKFXDEFPROJECTHPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include "stwtypes.hpp"
#include "CKFXVariableLists.hpp"
#include <QString>
#include <QStringList>
#include <QSettings>
#include "DiagLib_config.hpp"

namespace stw
{
namespace diag_lib
{
/* -- Defines ------------------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */
class C_KFXRAMNameIndex
{
public:
   QString c_FileName;
   uint16_t u16_ListIndex;
   C_KFXRAMNameIndex & operator =(const C_KFXRAMNameIndex & orc_Source);
   C_KFXRAMNameIndex();
   C_KFXRAMNameIndex(const C_KFXRAMNameIndex & orc_Source);
};

/* -- Global Variables ---------------------------------------------------------------------------------------------- */

/* -- Function Prototypes ------------------------------------------------------------------------------------------- */

///KEFEX DEF Project handling
class C_KFXDEFProject
{
private:
   static int32_t m_LoadRAMListFromFile(const QString & orc_FilePath, C_KFXVariableListBase & orc_List,
                                        QString & orc_ErrorText);
   static int32_t m_SortRAMLists(QList<C_KFXRAMNameIndex> & orc_Files,
                                 QString & orc_ErrorText);
   static int32_t m_FindRelatedFiles(const QString & orc_Directory,
                                     const QString & orc_DeviceName,
                                     QList<C_KFXRAMNameIndex> & orc_Files,
                                     QStringList & orc_Warnings);

   static void m_VarStringsToMinMax(const QString & orc_Min, const QString & orc_Max,
                                    C_KFXVariableBase & orc_Variable);

public:
   static E_TransmissionType TransTypeStringToEnum(const QString & orc_Text);
   static int32_t LoadRAMFiles(const QString & orc_Directory,
                               const QString & orc_DeviceName, C_KFXVariableLists & orc_Lists,
                               QString & orc_ErrorText,
                               QStringList & orc_Warnings);

   static int32_t LoadComments(const QString & orc_FileName, const QString & orc_DeviceName,
                               C_KFXVariableLists & orc_VariableLists,
                               QString * const opc_CommentDescriptions);
   static void LoadDefaultNames(QSettings * const opc_IniFile,
                                C_KFXVariableLists & orc_VariableLists);
};
}
}

/* -- Implementation ------------------------------------------------------------------------------------------------ */

#endif
