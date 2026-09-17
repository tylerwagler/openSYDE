void C_UsFiler::mh_SaveColumns(QSettings &orc_Ini,
                               const QString &orc_SectionName,
                               const QString &orc_IdBase,
                               const QList<int32_t> &orc_ColumnWidths) {
  const QString c_IdColumnCount =
      static_cast<QString>("%1Column_Count").arg(orc_IdBase);

  orc_Ini.setValue(orc_SectionName + "/" + c_IdColumnCount,
                   static_cast<int>(orc_ColumnWidths.size()));
  for (uint32_t u32_ItCol = 0; u32_ItCol < orc_ColumnWidths.size();
       ++u32_ItCol) {
    const QString c_IdColumn =
        static_cast<QString>("%1Column%2").arg(orc_IdBase).arg(u32_ItCol);
    orc_Ini.setValue(orc_SectionName + "/" + c_IdColumn,
                     orc_ColumnWidths[u32_ItCol]);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load columns

   \param[in,out]  orc_Ini             Ini
   \param[in]      orc_SectionName     Section name
   \param[in]      orc_IdBase          Id base
   \param[in,out]  orc_ColumnWidths    Column widths
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadColumns(QSettings &orc_Ini,
                               const QString &orc_SectionName,
                               const QString &orc_IdBase,
                               QList<int32_t> &orc_ColumnWidths) {
  const QString c_IdColumnCount =
      static_cast<QString>("%1Column_Count").arg(orc_IdBase);
  const int32_t s32_ColumnCount =
      orc_Ini.value(orc_SectionName + "/" + c_IdColumnCount, 0).toInt();

  orc_ColumnWidths.reserve(s32_ColumnCount);
  for (int32_t s32_ItCol = 0; s32_ItCol < s32_ColumnCount; ++s32_ItCol) {
    const QString c_IdColumn =
        static_cast<QString>("%1Column%2").arg(orc_IdBase).arg(s32_ItCol);
    orc_ColumnWidths.push_back(
        orc_Ini.value(orc_SectionName + "/" + c_IdColumn, 0).toInt());
  }
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief  Load Screenshot GIF Play timer from user settings .ini

   \param[in,out]  orc_UserSettings    User settings
   \param[in,out]  orc_Ini             Current ini
*/
//----------------------------------------------------------------------------------------------------------------------
void C_UsFiler::mh_LoadScreenshotGifSucessTimeout(C_UsHandler &orc_UserSettings,
                                                  QSettings &orc_Ini) {
  orc_UserSettings.SetScreenshotGifSucessTimeout(
      orc_Ini.value("Common/ScreenshotGifSucessTimeout", 3000).toInt());
