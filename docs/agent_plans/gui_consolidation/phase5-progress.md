# Phase 5 Progress — CAN Monitor GUI Elements

Audit of `libraries/opensyde_gui/src/can_monitor/can_monitor_gui_elements/` —
the CAN Monitor flavor of the main `opensyde_gui_elements/` directory.

Total: **25 classes across 11 widget-type subdirectories**. Plan estimate:
~10–15 eliminations.

## TODO (10 candidates)

Each one has empty ctor body, no overrides, no member variables, no public
methods beyond the ctor — passes the playbook bar.

- [ ] `check_box/C_CamOgeChxSettings` (parent `C_OgeChxToolTipBase`) → replace with `C_OgeChxToolTipBase` + `styleRole="cam-chx-settings"`
- [ ] `group_box/C_CamOgeGbxMessageGeneratorBackground` (parent `QGroupBox`) → `QGroupBox` + `styleRole="cam-gbx-message-generator-background"`
- [ ] `group_box/C_CamOgeGbxMessageSignalsBorder` (parent `QGroupBox`) → `QGroupBox` + `styleRole="cam-gbx-message-signals-border"`
- [x] `label/C_CamOgeLabStatusBar` → `C_OgeLabToolTipBase` + `styleRole="cam-lab-status-bar"` (B: `150c1586`, C: this commit)
- [ ] `line_edit/C_CamOgeLeIdPopUp` (parent `stw::opensyde_gui::C_TblEditLineEditBase`) → `C_TblEditLineEditBase` + `styleRole="cam-le-id-popup"`
- [ ] `line_edit/C_CamOgeLeTableEdit` (parent `stw::opensyde_gui::C_TblEditLineEditBase`) → `C_TblEditLineEditBase` + `styleRole="cam-le-table-edit"`
- [ ] `menu/C_CamOgeMuRecentProjects` (parent `QMenu`) → `QMenu` + `styleRole="cam-mu-recent-projects"`
- [ ] `push_button/C_CamOgePubAbout` (parent `C_OgePubToolTipBase`) → `C_OgePubToolTipBase` + `styleRole="cam-pub-about"`
- [ ] `push_button/C_CamOgePubSettingsAdd` (parent `C_OgePubToolTipBase`) → `C_OgePubToolTipBase` + `styleRole="cam-pub-settings-add"`
- [ ] `spin_box/C_CamOgeSpxProperties` (parent `C_OgeSpxToolTipBase`) → `C_OgeSpxToolTipBase` + `styleRole="cam-spx-properties"`

The `cam-` prefix on `styleRole` values namespaces them away from any
existing Phase 1 styleRoles. Verify against existing `.qss` selectors during
each class's Task A reference scan before committing the value.

## Skipped (has logic — do NOT migrate)

15 classes are functional and stay as-is.

### combo_box (4 — all override + install delegate in ctor)
- `C_CamOgeCbxDark` — `keyPressEvent` override + ctor installs `QStyledItemDelegate` and sets context-menu policy on scrollbars.
- `C_CamOgeCbxTable` — same pattern, installs `C_OgeCbxIconDelegate`.
- `C_CamOgeCbxTableSmall` — same pattern.
- `C_CamOgeCbxWhite` — same pattern.

### line_edit (1)
- `C_CamOgeLeFilePath` — `m_ResolveVariables` override.

### push_button (4)
- `C_CamOgePubDarkBrowse` — ctor sets an SVG icon.
- `C_CamOgePubPathVariablesBase` — `mousePressEvent` override + ctor installs a menu and icon.
- `C_CamOgePubPathVariables` — ctor calls `m_AddHeading` / `m_AddEntry` (real domain logic, not styling).
- `C_CamOgePubProjOp` — `paintEvent` override + member SVG icons + `mq_IconOnly` flag.

### splitter (1)
- `C_CamOgeSpi` — 3 overrides (paintEvent, eventFilter, splitterMoved handling) + ctor sets handle width.

### tool_button (1)
- `C_CamOgeTobRecentProjects` — `paintEvent` override + `mc_SvgIcon` member.

### widget (4)
- `C_CamOgeWiSectionHeader` — `paintEvent` override + `.ui` setup + `me_ButtonType` member + multi-button construction.
- `C_CamOgeWiSettingsBase` — `paintEvent` override.
- `C_CamOgeWiSettingSubSection` — 2 overrides + `.ui` setup.
- `C_CamOgeWiSpinBoxGroupTable` — ctor calls `m_Resize(28)` (ctor body has logic per playbook bar).

## Done
(none yet — surveys only)

## Notes
- The ratio (10/25 ≈ 40%) is consistent with the plan's 10–15 estimate.
- All 10 candidates inherit from a Phase 1 base class or a Qt builtin —
  no novel widget hierarchies. Migration follows the same Task A → B → C
  workflow as the Phase 1 closeouts.
- The .ui-side migration for these classes will typically be smaller than
  Phase 1's: each `C_CamOge*` class is referenced from a smaller set of
  `.ui` files (CAN Monitor-only). Expect single-digit reference counts per
  class for most of them.
- `C_CamOgePubDarkBrowse` is borderline — the only ctor body content is a
  `setIcon` call. Could be migrated by hoisting the icon-set to the call
  sites, but per Phase 1 precedent for ctor-side-effect classes
  (`C_OgeCbxFontProperties`, `C_OgeCbxIconOnly`, `C_OgeLeTableHalc`, etc.),
  these are skipped to keep the migration purely stylesheet-only.
