# Phase 1 Stylesheet Migration — Agent Playbook

This document is a step-by-step playbook for migrating stylesheet-only widget
classes to their base class + a `styleRole` dynamic property. Follow it one
batch at a time. **Do NOT attempt multiple widget families or more than ~5
classes in a single session.**

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

### Classification examples from this codebase

**STYLESHEET-ONLY** (migrate):
```
C_OgeSpxNumber.hpp  — only has constructor, no overrides, no members
C_OgeSpxNumber.cpp  — constructor body is empty {}
Parent: C_OgeSpxToolTipBase (has logic — keep as replacement)
-> Replace with C_OgeSpxToolTipBase + styleRole="number"
```

**HAS LOGIC** (skip):
```
C_OgeSpxFactor.hpp  — has stepBy() override, validate() override, static member
C_OgeSpxFactor.cpp  — constructor body sets values, overrides have logic
-> DO NOT MIGRATE
```

**HAS LOGIC** (skip — constructor sets member variables):
```
C_OgePubBackNavigation.hpp  — only has constructor (looks stylesheet-only!)
C_OgePubBackNavigation.cpp  — constructor sets margins, colors, fonts
-> DO NOT MIGRATE (constructor body is not empty)
```

---

## The migration pattern (per class)

For each stylesheet-only class `C_OgeFoo` that inherits from `C_OgeBase`:

### 1. Choose the styleRole name
- Convert the class-specific suffix to kebab-case
- Example: `C_OgePubMessageOk` -> `"message-ok"`
- Example: `C_OgeLabHeadingWidget` -> `"heading-widget"`

### 2. Find all references (BEFORE editing anything)
Run these searches and **write down the results** — you will need them for
every subsequent step:

```
grep -r "C_OgeFoo" --include="*.ui"     # .ui file references
grep -r "C_OgeFoo" --include="*.qss"    # stylesheet rules
grep -r "C_OgeFoo" --include="*.cpp"    # C++ instantiation or connect()
grep -r "C_OgeFoo" --include="*.hpp"    # C++ includes or member types
grep -r "C_OgeFoo" CMakeLists.txt       # build system references
```

**IMPORTANT:** Check for substring collisions. If `C_OgeFoo` is a prefix of
another class name (e.g., `C_OgeFooExtended`), you must use exact-match
patterns like `C_OgeFoo"` or `C_OgeFoo.hpp` to avoid clobbering the longer
name.

### 3. Update .ui files
For each .ui file that references the class:

**Replace the widget class:**
```xml
<!-- BEFORE -->
<widget class="stw::opensyde_gui_elements::C_OgeFoo" name="pc_Widget">

<!-- AFTER -->
<widget class="QPushButton" name="pc_Widget">
 <property name="styleRole" stdset="0">
  <string>my-role-name</string>
 </property>
```

Note: The replacement base class depends on what the deleted class inherited:
- If it inherited `QPushButton` / `QLabel` / etc. directly -> use that Qt class
- If it inherited a custom base that IS a promoted widget in the .ui file
  (like `C_OgePubToolTipBase`) -> use that base class instead

**Remove the customwidget entry** for the deleted class (the `<customwidget>`
block in the `<customwidgets>` section at the bottom of the .ui file). If the
base class is also a promoted widget, its customwidget entry should already
exist — just remove the deleted class's entry.

### 4. Update .qss files
Replace class-name selectors with property selectors:

```css
/* BEFORE */
stw--opensyde_gui_elements--C_OgeFoo { ... }
stw--opensyde_gui_elements--C_OgeFoo:hover { ... }

/* AFTER */
QPushButton[styleRole="my-role-name"] { ... }
QPushButton[styleRole="my-role-name"]:hover { ... }
```

Check ALL 3 apps' stylesheet directories:
- `opensyde_tool/src/styles/`
- `opensyde_can_monitor/src/can_monitor/styles/`
- `opensyde_syde_flash/src/syde_flash/styles/`

