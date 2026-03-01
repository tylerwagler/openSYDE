# STL to Qt Container Migration Reference

## Migration Status (2026-02-26)
- `std::vector` — Fully migrated (only comments and 3rd-party interfaces remain)
- `std::list` — In progress
- `std::map` — Pending
- `std::set` — Pending
- `std::string` — Pending

## std::list (~80 occurrences across ~26 files)

### Core library (libs/opensyde_core/)

**`C_OscSystemNameMaxCharLimitChangeReportItem` pattern** (~20 occurrences across 10+ files):
- `std::list<C_OscSystemNameMaxCharLimitChangeReportItem>` used as output parameter in `AdaptToEnumNaming()` calls
- Files: C_OscSystemNameMaxCharLimitChangeReportItem.hpp/.cpp, C_OscHalcConfig.hpp/.cpp, C_OscHalcConfigChannel.hpp/.cpp, C_OscHalcConfigDomain.hpp/.cpp, C_OscNodeDataPoolList.hpp/.cpp, C_OscNode.hpp/.cpp, C_OscNodeDataPool.hpp/.cpp, C_OscCanMessageContainer.hpp/.cpp, C_OscCanProtocol.hpp/.cpp
- Migration: Trivial `std::list` → `QList`, all use push_back() which QList supports

**Protocol drivers:**
- `C_OscComDriverBase.hpp`: `std::list<T_STWCAN_Msg_TX>`, `std::list<C_OscComDriverBaseCanMessage>` (CAN message queues)
- `C_OscCanDispatcherOsyRouter.hpp`: `std::list<T_STWCAN_Msg_RX>` (async message buffer)
- `C_OscRoutingCalculation.hpp`: `std::list<C_OscRoutingRoute>` (route-finding queue)
- `C_OscComLoggerProtocolOpenSyde.hpp`: `std::list<const C_OscComMessageLoggerOsySysDefConfig*>`

**IP Dispatcher (SKIP - platform-specific, will be done with std::map):**
- `C_OscIpDispatcherWinSock.hpp` (both Linux and Windows versions): `std::map<C_BufferIdentifier, std::list<QByteArray>>`

### GUI code (src/)

**CAN Monitor message trace (high-use buffer):**
- `C_CamMetTreeGuiBuffer.hpp/.cpp`: `std::list<C_CamMetTreeLoggerData>` buffer + signal
- `C_CamMetTreeModel.hpp/.cpp`: `AddRows(const std::list<C_CamMetTreeLoggerData>&)` + internal iteration
- `C_CamMetTreeView.hpp/.cpp`: `m_UpdateUi(const std::list<...>&)`, `qRegisterMetaType<std::list<...>>()`
- NOTE: Must update qRegisterMetaType to use QList type

**Signal properties:**
- `C_SdBueSignalPropertiesWidget.hpp/.cpp`: `std::list<E_Change>` local variables and parameters

**System views:**
- `C_SyvSeScene.hpp/.cpp`: `std::list<QDialog*>` member
- `C_SyvComDriverDiag.hpp/.cpp`: `QList<std::list<C_OscProtocolDriverOsy::C_DataPoolMetaData>>` — nested in QList
- `C_SyvDaItPaWriteWidget.cpp`: `std::list<QString>` → `QStringList`
- `C_SyvDaItPaImageRecordWidget.cpp`: `std::list<QString>` → `QStringList`

## std::map (~269 occurrences across ~55 files)

### Core library

**exports/code_generation/ (8 occurrences):**
- C_OscExportNode: `std::map<bool, int32_t>` → `QMap<bool, int32_t>`
- C_OscExportCommunicationStack: `std::map<int32_t, QList<C_OscCanSignal>>` → `QMap`

**system_update_package/ (18 occurrences):**
- C_OscSupNodeDefinitionFiler, C_OscSupServiceUpdatePackageLoad, C_OscSupServiceUpdatePackageV1
- All: `std::map<uint32_t, uint32_t>` → `QMap<uint32_t, uint32_t>`

**project/system/node/ (10 occurrences):**
- C_OscNodeDataPool.cpp: `std::map<QString, uint32_t>`, `std::map<uint32_t, bool>` (static caches)
- C_OscNode.cpp: `std::map<QList<uint32_t>, bool>`, `std::map<QString, uint32_t>`
- C_OscCanMessage.cpp: `std::map<QList<uint32_t>, bool>`

**protocol_drivers/ (34 occurrences):**
- C_OscProtocolDriverOsy: `std::map<uint8_t, uint8_t>`
- C_OscComDriverProtocol: `std::map<uint32_t, uint32_t>`
- C_OscComMessageLogger: `std::map<QString, C_OscComMessageLoggerFileBase*>`, `std::map<QString, C_OscComMessageLoggerOsySysDefConfig>`
- C_OscComAutoSupport: `std::map<int32_t, uint8_t>`, `std::map<int32_t, std::pair<bool, E_Type>>`

### GUI code

**Pattern: `std::map<E_Columns, uint32_t>` for column widths (~15 files):**
- C_SdBueMessageTableView, C_SdBueSignalTableView, C_SdBueJ1939AddMessagesFromCatalogTreeView
- C_SdNdeDalLogJobDataSelectionTableView, C_NagUnUsedProjectFilesTableView

