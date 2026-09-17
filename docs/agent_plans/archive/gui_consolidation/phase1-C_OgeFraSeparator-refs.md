# Reference scan — C_OgeFraSeparator

**Class:** `stw::opensyde_gui_elements::C_OgeFraSeparator`
**Parent:** `QFrame` (direct, no STW intermediate)
**Replacement:** `QFrame` + `styleRole="fra-separator"`
**Stylesheet-only:** ✅ confirmed (empty ctor body, no overrides, no members)

## Counts

| File type | Files | Refs |
|---|---|---|
| `.ui` | 96 | 168 widget instances + 96 customwidget blocks (360 total grep lines) |
| `.qss` | 3 (one Color.qss per app) | 13 selector lines |
| `.cpp` / `.hpp` (excluding the class itself) | 4 files | 6 lines |
| Build / lint | 4 files | 7 lines |

No substring collisions: no other class starts with `C_OgeFraSeparator`. Class
is namespace-prefixed in all .ui files (`stw::opensyde_gui_elements::C_OgeFraSeparator`).

## .ui files (96)

All 96 listed below. Most appear in `libraries/opensyde_gui/`; the rest are in
the three app dirs.

### `libraries/opensyde_gui/`
- can_monitor/can_monitor_settings: `C_CamMosDatabaseBusSelectionPopup.ui`, `C_CamMosDatabaseSelectionPopup.ui`, `C_CamMosDatabaseWidget.ui`, `C_CamMosDllWidget.ui`, `C_CamMosFilterPopup.ui`, `C_CamMosFilterWidget.ui`
- com_import_export: `C_CieDbcImportNodeAssignmentWidget.ui`, `C_CieDcfEdsImportNodeSelectWidget.ui`, `C_CieExportReportWidget.ui`, `C_CieImportDatapoolSelectWidget.ui`, `C_CieImportReportWidget.ui`, `C_RtfExportWidget.ui`
- navigable_gui: `C_NagMainWidget.ui`, `C_NagProjectSettingsPopupDialog.ui`, `C_NagToolSettingsPopupDialog.ui`, `C_NagToolTipWithImage.ui`, `C_NagUseCaseViewWidget.ui`, `unused_project_files/C_NagUnUsedProjectFilesPopUpDialog.ui`
- project_operations: `C_PopCreateServiceProjDialogWidget.ui`, `C_PopSaveAsDialogWidget.ui`
- system_definition root: `C_SdCodeGenerationDialog.ui`, `C_SdNodeComIfSetupWidget.ui`, `C_SdNodeToNodeConnectionSetupWidget.ui`
- system_definition/bus_edit: `C_SdBueBusEditPropertiesWidget.ui`, `C_SdBueImportCommMessagesWidget.ui`, `C_SdBueMessagePropertiesWidget.ui`, `C_SdBueMessageRxTimeoutConfig.ui`, `C_SdBueMessageSelectorWidget.ui`, `C_SdBueMessageSignalEditWidget.ui`, `C_SdBueSignalPropertiesWidget.ui`, `canopen/C_SdBueCoAddSignalsDialog.ui`, `j1939/C_SdBueJ1939AddMessagesFromCatalogDialog.ui`, `j1939/C_SdBueJ1939PgPropertiesDialog.ui`
- system_definition/node_edit: `C_SdNdeIpAddressConfigurationWidget.ui`, `C_SdNdeNodePropertiesWidget.ui`, `C_SdNdeProgrammingOptions.ui`
- system_definition/node_edit/canopen_manager: `C_SdNdeCoAddDeviceDialog.ui`, `C_SdNdeCoDeviceConfigWidget.ui`, `C_SdNdeCoDeviceEdsWidget.ui`, `C_SdNdeCoDeviceUpdateEdsDialog.ui`, `C_SdNdeCoManagerIntfWidget.ui`
- system_definition/node_edit/data_blocks: `C_SdNdeDbAddNewProject.ui`, `C_SdNdeDbProperties.ui`, `C_SdNdeDbSelectDataPools.ui`, `C_SdNdeDbViewWidget.ui`, `C_SdNdeDbWidget.ui`
- system_definition/node_edit/data_logger: `C_SdNdeDalLogJobAdditionalTriggerDialog.ui`, `C_SdNdeDalLogJobAdditionalTriggerPropertiesWidget.ui`, `C_SdNdeDalLogJobDataSelectionWidget.ui`, `C_SdNdeDalLogJobPropertiesWidget.ui`
- system_definition/node_edit/datapools: `C_SdNdeDpImportRamViewReport.ui`, `C_SdNdeDpListCommentDialog.ui`, `C_SdNdeDpProperties.ui`, `C_SdNdeDpSelectorAddWidget.ui`
- system_definition/node_edit/halc: `C_SdNdeHalcChannelDpPreviewPopUp.ui`, `C_SdNdeHalcChannelWidget.ui`, `C_SdNdeHalcConfigImportDialog.ui`, `C_SdNdeHalcDefUpdateDialog.ui`
- system_definition/node_edit/stw_flashloader_options: `C_SdNdeStwFlashloaderOptions.ui`
- system_views/dashboards: `C_SyvDaDashboardSettings.ui`, `C_SyvDaDashboardTabProperties.ui`
- system_views/dashboards/items/param: `C_SyvDaItPaImageRecordWidget.ui`, `C_SyvDaItPaImportReport.ui`, `C_SyvDaItPaWriteWidget.ui`
- system_views/dashboards/properties: `C_SyvDaPeBase.ui`, `C_SyvDaPeDataElementBrowse.ui`, `C_SyvDaPeUpdateModeConfiguration.ui`
- system_views/dashboards/tab_chart: `C_SyvDaChaPlotHandlerWidget.ui`
- system_views/device_configuration: `C_SyvDcWidget.ui`
- system_views/system_setup: `C_SyvSeDllConfigurationDialog.ui`
- system_views/system_update: `C_SyvUpInformationWidget.ui`, `C_SyvUpNodePropertiesDialog.ui`, `C_SyvUpProgressLog.ui`, `C_SyvUpSummaryWidgetSmall.ui`
- system_views/system_update/update_package: `C_SyvUpPacHexFileView.ui`, `C_SyvUpPacNodeWidget.ui`, `C_SyvUpPacParamSetFileAddPopUp.ui`, `C_SyvUpPacParamSetFileInfoPopUp.ui`, `C_SyvUpPacPemFileInfoPopUp.ui`, `C_SyvUpPacPemFileOptionsPopUp.ui`, `C_SyvUpPacSecureArchiveDialog.ui`, `C_SyvUpPacSecurityCertificatePackageDialog.ui`, `C_SyvUpPacServiceUpdatePackageDialog.ui`