Each app has: `Color.qss`, `Font.qss`, `PushButton.qss` (or `Label.qss`,
`SpinBox.qss`, etc. depending on widget type). Check ALL of them.

Also check `C_UtiStyleSheets.cpp` for dynamically-built stylesheet strings.

### 5. Update C++ code (if any)
If the class was instantiated in C++ (not just .ui files):
- Replace `#include "C_OgeFoo.hpp"` with the appropriate base class include
  (e.g., `#include <QPushButton>`)
- Replace `new C_OgeFoo(parent)` with:
  ```cpp
  auto * const pc_Button = new QPushButton(parent);
  pc_Button->setProperty("styleRole", "my-role-name");
  ```
- Update any `connect()` calls that reference `&C_OgeFoo::signal` to use the
  base class signal (e.g., `&QPushButton::clicked`)
- Update any member declarations from `C_OgeFoo *` to the base type

### 6. Delete the class files
- Delete `C_OgeFoo.hpp` and `C_OgeFoo.cpp`

### 7. Update CMakeLists.txt
Remove the deleted `.hpp` and `.cpp` files from all CMakeLists.txt files that
reference them. Check:
- `opensyde_tool/pjt/openSYDE/CMakeLists.txt`
- `opensyde_can_monitor/pjt/openSYDE_CAN_Monitor/CMakeLists.txt`
- `opensyde_syde_flash/pjt/openSYDE_SYDEflash/CMakeLists.txt`

Also check lint source lists if they exist.

### 8. Build check
Run an incremental build of ALL apps to verify nothing is broken:
```bash
./build.sh
```
This builds opensyde_tool, CAN Monitor, and SYDEflash. The shared GUI library
is used by all three, so you must check all of them.
If it fails, fix the specific error. Do NOT run `./build.sh -c` (clean build).

---

## Batch workflow with compaction checkpoints

Work on **ONE class at a time**. Each class is one commit. The only exception
is when two classes share the same QSS rule (e.g., a combined selector like
`C_OgeFooA, C_OgeFooB { ... }`) — migrate those together.

### STEP A: Survey and create progress file

**If `docs/phase1-progress.md` already exists**, read it and skip to Step B —
it tells you what to do next.

**If starting a new widget family**, survey it to find candidates:

1. List all .hpp files in the directory
2. For EACH class, read BOTH the .hpp AND .cpp (see "How to classify" above)
3. Also read the parent class .hpp to determine the correct replacement type
4. Create `docs/phase1-progress.md` with this exact format:

```markdown
# Phase 1 Progress — <Widget Family>

## TODO
- [ ] C_OgeFooA (inherits C_OgeBase) -> replace with C_OgeBase + styleRole="foo-a"
- [ ] C_OgeFooB (inherits C_OgeBase) -> replace with C_OgeBase + styleRole="foo-b"

## Skipped (has logic — do NOT migrate)
- C_OgeFooC — sets margins, colors in constructor
- C_OgeFooD — overrides paintEvent
- C_OgeFooE — has member variables and custom methods

## Done
(none yet)
```

**The TODO items must include the replacement base class.** Do not just write
the class name — write what it will be replaced with. This is determined by
reading the parent class .hpp.

Pick the first unchecked item from TODO for this iteration.

### STEP B: Migrate one class

Follow steps 1-7 above for a single class. Keep your edits minimal — only
touch files that reference the class you are migrating.

### STEP C: Build and verify

Run an incremental build of ALL apps to verify nothing is broken:
```bash
./build.sh
```
This builds opensyde_tool, CAN Monitor, and SYDEflash. The shared GUI library
is used by all three, so you must check all of them.
If it fails, fix the specific error. Do NOT run `./build.sh -c` (clean build).

### STEP D: Commit

