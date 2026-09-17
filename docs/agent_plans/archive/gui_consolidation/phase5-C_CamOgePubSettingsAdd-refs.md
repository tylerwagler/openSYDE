# Reference scan — C_CamOgePubSettingsAdd

**Class:** `stw::opensyde_gui_elements::C_CamOgePubSettingsAdd`
**Parent:** `C_OgePubToolTipBase`
**Replacement:** `C_OgePubToolTipBase` + `styleRole="cam-pub-settings-add"`
**Stylesheet-only:** ✅ confirmed (empty ctor body, no overrides, no members, only public method is the ctor)

## Counts

| File type | Files | Refs |
|---|---|---|
| `.ui` | 5 | 20 (4 widget instances + customwidget in `C_CamMosWidget.ui`; 1 widget + customwidget each in `C_CamMos{Database,Filter}Widget.ui` and `C_CamMosDllWidget.ui`; 3 widget instances + customwidget in `C_FlaSetWidget.ui`) |
| `.qss` | 6 | 18 selector lines (`Color.qss` × 6 + `Font.qss` × 1 + `PushButton.qss` × 2 in each app) |
| `.cpp` / `.hpp` (excluding the class itself) | 6 | 25 lines — heaviest C++ usage of all Phase 5 classes |
| Build / lint | 2 | 4 lines (both CMakeLists × .cpp + .hpp) |

This is the most-referenced of the Phase 5 classes — used as a typed pointer in `C_CamOgeWiSettingsBase` member containers, in `connect` calls in two widget consumers, and in the SYDEflash settings widget.

## .ui files (5 files)

- `libraries/opensyde_gui/src/can_monitor/can_monitor_settings/C_CamMosWidget.ui` — 4 instances: `pc_PbDatabase`, `pc_PbFilter`, `pc_PbLogging`, `pc_PbDllConfig` (the four section toggle buttons in the settings sidebar).
- `libraries/opensyde_gui/src/can_monitor/can_monitor_settings/C_CamMosDatabaseWidget.ui` — `pc_BtnAdd`.
- `libraries/opensyde_gui/src/can_monitor/can_monitor_settings/C_CamMosFilterWidget.ui` — `pc_BtnAdd`.
- `libraries/opensyde_gui/src/can_monitor/can_monitor_settings/C_CamMosDllWidget.ui` — `pc_PushButtonConfigure`.
- `opensyde_syde_flash/src/syde_flash/settings/C_FlaSetWidget.ui` — 3 instances: `pc_PbDll`, `pc_PbAdvSett`, `pc_PbProgress`.

Each file's customwidget block was rewritten from `<class>...C_CamOgePubSettingsAdd</class>` / `<header>C_CamOgePubSettingsAdd.hpp</header>` to `C_OgePubToolTipBase` / `C_OgePubToolTipBase.hpp`.

## .qss files (6 files, 18 lines)

- `Color.qss` — 6 selectors per app (background, border in multiple states, hover, pressed, checked, disabled)
- `Font.qss` — 1 line per app
- `PushButton.qss` — 2 selectors per app (one for the comma-grouped border-style:none rule, one for padding/text-align)

Migration pattern:
```css
/* before */
stw--opensyde_gui_elements--C_CamOgePubSettingsAdd,
stw--opensyde_gui_elements--C_OgePubToolTipBase[styleRole="configure"]
/* after */
stw--opensyde_gui_elements--C_OgePubToolTipBase[styleRole="cam-pub-settings-add"],
stw--opensyde_gui_elements--C_OgePubToolTipBase[styleRole="configure"]
```

## .cpp / .hpp consumers (6 files, 25 lines)

- `libraries/opensyde_gui/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingsBase.hpp` — 9 occurrences: `#include`, multiple method signatures (`mh_GetButton`, `m_InitSettingsSection`, `mh_InitSettingsButton`, `m_ShowPopup`, `m_GetPopupMovePoint`), and the `mc_Settings` `std::vector<QPair<...,C_CamOgePubSettingsAdd*>>` member type. All replaced with `C_OgePubToolTipBase`.
- `libraries/opensyde_gui/src/can_monitor/can_monitor_gui_elements/widget/C_CamOgeWiSettingsBase.cpp` — 12 occurrences: matching method bodies, `connect(opc_Button, &C_CamOgePubSettingsAdd::toggled, ...)`, iterator types. All replaced with `C_OgePubToolTipBase`.
- `libraries/opensyde_gui/src/can_monitor/can_monitor_settings/C_CamMosWidget.hpp` — 1 occurrence: `#include "C_CamOgePubSettingsAdd.hpp"` swapped for `#include "C_OgePubToolTipBase.hpp"`.
- `libraries/opensyde_gui/src/can_monitor/can_monitor_settings/C_CamMosDatabaseWidget.cpp:420` — `connect(this->mpc_Ui->pc_BtnAdd, &C_CamOgePubSettingsAdd::clicked, ...)` → `&C_OgePubToolTipBase::clicked`.
- `libraries/opensyde_gui/src/can_monitor/can_monitor_settings/C_CamMosFilterWidget.cpp:346` — same replacement.
- `opensyde_syde_flash/src/syde_flash/settings/C_FlaSetWidget.cpp:242` — typed iterator declaration; same replacement.

## Build / lint

- `opensyde_can_monitor/pjt/CMakeLists.txt` — .cpp + .hpp entries removed
- `opensyde_syde_flash/pjt/CMakeLists.txt` — .cpp + .hpp entries removed