### `opensyde_can_monitor/`
- `C_CamTitleBarWidget.ui`, `message_generator/C_CamGenKeySelect.ui`, `message_trace/C_CamMetSettingsPopup.ui`

### `opensyde_syde_flash/`
- `C_FlaTitleBarWidget.ui`, `bottom_bar/C_FlaBottomBar.ui`, `configure_node/C_FlaConNodeConfigPopup.ui`, `search_nodes/C_FlaSenSearchNodePopup.ui`, `update/C_FlaUpHexFileView.ui`, `update/C_FlaUpListItemWidget.ui`, `update/C_FlaUpProperties.ui`

### `opensyde_tool/`
- `graphic_items/style_setup/C_GiSyBaseWidget.ui`, `graphic_items/style_setup/C_GiSyColorSelectWidget.ui`, `implementation/C_ImpCodeGenerationReportWidget.ui`

## .qss files (3 files, 13 lines)

- `opensyde_can_monitor/src/can_monitor/styles/Color.qss`
  - L308 — `C_OgeFraSeparator[HasColor8Background="true"]` (in selector group)
  - L379 — `C_OgeFraSeparator` (bare class selector)
- `opensyde_syde_flash/src/syde_flash/styles/Color.qss`
  - L292 — `C_OgeFraSeparator[HasColor8Background="true"]`
  - L316 — `C_OgeFraSeparator[HasColor9Background="true"]`
  - L356 — `C_OgeFraSeparator,` (group leader)
- `opensyde_tool/src/styles/Color.qss`
  - L857 — `[HasColor7Background="true"]`
  - L948 — `[HasColor8Background="true"]`
  - L991 — `[HasColor9Background="true"]`
  - L1088 — bare `C_OgeFraSeparator,` group leader
  - L1089 — `[HasColor10Background="true"]`
  - L1785 — `[HasColor27Background="true"]`
  - L1930 — `[HasColor34Background="true"]`
  - L2041 — `[HasColor39Background="true"]`
  - L2432 — `[HasColor1000Background="true"]`

Each `[HasColorNBackground="true"]` selector is an attribute filter on top of
the bare class. Migration pattern (compose both attributes):
```css
/* before */
stw--opensyde_gui_elements--C_OgeFraSeparator[HasColor8Background="true"]
/* after */
QFrame[styleRole="fra-separator"][HasColor8Background="true"]
```

