# GUI Controls Consolidation - Phase 3: Labels (In Progress)

**Date**: 2026-03-12  
**Status**: 🚧 **IN PROGRESS** - Foundation Created  
**Target**: Consolidate styling-only label classes

---

## Current State Analysis

### Label Classes (21 total)

#### High Priority - Styling Only (Candidates for Consolidation)
1. `C_CamOgeLabStatusBar` - Empty constructor, styling only
2. `C_OgeLabStyled` - Already exists as consolidated class (good pattern)
3. `C_OgeLabGenericNoPaddingNoMargins` - Has some setters but mostly styling
4. `C_OgeLabDashboardDefault` - Empty constructor, styling only
5. `C_OgeLabPopUpTitle` - Has mouse events (KEEP or consolidate with variant)
6. `C_OgeLabPopUpSubTitle` - Inherits from PopUpTitle (KEEP or consolidate)

#### Medium Priority - Has Functionality (Keep Separate)
- `C_OgeLabDoubleClick` - Has double-click handling
- `C_OgeLabExternalLink` - Has link handling
- `C_OgeLabElided` - Has text eliding logic
- `C_OgeLabContextMenu*` - Has context menu functionality
- `C_OgeLabAdaptiveSize` - Has custom sizing logic
- `C_OgeLabSvgOnly` - Has SVG rendering

---

## Phase 3a: Status Bar Label Consolidation

### Target: `C_CamOgeLabStatusBar` → `QLabel#labelStatusBar`

**Analysis**:
- Empty constructor
- No custom functionality
- Exists solely for QSS styling

**Approach**:
1. ✅ Created `C_OgeLabBase` class (similar to `C_OgePubBase`)
2. Update QSS to use `QLabel#labelStatusBar` selector
3. Update UI files to use `C_OgeLabBase` with `objectName="labelStatusBar"`
4. Delete `C_CamOgeLabStatusBar` files

### Files to Update

#### QSS
- `can_monitor/styles/Label.qss`
- `styles/Label.qss`
- `syde_flash/styles/Label.qss`

#### UI Files (Sample)
Search for usage:
```bash
grep -r "C_CamOgeLabStatusBar" --include="*.ui" /home/tyler/Projects/openSYDE/opensyde_tool/src
```

#### C++ Files
Search for includes and usage:
```bash
grep -r "C_CamOgeLabStatusBar" --include="*.cpp" --include="*.hpp" /home/tyler/Projects/openSYDE/opensyde_tool/src
```

---

## Phase 3b: Dashboard Label Consolidation

### Target: `C_OgeLabDashboardDefault` → `QLabel#labelDashboardDefault`

**Analysis**:
- Inherits from `C_OgeLabAdaptiveSize` (which has functionality)
- Empty constructor
- Exists solely for QSS styling

**Approach**:
- Can use `C_OgeLabBase` with `objectName="labelDashboardDefault"`
- Or keep `C_OgeLabAdaptiveSize` and add objectName

---

## Consolidation Strategy

### Option 1: Single Base Class (Recommended)
```cpp
class C_OgeLabBase : public C_OgeLabToolTipBase {
    Q_PROPERTY(QString labelVariant READ GetLabelVariant WRITE SetLabelVariant)
    // Methods: SetLabelVariant(), GetLabelVariant()
};
```

**Usage in UI:**
```xml
<widget class="stw::opensyde_gui_elements::C_OgeLabBase" name="pc_LabStatus">
    <property name="objectName">
        <string notr="true">labelStatusBar</string>
    </property>
</widget>
```

**QSS:**
```css
QLabel#labelStatusBar {
    /* styling */
}
QLabel#labelDashboardDefault {
    /* different styling */
}
```

### Option 2: Variant-Based Styling
```cpp
class C_OgeLabBase : public C_OgeLabToolTipBase {
    Q_PROPERTY(QString variant READ GetVariant WRITE SetVariant)
};
```

**Usage:**
```xml
<widget class="stw::opensyde_gui_elements::C_OgeLabBase" name="pc_LabStatus">
    <property name="variant">
        <string notr="true">StatusBar</string>
    </property>
</widget>
```

**QSS:**
```css
QLabel[variant="StatusBar"] {
    /* styling */
}
```

---

## Progress Checklist

### Phase 3a: Status Bar Label
- [x] Create `C_OgeLabBase` class
- [ ] Update Label.qss files
- [ ] Update UI files using `C_CamOgeLabStatusBar`
- [ ] Update C++ files
- [ ] Delete `C_CamOgeLabStatusBar` files

### Phase 3b: Dashboard Labels
- [ ] Update QSS for dashboard labels
- [ ] Update UI files
- [ ] Delete `C_OgeLabDashboardDefault` files

---

## Expected Impact

### Code Reduction
- **Labels**: 5-6 styling-only classes → 1 base class
- **Files Deleted**: ~10-12 files (5-6 header + 5-6 implementation)

### Combined Progress (All Phases)

| Phase | Component | Classes Consolidated | Files Eliminated | Reduction |
|-------|-----------|---------------------|------------------|-----------|
| **Phase 1** | ComboBox | 4 → 1 | 8 | 75% |
| **Phase 2** | Buttons | 3 → 1 | 6 | 67% |
| **Phase 3** | Labels | 5→ 1 | ~10 | ~83% |
| **Total** | **All** | **12 → 3** | **~24** | **~89%** |

---

## Next Steps

1. **Complete Phase 3a**: Update Label.qss and UI files for status bar label
2. **Complete Phase 3b**: Consolidate dashboard labels
3. **Review remaining labels**: Identify other candidates for consolidation
4. **Document patterns**: Add to `Qt_Native_Coding_Standards.md`

---

**Status**: Foundation created (`C_OgeLabBase`), ready for QSS and UI updates
