# Reference scan — C_CamOgeSpxProperties

**Class:** `stw::opensyde_gui_elements::C_CamOgeSpxProperties`
**Parent:** `C_OgeSpxToolTipBase`
**Replacement:** `C_OgeSpxToolTipBase` + `styleRole="cam-spx-properties"`
**Stylesheet-only:** ✅ confirmed (empty ctor body, no overrides, no members, only public method is the ctor)

## Counts

| File type | Files | Refs |
|---|---|---|
| `.ui` | 4 | 12 (1 widget instance + customwidget block per file) |
| `.qss` | 6 | 40 selector lines (`Color.qss` × 10 + `Font.qss` × 1 + `SpinBox.qss` × 9 in each app) |
| `.cpp` / `.hpp` (excluding the class itself) | 0 | none |
| Build / lint | 2 | 4 lines (both CMakeLists × .cpp + .hpp) |

This is the heaviest qss-side migration of the Phase 5 batch — `SpinBox.qss` styles every spin-box pseudo-state (`::up-button`, `::down-button`, `::up-arrow`, `::down-arrow`, plus `:!enabled` variants).

## .ui files (4 files)

- `opensyde_can_monitor/src/can_monitor/message_generator/C_CamGenKeySelect.ui` — `pc_SpinBoxOffset`.
- `opensyde_can_monitor/src/can_monitor/message_trace/C_CamMetSettingsPopup.ui` — `pc_SpinBoxTraceBuffer`.
- `opensyde_syde_flash/src/syde_flash/configure_node/C_FlaConNodeConfigPopup.ui` — `pc_SpxNodeId`.
- `opensyde_syde_flash/src/syde_flash/properties/C_FlaPropWidget.ui` — `pc_SpxNodeId`.

Migration pattern (same in all four):
```xml
<!-- before -->
<widget class="stw::opensyde_gui_elements::C_CamOgeSpxProperties" name="...">
<!-- after -->
<widget class="stw::opensyde_gui_elements::C_OgeSpxToolTipBase" name="...">
 <property name="styleRole" stdset="0"><string>cam-spx-properties</string></property>
```

## .qss files (6 files, 40 lines)

- `Color.qss` — 10 selectors per app (background, border, text color across enabled/disabled/hover, plus arrow tinting)
- `Font.qss` — 1 line per app (font-family bundle)
- `SpinBox.qss` — 9 selectors per app:
  - root selector (border, padding)
  - `::down-button` and `::up-button` (border, width)
  - `::down-arrow` and `::up-arrow` (image, width)
  - `:!enabled` variants of each arrow

Migration pattern:
```css
/* before */
stw--opensyde_gui_elements--C_CamOgeSpxProperties::down-arrow
/* after */
stw--opensyde_gui_elements--C_OgeSpxToolTipBase[styleRole="cam-spx-properties"]::down-arrow
```

## .cpp / .hpp consumers

(none — Qt's uic generates the typed member pointers, but no C++ source references the class name directly)

## Build / lint

- `opensyde_can_monitor/pjt/CMakeLists.txt` — .cpp + .hpp entries removed
- `opensyde_syde_flash/pjt/CMakeLists.txt` — .cpp + .hpp entries removed
