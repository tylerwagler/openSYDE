void C_UsFiler::mh_SaveNode(QSettings &orc_Ini, const QString &orc_SectionName,
                            const QString &orc_NodeIdBase,
                            const QString &orc_NodeName,
                            const C_UsNode &orc_Node) {
  const QString c_NodeIdName =
      static_cast<QString>("%1Name").arg(orc_NodeIdBase);
  const QString c_CanOpenOvColumnId =
      static_cast<QString>("%1CANopenOverview").arg(orc_NodeIdBase);
  const QString c_CanOpenPdoOvColumnId =
      static_cast<QString>("%1CANopenPdoOverview").arg(orc_NodeIdBase);
  const QString c_CanOpenSelectedCanOpenManager =
      static_cast<QString>("%1CANopenManager").arg(orc_NodeIdBase);
  const QString c_CanOpenSelectedCanOpenDeviceInterfaceNumber =
      static_cast<QString>("%1CANopenDeviceInterfaceNumber")
          .arg(orc_NodeIdBase);
  const QString c_CanOpenSelectedCanOpenDeviceNodeName =
      static_cast<QString>("%1CANopenDeviceNodeName").arg(orc_NodeIdBase);
  const QString c_CanOpenSelectedCanOpenDeviceUseCaseIndex =
      static_cast<QString>("%1CANopenDeviceUseCase").arg(orc_NodeIdBase);
  const QString c_CanOpenExpandedCanOpenManager =
      static_cast<QString>("%1CANopenManagerExpanded#").arg(orc_NodeIdBase);
  const QString c_CanOpenExpandedCanOpenManagerCounter =
      static_cast<QString>("%1CANopenManagerExpandedCounter")
          .arg(orc_NodeIdBase);
  const QString c_CanOpenExpandedCanOpenDevices =
      static_cast<QString>("%1CANopenDevicesExpanded#").arg(orc_NodeIdBase);
  const QString c_CanOpenExpandedCanOpenDevicesCounter =
      static_cast<QString>("%1CANopenDevicesExpandedCounter")
          .arg(orc_NodeIdBase);
  const QString c_CanOpenExpandedCanOpenDevice =
      static_cast<QString>("%1CANopenDeviceExpanded#").arg(orc_NodeIdBase);
  const QString c_CanOpenExpandedCanOpenDeviceCounter =
      static_cast<QString>("%1CANopenDeviceExpandedCounter")
          .arg(orc_NodeIdBase);
  const QString c_CanOpenSelectedUseCaseOrInterface =
      static_cast<QString>("%1CANopenSelectedUseCaseOrInterface")
          .arg(orc_NodeIdBase);
  const QString c_HalcOvColumnId =
      static_cast<QString>("%1HALCOverview").arg(orc_NodeIdBase);
  const QString c_HalcConfigColumnId =
      static_cast<QString>("%1HALCParamConfig").arg(orc_NodeIdBase);
  const QString c_NodeIdSelectedHalcDomain =
      static_cast<QString>("%1Selected_HALC_domain").arg(orc_NodeIdBase);
  const QString c_NodeIdSelectedHalcChannel =
      static_cast<QString>("%1Selected_HALC_channel").arg(orc_NodeIdBase);
  const QString c_NodeIdDatapoolCount =
      static_cast<QString>("%1Datapool_count").arg(orc_NodeIdBase);
  const QString c_NodeIdSelectedDatapoolName =
      static_cast<QString>("%1Selected_datapool_name").arg(orc_NodeIdBase);
  const QString c_NodeIdSelectedProtocol =
      static_cast<QString>("%1Selected_protocol").arg(orc_NodeIdBase);
  const QString c_NodeIdSelectedInterface =
      static_cast<QString>("%1Selected_interface").arg(orc_NodeIdBase);
  const QString c_NodeIdSelectedDataLoggerLogJobIndex =
      static_cast<QString>("%1Selected_DataLogger_LogJobindex")
          .arg(orc_NodeIdBase);
  const QString c_DataLoggerOverviewWidgetSelected =
      static_cast<QString>("%1Selected_DataLogger_LogJob_Overview")
          .arg(orc_NodeIdBase);
  const QStringList c_DatapoolKeyList = orc_Node.GetDatapoolKeysInternal();
  int32_t s32_ItDatapool = 0;

  // Name
  orc_Ini.setValue(orc_SectionName + "/" + c_NodeIdName, orc_NodeName);

  // Selected datapool name
  orc_Ini.setValue(orc_SectionName + "/" + c_NodeIdSelectedDatapoolName,
                   orc_Node.GetSelectedDatapoolName());

  // Selected protocol
  orc_Ini.setValue(orc_SectionName + "/" + c_NodeIdSelectedProtocol,
                   static_cast<int32_t>(orc_Node.GetSelectedProtocol()));

  // Selected interface
  orc_Ini.setValue(orc_SectionName + "/" + c_NodeIdSelectedInterface,
                   static_cast<int32_t>(orc_Node.GetSelectedInterface()));

  // CANopen colums
  C_UsFiler::mh_SaveColumns(orc_Ini, orc_SectionName, c_CanOpenOvColumnId,
                            orc_Node.GetCanOpenOverviewColumnWidth());
  C_UsFiler::mh_SaveColumns(orc_Ini, orc_SectionName, c_CanOpenPdoOvColumnId,
                            orc_Node.GetCanOpenPdoOverviewColumnWidth());

  // CANopen
  uint32_t u32_InterfaceCounter = 0UL;
  const QHash<uint8_t, bool> c_Interfaces =
      orc_Node.GetExpandedCanOpenManager();
  for (QHash<uint8_t, bool>::const_iterator c_ItInterface =
           c_Interfaces.begin();
       c_ItInterface != c_Interfaces.end(); ++c_ItInterface) {
    orc_Ini.setValue(orc_SectionName + "/" +
                         (c_CanOpenExpandedCanOpenManager +
                          QString::number(u32_InterfaceCounter) +
                          "InterfaceNumber"),
                     c_ItInterface.key());
    orc_Ini.setValue(orc_SectionName + "/" +
                         (c_CanOpenExpandedCanOpenManager +
                          QString::number(u32_InterfaceCounter)),
                     c_ItInterface.value());
    u32_InterfaceCounter++;
  }
  orc_Ini.setValue(orc_SectionName + "/" +
                        c_CanOpenExpandedCanOpenManagerCounter,
                    static_cast<int>(c_Interfaces.size()));
  uint32_t u32_DevicesCounter = 0UL;
  QHash<uint8_t, bool> c_Devices = orc_Node.GetExpandedCanOpenDevices();
  for (QHash<uint8_t, bool>::const_iterator c_ItDevices = c_Devices.begin();
       c_ItDevices != c_Devices.end(); ++c_ItDevices) {
    orc_Ini.setValue(orc_SectionName + "/" +
                         (c_CanOpenExpandedCanOpenDevices +
                          QString::number(u32_DevicesCounter) +
                          "InterfaceNumber"),
                     c_ItDevices.key());
    orc_Ini.setValue(orc_SectionName + "/" +
                         (c_CanOpenExpandedCanOpenDevices +
                          QString::number(u32_DevicesCounter)),
                     c_ItDevices.value());
    u32_DevicesCounter++;
  }
  orc_Ini.setValue(orc_SectionName + "/" +
                        c_CanOpenExpandedCanOpenDevicesCounter,
                    static_cast<int>(c_Devices.size()));
  uint32_t u32_DeviceCounter = 0UL;
  QMap<std::pair<uint8_t, std::pair<uint8_t, QString>>, bool> c_Device =
      orc_Node.GetExpandedCanOpenDevice();
  for (QMap<std::pair<uint8_t, std::pair<uint8_t, QString>>,
            bool>::const_iterator c_ItDevice = c_Device.begin();
       c_ItDevice != c_Device.end(); ++c_ItDevice) {
    orc_Ini.setValue(orc_SectionName + "/" +
                         (c_CanOpenExpandedCanOpenDevice +
                          QString::number(u32_DeviceCounter) +
                          "InterfaceNumber"),
                     c_ItDevice.key().first);
    orc_Ini.setValue(orc_SectionName + "/" +
                         (c_CanOpenExpandedCanOpenDevice +
                          QString::number(u32_DeviceCounter) +
                          "DeviceInterfaceNumber"),
                     c_ItDevice.key().second.first);
    orc_Ini.setValue(orc_SectionName + "/" +
                         (c_CanOpenExpandedCanOpenDevice +
                          QString::number(u32_DeviceCounter) +
                          "DeviceNodeName"),
                     c_ItDevice.key().second.second);
    orc_Ini.setValue(orc_SectionName + "/" +
                         (c_CanOpenExpandedCanOpenDevice +
                          QString::number(u32_DeviceCounter)),
                     c_ItDevice.value());
    u32_DeviceCounter++;
  }
  orc_Ini.setValue(orc_SectionName + "/" +
                       c_CanOpenExpandedCanOpenDeviceCounter,
                   static_cast<int>(c_Device.size()));
  orc_Ini.setValue(orc_SectionName + "/" + c_CanOpenSelectedCanOpenManager,
                   static_cast<int32_t>(orc_Node.GetSelectedCanOpenManager()));
  orc_Ini.setValue(
      orc_SectionName + "/" + c_CanOpenSelectedCanOpenDeviceInterfaceNumber,
      static_cast<int32_t>(orc_Node.GetSelectedCanOpenDeviceInterfaceNumber()));
  orc_Ini.setValue(orc_SectionName + "/" +
                       c_CanOpenSelectedCanOpenDeviceNodeName,
                   orc_Node.GetSelectedCanOpenDeviceNodeName());
  orc_Ini.setValue(
      orc_SectionName + "/" + c_CanOpenSelectedCanOpenDeviceUseCaseIndex,
      static_cast<int32_t>(orc_Node.GetSelectedCanOpenDeviceUseCaseIndex()));

  orc_Ini.setValue(orc_SectionName + "/" + c_CanOpenSelectedUseCaseOrInterface,
                   orc_Node.GetCanOpenSelectedUseCaseOrInterface());

  // Selected HALC domain & channel
  orc_Ini.setValue(orc_SectionName + "/" + c_NodeIdSelectedHalcDomain,
                   orc_Node.GetSelectedHalcDomainName());

  orc_Ini.setValue(orc_SectionName + "/" + c_NodeIdSelectedHalcChannel,
                   orc_Node.GetSelectedHalcChannel());

  // HALC columns
  C_UsFiler::mh_SaveColumns(orc_Ini, orc_SectionName, c_HalcOvColumnId,
                            orc_Node.GetHalcOverviewColumnWidth());
  C_UsFiler::mh_SaveColumns(orc_Ini, orc_SectionName, c_HalcConfigColumnId,
                            orc_Node.GetHalcConfigColumnWidth());

  // Datapool count
  orc_Ini.setValue(orc_SectionName + "/" + c_NodeIdDatapoolCount,
                   static_cast<int>(c_DatapoolKeyList.size()));
  for (QStringList::const_iterator c_ItDatapoolKey =
           c_DatapoolKeyList.begin();
       c_ItDatapoolKey != c_DatapoolKeyList.end(); ++c_ItDatapoolKey) {
    const QString c_DatapoolIdBase = static_cast<QString>("%1Datapool%2")
                                         .arg(orc_NodeIdBase)
                                         .arg(s32_ItDatapool);
    const QString c_DatapoolName = *c_ItDatapoolKey;
    const C_UsNodeDatapool c_Datapool = orc_Node.GetDatapool(c_DatapoolName);
    mh_SaveDatapool(orc_Ini, orc_SectionName, c_DatapoolIdBase, c_DatapoolName,
                    c_Datapool);

    // Important iterator step
    ++s32_ItDatapool;
  }
  // DataLogger LogJob Index
  orc_Ini.setValue(
      orc_SectionName + "/" + c_NodeIdSelectedDataLoggerLogJobIndex,
      static_cast<int32_t>(orc_Node.GetSelectedDataLoggerLogJobIndex()));
  // DataLogger Overview widget selected
  orc_Ini.setValue(orc_SectionName + "/" + c_DataLoggerOverviewWidgetSelected,
                   orc_Node.GetIsOverviewWidgetSelected());
}

//----------------------------------------------------------------------------------------------------------------------
/*! \brief   Save bus part of user settings

   \param[in,out]  orc_Ini          Ini handler
   \param[in]      orc_SectionName  Section name
   \param[in]      orc_BusIdBase    Bus id base name
   \param[in]      orc_BusName      Bus name
   \param[in]      orc_Bus          Bus data
*/
//----------------------------------------------------------------------------------------------------------------------
