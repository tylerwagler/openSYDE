# Phase 1 Stylesheet Migration — Agent Playbook

This document is a step-by-step playbook for migrating stylesheet-only widget
classes to their base class + a `styleRole` dynamic property.

**CRITICAL:** Each migration is broken into 3 subtasks (A, B, C) that are
separate turns. Do NOT attempt multiple subtasks in the same turn. After each
subtask, end the turn and wait for the user to say "proceed".

---

## How to classify a class

For each candidate class, you MUST read BOTH the .hpp AND .cpp files. Then
apply this checklist:

### Step 1: Read the .hpp file

Check the class declaration. It is STYLESHEET-ONLY **only if ALL of these
are true**:
- The only public method is the constructor
- There are no `override` keywords anywhere in the class
- There are no member variables declared
- There are no signals or slots (Q_SIGNALS/Q_SLOTS/signals:/slots: sections)
- Q_OBJECT macro is acceptable (it's just for Qt metadata, not logic)

If any of the above fail, the class HAS LOGIC. Stop — do not migrate it.

### Step 2: Read the .cpp file

The constructor body MUST be completely empty — just `{}` with nothing inside.
The ENTIRE .cpp file should look like this (ignoring comments and boilerplate):

```cpp
C_OgeFooBar::C_OgeFooBar(QWidget * const opc_Parent) :
   C_OgeFooBase(opc_Parent)
{
}
```

If the constructor body contains ANY code — even a single line like
`this->setMinimum(0)` or `this->SetMargins(1, 6)` — the class HAS LOGIC.
Stop — do not migrate it.

**Do NOT be fooled by the file comment.** Many files say "This class does not
contain any functionality" in their comment block even when the constructor
DOES set member variables. Ignore comments — only trust the actual code.

### Step 3: Read the PARENT class .hpp file

Even if the class itself is stylesheet-only, you must understand what the
parent class is. Read the parent's .hpp to check whether it has real logic
(overrides, member variables, custom methods).

This matters because **the parent class becomes the replacement type** in .ui
files. You must replace the deleted class with its direct parent, NOT with
the Qt base class (`QSpinBox`, `QPushButton`, etc.).

Examples from this codebase:

| Deleted class | Parent class | Replacement in .ui |
|---|---|---|
| `C_OgeSpxNumber` | `C_OgeSpxToolTipBase` | `C_OgeSpxToolTipBase` |
| `C_OgeSpxFactorProperties` | `C_OgeSpxFactor` | `C_OgeSpxFactor` |
| `C_OgePubMessageOk` | `QPushButton` | `QPushButton` |

**NEVER replace with a grandparent or Qt base class.** If `C_OgeSpxNumber`
inherits `C_OgeSpxToolTipBase` which inherits `QSpinBox`, the replacement
is `C_OgeSpxToolTipBase`, NOT `QSpinBox`. The parent has tooltip, context
menu, and validation logic that would be lost.

---

## Survey (once per widget family)

**This is a separate turn from Tasks A/B/C below.** Do the survey first, then
stop and wait for the user to pick which class to migrate next.

1. List all .hpp files in the target directory (e.g., `spin_box/`)
2. For EACH class, read BOTH the .hpp AND .cpp
3. Read each candidate's parent .hpp to determine the correct replacement type
4. Create `docs/agent_plans/gui_consolidation/phase1-progress.md` with this exact format:

```markdown
# Phase 1 Progress — <Widget Family>

## TODO
- [ ] C_OgeFooA (inherits C_OgeBase) -> replace with C_OgeBase + styleRole="foo-a"
- [ ] C_OgeFooB (inherits C_OgeBase) -> replace with C_OgeBase + styleRole="foo-b"

## Skipped (has logic — do NOT migrate)
- C_OgeFooC — sets margins, colors in constructor
- C_OgeFooD — overrides paintEvent

## Done
(none yet)
```

Then end the turn. Do NOT start Task A in the same turn.

---

## The 3-task workflow (one class per 3 turns)

Work on **ONE class at a time**, broken into 3 separate turns.

### TASK A — Reference scan (NO CODE CHANGES, NO COMMIT)

Goal: produce a complete list of every file that references the class.

1. Run exhaustive greps across all file types. The greps must cover the ENTIRE
   repo from the project root, not just the class's own directory:
   ```bash
   grep -rn "C_OgeFoo" --include="*.ui" .
   grep -rn "C_OgeFoo" --include="*.qss" .
   grep -rn "C_OgeFoo" --include="*.cpp" .
   grep -rn "C_OgeFoo" --include="*.hpp" .
   grep -rn "C_OgeFoo" --include="CMakeLists.txt" .
   ```

   **CRITICAL:** The .cpp/.hpp grep almost always returns MORE files than just
   `C_OgeFoo.hpp` and `C_OgeFoo.cpp`. If your .cpp grep only finds the class's
   own two files, YOU HAVE PROBABLY MISSED SOMETHING. Stylesheet-only classes
   are referenced from application code too — in `connect()` calls like
   `&C_OgeFoo::valueChanged`, in member declarations like `C_OgeFoo * mpc_X`,
   in `#include` directives. Verify by explicitly searching for these patterns:
   ```bash
   grep -rn "C_OgeFoo::" --include="*.cpp" --include="*.hpp" .
   grep -rn "C_OgeFoo \*" --include="*.cpp" --include="*.hpp" .
   grep -rn "C_OgeFoo.hpp" --include="*.cpp" --include="*.hpp" .
   ```

   **Watch for substring collisions.** If another class has `C_OgeFoo` as a
   prefix (e.g., `C_OgeFooBar`), filter those out manually. Use patterns like
   `C_OgeFoo"` or `C_OgeFoo\.` to get exact matches.

   **Watch for misspellings.** The codebase has pre-existing typos in stylesheet
   selectors (e.g., `C_OgeSpxNumbe` missing the final `r`). Grep for likely
   typos too: `grep -rn "C_OgeFo[^oa-zA-Z]" --include="*.qss" .` (truncated
   variants). If you find any, note them as "pre-existing typos to fix" in the
   ref list.

2. Also check `C_UtiStyleSheets.cpp` for dynamically-built stylesheet strings
   that reference the class by name.

3. Write the results to
   `docs/agent_plans/gui_consolidation/phase1-<class>-refs.md` with this format:

   ```markdown
   # Reference scan — C_OgeFoo

   ## .ui files (N)
   - libraries/.../A.ui (lines: 12, 45, 203)
   - opensyde_tool/.../B.ui (lines: 88, 129)
   ...

   ## .qss files (N)
   - opensyde_tool/src/styles/Color.qss (lines: 629, 812, 1034, ...)
   - opensyde_tool/src/styles/Font.qss (line: 130)

   ## .cpp/.hpp files (N)
   - libraries/.../X.cpp (lines: 181, 186) — signal connect
   - libraries/.../Y.cpp (line: 3194) — member type declaration

   ## CMakeLists.txt files
   - opensyde_tool/pjt/openSYDE/CMakeLists.txt (.cpp entry, .hpp entry)

   ## Other
   - (none) OR list any `C_UtiStyleSheets.cpp` references, lint sources, etc.
   ```

4. **End the turn.** Tell the user the ref counts per category so they can
   decide whether to proceed.

### TASK B — Migrate active references (CLASS STILL EXISTS)

Goal: update every reference to use the new base class + styleRole, while
leaving the old class file intact.

**Do not delete the class files yet.** Do not touch CMakeLists.txt yet. The
old class continues to exist — this keeps the build green between B and C.

For each file in `docs/agent_plans/gui_consolidation/phase1-<class>-refs.md`:

1. **.ui files:** Replace the widget class and add styleRole property:
   ```xml
   <!-- BEFORE -->
   <widget class="stw::opensyde_gui_elements::C_OgeFoo" name="pc_Widget">

   <!-- AFTER -->
   <widget class="stw::opensyde_gui_elements::C_OgeBase" name="pc_Widget">
    <property name="styleRole" stdset="0">
     <string>foo-role</string>
    </property>
   ```
   Also remove the `<customwidget>` entry for the old class at the bottom of
   the .ui file. If the new base class already has a customwidget entry, leave
   it alone. If not, add one.

   **PRESERVE EXISTING INDENTATION.** Do not re-indent surrounding lines. The
   Edit tool's `old_string` must match the file's existing indentation exactly.

2. **.qss files:** Replace class-name selectors with the proper pattern:
   ```css
   /* BEFORE */
   stw--opensyde_gui_elements--C_OgeFoo { ... }

   /* AFTER */
   stw--opensyde_gui_elements--C_OgeBase[styleRole="foo-role"] { ... }
   ```
   Use the **full base class name** (with `stw--opensyde_gui_elements--`), not
   the Qt base class. This matches the established pattern from the label
   migration.

3. **.cpp/.hpp files:** Update signal references and member types:
   - `&C_OgeFoo::valueChanged` -> `&C_OgeBase::valueChanged` (or the Qt base
     class if the signal is inherited from there)
   - Member declarations `C_OgeFoo * mpc_X` -> `C_OgeBase * mpc_X`
   - `#include "C_OgeFoo.hpp"` -> `#include "C_OgeBase.hpp"`

4. **DO NOT:**
   - Delete `C_OgeFoo.hpp`/`.cpp` (Task C does that)
   - Remove anything from CMakeLists.txt (Task C does that)
   - Re-indent surrounding lines
   - Use sed, awk, Python scripts, or any shell-based text replacement on
     ANY file (.ui, .qss, .cpp, .hpp, CMakeLists — all of them). Use the
     Edit tool exclusively. Shell scripts cause silent partial matches,
     whitespace drift, and leave no audit trail. The Edit tool's
     `old_string` must be unique and match existing indentation exactly —
     if that's hard for a given edit, do multiple smaller edits, don't
     reach for sed.

5. **Verification gate — both must pass before commit:**

   a. Run the grep again. Every hit in source files must be gone:
      ```bash
      grep -rn "C_OgeFoo" --include="*.ui" --include="*.qss" \
           --include="*.cpp" --include="*.hpp" .
      ```
      Expected output: only hits inside the class's own `C_OgeFoo.hpp`/`.cpp`
      (which still exist) and inside `docs/agent_plans/gui_consolidation/phase1-<class>-refs.md`. **Nothing
      else.** Paste the grep output in your response.

   b. Run the build:
      ```bash
      ./build.sh
      ```
      This builds all three apps (opensyde_tool, CAN Monitor, SYDEflash).
      **Paste the tail of the build output showing "0 errors" or equivalent
      success indicator.** If the build fails, fix the error and re-run.
      Do NOT commit a broken build.

6. If both gates pass, commit:
   ```
   Migrate C_OgeFoo references to C_OgeBase + styleRole="foo-role"

   Update all .ui, .qss, and C++ references to use the promoted base class
   with a styleRole dynamic property. The C_OgeFoo class itself is still
   present and will be removed in a follow-up commit.
   ```

7. **End the turn.** Wait for user to say "proceed to Task C".

### TASK C — Delete class + CMakeLists cleanup

Goal: remove the now-unused class and its build system entries.

1. Delete the class files:
   ```bash
   rm libraries/opensyde_gui/src/opensyde_gui_elements/<type>/C_OgeFoo.hpp
   rm libraries/opensyde_gui/src/opensyde_gui_elements/<type>/C_OgeFoo.cpp
   ```

2. Remove the entries from all relevant CMakeLists.txt:
   - `opensyde_tool/pjt/openSYDE/CMakeLists.txt`
   - `opensyde_can_monitor/pjt/openSYDE_CAN_Monitor/CMakeLists.txt`
   - `opensyde_syde_flash/pjt/openSYDE_SYDEflash/CMakeLists.txt`

   **PRESERVE EXISTING INDENTATION.** The CMakeLists.txt uses 3-space
   indentation. Do not introduce 4-space or tab indentation.

3. **Verification gate — both must pass before commit:**

   a. Grep for the class name — must be zero hits in source and build files:
      ```bash
      grep -rn "C_OgeFoo" --include="*.ui" --include="*.qss" \
           --include="*.cpp" --include="*.hpp" \
           --include="CMakeLists.txt" --include="*.txt" .
      ```
      Expected output: hits only in `docs/` files. **Also check:**
      - `build_test/pclint_config/openSYDE/openSYDE_lint_sources.txt` (or similar lint sources files)
      - Any other tracked text files that list source files
      
      Paste the grep output.

   b. Run the build:
      ```bash
      ./build.sh
      ```
      Paste the tail showing success. If it fails, fix and re-run.

4. If both gates pass, commit:
   ```
   Remove empty C_OgeFoo class

   All references migrated to C_OgeBase + styleRole in the previous commit.
   C_OgeFoo is no longer referenced anywhere.
   ```

5. Update `docs/agent_plans/gui_consolidation/phase1-progress.md`:
   - Move the class from TODO to Done with the two commit hashes
   - Check the box

6. **End the turn.** Wait for user to pick the next class.

---

## Common mistakes to avoid

1. **DO NOT combine tasks in one turn.** Each task (Survey, A, B, C) is a
   separate turn. End the turn after each one.

2. **DO NOT commit without pasting the build output and the grep output.**
   Both gates are required. If you can't paste both, you haven't done them.

3. **DO NOT migrate classes that set member variables in the constructor.**
   Read the .cpp. If the constructor body is not `{}`, skip it.

4. **DO NOT replace with the Qt base class** when the parent is a custom
   class with logic. Use the direct parent (`C_OgeSpxToolTipBase`), not the
   grandparent (`QSpinBox`).

5. **DO NOT re-indent surrounding lines.** Preserve existing whitespace.
   The Edit tool's `old_string` must match the file exactly. If your diff
   shows whitespace-only changes on lines you didn't mean to touch, STOP
   and revert those lines.

6. **DO NOT use sed, awk, Python scripts, or any shell-based text
   replacement on ANY file.** Use the Edit tool exclusively. Shell tools
   cause silent partial matches, whitespace drift, and leave no audit
   trail. This applies to .ui, .qss, .cpp, .hpp, and CMakeLists equally.

7. **DO NOT forget to check all 3 apps' stylesheets** (opensyde_tool,
   opensyde_can_monitor, opensyde_syde_flash). A class might only appear in
   one app's .ui files but be styled in all 3 apps' .qss files.

