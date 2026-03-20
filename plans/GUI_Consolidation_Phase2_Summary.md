# GUI Controls Consolidation - Phase 2: Button & Label Consolidation (In Progress)

**Date**: 2026-03-12  
**Status**: 🚧 **IN PROGRESS** - Foundation Complete  
**Target**: Consolidate styling-only button and label classes

---

## Phase 2 Overview

Following the successful Phase 1 ComboBox consolidation, Phase 2 targets the next largest category of redundant GUI classes: **push buttons** and **labels**.

### Current State Analysis

#### Push Buttons (35+ classes)
- **CAN Monitor specific**: 6 classes (`C_CamOgePub*`)
- **General purpose**: 29+ classes (`C_OgePub*`)

Many of these classes are styling-only wrappers with no functional differences.

#### Labels (21+ classes)
- **CAN Monitor specific**: 1 class (`C_CamOgeLabStatusBar`)
- **General purpose**: 20+ classes (`C_OgeLab*`)

---

## Phase 2a: Push Button Consolidation

### Target Classes for Consolidation

#### High Priority (Styling-only, empty constructors)
1. `C_CamOgePubSettingsAdd` → `QButton#buttonAddSettings`
2. `C_CamOgePubAbout` → `QButton#buttonAbout`
3. `C_CamOgePubDarkBrowse` → `QButton#buttonDarkBrowse`
4. `C_CamOgePubPathVariables` → `QButton#buttonPathVariables`
5. `C_CamOgePubPathVariablesBase` → `QButton#buttonPathVariablesBase`

#### Medium Priority (Some custom functionality)
6. `C_CamOgePubProjOp` - Has custom SVG rendering (KEEP as-is or extend base)

### Completed Work

#### 1. Created `C_OgePubBase` Class
**Files:**
- `opensyde_gui_elements/push_button/C_OgePubBase.hpp`
- `opensyde_gui_elements/push_button/C_OgePubBase.cpp`

**Features:**
- Inherits from `C_OgePubToolTipBase` (preserves tooltip functionality)
- Q_PROPERTY for variant-based styling
- Methods: `SetButtonVariant()`, `GetButtonVariant()`

#### 2. Updated QSS Styling
**File:** `can_monitor/styles/PushButton.qss`

**Before:**
```css
stw--opensyde_gui_elements--C_CamOgePubSettingsAdd { padding: 1px; }
stw--opensyde_gui_elements--C_CamOgePubDarkBrowse { border-style: solid; }
stw--opensyde_gui_elements--C_CamOgePubAbout { border-style: solid; }
```

**After:**
```css
QPushButton#buttonAddSettings { padding: 1px; }
QPushButton#buttonDarkBrowse { border-style: solid; }
QPushButton#buttonAbout { border-style: solid; }
```

### Remaining Work

#### 3. Update UI Files
Target files to update:
- `can_monitor/C_CamTitleBarWidget.ui` - Uses `C_CamOgePubProjOp`, `C_CamOgePubAbout`
- `can_monitor/can_monitor_settings/C_CamMosDatabaseWidget.ui` - Uses `C_CamOgePubSettingsAdd`
- `can_monitor/can_monitor_settings/C_CamMosFilterWidget.ui` - Uses `C_CamOgePubSettingsAdd`
- `can_monitor/can_monitor_settings/C_CamMosLoggingWidget.ui` - Uses `C_CamOgePubDarkBrowse`
- `can_monitor/can_monitor_settings/C_CamMosWidget.ui` - Uses `C_CamOgePubPathVariables`

#### 4. Update C++ Code
Files that create buttons programmatically need updating.

#### 5. Delete Obsolete Classes
- `C_CamOgePubSettingsAdd.*`
- `C_CamOgePubAbout.*`
- `C_CamOgePubDarkBrowse.*`
- `C_CamOgePubPathVariables.*`
- `C_CamOgePubPathVariablesBase.*`

---

## Phase 2b: Label Consolidation (Not Started)

### Target Classes

#### High Priority (Styling-only)
1. `C_CamOgeLabStatusBar` → `QLabel#labelStatusBar`
2. `C_OgeLabStyled` → `QLabel#labelStyled`
3. `C_OgeLabDashboardDefault` → `QLabel#labelDashboardDefault`
4. `C_OgeLabPopUpTitle` → `QLabel#labelPopUpTitle`
5. `C_OgeLabPopUpSubTitle` → `QLabel#labelPopUpSubTitle`

#### Medium Priority (Some functionality)
- `C_OgeLabElided` - Has eliding text functionality (KEEP or extend)
- `C_OgeLabDoubleClick` - Has double-click handling (KEEP)
- `C_OgeLabExternalLink` - Has link handling (KEEP)

### Approach
Similar to buttons:
1. Create `C_OgeLabBase` class with variant property
2. Update QSS to use `QLabel#objectName` selectors
3. Update UI files
4. Delete styling-only classes

---

## Benefits Expected

### Code Reduction
- **Buttons**: 5 classes → 1 (`C_OgePubBase`)
- **Labels**: 5+ classes → 1 (`C_OgeLabBase`)
- **Total**: ~10 files deleted

### Maintainability
- Single class per control type instead of styling variants
- QSS-driven styling without code changes
- Consistent patterns across the application

### Alignment with Qt Best Practices
- Uses Qt's objectName property system
- Declarative styling in QSS
- Reduced class hierarchy complexity

---

## Progress Checklist

### Phase 2a: Buttons
- [x] Audit all button classes
- [x] Create `C_OgePubBase` helper class
- [x] Update QSS PushButton.qss
- [ ] Update UI files (5 files)
- [ ] Update C++ code
- [ ] Delete obsolete classes (5 pairs)
- [ ] Build and test

### Phase 2b: Labels
- [ ] Audit all label classes
- [ ] Create `C_OgeLabBase` helper class
- [ ] Update QSS Label.qss
- [ ] Update UI files
- [ ] Update C++ code
- [ ] Delete obsolete classes
- [ ] Build and test

---

## Next Steps

1. **Complete Phase 2a**: Update remaining UI files and delete obsolete button classes
2. **Begin Phase 2b**: Create label base class and update QSS
3. **Document patterns**: Update `Qt_Native_Coding_Standards.md` with consolidation guidelines

---

**Status**: Foundation complete, ready for UI file updates
