// Prototype: aggressive idiomatic-QSettings refactor of C_UsFiler::mh_SaveNode.
// Uses beginGroup() and beginWriteArray() throughout. CHANGES the on-disk INI
// format — see "On-disk format change" note at the bottom.

namespace
{
//-----------------------------------------------------------------------------
// Saves a container as a QSettings array. Caller's lambda writes per-item
// fields into the current array slot.
template <typename Container, typename ItemWriter>
void mh_SaveArray(QSettings & orc_Ini, const QString & orc_ArrayName,
                  const Container & orc_Items, const ItemWriter & orc_WriteItem)
{
   orc_Ini.beginWriteArray(orc_ArrayName, static_cast<int>(orc_Items.size()));
   int s32_Index = 0;
   for (auto c_It = orc_Items.cbegin(); c_It != orc_Items.cend(); ++c_It)
   {
      orc_Ini.setArrayIndex(s32_Index);
      orc_WriteItem(c_It);
      ++s32_Index;
   }
   orc_Ini.endArray();
}
} // namespace

//-----------------------------------------------------------------------------
void C_UsFiler::mh_SaveNode(QSettings & orc_Ini, const QString & orc_SectionName,
                            const QString & orc_NodeIdBase, const QString & orc_NodeName,
                            const C_UsNode & orc_Node)
{
   orc_Ini.beginGroup(orc_SectionName);
   orc_Ini.beginGroup(orc_NodeIdBase);

   orc_Ini.setValue("Name",                 orc_NodeName);
   orc_Ini.setValue("SelectedDatapoolName", orc_Node.GetSelectedDatapoolName());
   orc_Ini.setValue("SelectedProtocol",     static_cast<int32_t>(orc_Node.GetSelectedProtocol()));
   orc_Ini.setValue("SelectedInterface",    static_cast<int32_t>(orc_Node.GetSelectedInterface()));

   mh_SaveColumns(orc_Ini, "CANopenOverviewColumns",    orc_Node.GetCanOpenOverviewColumnWidth());
   mh_SaveColumns(orc_Ini, "CANopenPdoOverviewColumns", orc_Node.GetCanOpenPdoOverviewColumnWidth());

   // CANopen expansion state — three near-identical patterns
   mh_SaveArray(orc_Ini, "CANopenManagerExpanded", orc_Node.GetExpandedCanOpenManager(),
      [&orc_Ini](auto c_It) {
         orc_Ini.setValue("InterfaceNumber", c_It.key());
         orc_Ini.setValue("Expanded",        c_It.value());
      });

   mh_SaveArray(orc_Ini, "CANopenDevicesExpanded", orc_Node.GetExpandedCanOpenDevices(),
      [&orc_Ini](auto c_It) {
         orc_Ini.setValue("InterfaceNumber", c_It.key());
         orc_Ini.setValue("Expanded",        c_It.value());
      });

   mh_SaveArray(orc_Ini, "CANopenDeviceExpanded", orc_Node.GetExpandedCanOpenDevice(),
      [&orc_Ini](auto c_It) {
         const auto & rc_Key = c_It.key();
         orc_Ini.setValue("InterfaceNumber",       rc_Key.first);
         orc_Ini.setValue("DeviceInterfaceNumber", rc_Key.second.first);
         orc_Ini.setValue("DeviceNodeName",        rc_Key.second.second);
         orc_Ini.setValue("Expanded",              c_It.value());
      });

   orc_Ini.setValue("CANopenManager",
                    static_cast<int32_t>(orc_Node.GetSelectedCanOpenManager()));
   orc_Ini.setValue("CANopenDeviceInterfaceNumber",
                    static_cast<int32_t>(orc_Node.GetSelectedCanOpenDeviceInterfaceNumber()));
   orc_Ini.setValue("CANopenDeviceNodeName", orc_Node.GetSelectedCanOpenDeviceNodeName());
   orc_Ini.setValue("CANopenDeviceUseCase",
                    static_cast<int32_t>(orc_Node.GetSelectedCanOpenDeviceUseCaseIndex()));
   orc_Ini.setValue("CANopenSelectedUseCaseOrInterface",
                    orc_Node.GetCanOpenSelectedUseCaseOrInterface());

   orc_Ini.setValue("SelectedHalcDomain",  orc_Node.GetSelectedHalcDomainName());
   orc_Ini.setValue("SelectedHalcChannel", orc_Node.GetSelectedHalcChannel());

   mh_SaveColumns(orc_Ini, "HALCOverviewColumns",    orc_Node.GetHalcOverviewColumnWidth());
   mh_SaveColumns(orc_Ini, "HALCParamConfigColumns", orc_Node.GetHalcConfigColumnWidth());

   // Datapools — each slot stores name + nested datapool data
   const QStringList c_DatapoolKeys = orc_Node.GetDatapoolKeysInternal();
   mh_SaveArray(orc_Ini, "Datapools", c_DatapoolKeys,
      [&orc_Ini, &orc_Node](auto c_It) {
         const QString & rc_Name = *c_It;
         orc_Ini.setValue("Name", rc_Name);
         mh_SaveDatapool(orc_Ini, orc_Node.GetDatapool(rc_Name));
      });

   orc_Ini.setValue("SelectedDataLoggerLogJobIndex",
                    static_cast<int32_t>(orc_Node.GetSelectedDataLoggerLogJobIndex()));
   orc_Ini.setValue("DataLoggerLogJobOverviewSelected",
                    orc_Node.GetIsOverviewWidgetSelected());

   orc_Ini.endGroup();
   orc_Ini.endGroup();
}

//-----------------------------------------------------------------------------
// Updated helper signatures — sections/prefixes flow via the active QSettings
// group rather than as parameters.

void C_UsFiler::mh_SaveColumns(QSettings & orc_Ini, const QString & orc_ArrayName,
                               const QList<int32_t> & orc_ColumnWidths)
{
   orc_Ini.beginWriteArray(orc_ArrayName, static_cast<int>(orc_ColumnWidths.size()));
   for (int s32_It = 0; s32_It < orc_ColumnWidths.size(); ++s32_It)
   {
      orc_Ini.setArrayIndex(s32_It);
      orc_Ini.setValue("Width", orc_ColumnWidths[s32_It]);
   }
   orc_Ini.endArray();
}

// On-disk format change
// =====================
// Before (dev/legacy):
//   [ProjectFoo]
//   Node1Name=MyNode
//   Node1CANopenManagerExpanded#0InterfaceNumber=2
//   Node1CANopenManagerExpanded#0=true
//   Node1CANopenManagerExpandedCounter=3
//   Node1Column0=120
//   Node1Column1=80
//   Node1Column_Count=2
//
// After (idiomatic):
//   [ProjectFoo/Node1]
//   Name=MyNode
//   CANopenManagerExpanded\size=3
//   CANopenManagerExpanded\1\InterfaceNumber=2
//   CANopenManagerExpanded\1\Expanded=true
//   CANopenOverviewColumns\size=2
//   CANopenOverviewColumns\1\Width=120
//   CANopenOverviewColumns\2\Width=80
//
// Migration: on first load after the upgrade, detect legacy keys
// (presence of "<NodeIdBase>Name" instead of "<NodeIdBase>/Name") and
// translate to the new layout. One-shot, runs once per user.
