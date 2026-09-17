# Phase 1 Progress — Tab Widget

## TODO
(none)

## Skipped (has logic — do NOT migrate)
- `C_OgeTawAlternative` — ctor calls `setUsesScrollButtons(false)`, `tabBar()->setExpanding(false)`, and `tabBar()->setFont(...)` (font set in code because stylesheet-time font set breaks tab-width calc per inline comment).
- `C_OgeTawPageNavi` — ctor calls `tabBar()->setExpanding(false)`. Inherits `C_OgeTawToolTipBase`.
- `C_OgeTawSelector` — ctor calls `tabBar()->setExpanding(false)`. Inherits `C_OgeTawToolTipBase`.
- `C_OgeTawToolBox` — ctor calls `setUsesScrollButtons(false)` and installs a custom `C_OgeTabBar` via `setTabBar(...)`.
- `C_OgeTawToolTipBase` — functional base: tooltip member state, `mouseMoveEvent` and `event` overrides, `SetToolTipInformation` API.

## Done
(none — no migration candidates)

## Notes
All five tab widget classes fail the playbook bar (ctor body must be `{}`, no overrides/members) and stay as-is. No `.ui`, `.qss`, or build changes were made for this family.
