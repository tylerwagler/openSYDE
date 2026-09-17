# Reference scan — C_CamOgeLabStatusBar

**Class:** `stw::opensyde_gui_elements::C_CamOgeLabStatusBar`
**Parent:** `C_OgeLabToolTipBase` (functional — preserves tooltip/context-menu behavior)
**Replacement:** `C_OgeLabToolTipBase` + `styleRole="cam-lab-status-bar"`
**Stylesheet-only:** ✅ confirmed (empty ctor body, no overrides, no members, only public method is the ctor)

## Counts

| File type | Files | Refs |
|---|---|---|
| `.ui` | 1 | 3 widget instances + 1 customwidget block |
| `.qss` | 4 | 6 selector lines (Color.qss + Font.qss in both apps) |
| `.cpp` / `.hpp` (excluding the class itself) | 0 | none |
| Build / lint | 2 | 4 lines (both CMakeLists.txt × .cpp + .hpp) |

No substring collisions: `grep 'C_CamOgeLabStatusBar[A-Za-z]'` returns
nothing. No `C_UtiStyleSheets.cpp` dynamic refs.

## .ui files (1)

`opensyde_can_monitor/src/can_monitor/message_trace/C_CamMetStatusBarWidget.ui`
- L42 — `<widget class="...C_CamOgeLabStatusBar" name="pc_TxInfoLabel">`
- L77 — `<widget class="...C_CamOgeLabStatusBar" name="pc_ActiveFiltersLabel">`
- L112 — `<widget class="...C_CamOgeLabStatusBar" name="pc_BusLoadLabel">`
- L163–L166 — `<customwidget>` block

## .qss files (4 files, 6 lines)

### CAN Monitor app
- `opensyde_can_monitor/src/can_monitor/styles/Font.qss`
  - L109 — bare class selector (in a group)
- `opensyde_can_monitor/src/can_monitor/styles/Color.qss`
  - L237 — bare class selector
  - L491 — `[Error="true"]` attribute filter

### SYDEflash app
- `opensyde_syde_flash/src/syde_flash/styles/Font.qss`
  - L112 — bare class selector (in a group)
- `opensyde_syde_flash/src/syde_flash/styles/Color.qss`
  - L211 — bare class selector (in a group)
  - L467 — `[Error="true"]` attribute filter

Note: both apps style this class (and both compile its `.cpp` per
CMakeLists). This is consistent with the existing pattern for shared
CAN-monitor-derived widgets — they live under `can_monitor_gui_elements/`
but are reused by both apps.

Migration pattern:
```css
/* before */
stw--opensyde_gui_elements--C_CamOgeLabStatusBar
/* after */
stw--opensyde_gui_elements--C_OgeLabToolTipBase[styleRole="cam-lab-status-bar"]

/* before */
stw--opensyde_gui_elements--C_CamOgeLabStatusBar[Error="true"]
/* after */
stw--opensyde_gui_elements--C_OgeLabToolTipBase[styleRole="cam-lab-status-bar"][Error="true"]
```

## .cpp / .hpp consumers

(none — only the class's own `C_CamOgeLabStatusBar.{hpp,cpp}` files mention
the symbol. No programmatic instantiation, no member declarations
elsewhere.)

## Build / lint

- `opensyde_can_monitor/pjt/CMakeLists.txt` L198 (.cpp), L406 (.hpp)
- `opensyde_syde_flash/pjt/CMakeLists.txt` L132 (.cpp), L274 (.hpp)

(No entries in `build_test/pclint_config/openSYDE/openSYDE_lint_sources.txt`
— that file lists openSYDE main-tool sources only; `C_CamOgeLabStatusBar`
isn't compiled into openSYDE.)
