# openSYDE TODO

Cross-cutting follow-ups that aren't tied to a specific consolidation phase.
For phase-specific work, see `docs/agent_plans/`.

## Dark Mode

openSYDE, CAN Monitor, and SYDEflash all render with their built-in light
palette regardless of the system theme. When the host desktop is in dark
mode the apps stay bright, which is jarring at night.

Implement dark-mode support so the apps respect the system theme (or expose
an in-app toggle). Notes:

- Some widget classes already have a `SetDarkTheme` hook (e.g. line edits) —
  start by auditing what's already wired up before adding a new mechanism.
- The Qt stylesheets in `libraries/opensyde_gui/src/styles/` define the
  current light palette; a dark variant + a runtime swap is the likely shape
  of the fix.
- Touches all three apps; should land behind a single toggle so we don't
  diverge per-app.

## Audit "low-value" UX features for removal

Done so far (commit `ebe6c6b0`): the Ctrl+Enter→accept handler from
`C_OgePopUpContentBase` and 28 subclass overrides got ripped out. The
handler was actively bad — it swallowed plain Enter, suppressing Qt's
standard "Enter activates the default button" and replacing it with an
undiscoverable shortcut. Stock Qt behaviour now works on all 60
migrated popups.

A quick `grep` of the rest of the codebase didn't surface another case
with the same pathology (taking away a default Qt behaviour and
replacing it with worse). The 8 other files that use the same
`q_CallOrg`/`q_CallOrig` pattern are all *adding* keyboard shortcuts to
table/tree views (Delete, `+`, arrows, Enter-to-edit) — feature
additions, not regressions.

Lower-priority code-simplification candidates from the audit:

- `C_TblViewToolTipBase::keyPressEvent` reimplements "Enter to begin
  editing" and "Enter to toggle checkbox" by hand. Both are expressible
  declaratively via `setEditTriggers(QAbstractItemView::EditKeyPressed)`
  and Qt's built-in checkable-item handling — would simplify the code
  without changing behaviour.
- The bespoke title-bar machinery (Phase 4 of the GUI consolidation
  plan) is the next big candidate for "does this earn its keep?"
  scrutiny.
