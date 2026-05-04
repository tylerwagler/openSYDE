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

Phase 3 consolidated the Ctrl+Enter→accept handler into
`C_OgePopUpContentBase`, but it's worth asking whether the feature pays
its keep at all. A standard Qt pop-up already accepts on the OK button;
forcing users to learn Ctrl+Enter (vs plain Enter, which the existing
code suppresses) is a quirky shortcut that probably no one uses.

When time permits, audit similar features across the apps for
"do-we-need-this?" candidates and rip out the ones that aren't earning
their complexity. Likely candidates beyond Ctrl+Enter: the bespoke title
bar machinery (Phase 4 territory), custom tooltip plumbing, anywhere
else there's a 20+-line custom override for a behavior Qt already does.
