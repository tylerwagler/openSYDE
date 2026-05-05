// Prototype: format-compatible refactor of C_UsFiler::mh_SaveNode using QSettings idioms.
// Preserves the exact on-disk INI key layout produced by the dev migration —
// no migration of existing user settings files is required.

namespace
{
//-----------------------------------------------------------------------------
// Saves a "<base>0Field=...", "<base>0=...", ..., "<base>Counter=N" pattern.
// The lambda writes any extra key-derived fields under "<base><idx><suffix>".
template <typename Container, typename KeyWriter>
void mh_SaveExpansionState(QSettings & orc_Ini, const QString & orc_KeyBase,
                           const Container & orc_Items, const KeyWriter & orc_WriteExtras)
{
   uint32_t u32_Counter = 0UL;
   for (auto c_It = orc_Items.cbegin(); c_It != orc_Items.cend(); ++c_It)
   {
      const QString c_IndexedKey = orc_KeyBase + QString::number(u32_Counter);
      orc_WriteExtras(c_It, c_IndexedKey);
      orc_Ini.setValue(c_IndexedKey, c_It.value());
      ++u32_Counter;
   }
   orc_Ini.setValue(orc_KeyBase + "Counter", static_cast<int>(orc_Items.size()));
}
} // namespace

//-----------------------------------------------------------------------------
void C_UsFiler::mh_SaveNode(QSettings & orc_Ini, const QString & orc_SectionName,
                            const QString & orc_NodeIdBase,
                            const QString & orc_NodeName,
                            const C_UsNode & orc_Node)
{
   orc_Ini.beginGroup(orc_SectionName);

   // Build a node-prefixed key — preserves legacy "<NodeIdBase>Field" format.
   const auto k = [&orc_NodeIdBase](const char * const opcn_Suffix) {
      return orc_NodeIdBase + QLatin1String(opcn_Suffix);
   };

   orc_Ini.setValue(k("Name"), orc_NodeName);
   orc_Ini.setValue(k("Selected_datapool_name"), orc_Node.GetSelectedDatapoolName());
   orc_Ini.setValue(k("Selected_protocol"),  static_cast<int32_t>(orc_Node.GetSelectedProtocol()));
   orc_Ini.setValue(k("Selected_interface"), static_cast<int32_t>(orc_Node.GetSelectedInterface()));

   // CANopen overview columns
   mh_SaveColumns(orc_Ini, k("CANopenOverview"),    orc_Node.GetCanOpenOverviewColumnWidth());
   mh_SaveColumns(orc_Ini, k("CANopenPdoOverview"), orc_Node.GetCanOpenPdoOverviewColumnWidth());

   // CANopen expansion state — three near-identical patterns collapsed
   {
      const QHash<uint8_t, bool> c_Interfaces = orc_Node.GetExpandedCanOpenManager();
      mh_SaveExpansionState(orc_Ini, k("CANopenManagerExpanded#"), c_Interfaces,
         [&orc_Ini](auto c_It, const QString & orc_Prefix) {
            orc_Ini.setValue(orc_Prefix + "InterfaceNumber", c_It.key());
         });
   }
   {
      const QHash<uint8_t, bool> c_Devices = orc_Node.GetExpandedCanOpenDevices();
      mh_SaveExpansionState(orc_Ini, k("CANopenDevicesExpanded#"), c_Devices,
         [&orc_Ini](auto c_It, const QString & orc_Prefix) {
            orc_Ini.setValue(orc_Prefix + "InterfaceNumber", c_It.key());
         });
   }
   {
      const QMap<std::pair<uint8_t, std::pair<uint8_t, QString> >, bool> c_Device =
         orc_Node.GetExpandedCanOpenDevice();
      mh_SaveExpansionState(orc_Ini, k("CANopenDeviceExpanded#"), c_Device,
         [&orc_Ini](auto c_It, const QString & orc_Prefix) {
            const auto & rc_Key = c_It.key();
            orc_Ini.setValue(orc_Prefix + "InterfaceNumber",       rc_Key.first);
            orc_Ini.setValue(orc_Prefix + "DeviceInterfaceNumber", rc_Key.second.first);
            orc_Ini.setValue(orc_Prefix + "DeviceNodeName",        rc_Key.second.second);
         });
   }

   orc_Ini.setValue(k("CANopenManager"),
                    static_cast<int32_t>(orc_Node.GetSelectedCanOpenManager()));
   orc_Ini.setValue(k("CANopenDeviceInterfaceNumber"),
                    static_cast<int32_t>(orc_Node.GetSelectedCanOpenDeviceInterfaceNumber()));
   orc_Ini.setValue(k("CANopenDeviceNodeName"),
                    orc_Node.GetSelectedCanOpenDeviceNodeName());
   orc_Ini.setValue(k("CANopenDeviceUseCase"),
                    static_cast<int32_t>(orc_Node.GetSelectedCanOpenDeviceUseCaseIndex()));
   orc_Ini.setValue(k("CANopenSelectedUseCaseOrInterface"),
                    orc_Node.GetCanOpenSelectedUseCaseOrInterface());

   // Selected HALC domain & channel
   orc_Ini.setValue(k("Selected_HALC_domain"),  orc_Node.GetSelectedHalcDomainName());
   orc_Ini.setValue(k("Selected_HALC_channel"), orc_Node.GetSelectedHalcChannel());

   // HALC columns
   mh_SaveColumns(orc_Ini, k("HALCOverview"),    orc_Node.GetHalcOverviewColumnWidth());
   mh_SaveColumns(orc_Ini, k("HALCParamConfig"), orc_Node.GetHalcConfigColumnWidth());

   // Datapools
   const QStringList c_DatapoolKeys = orc_Node.GetDatapoolKeysInternal();
   orc_Ini.setValue(k("Datapool_count"), static_cast<int>(c_DatapoolKeys.size()));
   int32_t s32_ItDatapool = 0;
   for (const QString & rc_DatapoolName : c_DatapoolKeys)
   {
      const QString c_DatapoolIdBase =
         QStringLiteral("%1Datapool%2").arg(orc_NodeIdBase).arg(s32_ItDatapool);
      mh_SaveDatapool(orc_Ini, c_DatapoolIdBase, rc_DatapoolName, orc_Node.GetDatapool(rc_DatapoolName));
      ++s32_ItDatapool;
   }

   // DataLogger
   orc_Ini.setValue(k("Selected_DataLogger_LogJobindex"),
                    static_cast<int32_t>(orc_Node.GetSelectedDataLoggerLogJobIndex()));
   orc_Ini.setValue(k("Selected_DataLogger_LogJob_Overview"),
                    orc_Node.GetIsOverviewWidgetSelected());

   orc_Ini.endGroup();
}
