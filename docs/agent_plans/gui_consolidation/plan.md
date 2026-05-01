# Plan: Reduce Redundant GUI Items in openSYDE

## Context

The openSYDE GUI layer contains **~275 custom widget classes** across `opensyde_gui_elements/`, of which **148 are documented as having zero functionality** — they exist solely so Qt stylesheets can target them by C++ class name. This pattern creates massive file bloat (296+ source files for empty shells), slows compilation, pollutes the class hierarchy, and makes the codebase harder to navigate. Beyond the stylesheet classes, there are additional redundancies in dashboard property panels, popup dialogs, title bars, and model-view-delegate implementations.

**Total estimated reduction: ~170-200 source files (.hpp/.cpp pairs), ~5,000-8,000 lines of boilerplate.**

---

## Status Summary (as of 2026-05-01)

| Phase | Status | Notes |
|-------|--------|-------|
| Phase 1 (Labels) | ✅ **Complete** | 58/58 stylesheet-only label classes removed |
| Phase 1 (Push Buttons) | ✅ **Complete** | 17 stylesheet-only classes removed; ~27 remaining all have real logic |
| Phase 1 (Spin Boxes) | ✅ **Complete** | 10/10 stylesheet-only spin box classes migrated; 14 have real logic |
| Phase 1 (Check Boxes) | ✅ **Complete** | 8/8 stylesheet-only check box classes migrated; 3 have real logic |
| Phase 1 (Group Boxes) | ✅ **Complete** | 9/9 stylesheet-only classes migrated; 8 with logic kept |
| Phase 1 (Combo Boxes) | ✅ **No candidates** | All 12 classes have ctor logic (font/halc/icon-only set delegate or fixed height), real overrides, public API, or member state. See phase1-progress-combo-box.md. |
| Phase 1 (Line Edits) | ⚠ **Partially done** | C_OgeLeProperties{Name}, C_OgeLeTable, C_OgeLeListHeader migrated; remaining classes all have logic or runtime API |
| Phase 1 (Menu) | ✅ **Complete** | 1/2 migrated (TabChartFit); Sections kept (has logic) |
| Phase 1 (Scroll Area) | ✅ **Complete** | 1/2 migrated (NaviBar); Main kept (has logic) |
| Phase 1 (Slider) | ✅ **Complete** | 0/2 migrated (both have logic) |
| Phase 1 (Radio Button) | ✅ **Complete** | 1/3 migrated (CheckBox); Properties/ToolTipBase kept |
| Phase 1 (Frame) | ✅ **Complete** | 3/4 deleted: Border, Frame (earlier), Separator (B1–C: 168 widgets across 96 .ui files, factory in C_SebToolboxUtil rewritten, 14 qss selectors converted). C_OgeFraGeneric kept (functional `SetBorderColor`/`SetBackgroundColor` API). |
| Phase 1 (Text Browser) | ✅ **Complete** | 2/4 migrated (MessageDetails, Report); ContextMenuBase/Label kept |
| Phase 1 (Tab Widget) | ✅ **No candidates** | All 5 classes have ctor logic (setUsesScrollButtons / setExpanding / custom tabBar / font) or real overrides (ToolTipBase). See phase1-progress-tab-widget.md. |
| Phase 1 (Widget) | ⚠ **Partially done** | C_OgeWi{Param,Table}SpinBoxGroup migrated; 13 with logic kept; remaining "stylesheet-only" candidates all override paintEvent (skipped per migration bar) |
| Phase 1 (Splitter / Tool Button) | ✅ **No candidates** | All splitter classes have logic; tool_button has only the tooltip base |
| Phase 2 | ⏳ Not started | Dashboard property panels |
| Phase 3 | ⏳ Not started | Popup dialog boilerplate |
| Phase 4 | ⏳ Not started | Title bar unification |
| Phase 5 | ⏳ Not started | CAN Monitor element dedup |
| Phase 6 | ⏳ Not started | MVD triplication in dashboard items |

### Phase 1 Label Portion — Completion Details

**Started:** 70 label classes (58 stylesheet-only)
**Now:** 14 label classes (all with real functionality)

**Functional labels kept:**
`C_OgeLabAdaptiveSize`, `C_OgeLabContextMenuBase`, `C_OgeLabDashboardDefault`, `C_OgeLabDashboardLabelValue`, `C_OgeLabDashboardProgressBarMaximum`, `C_OgeLabDoubleClick`, `C_OgeLabElided`, `C_OgeLabExternalLink`, `C_OgeLabFrameError`, `C_OgeLabGenericNoPaddingNoMargins`, `C_OgeLabGenericWithContextMenu`, `C_OgeLabPopUpTitle`, `C_OgeLabSvgOnly`, `C_OgeLabToolTipBase`

