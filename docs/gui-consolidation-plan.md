# Plan: Reduce Redundant GUI Items in openSYDE

## Context

The openSYDE GUI layer contains **~275 custom widget classes** across `opensyde_gui_elements/`, of which **148 are documented as having zero functionality** — they exist solely so Qt stylesheets can target them by C++ class name. This pattern creates massive file bloat (296+ source files for empty shells), slows compilation, pollutes the class hierarchy, and makes the codebase harder to navigate. Beyond the stylesheet classes, there are additional redundancies in dashboard property panels, popup dialogs, title bars, and model-view-delegate implementations.

**Total estimated reduction: ~170-200 source files (.hpp/.cpp pairs), ~5,000-8,000 lines of boilerplate.**

---

## Phase 1: Eliminate Stylesheet-Only Classes via Qt Dynamic Properties

**Impact: ~148 classes -> 0 | ~296 files removed**
**Risk: Medium-High (touches every .ui file and all .qss files)**

### Problem
Every styling variation requires a dedicated C++ class:
```cpp
// C_OgeLabHeadingWidget.cpp — entire implementation:
C_OgeLabHeadingWidget::C_OgeLabHeadingWidget(QWidget * const opc_Parent) :
   QLabel(opc_Parent) {}
```
Then targeted in `.qss`:
```css
stw--opensyde_gui_elements--C_OgeLabHeadingWidget { font: 18px "Segoe UI"; }
```

### Solution
Replace class-name selectors with **Qt dynamic property selectors**. Qt natively supports:
```css
QLabel[styleRole="heading-widget"] { font: 18px "Segoe UI"; }
```

### Approach

#### Step 1: Create a style-role registration utility
- File: `libraries/opensyde_gui/src/util/C_OgeStyleRole.hpp/cpp`
- A simple helper that applies a `styleRole` property in the constructor:
  ```cpp
  // Usage: In .ui files, promote QLabel to C_OgeLabToolTipBase, then
  // call C_OgeStyleRole::Apply(widget, "heading-widget") in setupUi or constructor
  ```
- Alternatively, create a **single generic styled class per base type** that accepts a role string:
  ```cpp
  class C_OgeLabStyled : public C_OgeLabToolTipBase {
  public:
      C_OgeLabStyled(const QString& role, QWidget* parent = nullptr);
  };
  ```

#### Step 2: Migrate .qss files
- Convert class-name selectors to property selectors
- File-by-file in `opensyde_tool/src/styles/`, `opensyde_can_monitor/src/can_monitor/styles/`, `opensyde_syde_flash/src/syde_flash/styles/`
- Example: `stw--opensyde_gui_elements--C_OgeLabHeadingWidget` -> `C_OgeLabToolTipBase[styleRole="heading-widget"]`

#### Step 3: Update .ui files
- All 213 `.ui` files that reference stylesheet-only classes need their `<widget class="C_OgeLabHeadingWidget">` changed to `<widget class="C_OgeLabToolTipBase">` (or the generic styled variant) with a `<property name="styleRole"><string>heading-widget</string></property>`

#### Step 4: Remove empty class files
- Delete the 148 stylesheet-only `.hpp/.cpp` pairs
- Update CMakeLists.txt / .pri files

#### Categories to consolidate (by widget type):
| Type | Stylesheet-only count | Base class to keep |
|------|----------------------|-------------------|
| Label | ~54 | C_OgeLabToolTipBase |
| PushButton | ~26 | C_OgePubToolTipBase |
| SpinBox | ~15 | Keep functional bases |
| ComboBox | ~10 | Keep functional bases |
| CheckBox | ~8 | Keep functional bases |
| GroupBox | ~12 | Keep functional bases |
| LineEdit | ~8 | Keep functional bases |
| Others | ~15 | Various |

### Critical files:
- `libraries/opensyde_gui/src/util/C_UtiStyleSheets.cpp` (loads all .qss files)
- `opensyde_tool/src/styles/*.qss` (24 stylesheet files, 5,960 lines total)
- All `opensyde_gui_elements/` subdirectories (18 widget-type dirs)

---

## Phase 2: Consolidate Dashboard Property Panels

**Impact: 6 classes -> 1 parameterized base | ~12 files reduced to ~2**

### Problem
`C_SyvDaPeLabel`, `C_SyvDaPePieChart`, `C_SyvDaPeProgressBar`, `C_SyvDaPeSlider`, `C_SyvDaPeSpinBox`, `C_SyvDaPeToggle` all follow an identical pattern:
- Same constructor signature
- Same `InitStaticNames()` + `m_UpdatePreview()` interface
- Same signal wiring boilerplate

### Solution
Extract a generic `C_SyvDaPeWidgetType` base that takes the widget type as a parameter/template, with per-type configuration via a strategy or variant rather than separate classes.

