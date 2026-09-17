# Reference scan — C_OgeChxPropertiesSmall

Replacement: `C_OgeChxToolTipBase` + `styleRole="chx-properties-small"`
(Grandparent replacement — direct parent C_OgeChxProperties is also being deleted.)

## .ui files (5)
- libraries/opensyde_gui/src/system_definition/bus_edit/C_SdBueBusEditPropertiesWidget.ui
  - Lines 360, 772 — widgets; 863, 865 — customwidget
- libraries/opensyde_gui/src/system_definition/bus_edit/C_SdBueMessagePropertiesWidget.ui
  - Line 555 — widget; 1330, 1332 — customwidget
- libraries/opensyde_gui/src/system_definition/node_edit/canopen_manager/C_SdNdeCoDeviceConfigWidget.ui
  - Lines 221, 358, 367, 374, 774, 797, 848 — widgets (7); 928, 930 — customwidget
- libraries/opensyde_gui/src/system_definition/node_edit/canopen_manager/C_SdNdeCoManagerIntfWidget.ui
  - Lines 221, 371, 378, 564, 840, 1109 — widgets (6); 1192, 1194 — customwidget
- libraries/opensyde_gui/src/system_definition/node_edit/data_logger/C_SdNdeDalLogJobAdditionalTriggerPropertiesWidget.ui
  - Line 152 — widget; 561, 563 — customwidget

**Total: 17 widget instances, 5 customwidget blocks**

## .qss (1 file, 2 selectors)
- opensyde_tool/src/styles/CheckBox.qss lines 9, 18

## .cpp/.hpp (0 external)

## CMakeLists + lint
- opensyde_tool/pjt/openSYDE/CMakeLists.txt (lines 688, 1506)
- build_test/pclint_config/openSYDE/openSYDE_lint_sources.txt (line 520)