**project_gui/system_definition/ (~57 occurrences):**
- C_PuiSdHandlerHalc, C_PuiSdHandlerData, C_PuiSdHandlerFiler
- C_PuiSdNodeDataPoolListElementIdSyncUtil (extensive)
- Key type: `std::map<C_OscNodeDataPoolListElementOptArrayId, C_PuiSdLastKnownHalElementId>`

**project_gui/system_views/ (~30 occurrences):**
- C_PuiSvDashboard, C_PuiSvData, C_PuiSvHandler

**Iterator pattern changes:**
- `it->first` → `it.key()`
- `it->second` → `it.value()`
- `emplace(std::pair<>())` → `insert(key, value)`
- `std::map::find()` → `QMap::find()` (same API)
- `lower_bound()` → `lowerBound()` (different casing in QMap)

### 3RD PARTY — DO NOT MIGRATE:
- `src/com_import_export/C_CieExportDbc.cpp/.hpp`: `std::map<std::string, Vector::DBC::*>` (Vector DBC library)
- `src/com_import_export/C_CieImportDbc.cpp`: Same Vector DBC interface

### Special cases:
- `std::map<std::pair<uint8_t, std::pair<uint8_t, QString>>, bool>` in C_UsNode, C_SdNdeCoConfigTreeView — nested pair key, QMap works if operator< defined (std::pair has it)
- `std::map<QList<uint32_t>, bool>` in C_OscNode, C_OscCanMessage — QList has operator<, so QMap works

## std::set (~118 occurrences)

### Core library (30 occurrences)

**Multiplexer values:**
- C_OscCanSignal.hpp: `std::set<uint16_t> GetDataBytesBitPositionsOfSignal()` — PUBLIC API
- C_OscCanMessage.hpp: `std::set<uint16_t> GetMultiplexerValues()` — PUBLIC API
- C_OscExportCommunicationStack.cpp: `std::set<uint16_t>` for multiplexer values
- WARNING: std::set is sorted, QSet is unordered. Some callers may depend on sorted order.

**Protocol driver node tracking:**
- C_OscComDriverProtocol.hpp: `std::set<uint32_t>` for defect nodes, skip-node sets
- C_OscSuSequences.cpp: `std::set<uint32_t>` for XFL reset tracking
- NOTE: `SendTesterPresent()` takes `const std::set<uint32_t> *const opc_SkipNodes = NULL` — public API

### GUI code (88 occurrences)

**Message layout viewer — NEEDS CARE:**
- C_SdBueMlvSignalManager.hpp/.cpp: `std::set<uint16_t>` with **reverse_iterator** usage
- QSet has no rbegin()/rend() — need to collect to QList, sort, reverse-iterate

**Persistent iterator pattern — NEEDS REFACTORING:**
- C_SyvDaItPaWriteWidget.hpp: member `mc_NotificationIterator` stored as `std::set` iterator
- Can't store QSet iterator as member (undefined behavior after modifications)

**Custom type keys — NEED qHash:**
- C_OscCanMessageUniqueId, C_OscNodeDataPoolListElementOptArrayId, C_PuiSvDbNodeDataPoolListElementId
- These types need `qHash()` overload and `operator==()` for QSet, OR use QMap for sorted behavior

**Simple uint32/uint16 sets (majority of cases):**
- C_SyvComDriverDiag, C_SyvDcExistingNodeList, C_SyvDcSequences
- C_CamMetTreeModel, C_SdBueMlvWidget, C_SdBueMessageSelectorTreeWidget

## std::string (~55 occurrences)

### Core library (15 occurrences)

**Key upstream changes (cascade-fix pattern):**
- `C_OscNodeDataPoolContent::GetValueAsScaledString(std::string&)` — returns in std::string. Changing to QString would cascade-fix 3+ callers
- `C_OscHalcDefContent::SetStringValue(const std::string&)` / `GetStringValue(std::string&)` — has QString overloads, std::string versions can likely be removed
- `C_OscHalcConfig::SetChannelParameterConfigStringValue(bool, const std::string&)` — called via `.toStdString()` from callers
- `C_OscSupServiceUpdatePackageBase.cpp`: `std::stringstream` for hex formatting → `QString::number()`

### GUI code (40 occurrences)

**3RD PARTY — DO NOT MIGRATE:**
- `C_CieExportDbc.cpp/.hpp`, `C_CieImportDbc.cpp`: `std::string` for Vector DBC library API

**INI file keys:**
- `C_UsFiler.hpp`: `static const std::string` constants → `static const QString`

**Intermediaries (auto-fixed by upstream changes):**
- C_SdNdeDpContentUtil.cpp, C_SdNdeHalcConfigTreeModel.cpp, C_PuiSdNodeDataPoolListElementIdSyncUtil.cpp
- All convert QString→std::string to call core APIs that should return QString

## IP Dispatcher (DEFER)
Both Windows and Linux versions of C_OscIpDispatcherWinSock use `std::map<C_BufferIdentifier, std::list<QByteArray>>`. This is low-level platform code. Migrate together as a unit (both std::map and std::list).
