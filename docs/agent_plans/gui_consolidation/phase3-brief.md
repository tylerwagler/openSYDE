# Phase 3 Brief — Popup Dialog Boilerplate Consolidation

This brief is a self-contained handoff for resuming Phase 3 in a fresh session.
The scope was expanded beyond `plan.md`'s original 7 dialogs after a survey
showed the same boilerplate exists in **62 files repo-wide**.

## Decision: Go wide (52 files)

The original plan targeted ~7 dialogs in
`libraries/opensyde_gui/src/system_views/system_update/update_package/`. Survey
found the same `mrc_ParentDialog` + 22-line `keyPressEvent` Ctrl+Enter→accept
boilerplate duplicated in 62 files. The user opted to extract once, adopt
broadly. Scope: **52 files** (see exclusions below).

## What gets extracted

Every candidate has these three things in common:

1. A `C_OgePopUpDialog & mrc_ParentDialog;` private member (a reference, not a
   pointer; field name is exactly `mrc_ParentDialog`).
2. A `keyPressEvent` override whose body is byte-for-byte identical except for
   one call site — see "Variant" below.
3. The ctor stores the parent: `mrc_ParentDialog(orc_Parent)` in the
   initializer list, then later `this->mrc_ParentDialog.SetWidget(this);` in
   the body.

## Two variants of the keyPressEvent body

Both wrap the same modifier check. The only line that differs is what gets
called when Ctrl+Enter (no Alt, no Shift) is detected:

- **Variant A** — `this->mrc_ParentDialog.accept();` directly. Common in
  `update_package/`.
- **Variant B** — `this->m_OkClicked();`. Common in `system_definition/` and
  `com_import_export/`. The class-defined `m_OkClicked` typically does
  pre-flight validation/save before calling `mrc_ParentDialog.accept()`.

Both variants must work after migration without changing observable behavior.

## Base class design

Place at
`libraries/opensyde_gui/src/opensyde_gui_elements/C_OgePopUpContentBase.{hpp,cpp}`,
next to existing `C_OgePopUpDialog`.

```cpp
namespace stw::opensyde_gui_elements {

class C_OgePopUpContentBase :
   public QWidget
{
   Q_OBJECT

public:
   explicit C_OgePopUpContentBase(C_OgePopUpDialog & orc_Parent,
                                  QWidget * const opc_Parent = NULL);

protected:
   void keyPressEvent(QKeyEvent * const opc_KeyEvent) override;

   /// Called when the user presses Ctrl+Enter (no Alt, no Shift).
   /// Default implementation: this->mrc_ParentDialog.accept().
   /// Override to add pre-flight logic (validation, save, etc).
   virtual void m_OnEnterAccept(void);

   C_OgePopUpDialog & mrc_ParentDialog;

private:
   //Avoid call
   C_OgePopUpContentBase(const C_OgePopUpContentBase &);
   C_OgePopUpContentBase & operator =(const C_OgePopUpContentBase &) &;
};

}
```

Body of `keyPressEvent` is the existing 22-line block with the call site
replaced by `this->m_OnEnterAccept()`.

## Per-file migration steps

For each of the 52 candidates:

1. **Header**:
   - Change `public QWidget` → `public stw::opensyde_gui_elements::C_OgePopUpContentBase`
   - Add `#include "C_OgePopUpContentBase.hpp"`, remove direct `#include <QWidget>` if no longer needed (and direct `C_OgePopUpDialog.hpp` include if only used for the member type).
   - Remove the `mrc_ParentDialog` member declaration (base provides it).
   - Remove the `keyPressEvent` override declaration.
   - For Variant B classes: add `void m_OnEnterAccept(void) override;` (calls the existing `m_OkClicked`).
   - Keep the lint comment `//lint -e{1725} ...` only if it referenced the now-removed member; otherwise drop it.

2. **CPP**:
   - Update ctor initializer: `QWidget(opc_Parent),` → `C_OgePopUpContentBase(orc_Parent, opc_Parent),`. **Drop** the `mrc_ParentDialog(orc_Parent),` line — base class handles it.
   - Delete the entire `keyPressEvent` function body.
   - For Variant B: add a 3-line `m_OnEnterAccept` override that calls `this->m_OkClicked();`.

3. **Build** after each batch (see below).

## Excluded from migration

These 10 files share the same `keyPressEvent` boilerplate but **are not in
scope** for this refactor:

**2 inherit `QDialog`, not `QWidget`** (different lifecycle, would need a
sibling `C_OgePopUpDialogContentBase`):

- `libraries/opensyde_gui/src/opensyde_gui_elements/widget/C_OgeWiCustomMessage.cpp`
- `libraries/opensyde_gui/src/project_operations/C_PopPasswordDialogWidget.cpp`

**8 inherit `QWidget` but use a different parent-dialog field name** (rename
would be a deeper change worth its own pass):

- `libraries/opensyde_gui/src/system_definition/bus_edit/C_SdBueMessageRxTimeoutConfig.cpp` — `mpc_ParentDialog` (pointer)
- `libraries/opensyde_gui/src/system_definition/node_edit/datapools/C_SdNdeDpListDataSetWidget.cpp`
- `libraries/opensyde_gui/src/system_definition/node_edit/datapools/C_SdNdeDpListArrayEditWidget.cpp`
- `libraries/opensyde_gui/src/system_definition/node_edit/datapools/C_SdNdeDpProperties.cpp`
- `libraries/opensyde_gui/src/system_views/dashboards/C_SyvDaDashboardTabProperties.cpp`
- `libraries/opensyde_gui/src/system_views/dashboards/C_SyvDaDashboardSettings.cpp` — `mrc_Parent` (different name)
- `libraries/opensyde_gui/src/system_views/dashboards/properties/C_SyvDaPeBase.cpp`
- `libraries/opensyde_gui/src/system_views/dashboards/items/param/array_editor/C_SyvDaItPaArWidget.cpp`