### Critical files:
- `libraries/opensyde_gui/src/system_views/dashboards/properties/C_SyvDaPeBase.hpp` (1543 lines)
- `libraries/opensyde_gui/src/system_views/dashboards/properties/C_SyvDaPe{Label,PieChart,ProgressBar,Slider,SpinBox,Toggle}.hpp/cpp/ui`

---

## Phase 3: Consolidate Popup Dialog Boilerplate

**Impact: ~7 dialog classes simplified | ~70% boilerplate reduction per class**

### Problem
All popup/info dialogs in `system_views/system_update/update_package/` repeat:
- Constructor storing `C_OgePopUpDialog & mrc_ParentDialog`
- Identical `keyPressEvent()` override
- Identical `m_OkClicked()` / `m_CancelClicked()` slots

### Solution
Create `C_OgePopUpContentBase` — a base class for popup content widgets that handles the `C_OgePopUpDialog` lifecycle boilerplate. Subclasses only implement their unique content.

### Critical files:
- `libraries/opensyde_gui/src/system_views/system_update/update_package/C_SyvUpPac*PopUp.hpp/cpp`
- `libraries/opensyde_gui/src/system_views/system_update/update_package/C_SyvUpPac*Dialog.hpp/cpp`

---

## Phase 4: Unify Title Bar Widgets

**Impact: 2 parallel implementations -> 1 shared base + 2 thin subclasses**

### Problem
`C_CamTitleBarWidget` and `C_FlaTitleBarWidget` are independent implementations of the same concept.

### Solution
Extract shared title bar logic into `C_OgeTitleBarWidget` in the shared library. CAN Monitor and SydeFlash subclass it for app-specific branding only.

### Critical files:
- `opensyde_can_monitor/src/can_monitor/C_CamTitleBarWidget.hpp/cpp/ui`
- `opensyde_syde_flash/src/syde_flash/C_FlaTitleBarWidget.hpp/cpp/ui`

---

## Phase 5: Consolidate CAN Monitor GUI Elements

**Impact: ~24 classes reviewed, ~10-15 eliminated**

### Problem
`can_monitor_gui_elements/` mirrors the structure of `opensyde_gui_elements/` with app-specific variants. Some are justified (they override behavior), but many are stylesheet-only duplicates that Phase 1 would already address.

### Solution
After Phase 1, audit remaining CAN Monitor element classes. Those that only add styling can use the property-based approach. Those with real overrides stay as proper subclasses.

### Critical files:
- `libraries/opensyde_gui/src/can_monitor/can_monitor_gui_elements/` (all subdirs)

---

## Phase 6: Reduce MVD (Model-View-Delegate) Triplication

**Impact: 9 files -> 3-4 with shared base classes**

### Problem
Three separate Model-View-Delegate implementations for very similar table/tree patterns:
1. Parameter Tree (C_SyvDaItPaTree{Model,View,Delegate})
2. Array Editor (C_SyvDaItPaAr{Model,View,Delegate})
3. Dashboard Table (C_SyvDaItTa{Model,View,Delegate})

### Solution
Extract common base classes for the Model and Delegate where the data-access pattern is shared. Views may already share enough via Qt's built-in view classes.

### Critical files:
- `libraries/opensyde_gui/src/system_views/dashboards/items/param/`
- `libraries/opensyde_gui/src/system_views/dashboards/items/param/array_editor/`
- `libraries/opensyde_gui/src/system_views/dashboards/items/table/`

---

## Execution Order & Dependencies

```
Phase 1 (Stylesheet classes)    <-- Do first, biggest impact, no deps
  |
Phase 5 (CAN Monitor elements)  <-- Depends on Phase 1 approach
  |
Phase 2 (Dashboard properties)  <-- Independent
Phase 3 (Popup boilerplate)     <-- Independent
Phase 4 (Title bars)            <-- Independent
Phase 6 (MVD consolidation)     <-- Independent, lowest priority
```

## Verification

- **Build**: All three applications (opensyde_tool, opensyde_can_monitor, opensyde_syde_flash) must compile cleanly
- **Visual regression**: Screenshot comparison of key screens before/after to ensure styling is preserved
- **Runtime**: Launch each app, verify widgets render with correct fonts/colors/sizes
- **Stylesheet hot-reload** (if supported): Verify property selectors work at runtime
- **.ui file integrity**: Open modified .ui files in Qt Designer to verify they load correctly

## Risks & Mitigations

| Risk | Mitigation |
|------|-----------|
| Qt property selectors have different specificity than class selectors | Test with a small batch first (e.g., 5 label types) before bulk migration |
| .ui files may need Qt Designer compatibility | Verify promoted widget approach works in Designer |
| Build system references to removed files | Script the CMakeLists/pri cleanup |
| Runtime stylesheet performance with many property selectors | Benchmark — unlikely to be measurable |
