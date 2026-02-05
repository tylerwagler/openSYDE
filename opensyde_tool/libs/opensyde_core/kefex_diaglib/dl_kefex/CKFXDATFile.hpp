#ifndef CKFXDATFILEHPP
#define CKFXDATFILEHPP

#include "stwtypes.hpp"
#include <QString>
#include "CKFXVariableLists.hpp"

#include "DiagLib_config.hpp"

namespace stw
{
namespace diag_lib
{
///handling of "DAT" files (containing default values or data sets)
class C_KFXDATFile
{
private:
   static void m_GetWordFromBuffer(uint16_t * const opu16_Value, const uint8_t ** const oppu8_Buffer);
   static void m_GetDWordFromBuffer(uint32_t * const opu32_Value, const uint8_t ** const oppu8_Buffer);
   static void m_GetStringFromBuffer(QString & orc_String, const uint8_t ** const oppu8_Buffer);

   static int32_t m_BufferToLists(const uint8_t * const opu8_Buffer, C_KFXVariableLists & orc_VariableLists,
                                  const uint16_t ou16_NumListsInDat, const uint32_t ou32_NumBytesTotal,
                                  const bool oq_SingleList, const uint16_t ou16_SingleListIndex,
                                  QList<uint8_t> * const opc_ListsLoaded);

public:
   //DAT -> Default
   static int32_t LoadDATList(const QString & orc_FileName, const QString & orc_DeviceName,
                              C_KFXVariableLists & orc_VariableLists, const int32_t os32_ListIndex,
                              QList<uint8_t> * const opc_ListsLoaded = NULL);
   static int32_t LoadDATAllLists(const QString & orc_FileName,
                                  const QString & orc_DeviceName, C_KFXVariableLists & orc_VariableLists,
                                  QList<uint8_t> * const opc_ListsLoaded = NULL);
};
}
}

#endif