Commit with a message like:
```
Replace C_OgeFoo with <BaseClass> + styleRole="foo"

C_OgeFoo was an empty <BaseClass> wrapper existing solely for QSS targeting.
Replaced with <BaseClass> + styleRole dynamic property across all 3 apps.
```

### STEP E: Update progress and compact

**This is a compaction checkpoint.** After every commit:

1. Update `docs/phase1-progress.md`:
   - Move the class you just finished from TODO to Done (with its commit hash)
   - Check the box: `- [x] C_OgeFoo ...`
2. Compact your context

When you resume after compaction, read these two files to pick up where you
left off:
- `docs/phase1-stylesheet-migration-playbook.md` (this file — the recipe)
- `docs/phase1-progress.md` (the progress tracker — tells you what's next)

---

## Per-family migration notes

### Push Buttons (Phase 1 — Push Buttons)
- **Status:** 17 classes migrated. All remaining ~27 classes have real logic.
  Push button migration is COMPLETE.
- **Base class used:** `QPushButton` (for classes that inherited `QPushButton`
  or `C_OgePubToolTipBase`)
- **styleRole values already in use:** `message-ok`, `message-cancel`,
  `color-only`, `navigation`, `text-only`, `text-only-underline`,
  `toggle`, `toggle-bold`, `zoom-notification`, `text-with-border-edit`,
  `text-only-edit`, `configure`, `data-logger-jobs-overview`,
  `halc-overview`, `halc-panel`, `dialog`, `cancel`

### Spin Boxes (Phase 1 — Spin Boxes)
- **Status:** Not started. Survey needed.
- **Directory:** `libraries/opensyde_gui/src/opensyde_gui_elements/spin_box/`
- **Likely base classes:** `QSpinBox`, `QDoubleSpinBox`, or a custom base
  with tooltip support
- **Start by:** Reading every .cpp in the spin_box/ directory to classify

### Combo Boxes (Phase 1 — Combo Boxes)
- **Status:** Not started. Survey needed.
- **Directory:** `libraries/opensyde_gui/src/opensyde_gui_elements/combo_box/`

### Check Boxes (Phase 1 — Check Boxes)
- **Status:** Not started. Survey needed.
- **Directory:** `libraries/opensyde_gui/src/opensyde_gui_elements/check_box/`

### Group Boxes (Phase 1 — Group Boxes)
- **Status:** Not started. Survey needed.
- **Directory:** `libraries/opensyde_gui/src/opensyde_gui_elements/group_box/`

### Line Edits (Phase 1 — Line Edits)
- **Status:** Not started. Survey needed.
- **Directory:** `libraries/opensyde_gui/src/opensyde_gui_elements/line_edit/`

### Other types (frame, scroll_area, tab_widget, etc.)
- **Status:** Not started. Survey needed.

---

## Common mistakes to avoid

1. **DO NOT migrate classes that set member variables in the constructor.**
   Read the .cpp. If the constructor body is not `{}`, skip it.

2. **DO NOT migrate more than one class per commit.** Context fills up fast
   with .ui XML edits. The only exception is classes that share QSS rules.

3. **DO NOT use sed/awk/Python scripts to edit .ui files.** They break XML
   structure. Use the Edit tool directly.

4. **DO NOT forget to check all 3 apps' stylesheets.** A class might only
   appear in one app's .ui files but be styled in all 3 apps' .qss files.

5. **DO NOT forget `C_UtiStyleSheets.cpp`.** Some stylesheet rules are built
   dynamically in C++ code, not in .qss files.

6. **DO NOT run clean builds.** `./build.sh -c` takes over an hour. Use
   incremental builds only.

7. **Watch for substring collisions** when grepping. `C_OgeSpxFoo` will match
   inside `C_OgeSpxFooBar`. Use `C_OgeSpxFoo["\.]` or similar patterns.

8. **DO NOT combine widget families in one session.** Finish all batches of
   one type (e.g., all spin boxes) before starting another type.