8. **DO NOT forget `C_UtiStyleSheets.cpp`.** Some stylesheet rules are built
   dynamically in C++ code, not in .qss files.

9. **DO NOT run clean builds.** `./build.sh -c` takes over an hour. Use
   incremental builds only.

10. **Watch for substring collisions** when grepping. `C_OgeSpxFoo` will match
    inside `C_OgeSpxFooBar`. Use `C_OgeSpxFoo["\.]` or similar patterns.

11. **DO NOT forget lint sources files.** Check `build_test/pclint_config/*/openSYDE_lint_sources.txt`
    (or similar) for references to deleted files. These are tracked files that must be updated
    whenever a source file is removed, otherwise PC-Lint will fail.

12. **DO NOT combine widget families in one session.** Finish all candidates
    of one type (e.g., all spin boxes) before starting another type.

---

## Per-family migration notes

### Push Buttons
- **Status:** ✅ Complete. 17 classes migrated. Remaining ~27 all have real
  logic.
- **styleRole values already in use:** `message-ok`, `message-cancel`,
  `color-only`, `navigation`, `text-only`, `text-only-underline`, `toggle`,
  `toggle-bold`, `zoom-notification`, `text-with-border-edit`,
  `text-only-edit`, `configure`, `data-logger-jobs-overview`, `halc-overview`,
  `halc-panel`, `dialog`, `cancel`

### Spin Boxes
- **Status:** Not started.
- **Directory:** `libraries/opensyde_gui/src/opensyde_gui_elements/spin_box/`

### Combo Boxes, Check Boxes, Group Boxes, Line Edits, others
- **Status:** Not started.
