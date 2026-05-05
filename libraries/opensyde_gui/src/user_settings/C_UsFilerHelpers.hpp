//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       QSettings array-persistence helpers for user settings (header)

   Idiomatic-Qt persistence helpers shared across the C_UsFiler implementations.
   Wrap QSettings::beginWriteArray / beginReadArray with a callable that handles
   the per-item fields, eliminating the manual counter+loop pattern.

   \copyright   Copyright 2026 Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
#ifndef C_USFILERHELPERS_HPP
#define C_USFILERHELPERS_HPP

/* -- Includes ------------------------------------------------------------------------------------------------------ */
#include <QSettings>
#include <QString>

#include "stwtypes.hpp"

/* -- Namespace ----------------------------------------------------------------------------------------------------- */
namespace stw
{
namespace opensyde_gui_logic
{
/* -- Global Constants ---------------------------------------------------------------------------------------------- */

/* -- Types --------------------------------------------------------------------------------------------------------- */

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Persist a container as a QSettings array.

   Wraps QSettings::beginWriteArray / endArray. The caller's writer is invoked
   once per element with an iterator into the container; QSettings has its array
   index already pointed at the corresponding slot, so the writer just calls
   orc_Ini.setValue("Field", ...).

   \param[in,out]  orc_Ini         Open QSettings instance
   \param[in]      orc_ArrayName   Array key (relative to active group)
   \param[in]      orc_Items       Container providing cbegin()/cend()/size()
   \param[in]      orc_WriteItem   Callable: void(decltype(orc_Items.cbegin()))
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename Container, typename ItemWriter>
void h_SaveArray(QSettings & orc_Ini, const QString & orc_ArrayName, const Container & orc_Items,
                 const ItemWriter & orc_WriteItem)
{
   orc_Ini.beginWriteArray(orc_ArrayName, static_cast<int32_t>(orc_Items.size()));
   int32_t s32_Index = 0;
   for (auto c_It = orc_Items.cbegin(); c_It != orc_Items.cend(); ++c_It)
   {
      orc_Ini.setArrayIndex(s32_Index);
      orc_WriteItem(c_It);
      ++s32_Index;
   }
   orc_Ini.endArray();
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Read a QSettings array, invoking a callback per element.

   Wraps QSettings::beginReadArray / endArray. The reader is invoked once per
   array index; QSettings has the slot pre-selected, so the reader just calls
   orc_Ini.value("Field") to extract.

   \param[in,out]  orc_Ini         Open QSettings instance
   \param[in]      orc_ArrayName   Array key (relative to active group)
   \param[in]      orc_ReadItem    Callable: void(int32_t s32_Index)
*/
//----------------------------------------------------------------------------------------------------------------------
template <typename ItemReader>
void h_LoadArray(QSettings & orc_Ini, const QString & orc_ArrayName, const ItemReader & orc_ReadItem)
{
   const int32_t s32_Size = orc_Ini.beginReadArray(orc_ArrayName);
   for (int32_t s32_Index = 0; s32_Index < s32_Size; ++s32_Index)
   {
      orc_Ini.setArrayIndex(s32_Index);
      orc_ReadItem(s32_Index);
   }
   orc_Ini.endArray();
}

/* -- Extern Global Variables --------------------------------------------------------------------------------------- */
}
} //end of namespace

#endif
