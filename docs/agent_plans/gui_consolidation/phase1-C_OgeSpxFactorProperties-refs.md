# Reference scan — C_OgeSpxFactorProperties

Replacement: `C_OgeSpxFactor` + `styleRole="spx-factor-properties"`

## .ui files (2 files)
- libraries/opensyde_gui/src/system_definition/bus_edit/C_SdBueSignalPropertiesWidget.ui
  - Line 794: widget instance `pc_DoubleSpinBoxFactor`
  - Lines 1314, 1316: customwidget entry
- libraries/opensyde_gui/src/system_views/dashboards/properties/C_SyvDaPeBase.ui
  - Line 1180: widget instance `pc_DoubleSpinBoxFactor`
  - Lines 1911, 1913: customwidget entry

Note: parent class `C_OgeSpxFactor` may already be a customwidget in these .ui files — check and only add if absent.

## .qss files (3 files, 26 selectors)
- opensyde_tool/src/styles/SpinBox.qss (lines: 6, 32, 64, 100, 133, 192, 208, 224, 241, 258, 287) — 11 selectors
- opensyde_tool/src/styles/Color.qss (lines: 448, 452, 635, 818, 1051, 1177, 1294, 1306, 1473, 1499, 1503, 1634, 1679, 1739) — 14 selectors
- opensyde_tool/src/styles/Font.qss (line 136) — 1 selector

## .cpp/.hpp files (0 external references)
No signal connects, no type declarations, no includes outside the class's own files.

## CMakeLists.txt
- opensyde_tool/pjt/openSYDE/CMakeLists.txt (lines 501, 1327)

## Lint sources
- build_test/pclint_config/openSYDE/openSYDE_lint_sources.txt (line 333)

## Summary

| Category | Count |
|---|---|
| .ui files | 2 |
| .qss selectors | 26 |
| External C++ refs | 0 |
| CMakeLists entries | 2 |
| Lint sources | 1 |
