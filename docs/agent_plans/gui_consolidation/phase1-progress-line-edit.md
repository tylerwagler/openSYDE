# Phase 1 Progress — Line Edit

## TODO
(none)

## Skipped (has logic — do NOT migrate)

### Functional bases / classes with substantial public API
- `C_OgeLeContextMenuBase` — `mpc_ContextMenu` member; private `m_InitContextMenu` / `m_SetupContextMenu` / `m_OnCustomContextMenuRequested`. Functional base used by other line edits.
- `C_OgeLeToolTipBase` — multiple inheritance with `C_OgeToolTipBase`; `event` override. Functional base.
- `C_OgeLeFilePathBase` — 7 overrides (focus/key/show/resize/drag/drop), `SigPathDropped` signal, 5 member variables, large public API (`SetPath`, `SetDragAndDropActiveForFolder`/`File`, `GetPath`, `InsertVariable`, `UpdateText`, virtual `m_ResolveVariables`).
- `C_OgeLeFilePath` — `m_ResolveVariables` override, `mc_DbProjectPath` member, `SetDbProjectPath` API.
- `C_OgeLeIpAddress` — three signals (`SignalFocusIn` / `SignalFocusOut` / `SignalTabKey`), `focusInEvent` / `focusOutEvent` overrides.
- `C_OgeLeListHeader` — 3 members, `SigFocus` signal, `focusInEvent`+`mousePressEvent` overrides, public API (`SetCounter`, `SetName`, `GetName`), multiple private slots.
- `C_OgeLePlaceholderVar` — `focusOutEvent` override, 2 members, public API (`InsertVariable`, `SetDbProjectPath`).
- `C_OgeLeNavigation` — `keyPressEvent` override + `SigEscape` signal (escape key handling, not styling).
- `C_OgeLeComboBox` — `mousePressEvent` override that triggers parent combo-box popup logic.
- `C_OgeLeProperties` — public `SetBackgroundColor` API (despite the file comment claiming "no functionality").
- `C_OgeLeSearch` — public `SetDarkTheme` / `SetLightTheme` methods that runtime-set inline stylesheets.

### Classes with ctor side-effects (would be candidates if ctor body were empty)
- `C_OgeLeTableHalc` — ctor pins `setMinimumHeight(24)` / `setMaximumHeight(24)` (similar to the corresponding combo-box class).

## Done
(none — no migration candidates remaining)

## Notes
The plan.md status note mentioning "C_OgeLeProperties{Name}, C_OgeLeTable, C_OgeLeListHeader migrated" referred to classes that were either renamed/consolidated or deleted in earlier work; the current set of 12 classes in `line_edit/` all fail the playbook bar (empty ctor body + no overrides/members/public API beyond the ctor). No `.ui`, `.qss`, or build changes were made for this family in the close-out turn.

`C_OgeLeTableHalc` could in principle be migrated by hoisting the min/max-height calls to the call sites, but that's a behavioural refactor rather than a stylesheet-only migration and is out of scope for Phase 1 (same shape as the combo-box ctor-side-effect candidates).
