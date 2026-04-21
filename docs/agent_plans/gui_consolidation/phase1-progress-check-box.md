# Phase 1 Progress — Check Box (COMPLETE ✓)

## Done (migrated in dependency order)
- [x] C_OgeChxPropertiesSmall (Task B: ce29a58e, Task C: 927bb462) — migrated to C_OgeChxToolTipBase + styleRole="chx-properties-small" (grandparent replacement; direct parent Properties was also stylesheet-only)
- [x] C_OgeChxDefaultCheckedDisabled (Task B: 3f7e76d3, Task C: bc483451) — migrated to C_OgeChxToolTipBase + styleRole="chx-default-checked-disabled" (used across all 3 apps)
- [x] C_OgeChxProperties (Task B: 2758bb47, Task C: cc51ea5f) — migrated to C_OgeChxToolTipBase + styleRole="chx-properties" (7 .ui files, 31 .qss selectors, 13 signal connects across all 3 apps — largest migration)
- [x] C_OgeChxSystemCommisioningEdit (Task B: b52bc37a, Task C: f8fe8e36) — migrated to C_OgeChxToolTipBase + styleRole="chx-system-commisioning-edit"
- [x] C_OgeChxTiny (Task B: ee67a9d7, Task C: 35b5e885) — migrated to C_OgeChxToolTipBase + styleRole="chx-tiny"
- [x] C_OgeChxTristate (Task B: 635cfc89, Task C: 68333ef6) — migrated to C_OgeChxTristateBase + styleRole="chx-tristate" (programmatic-only, 3 `new` sites refactored)
- [x] C_OgeChxTristateTransparentError (Task B: fb8edb1f, Task C: 8b41a68a) — migrated to QCheckBox + styleRole="chx-tristate-transparent-error" (inherits QCheckBox directly)
- [x] C_OgeChxTristateTransparentToggle (Task B: 3c0875bb, Task C: 948aa74c) — migrated to C_OgeChxTristateBase + styleRole="chx-tristate-transparent-toggle" (10 widget instances across 8 .ui files)

## Skipped (has logic — do NOT migrate)
- C_OgeChxDefaultSmall — has Q_SIGNALS, overloaded constructor, member vars (mu32_Index, mu32_SubIndex)
- C_OgeChxToolTipBase — base class with event() override
- C_OgeChxTristateBase — base class with resizeEvent() override

---

## Survey Summary
- **Total classes surveyed:** 11
- **Migrated (stylesheet-only):** 8 ✓
- **Skipped (has logic):** 3

## Notes
- Used `chx-` prefix for all styleRoles.
- Substring collision risk with Tristate: `C_OgeChxTristate` is a prefix of
  `C_OgeChxTristateBase` (kept), `C_OgeChxTristateTransparentError`, and
  `C_OgeChxTristateTransparentToggle`. Used exact-match patterns (ending
  with space, `,`, `::`, or `.hpp`) for all replacements to avoid
  miscorrections. Post-migration collision check
  (`\[styleRole="[^"]*"\][A-Za-z]`) came back empty for all classes.
