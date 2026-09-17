# Reference scan — C_CamOgeLeIdPopUp

**Class:** `stw::opensyde_gui_elements::C_CamOgeLeIdPopUp`
**Parent:** `C_TblEditLineEditBase` (provides `SetMinFromVariant` API used by callers)
**Replacement:** `C_TblEditLineEditBase` + `styleRole="cam-le-id-popup"`
**Stylesheet-only:** ✅ confirmed (empty ctor body, no overrides, no members, only public method is the ctor)

## Counts

| File type | Files | Refs |
|---|---|---|
| `.ui` | 2 | 7 (4 in `C_CamMosFilterPopup.ui` covering 2 widget instances + 1 customwidget block; 3 in `C_CamGenKeySelect.ui` covering 1 widget + customwidget) |
| `.qss` | 6 | 14 selector lines (`Color.qss` × 5 + `Font.qss` × 1 + `LineEdit.qss` × 1 in each app) |
| `.cpp` / `.hpp` (excluding the class itself) | 1 | 6 lines in `C_CamMosFilterPopup.cpp` (`connect`/`disconnect` calls using `&C_CamOgeLeIdPopUp::editingFinished`) |
| Build / lint | 2 | 4 lines (both CMakeLists × .cpp + .hpp) |

## .ui files (2 files)

- `libraries/opensyde_gui/src/can_monitor/can_monitor_settings/C_CamMosFilterPopup.ui` — `pc_LeCanIdStart`, `pc_LeCanIdEnd` (used inside the filter dialog for CAN ID range entry); customwidget block.
- `opensyde_can_monitor/src/can_monitor/message_generator/C_CamGenKeySelect.ui` — `pc_LineEditKey`; customwidget block.

Migration replaced both `<widget class="...C_CamOgeLeIdPopUp">` instances with `<widget class="stw::opensyde_gui::C_TblEditLineEditBase">` + `<property name="styleRole">cam-le-id-popup</property>`. Note `C_TblEditLineEditBase` lives in the `stw::opensyde_gui` namespace, not `stw::opensyde_gui_elements`.

## .qss files (6 files, 14 lines)

Both apps (CAN Monitor + SYDEflash) have parallel selectors:
- `Color.qss` — 5 selector lines per app (background-color, border-color in multiple states)
- `Font.qss` — 1 line per app (font-family bundle)
- `LineEdit.qss` — 1 selector per app (border + padding)

Migration pattern:
```css
/* before */
stw--opensyde_gui_elements--C_CamOgeLeIdPopUp,
/* after */
stw--opensyde_gui--C_TblEditLineEditBase[styleRole="cam-le-id-popup"],
```

## .cpp / .hpp consumers

- `libraries/opensyde_gui/src/can_monitor/can_monitor_settings/C_CamMosFilterPopup.cpp` — 6 `connect`/`disconnect` calls using `&C_CamOgeLeIdPopUp::editingFinished` (lines 117, 119, 847, 852, 899, 904 in the pre-migration source). Migrated to `&C_TblEditLineEditBase::editingFinished` (the signal is inherited from `QLineEdit` either way; the change keeps the pointer-to-member type compatible with the new `pc_LeCanId{Start,End}` widget pointer types).

## Build / lint

- `opensyde_can_monitor/pjt/CMakeLists.txt` — .cpp + .hpp entries removed
- `opensyde_syde_flash/pjt/CMakeLists.txt` — .cpp + .hpp entries removed
