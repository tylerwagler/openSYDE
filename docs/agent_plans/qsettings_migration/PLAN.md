# QSettings Migration Plan

## Goal

Replace the custom `C_SclIniFile`-based user-settings persistence in all three
GUI apps with idiomatic `QSettings` usage (`beginGroup`, `beginWriteArray`),
shrinking the user_settings code by ~60% in the process and removing the
`C_SclIniFile` dependency from GUI code entirely.

## Why not cherry-pick from dev

The dev branch already migrated main GUI and CAN Monitor to `QSettings`, but as
a mechanical 1:1 substitution that preserved every `C_SclIniFile` idiom (manual
key concatenation, parameter-passed section names, hand-rolled counter loops).
The result actually **grew** the file (`C_UsFiler.cpp`: 2,531 → 2,981 LOC)
because none of QSettings' structural features were used.

This plan does the migration from scratch using QSettings idioms properly.
Format-compatible and aggressive prototypes already validated:

| Function | Original (dev) | Format-compat | Aggressive |
|---|---:|---:|---:|
| `mh_SaveNode` | 186 LOC | 87 LOC (-53%) | 57 LOC (-69%) |

Aggressive approach chosen — full idiomatic Qt usage, accepting an on-disk
INI format change with a one-shot legacy migrator.

## Scope

Three apps, each with its own copy of `C_UsFiler` + `C_UsHandler`:

| App | Path | Current LOC (Filer) |
|---|---|---:|
| Main GUI | `libraries/opensyde_gui/src/user_settings/` | 2,531 |
| CAN Monitor | `opensyde_can_monitor/src/can_monitor/user_settings/` | 485 |
| SYDEflash | `opensyde_syde_flash/src/syde_flash/user_settings/` | 355 |

`C_UsHandler` (~3,026 / ~smaller / smaller) only changes if its public API
needs adjustment; the persistence layer is `C_UsFiler`.

## Strategy

### Idiomatic patterns to apply throughout

1. **`beginGroup(section)` / `beginGroup(idBase)`** instead of manual key
   concatenation. Eliminates the giant `c_X = ...arg(orc_NodeIdBase)` blocks
   at the top of every `mh_SaveX` function.
2. **`beginWriteArray(name, size)`** for any container persisted as
   `<base>0Field=...`, `<base>0=...`, `<base>Counter=N`. Qt manages the size
   key automatically.
3. **Generic helper templates** (`mh_SaveArray`, `mh_LoadArray`) for the 15+
   sites in each file that share the counter+loop+separate-Counter pattern.
4. **Helper signatures simplified**: `mh_SaveColumns`, `mh_SaveDatapool`,
   `mh_SaveBus`, etc. drop their `orc_SectionName` and `orc_IdBase` parameters
   since the active QSettings group provides the prefix.

### Format change & migration

The new layout is **not byte-compatible** with the legacy `C_SclIniFile` output.
Example:

```ini
# Before
[ProjectFoo]
Node1Name=MyNode
Node1CANopenManagerExpanded#0InterfaceNumber=2
Node1CANopenManagerExpanded#0=true
Node1CANopenManagerExpandedCounter=3

# After
[ProjectFoo/Node1]
Name=MyNode
CANopenManagerExpanded\size=3
CANopenManagerExpanded\1\InterfaceNumber=2
CANopenManagerExpanded\1\Expanded=true
```

**One-shot legacy migrator** (~150 LOC, deleted after one release):

- On `C_UsFiler::Load()`, detect legacy format by checking for sentinel keys
  at the section level (e.g., `<NodeIdBase>Name` instead of `<NodeIdBase>/Name`).
- If detected, read the legacy file with `C_SclIniFile`, write a new-format
  file with `QSettings`, archive the old file as `<name>.legacy.ini`.
- Subsequent loads see the new format and skip the migrator.

## Phased execution

Each phase ends at a green build + manual round-trip verification (save,
restart app, confirm settings preserved).

### Phase 0 — Shared helpers & test fixtures

- [ ] Add `mh_SaveArray<Container, ItemWriter>(QSettings&, name, items, writer)`
      and `mh_LoadArray<Container, ItemReader>(QSettings&, name, out, reader)`
      template helpers. Place in a new private header used by all three apps,
      OR duplicate per-app for now (decide based on how `C_UsHandler` types
      diverge).
- [ ] Add a small INI round-trip fixture (write known C_UsHandler state, reload,
      assert equality of all relevant fields). Run it in a unit test if there
      is a test harness, otherwise as a debug-build assertion.

**Build gate:** main GUI compiles, no behavior change yet.

### Phase 1 — Main GUI save path

- [ ] Rewrite `C_UsFiler::mh_SaveNode` (prototype already validated, 57 LOC).
- [ ] Rewrite `C_UsFiler::mh_SaveBus`.
- [ ] Rewrite `C_UsFiler::mh_SaveDatapool`.
- [ ] Rewrite `C_UsFiler::mh_SaveList`.
- [ ] Rewrite `C_UsFiler::mh_SaveView` (and its many sub-savers).
- [ ] Rewrite `C_UsFiler::mh_SaveDashboard`.
- [ ] Rewrite `C_UsFiler::mh_SaveColumns` and other small leaf helpers.
- [ ] Rewrite `C_UsFiler::Save` entry point.

**Build gate:** main GUI compiles. Save path produces new-format INI when
launched. Old settings still load via legacy reader (next phase).

### Phase 2 — Main GUI load path

