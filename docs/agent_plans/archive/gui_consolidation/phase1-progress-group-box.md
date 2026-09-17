# Phase 1 Progress — Group Boxes

Survey date: 2026-04-27
Directory: `libraries/opensyde_gui/src/opensyde_gui_elements/group_box/`

17 classes total: 9 stylesheet-only, 8 with logic.

## TODO (stylesheet-only — safe to migrate)

(none — group_box family stylesheet-only classes all migrated)

## Done

- [x] **C_OgeGbxTransparent** -> `QGroupBox` + `styleRole="transparent"` (49 .ui files; 5 programmatic uses) — `5575cf2c` (migrate), `c4b8da1e` (delete)
- [x] **C_OgeGbx** (inherits `QGroupBox`) -> `QGroupBox` + `styleRole="default"` (71 .ui files, 91 widget instances) — `00407b83` (migrate), `63bbfd20` (delete), `f03a22d6` (cleanup empty customwidget blocks left by remote agent)
- [x] **C_OgeGbxRead** -> dropped unused QSS rules; class deleted (no .ui or C++ refs anywhere) — `5037f7ac` (migrate), `f916617c` (delete)
- [x] **C_OgeGbxListHeader** -> `QGroupBox[styleRole="list-header"]` — `5037f7ac` (migrate), `f916617c` (delete)
- [x] **C_OgeGbxToolTip** -> `QGroupBox[styleRole="tooltip"]` — `5037f7ac` (migrate), `f916617c` (delete)
- [x] **C_OgeGbxGrayed** -> `QGroupBox[styleRole="grayed"]` (10 .ui files) — `dfd4adfa` (migrate), `ac380e57` (delete)
- [x] **C_OgeGbxNavigationHeadingBackground** -> `QGroupBox[styleRole="navigation-heading-background"]` — `4434fedb` (migrate), `491990e8` (delete)
- [x] **C_OgeGbxNavigationSection2** -> `QGroupBox[styleRole="navigation-section-2"]` — `4434fedb` (migrate), `491990e8` (delete)
- [x] **C_OgeGbxTransparentToolBarSearch** -> `QGroupBox[styleRole="transparent-toolbar-search"]` — `4434fedb` (migrate), `491990e8` (delete)

## Skipped (has logic — do NOT migrate)

- **C_OgeGbxGrayedEdit** — constructor calls `h_ApplyStylesheetProperty(this, "Edit", false)`
- **C_OgeGbxHover** — constructor calls `h_ApplyStylesheetProperty(this, "DarkMode", false)`
- **C_OgeGbxMouseMove** — declares Q_SIGNALS (SigMousePress/Release/Move); overrides `mousePressEvent`/`mouseReleaseEvent`/`mouseMoveEvent`
- **C_OgeGbxNavigationSection** — has public `SetSpecialBackground` method; constructor sets stylesheet property
- **C_OgeGbxNavigationSectionSelected** — has public `SetActive` method; constructor calls `SetActive(false)`
- **C_OgeGbxNavigationTab** — has public `SetIncludeLeftBorder` method; constructor sets stylesheet property
- **C_OgeGbxToolTipBase** — multiple inheritance from `C_OgeToolTipBase` mixin; overrides `event()` for tooltip dispatch
- **C_OgeGbxTransparentDashboard** — declares `SigResized` signal; overrides `resizeEvent`

## Notes

- **C_OgeGbxTransparent migration target — superseded:** Earlier I assumed the replacement had to be `C_OgeGbxToolTipBase` to preserve tooltip behavior. After auditing, no caller invokes `SetToolTipInformation` on any of the 93 widget instances — only push-button tooltips are wired. Combined with the fact that `C_OgeGbxToolTipBase` lacks `Q_OBJECT` (so `stw--opensyde_gui_elements--C_OgeGbxToolTipBase[styleRole=...]` selectors don't match at runtime), the correct target is plain `QGroupBox` + `styleRole="transparent"`. The `QGroupBox[styleRole=...]` selector matches reliably regardless of leaf-class metaobject.

- **Most candidates inherit QGroupBox directly**, so the .ui replacement is straightforward `<widget class="QGroupBox" ...>` with the styleRole property — no customwidget block needed for QGroupBox.

- **Eight classes with real logic** is high for this family — many group boxes are functional containers with mouse/resize/tooltip behavior, not pure styling shells. After this round, the group_box dir won't be eliminated, just shrunk to 8 functional classes.
