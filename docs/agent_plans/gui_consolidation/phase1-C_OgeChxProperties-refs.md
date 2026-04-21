# Reference scan — C_OgeChxProperties

Replacement: `C_OgeChxToolTipBase` + `styleRole="chx-properties"`

## .ui files (7, 10 widgets + 7 customwidget blocks)
- C_CamMosFilterPopup.ui
- C_SdBueSignalPropertiesWidget.ui
- C_SdBueMessagePropertiesWidget.ui (deleted customwidget rather than renamed — ToolTipBase already present from PropertiesSmall migration)
- C_SdNdeStwFlashloaderOptions.ui
- C_SyvDaItPaImageRecordWidget.ui
- C_SyvDaItPaWriteWidget.ui (2 widgets)
- C_SyvDaPeBase.ui (3 widgets)

## .qss (all 3 apps)
- opensyde_tool/src/styles: Color.qss, CheckBox.qss, Font.qss (11 selectors)
- opensyde_syde_flash: CheckBox.qss, Color.qss (10 selectors)
- opensyde_can_monitor: CheckBox.qss, Color.qss (10 selectors)

## .cpp (6 files, ~13 signal connects)
- C_CamMosFilterPopup.cpp, C_SdBueMessagePropertiesWidget.cpp,
  C_SdBueSignalPropertiesWidget.cpp, C_SyvDaItPaImageRecordWidget.cpp,
  C_SyvDaItPaWriteWidget.cpp, C_SyvDaPeBase.cpp

## CMakeLists (all 3) + lint
