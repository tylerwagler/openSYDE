# Reference scan — C_OgeSpxDoubleProperties

Replacement: `C_OgeSpxDoubleToolTipBase` + `styleRole="spx-double-properties"`

## .ui files (2)
- libraries/opensyde_gui/src/system_definition/bus_edit/C_SdBueSignalPropertiesWidget.ui
  - Line 604: widget instance `pc_DoubleSpinBoxOffset`
  - Lines 1312, 1314: customwidget entry
- libraries/opensyde_gui/src/system_views/dashboards/properties/C_SyvDaPeBase.ui
  - Line 1222: widget instance `pc_DoubleSpinBoxOffset`
  - Lines 1909, 1911: customwidget entry

## .qss files (3, 27 selectors)
- opensyde_tool/src/styles/SpinBox.qss (11)
- opensyde_tool/src/styles/Color.qss (15)
- opensyde_tool/src/styles/Font.qss (1)

## .cpp/.hpp (0 external)

## CMakeLists + lint sources
- opensyde_tool/pjt/openSYDE/CMakeLists.txt (lines 498, 1321)
- build_test/pclint_config/openSYDE/openSYDE_lint_sources.txt (line 330)
