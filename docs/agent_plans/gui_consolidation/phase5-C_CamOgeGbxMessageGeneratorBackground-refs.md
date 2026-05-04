# Reference scan — C_CamOgeGbxMessageGeneratorBackground

**Class:** `stw::opensyde_gui_elements::C_CamOgeGbxMessageGeneratorBackground`
**Parent:** `QGroupBox`
**Replacement:** `QGroupBox` + `styleRole="cam-gbx-message-generator-background"`
**Stylesheet-only:** ✅ confirmed (empty ctor body, no overrides, no members, only public method is the ctor)

## Counts

| File type | Files | Refs |
|---|---|---|
| `.ui` | 2 | 4 widget instances + 2 customwidget blocks |
| `.qss` | 2 | 4 selector lines (Color.qss in both apps) |
| `.cpp` / `.hpp` (excluding the class itself) | 0 | none |
| Build / lint | 2 | 4 lines (both CMakeLists.txt × .cpp + .hpp) |

No substring collisions: `grep 'C_CamOgeGbxMessageGeneratorBackground[A-Za-z]'` returns nothing.

## .ui files (2)

`opensyde_can_monitor/src/can_monitor/message_generator/C_CamGenMessagesWidget.ui`
- Widget instance + customwidget block

`opensyde_can_monitor/src/can_monitor/message_generator/signals/C_CamGenSigWidget.ui`
- Widget instance + customwidget block

## .qss files (2 files, 4 lines)

### CAN Monitor app
- `opensyde_can_monitor/src/can_monitor/styles/Color.qss` (2 selectors)

### SYDEflash app
- `opensyde_syde_flash/src/syde_flash/styles/Color.qss` (2 selectors)

## .cpp / .hpp consumers

(none)

## Build / lint

- `opensyde_can_monitor/pjt/CMakeLists.txt` (.cpp + .hpp)
- `opensyde_syde_flash/pjt/CMakeLists.txt` (.cpp + .hpp)