- [ ] Rewrite all `mh_LoadX` mirrors of the savers.
- [ ] Rewrite `C_UsFiler::Load` entry point. Reads new format only; no legacy
      handling yet.

**Build gate:** main GUI builds, save→load round-trips with the new format.
**Note:** at this point existing user settings files won't load. That's fixed
in Phase 3 before this lands on a release branch.

### Phase 3 — Legacy format migrator

- [ ] Implement `C_UsFiler::mh_MigrateLegacy(QString path)`:
  - Detect legacy format (sentinel: `Name` key at section level rather than as
    a sub-group).
  - Parse with `C_SclIniFile` (same code as pre-migration `mh_LoadX`).
  - Write back with new `QSettings` saver.
  - Rename original to `<file>.legacy.ini`.
- [ ] Wire into `C_UsFiler::Load` as a pre-step.
- [ ] Test on real user-settings files from existing installs.

**Build gate:** Loading any pre-migration user-settings file produces an
identical in-memory `C_UsHandler` state and writes a new-format file. Legacy
file preserved on disk.

### Phase 4 — CAN Monitor migration

- [ ] Repeat the pattern on `opensyde_can_monitor/src/can_monitor/user_settings/`.
      Smaller surface (~485 LOC), reuse the helpers from Phase 0.
- [ ] CAN Monitor legacy migrator (mostly identical structure, different keys).

**Build gate:** CAN Monitor builds, round-trip clean, legacy migrator works.

### Phase 5 — SYDEflash migration

- [ ] Same on `opensyde_syde_flash/src/syde_flash/user_settings/`. ~355 LOC.
- [ ] SYDEflash legacy migrator.

**Build gate:** SYDEflash builds, round-trip clean, legacy migrator works.

### Phase 6 — Cleanup

- [ ] Remove `C_SclIniFile` includes from all three `C_UsFiler.cpp` files.
- [ ] Verify no GUI code outside `user_settings/` still uses `C_SclIniFile`
      (currently 98 hits across all GUI code — most are inside `C_UsFiler`,
      but audit the rest).
- [ ] Schedule legacy-migrator removal for the release after this one
      (mark with `// TODO: remove after R<n+1>`).

## Risks & mitigations

| Risk | Mitigation |
|---|---|
| Round-trip data loss on boundary cases (empty containers, special characters in keys) | Phase 0 round-trip fixture covers these explicitly. |
| `QSettings` file path differs from current location | Keep the explicit `QString` path passed to the `QSettings` constructor; do not use the org/app-name default. |
| Concurrency: in-flight writes during shutdown | Call `QSettings::sync()` at end of every `Save`. |
| Legacy migrator misclassifies a partially-migrated file | Detect by presence of sentinel `Name` at section level only when no `Name` exists as a sub-group. Idempotent: running twice on a new-format file is a no-op. |
| Helper-template error messages obscure when types mismatch | Constrain with `static_assert` on the iterator/value types; not worth concepts unless we move to C++20 first. |
| Three-way comparison synthesis (C++20 readiness) | N/A for this migration. Out of scope. |

## LOC estimates

| Component | Before (dev/linux) | After (estimate) | Δ |
|---|---:|---:|---:|
| Main GUI `C_UsFiler.cpp` | 2,531 | ~900–1,100 | −1,400 to −1,600 |
| CAN Monitor `C_UsFiler.cpp` | 485 | ~250 | −235 |
| SYDEflash `C_UsFiler.cpp` | 355 | ~200 | −155 |
| Legacy migrator (one-shot) | 0 | ~150 (per app) | +450 (temporary) |
| **Net (after legacy removed)** | **3,371** | **~1,400** | **−1,970 LOC** |

## Out of scope

The following are *not* part of this migration even though they touched
during scoping:

- **`C_GtGetText` → Qt Linguist**: 5,589 call sites; separate, much larger
  effort.
- **`C_SclString` → `QString` in GUI**: gradual policy, not a project.
- **`C_OgeWiCustomMessage` → `QMessageBox`**: independent cleanup.
- **C++17 → C++20 jump**: independent (codebase is already clean for it).
- **`C_UsHandler` API changes**: only touched if a clean `QSettings`-based
  `C_UsFiler` requires it; not a goal in itself.

## Verification checklist (per phase)

- [ ] Target app compiles (no new warnings introduced).
- [ ] Manual round-trip: change a setting via UI, exit, restart, confirm
      preserved.
- [ ] If round-trip fixture exists: it passes for the migrated functions.
- [ ] If a legacy user-settings file from a prior install is available, it
      loads cleanly post-Phase 3.
- [ ] Inspect the on-disk INI to confirm new key layout matches design.

## References

- Prototype (aggressive, `beginWriteArray`): `prototypes/proto_aggressive_mh_SaveNode.cpp` ← canonical pattern
- Prototype (format-compatible): `prototypes/proto_mh_SaveNode.cpp` ← reference only, not the chosen approach
- Dev-branch source for diffing: `prototypes/dev_mh_SaveNode.cpp`, `prototypes/dev_mh_SaveColumns.cpp`
- Dev branch state for reference (do **not** cherry-pick): `origin/dev`
  - Main GUI: `opensyde_tool/src/user_settings/C_UsFiler.cpp` (migrated 1:1)
  - CAN Monitor: `opensyde_tool/src/can_monitor/user_settings/C_UsFiler.cpp` (migrated 1:1)
  - SYDEflash: `opensyde_tool/src/syde_flash/user_settings/C_UsFiler.cpp` (not migrated)
