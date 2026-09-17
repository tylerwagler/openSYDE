# Phase 1 Progress — Widget

## TODO
(none)

## Skipped (has logic — do NOT migrate)

All 20 widget classes (excluding the `C_OgeWiUtil` namespace utility) have at least one of: ctor body content, overrides, member variables, or .ui setup. None qualify as stylesheet-only by the playbook bar.

### Classes with paintEvent / event overrides + members
- `C_OgeWiBopperle` — `paintEvent` override, members initialized in ctor.
- `C_OgeWiBorder` — `paintEvent` override.
- `C_OgeWiDashboardPieChart` — 2 overrides + members.
- `C_OgeWiDashboardSpinBoxGroup` — 2 overrides + members.
- `C_OgeWiEditBackground` — paint override.
- `C_OgeWiHover` — 5 overrides + members + `.ui`.
- `C_OgeWiOnlyBackground` — `paintEvent` override + ctor calls `SetBackgroundColor(0)`.
- `C_OgeWiProgressBar` — `paintEvent` override + members.
- `C_OgeWiProgressBopperle` — `paintEvent` override + ctor sets brushes.
- `C_OgeWiSpinBoxGroup` — 8 overrides + .ui (consolidated base from earlier migration; this is the destination, not a candidate).
- `C_OgeWiWithToolTip` — `event` override + multiple inheritance with `C_OgeToolTipBase`.

### Widgets with `.ui` setup and substantial logic
- `C_OgeWiCustomMessage` — 2 overrides + .ui.
- `C_OgeWiDashboardTab` — 3 overrides + .ui + multiple member actions.
- `C_OgeWiError` — `.ui` setup + `mpc_ParentDialog` member.
- `C_OgeWiFixPosition` — `.ui` setup + ctor draws an icon.
- `C_OgeWiFontConfig` — `.ui` setup + ctor sets a font.
- `C_OgeWiNavigationTab` — 3 overrides + .ui + active flag.
- `C_OgeWiPieChart` — 2 overrides + .ui setup.
- `C_OgeWiProgressLog` — `.ui` setup with calls in ctor.
- `C_OgeWiSpinBoxGroupProperties` — ctor body calls `m_Resize()` (logic in ctor body).

### Utility (not a widget class)
- `C_OgeWiUtil` — namespace of static helpers (`h_ApplyStylesheetProperty`, etc.). Not a candidate.

## Done
- Earlier migration: `C_OgeWiParamSpinBoxGroup`, `C_OgeWiTableSpinBoxGroup` consolidated into `C_OgeWiSpinBoxGroup` + `styleRole` (commits `08c401df` + `55cce8b3`).

## Notes
The plan.md status note ("remaining 'stylesheet-only' candidates all override paintEvent — skipped per migration bar") is consistent with this survey: every paint-overriding widget class needs to be a unique C++ type so its `paintEvent` can be invoked, regardless of whether the qss selector targets it by class name or by `styleRole`. Migration to `QWidget + styleRole` would lose the override.

No `.ui`, `.qss`, or build changes were made for this family in the close-out turn.
