# Reference scan — C_OgeSpxNumber

## .ui files (21 files, 42 lines)
- libraries/opensyde_gui/src/navigable_gui/C_NagProjectSettingsPopupDialog.ui (lines: 180, 628, 630)
- libraries/opensyde_gui/src/system_definition/bus_edit/j1939/C_SdBueJ1939PgPropertiesDialog.ui (lines: 174, 229, 313, 502, 504)
- libraries/opensyde_gui/src/system_definition/bus_edit/C_SdBueMessagePropertiesWidget.ui (lines: 526, 601, 661, 750, 776, 890, 925, 1304, 1306)
- libraries/opensyde_gui/src/system_definition/bus_edit/C_SdBueMessageRxTimeoutConfig.ui (lines: 262, 446, 448)
- libraries/opensyde_gui/src/system_definition/bus_edit/C_SdBueBusEditPropertiesWidget.ui (lines: 400, 834, 836)
- libraries/opensyde_gui/src/system_definition/bus_edit/C_SdBueSignalPropertiesWidget.ui (lines: 652, 706, 966, 1186, 1267, 1269)
- libraries/opensyde_gui/src/system_definition/node_edit/canopen_manager/C_SdNdeCoDeviceConfigWidget.ui (lines: 186, 648, 683, 903, 905)
- libraries/opensyde_gui/src/system_definition/node_edit/canopen_manager/C_SdNdeCoManagerIntfWidget.ui (lines: 186, 419, 710, 810, 1050, 1172, 1174)
- libraries/opensyde_gui/src/system_definition/node_edit/data_blocks/C_SdNdeDbProperties.ui (lines: 900, 1975, 1977)
- libraries/opensyde_gui/src/system_definition/node_edit/data_logger/C_SdNdeDalLogJobGeneralSettingsWidget.ui (lines: 153, 232, 278, 280)
- libraries/opensyde_gui/src/system_definition/node_edit/data_logger/C_SdNdeDalLogJobPropertiesWidget.ui (lines: 256, 693, 901, 1161, 1163)
- libraries/opensyde_gui/src/system_definition/node_edit/datapools/C_SdNdeDpProperties.ui (lines: 967, 1037, 1421, 1423)
- libraries/opensyde_gui/src/system_definition/node_edit/C_SdNdeProgrammingOptions.ui (lines: 331, 347, 399, 590, 592)
- libraries/opensyde_gui/src/system_views/dashboards/C_SyvDaDashboardSettings.ui (lines: 176, 243, 310, 498, 500)
- opensyde_tool/src/graphic_items/style_setup/C_GiSyBoundaryWidget.ui (lines: 81, 241, 243)
- opensyde_tool/src/graphic_items/style_setup/C_GiSyLineWidget.ui (lines: 78, 559, 561)

## .qss files (2 files, 12 lines)
- opensyde_tool/src/styles/Color.qss (lines: 632, 815, 1037, 1174, 1455, 1469, 1496, 1631, 1676, 1736)
- opensyde_tool/src/styles/Font.qss (line: 133)

## .cpp/.hpp files (5 files, 43 lines)
### Class definition files:
- libraries/opensyde_gui/src/opensyde_gui_elements/spin_box/C_OgeSpxNumber.cpp (lines: 18, 43)
- libraries/opensyde_gui/src/opensyde_gui_elements/spin_box/C_OgeSpxNumber.hpp (lines: 27, 33)

### Application code (signal connect/disconnect):
- libraries/opensyde_gui/src/system_definition/bus_edit/C_SdBueSignalPropertiesWidget.cpp (lines: 2141, 2161, 2203, 2223) — signal connect
- libraries/opensyde_gui/src/system_definition/bus_edit/C_SdBueBusEditPropertiesWidget.cpp (lines: 220, 436) — signal connect
- libraries/opensyde_gui/src/system_definition/bus_edit/C_SdBueMessagePropertiesWidget.cpp (lines: 181, 186, 1583, 1589, 1599, 1616, 1623, 1633, 3050, 3054, 3058, 3062, 3088, 3092, 3096, 3100) — signal connect/disconnect
- libraries/opensyde_gui/src/system_definition/bus_edit/C_SdBueMessagePropertiesWidget.cpp (line: 3194) — member type declaration (C_OgeSpxNumber * const pc_VisibleSpinBox)

### Generated moc files (build artifacts):
- build/Release/opensyde/openSYDE_autogen/3JI7TJTTWZ/moc_C_OgeSpxNumber.cpp (lines: 2, 9, 19, 34, 37, 41, 50, 53, 55, 56, 59, 63, 65, 72, 77, 80, 85)

## CMakeLists.txt files (2 entries)
- opensyde_tool/pjt/openSYDE/CMakeLists.txt (lines: 451, 1279) — .cpp entry, .hpp entry

## Other
- No references found in C_UtiStyleSheets.cpp
- No references found in opensyde_can_monitor CMakeLists.txt
- No references found in opensyde_syde_flash CMakeLists.txt

---

## Summary counts

| Category | Files | Lines |
|----------|-------|-------|
| .ui files | 16 | 42 |
| .qss files | 2 | 12 |
| .cpp/.hpp files | 5 | 43 |
| CMakeLists.txt | 1 | 2 |
| **Total** | **24** | **99** |

Note: Generated moc files in build/ directory are build artifacts and will be regenerated automatically. They should be excluded from migration scope.