**styleRole values introduced** (the list of dynamic-property values the stylesheets now match against):
`heading-widget-title`, `heading-widget-sub-title`, `heading-widget-sub-sub-title`, `update-app-path`, `update-app-version`, `update-heading`, `update-node-title`, `update-time`, `toolbar-no-search-result`, `toolbar-search-result-title`, `toolbar-search-result-subtitle`, `toolbox-heading-group`, `toolbox-heading-group-big`, `bus-type`, `category-sub-heading`, `color-only`, `heading-properties`, `heading-tool-tip`, `list-header`, `list-header-heading`, `navigation-sub-heading`, `node-type`, `properties-sub-heading`, `node-data-pool-selected-items`, `node-prop-com-if-table`, `progress-log-heading`, `progress-text`, `topology-toolbox-user-nodes`, `dashboard-tab`, `list-header-highlighted`, `status-information`, `status-information-small`, `category-heading`, `com-list-header`, `state-info`, `progress-log-item`, `dashboard-label-caption`, `heading-message`, `description-message`, `generic-bubble`, `popup-title`, `popup-sub-title`, `dashboard-chart`, `group-item-value`, `group-sub-item`, `group-item`, `heading-group-bold`

**Promoted base classes used in .ui files** (instead of deleted stylesheet-only subclasses):
- `QLabel` — for classes that inherited `QLabel` directly
- `C_OgeLabToolTipBase` — for classes that inherited `C_OgeLabToolTipBase` (preserves tooltip)
- `C_OgeLabContextMenuBase` — for classes that inherited `C_OgeLabContextMenuBase` (preserves tooltip + context menu)
- `C_OgeLabGenericNoPaddingNoMargins` — for classes that inherited this (preserves property setters)
- `C_OgeLabPopUpTitle` — for PopUpSubTitle (preserves mouse signals)

**Commits (in order on `gui-consolidation` branch):**
- `4998a37a` — Batch 1: 5 HeadingWidget classes
- `2832f8e0` — Build fix: restore osy_git_data_model_monitor, QCustomPlot keyword wrapper
- `3a711db6` — Batches 2-7: 52 label classes in bulk
- `d470ec40` — Fix syde_coder_c Linux build (separate from GUI work)
- `37d4cedc` — Final 6 label classes (DashboardChart, GroupItemValue, GroupSubItem, GroupItem, HeadingGroupBold; also PopUpSubTitle from earlier)

**Aggregate footprint:**
- ~112 source files deleted (56 classes × 2)
- 240+ widget references migrated across ~150 .ui files
- ~50 .qss rule updates across 6 .qss files in 3 applications
- 3 `CMakeLists.txt` files cleaned up
- A handful of C++ files updated where classes were instantiated programmatically (`C_SebToolboxUtil.cpp`, `C_SdNdeNodePropertiesWidget.cpp`, `C_SdTopologyToolbox.hpp/cpp`, `C_SyvUpProgressLog.cpp`) — all now construct the appropriate base class and call `setProperty("styleRole", ...)`
- Dynamic stylesheet strings in `C_UtiStyleSheets.cpp` updated for `dashboard-chart` transparency rules

### Lessons Learned for Future Phase 1 Batches

1. **Substring collision is the biggest footgun** — e.g., `C_OgeLabListHeader` matched inside `C_OgeLabListHeaderHeading` and `C_OgeLabListHeaderHighlighted`. Always check for name prefix collisions before bulk replacement. The safer pattern for exact-match is to include the trailing `"` or `.hpp` in the search string.

2. **Manual Edit tool for small batches, Python script for large ones is OK once collisions are verified absent.** Don't use Python for the first batch of a new class family — always do a few manual ones first to understand the variant shapes, then script the tail.

3. **For classes used in C++ code** (instantiated programmatically): the class's base type may also need to be updated in header member declarations, not just the .cpp construction site.

4. **For classes whose parent class is used as a promoted widget elsewhere** (e.g., `C_OgeLabToolTipBase` itself): the customwidget declaration for the parent is often already present in files that used the subclass, so scan first — we can just remove the child's customwidget entry rather than updating it.

5. **Check the dynamic stylesheet builder** (`C_UtiStyleSheets.cpp`) for any runtime-built selectors that reference the class. These are compiled C++ strings and easy to miss in grep if you only look at `.qss`.

6. **The `.` prefix in Qt QSS selectors** (`.C_OgeLabPopUpTitle`) means "exact class match, no subclasses". When removing a subclass that was distinguished this way, update the rule to use the `styleRole` attribute instead of class-exact matching.

---

## Phase 1: Eliminate Stylesheet-Only Classes via Qt Dynamic Properties

**Impact: ~148 classes -> 0 | ~296 files removed**
**Risk: Medium-High (touches every .ui file and all .qss files)**

**Progress: ~117/148 stylesheet-only classes migrated. The remaining ~31 candidates from the original 148 either turned out to have ctor side-effects, real overrides, or public API beyond the ctor — those are documented as "no candidates" or "kept" per family in the status table above and the per-family progress docs.**

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