## .cpp / .hpp files (4 consumers, 6 lines)

### `libraries/opensyde_gui/src/system_views/dashboards/C_SyvDaDashboardToolbox.hpp`
- L16 — `#include "C_OgeFraSeparator.hpp"`
- L56 — `stw::opensyde_gui_elements::C_OgeFraSeparator * mpc_FrameSeparatorReadingWidgets;`
- L57 — `stw::opensyde_gui_elements::C_OgeFraSeparator * mpc_FrameSeparatorWritingWidgets;`

→ Change include to `<QFrame>`, change member types to `QFrame *`. Then look at where these members are constructed (likely in the .cpp) and add `setProperty("styleRole", "fra-separator")` there.

### `libraries/opensyde_gui/src/system_views/system_update/C_SyvUpProgressLog.cpp`
- L21 — `#include "C_OgeFraSeparator.hpp"`

→ Header is unused in this file as a type? Likely created via `new C_OgeFraSeparator`. Verify when migrating; replace include with `<QFrame>` if construction is the only use, or remove if a transitively-included QFrame already covers it.

### `libraries/opensyde_gui/src/system_views/system_update/update_package/C_SyvUpPacNodeWidget.{hpp,cpp}`
- `.hpp` L27 — `#include "C_OgeFraSeparator.hpp"`
- `.hpp` L127 — `stw::opensyde_gui_elements::C_OgeFraSeparator * mpc_FilesWidgetSeparator;`
- `.cpp` L1422 — `stw::opensyde_gui_elements::C_OgeFraSeparator * const pc_Separator =`
- `.cpp` L1423 — `      new stw::opensyde_gui_elements::C_OgeFraSeparator(this);`

→ Change include + member type + local + `new`. Add `setProperty("styleRole", "fra-separator")` on construction.

### `opensyde_tool/src/scene_base/C_SebToolboxUtil.{hpp,cpp}`
- `.hpp` L19 — `#include "C_OgeFraSeparator.hpp"`
- `.hpp` L37 — `static stw::opensyde_gui_elements::C_OgeFraSeparator * h_AddNewHeading(...)`
- `.cpp` L111 — function definition signature returns `C_OgeFraSeparator *`
- `.cpp` L114 — `C_OgeFraSeparator * pc_FrameSeparator = NULL;`
- `.cpp` L120 — `pc_FrameSeparator = new C_OgeFraSeparator(opc_Parent);`
- `.cpp` L178 — `C_OgeFraSeparator * const pc_FrameSeparator = new C_OgeFraSeparator(opc_Parent);`

→ Factory function signature change: return `QFrame *` instead. All callers will get `QFrame *`. Both `new` sites need `setProperty("styleRole", "fra-separator")`. Need to grep callers of `h_AddNewHeading` to confirm none of them rely on the `C_OgeFraSeparator *` type specifically.

## CMakeLists.txt + lint sources (4 files, 7 lines)

- `opensyde_tool/pjt/openSYDE/CMakeLists.txt` L393 (.cpp), L1178 (.hpp)
- `opensyde_can_monitor/pjt/CMakeLists.txt` L208 (.cpp), L418 (.hpp)
- `opensyde_syde_flash/pjt/CMakeLists.txt` L170 (.cpp), L314 (.hpp)
- `build_test/pclint_config/openSYDE/openSYDE_lint_sources.txt` L225 (.cpp)

These are removed in Task C only.

## C_UtiStyleSheets.cpp dynamic strings

No matches (`grep 'C_OgeFraSeparator' libraries/opensyde_gui/src/util/C_UtiStyleSheets.cpp` returns nothing — verified by the broader grep above).

## Substring collision check

`grep -rn 'C_OgeFraSeparator' --include="*"` returns no class with this as a
prefix; `replace_all` on the bare token in any single file is safe **after
verifying** no false hits within that file.

## Migration strategy notes

Given the size, Task B should be split across multiple commits to keep diffs
reviewable. Suggested split:

1. **B1** — C++ consumers (factory + 3 widget files): smallest piece, unblocks the rest.
2. **B2** — `.qss` rules across 3 apps (1 file × 3 apps; 13 selector lines total).
3. **B3** — `.ui` files in shared `libraries/opensyde_gui/` (~83 files).
4. **B4** — `.ui` files in 3 app dirs (~13 files).

Build after each Bx; class still exists so build stays green throughout.
Then **Task C** = delete + CMakeLists + lint sources cleanup.
