# Reference scan — C_CamOgePubAbout

**Class:** `stw::opensyde_gui_elements::C_CamOgePubAbout`
**Parent:** `C_OgePubToolTipBase` (preserves tooltip-on-hover behavior)
**Replacement:** `C_OgePubToolTipBase` + `styleRole="cam-pub-about"`
**Stylesheet-only:** ✅ confirmed (empty ctor body, no overrides, no members, only public method is the ctor)

## Counts

| File type | Files | Refs |
|---|---|---|
| `.ui` | 2 | 6 (3 lines per app × widget instance + customwidget block) |
| `.qss` | 6 | 18 selector lines (`Color.qss` × 7 + `Font.qss` × 1 + `PushButton.qss` × 1 in each app) |
| `.cpp` / `.hpp` (excluding the class itself) | 0 | none |
| Build / lint | 2 | 4 lines (both CMakeLists × .cpp + .hpp) |

## .ui files (2 files)

- `opensyde_can_monitor/src/can_monitor/C_CamTitleBarWidget.ui` — `pc_PushButtonAbout`; customwidget block.
- `opensyde_syde_flash/src/syde_flash/C_FlaTitleBarWidget.ui` — `pc_PushButtonAbout`; customwidget block.

Migration pattern (same in both apps):
```xml
<!-- before -->
<widget class="stw::opensyde_gui_elements::C_CamOgePubAbout" name="pc_PushButtonAbout">
<!-- after -->
<widget class="stw::opensyde_gui_elements::C_OgePubToolTipBase" name="pc_PushButtonAbout">
 <property name="styleRole" stdset="0"><string>cam-pub-about</string></property>
```

## .qss files (6 files, 18 lines)

- `Color.qss` — 7 selectors per app (background, border, hover, pressed, etc.)
- `Font.qss` — 1 line per app
- `PushButton.qss` — 1 selector per app (border, padding for the title-bar About button)

Migration pattern:
```css
/* before */
stw--opensyde_gui_elements--C_CamOgePubAbout
/* after */
stw--opensyde_gui_elements--C_OgePubToolTipBase[styleRole="cam-pub-about"]
```

## .cpp / .hpp consumers

(none — Qt's uic generates `pc_PushButtonAbout` as a typed pointer in the `Ui::C_CamTitleBarWidget` / `Ui::C_FlaTitleBarWidget` struct, but no C++ source references the class name directly)

## Build / lint

- `opensyde_can_monitor/pjt/CMakeLists.txt` — .cpp + .hpp entries removed
- `opensyde_syde_flash/pjt/CMakeLists.txt` — .cpp + .hpp entries removed
