# Archived plans

Finished efforts. Kept because the reasoning is worth more than the checklists —
particularly the **won't-fix** decisions, which are the expensive thing to
rediscover.

Open work lives in `../ROADMAP.md`. Durable findings live in `../FINDINGS.md`.

**These documents are frozen.** Statuses inside them were accurate when the work
finished and have not been maintained since. Two of them were found to be wrong
during the 2026-09-17 consolidation — see below. If an archived plan says something
is open, check the code, then record the answer in `../ROADMAP.md` rather than here.

| Plan | What it did | Note |
|------|------------|------|
| `codebase_audit/` | The 8-phase master plan: tests and CI, correctness bugs, `C_SclDynamicArray` and `C_SclString` removal, AES replacement, error-handling modernisation, concurrency, performance, build system. `analysis.md` is the underlying audit; `PHASE3_PLAN.md` the string-migration detail | Phases 0–6 and 8 complete. **7.2 and 7.3 remain open** and have moved to `../ROADMAP.md` |
| `core_sweep/` | Four-dimension audit of `opensyde_core` — modernisation, performance, dedup, smells. 19 items | 14 fixed. **The 5 open items have moved to `../ROADMAP.md`** |
| `gui_consolidation/` | Removing redundant GUI widget classes: ~148 stylesheet-only shells plus dashboard panels, popups, title bars. Six phases, ~30 tracking files | Complete. Phase 6 closed **won't-fix**: the three model-view-delegates inherit from different model bases and solve structurally different problems — they are not parallel implementations |
| `qsettings_migration/` | Replace `C_SclIniFile`-based user settings with idiomatic `QSettings` across the three GUI apps | **Stale.** Document says "not started"; all three apps were already migrated when this was checked on 2026-09-17. Worth reading only for the rationale on *not* cherry-picking the `dev` branch version, which grew the file by mechanically preserving every `C_SclIniFile` idiom |
| `stwtypes_removal/` | Retire the STW type aliases in favour of standard fixed-width types | **Stale.** Document says "scoping, not yet started". `stwtypes.hpp` (C++) has been removed; `stwtypes.h` (C) deliberately remains as the contract for generated controller code |

## Why two plans were wrong

Both described completed work as unstarted. Neither was misleading on purpose —
the work was done and the document was simply never closed out. It cost a few
minutes to detect (grep the code for the thing the plan says still exists) and
would have cost a great deal more to act on.

The lesson is cheap to apply: a plan that is not the working document for active
work should be archived the day it stops being one.