Worth flagging these as a "Phase 3 follow-up" once the main migration lands.

## Suggested staging (5 commits)

To get the full file list for a bucket, use:
```bash
grep -rln "Handle all enter key cases manually" \
  /home/tyler/Projects/Crane/openSYDE/libraries/ \
  /home/tyler/Projects/Crane/openSYDE/opensyde_can_monitor/ \
  /home/tyler/Projects/Crane/openSYDE/opensyde_syde_flash/
```
…then filter out the 10 excluded files.

| # | Commit | Files | Notes |
|---|--------|-------|-------|
| 1 | Add `C_OgePopUpContentBase` | 2 (new .hpp/.cpp) + CMakeLists adds | No consumers yet — verifies headers + namespace correctness |
| 2 | Migrate `update_package/` | 7 dialogs | Original Phase 3 scope. Mostly Variant A. Proves the migration on a small batch |
| 3 | Migrate `system_definition/` | ~25 files | Biggest bucket. Mix of A and B variants |
| 4 | Migrate `system_views/` (non-dashboard) | ~10 files | The 8 dashboard outliers are excluded |
| 5 | Migrate remaining buckets | ~10 files | `com_import_export/`, `navigable_gui/`, `project_operations/`, `can_monitor/`, app dirs |

After each commit, build BOTH affected apps:

```bash
./build.sh canmonitor sydeflash    # if can_monitor or syde_flash files touched
./build.sh opensyde                # for libraries/opensyde_gui/ files
```

For the largest commits (3 and 5), build all three apps:
```bash
./build.sh opensyde canmonitor sydeflash
```

## Build verification per file

The two failure modes from the previous Phase 5 agent that bit on this
codebase:

1. **uic-generated headers fall back to lowercase `.h` when the customwidget
   declaration is missing.** This surfaces as
   `fatal error: c_<classname>.h: No such file or directory`. **Not relevant
   to Phase 3** — we're not changing widget classes in `.ui` files. Just
   noting so it doesn't surprise.

2. **Inheritance change ripples through pointer-to-member signal connects.**
   When `pc_BtnAdd` changes type, `&OldClass::clicked` no longer binds. **Not
   relevant to Phase 3** — we're changing C++ inheritance, not widget types
   in `.ui`. The `m_OkClicked` slots stay on the same class.

The actual Phase 3 risks:

- **Forgetting to drop `mrc_ParentDialog(orc_Parent),` from the ctor
  initializer.** If both base and subclass try to initialize it, compile
  error: `member initializer 'mrc_ParentDialog' does not name a non-static
  data member or base class`.

- **`m_OkClicked` access level.** Many subclasses have it as `private`. The
  Variant B override needs `m_OnEnterAccept` (which calls m_OkClicked) to be
  in the same class — works fine since `m_OnEnterAccept` becomes a protected
  override on the subclass too.

- **Headers with a `private:` block containing only `mrc_ParentDialog` and the
  `Avoid call` lines.** After removing `mrc_ParentDialog`, the `private:`
  block may become odd-looking but stays valid.

## Lessons from previous Phase 5 work (relevant context)

The previous agent on Phase 5:
- Stopped before checking the build → landed deletions with ~12 unmigrated
  consumers across .ui/.cpp/.qss
- Skipped Task A reference scans → never enumerated the consumers
- Made mechanical mistakes: dropped namespace prefixes on `.ui` widget class
  names (`stw::opensyde_gui_elements::Foo` → bare `Foo`); used C++ namespace
  syntax in qss (`stw::opensyde_gui::` instead of `stw--opensyde_gui--`);
  duplicated qss stanzas; bumped indentation by one space.

For Phase 3, the equivalent guardrails:

- After every batch commit, run the build for affected apps. Don't batch
  commits without verifying.
- Run `grep -r "mrc_ParentDialog\b" <migrated-file>` after each migration —
  there should be exactly **zero** occurrences (the member declaration and
  ctor init are both gone; the body uses `this->mrc_ParentDialog` which
  resolves through the base).
- Run `grep -r "::keyPressEvent" <migrated-file>` — zero occurrences expected
  in the migrated file.
- After all batches, run a final repo-wide grep for the
  `Handle all enter key cases manually` comment — it should appear in
  exactly the 10 excluded files (and the new base class).

## Status before this work

Branch `gui-consolidation` is ahead of `dev` with these recent commits:

- `b8c40fcd` Phase 5 housekeeping: status + backfilled survey docs
- `197819f1` Fix doubled-card C_NagToolTip rendering
- `3298ba6e` Phase 5: migrate 9 empty CAN Monitor GUI elements to base + styleRole

Phase 5 is fully done (10 of 25 CAN Monitor GUI elements eliminated). Plan.md
is up to date.

## Suggested opening prompt for the new session

```
Phase 3 of the GUI consolidation plan: read
docs/agent_plans/gui_consolidation/phase3-brief.md and execute it. Start with
commit 1 (the new base class), build to verify, then proceed through the 5
staged commits. After each, run the build and confirm zero stale references
to mrc_ParentDialog/keyPressEvent in the migrated files.
```
