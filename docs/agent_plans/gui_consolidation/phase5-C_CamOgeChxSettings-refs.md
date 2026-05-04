# Reference scan — C_CamOgeChxSettings

**Class:** `stw::opensyde_gui_elements::C_CamOgeChxSettings`
**Parent:** `C_OgeChxToolTipBase` (functional — preserves tooltip/context-menu behavior)
**Replacement:** `C_OgeChxToolTipBase` + `styleRole="cam-chx-settings"`
**Stylesheet-only:** ✅ confirmed (empty ctor body, no overrides, no members, only public method is the ctor)

## Counts

| File type | Files | Refs |
|---|---|---|
| `.ui` | 0 | none |
| `.qss` | 4 | 9 selector lines (Color.qss + Font.qss in both apps) |
| `.cpp` / `.hpp` (excluding the class itself) | 0 | none |
| Build / lint | 2 | 4 lines (both CMakeLists.txt × .cpp + .hpp) |

No substring collisions: `grep 'C_CamOgeChxSettings[A-Za-z]'` returns nothing. No `C_UtiStyleSheets.cpp` dynamic refs.

## .ui files

(none — this class is only used via QSS selectors, no customwidget instances)

## .qss files (4 files, 9 lines)

### CAN Monitor app
- `opensyde_can_monitor/src/can_monitor/styles/Font.qss`
  - L86 — bare class selector (in a group)
- `opensyde_can_monitor/src/can_monitor/styles/Color.qss`
  - L70 — `:enabled` selector
  - L314 — `::indicator:!checked:enabled`
  - L516 — `::indicator:checked:enabled`
  - L524 — `::indicator:checked:enabled` (duplicate/override)
  - L689 — `:disabled` selector
  - L701 — `::indicator:!checked:disabled`
  - L708 — `::indicator:checked:disabled`
  - L713 — `::indicator:checked:disabled` (duplicate/override)

### SYDEflash app
- `opensyde_syde_flash/src/syde_flash/styles/Font.qss`
  - L87 — bare class selector (in a group)
- `opensyde_syde_flash/src/syde_flash/styles/Color.qss`
  - L62 — `:enabled` selector
  - L298 — `::indicator:!checked:enabled`
  - L495 — `::indicator:checked:enabled`
  - L501 — `::indicator:checked:enabled` (duplicate/override)
  - L675 — `:disabled` selector
  - L688 — `::indicator:!checked:disabled`
  - L695 — `::indicator:checked:disabled`
  - L700 — `::indicator:checked:disabled` (duplicate/override)

Migration pattern:
```css
/* before */
stw--opensyde_gui_elements--C_CamOgeChxSettings:enabled
/* after */
stw--opensyde_gui_elements--C_OgeChxToolTipBase[styleRole="cam-chx-settings"]:enabled

/* before */
stw--opensyde_gui_elements--C_CamOgeChxSettings::indicator:checked:enabled
/* after */
stw--opensyde_gui_elements--C_OgeChxToolTipBase[styleRole="cam-chx-settings"]::indicator:checked:enabled
```

## .cpp / .hpp consumers

(none — only the class's own `C_CamOgeChxSettings.{hpp,cpp}` files mention the symbol)

## Build / lint

- `opensyde_can_monitor/pjt/CMakeLists.txt` L225 (.cpp), L433 (.hpp)
- `opensyde_syde_flash/pjt/CMakeLists.txt` L125 (.cpp), L266 (.hpp)
