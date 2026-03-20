# GUI Controls Consolidation Plan

**Created**: 2026-03-12  
**Status**: 🚧 **PLANNING PHASE** - Ready for Implementation  
**Priority**: HIGH  
**Scope**: CAN Monitor & SYDEflash GUI Components  
**Estimated Effort**: 40-60 hours (phased over 2-3 weeks)

---

## Executive Summary

The openSYDE application currently has **200+ custom GUI control classes** that exist primarily for styling purposes. This creates significant maintenance overhead and violates Qt best practices.

### Current State Problem

```cpp
// Example: Four nearly identical combobox classes
class C_CamOgeCbxDark : public C_OgeCbxToolTipBase { /* Just sets a delegate */ };
class C_CamOgeCbxWhite : public C_OgeCbxToolTipBase { /* Just sets a different delegate */ };
class C_CamOgeCbxTable : public C_OgeCbxToolTipBase { /* Just sets another delegate */ };
class C_CamOgeCbxTableSmall : public C_OgeCbxToolTipBase { /* Just sets yet another delegate */ };
```

**The Reality**: These classes have no functional differences except for styling. They exist solely to be targeted by QSS selectors.

### Target State

```cpp
// Single base class with objectName-based styling
auto* comboBox = new QComboBox();
comboBox->setObjectName("comboBoxDark");  // Styling via QSS only

// Or use Qt properties for variants
comboBox->setProperty("variant", "table");
```

---

## Scope Analysis

### GUI Element Categories

| Category | Count | Consolidation Potential |
|----------|-------|------------------------|
| **Push Buttons** | ~50 | **HIGH** - Most are styling-only |
| **Combo Boxes** | ~15 | **HIGH** - Can use properties/objectName |
| **Line Edits** | ~15 | **MEDIUM** - Some have validation logic |
| **Spin Boxes** | ~30 | **MEDIUM** - Some have custom behavior |
| **Labels** | ~20 | **HIGH** - Mostly styling variations |
| **Check Boxes** | ~8 | **MEDIUM** - Some have custom tooltip behavior |
| **Group Boxes** | ~10 | **MEDIUM** - Some have layout logic |
| **Widgets** | ~40 | **LOW** - Many contain actual functionality |
| **Tool Buttons** | ~5 | **HIGH** - Simple styling variants |
| **Scroll Areas** | ~8 | **MEDIUM** - Some have custom scrolling |
| **Menu/Tree/Tab** | ~15 | **MEDIUM** - Mix of functionality and styling |

**Total**: ~200+ classes

### High-Priority Consolidation Targets

These classes should be consolidated first (simplest wins):

#### 1. ComboBox Variants (4 classes → 1)
- `C_CamOgeCbxDark` → `QComboBox` with `objectName="comboBoxDark"`
- `C_CamOgeCbxWhite` → `QComboBox` with `objectName="comboBoxWhite"`
- `C_CamOgeCbxTable` → `QComboBox` with `objectName="comboBoxTable"`
- `C_CamOgeCbxTableSmall` → `QComboBox` with `objectName="comboBoxTableSmall"`

**Rationale**: Only difference is QSS styling and delegate assignment.

#### 2. Simple Push Buttons (10+ classes → 2-3)
- `C_CamOgePubSettingsAdd` → `QPushButton` with `objectName="buttonAdd"`
- `C_CamOgePubAbout` → `QPushButton` with `objectName="buttonAbout"`
- `C_CamOgePubProjOp` → `QPushButton` with `objectName="buttonProjectOptions"`
- `C_CamOgePubDarkBrowse` → `QPushButton` with `objectName="buttonBrowse"`

**Rationale**: Empty constructors, no custom logic.

#### 3. Label Variants (5+ classes → 1-2)
- `C_CamOgeLabStatusBar` → `QLabel` with `objectName="labelStatusBar"`

**Rationale**: Styling-only variations.

---

## Migration Strategy

### Phase 1: Foundation (Week 1)

**Goal**: Establish patterns and consolidate simplest cases

#### Tasks
1. **Create Base Documentation**
   - [x] This implementation plan
   - [ ] Update `Qt_Native_Coding_Standards.md` with GUI consolidation guidelines
   - [ ] Create QSS naming conventions document

2. **Audit All GUI Classes**
   - [ ] Review all 200+ classes
   - [ ] Categorize by consolidation potential
   - [ ] Identify classes with actual functionality vs. styling-only

3. **Create Consolidation Patterns**
   - [ ] Define objectName naming convention
   - [ ] Define property-based variant system
   - [ ] Create QSS selector patterns

4. **Pilot Consolidation** (4 ComboBox classes)
   - [ ] Remove 4 C_CamOgeCbx* classes
   - [ ] Update all .ui files to use QComboBox directly
   - [ ] Update QSS to use objectName selectors
   - [ ] Test thoroughly

### Phase 2: High-Impact Consolidation (Week 2)

**Goal**: Consolidate all styling-only button and label classes

#### Tasks
1. **Button Consolidation** (15-20 classes)
   - [ ] Identify all empty/button-only classes
   - [ ] Create `objectName` mapping
   - [ ] Update QSS selectors
   - [ ] Update all usage sites

2. **Label Consolidation** (10-15 classes)
   - [ ] Same process as buttons

3. **Update Documentation**
   - [ ] Document which classes were consolidated
   - [ ] Provide migration guide for future changes

### Phase 3: Moderate Complexity (Week 3)

**Goal**: Handle classes with some functionality

#### Tasks
1. **Spin Box Consolidation**
   - [ ] Identify common base functionality
   - [ ] Create parameterized variants using properties
   - [ ] Keep only classes with unique behavior

2. **Line Edit Consolidation**
   - [ ] Similar approach to spin boxes
   - [ ] Validate that validation logic is preserved

3. **Check Box Consolidation**
   - [ ] Preserve tooltip behavior
   - [ ] Use properties for styling variants

### Phase 4: Complex Cases & Cleanup (Week 4+)

**Goal**: Handle remaining complex cases and finalize

#### Tasks
1. **Widget Consolidation**
   - [ ] Review each widget class
   - [ ] Determine if can be replaced with standard Qt + composition
   - [ ] Keep only truly unique widgets

2. **Legacy Code Cleanup**
   - [ ] Remove consolidated class files
   - [ ] Update includes
   - [ ] Remove obsolete QSS rules

3. **Final Testing**
   - [ ] Full regression testing
   - [ ] Performance comparison
   - [ ] Documentation update

---

## Technical Approach

### Pattern 1: ObjectName-Based Styling

**Before:**
```cpp
// C_CamOgeCbxDark.hpp
class C_CamOgeCbxDark : public C_OgeCbxToolTipBase { };

// C_CamOgeCbxDark.cpp
C_CamOgeCbxDark::C_CamOgeCbxDark(QWidget * parent) :
   C_OgeCbxToolTipBase(parent)
{
   QStyledItemDelegate * delegate = new QStyledItemDelegate();
   this->setItemDelegate(delegate);
}
```

**After:**
```cpp
// No custom class needed
auto* comboBox = new QComboBox(parent);
comboBox->setObjectName("comboBoxDark");
comboBox->setItemDelegate(new QStyledItemDelegate());

// QSS
QComboBox#comboBoxDark {
   /* styling */
}
QComboBox#comboBoxDark QAbstractItemView::item {
   padding: 5px;
}
```

### Pattern 2: Property-Based Variants

**Before:**
```cpp
// Multiple button classes
class C_OgePubStyledCancel : public QPushButton { };
class C_OgePubStyledDialog : public QPushButton { };
class C_OgePubStyledMessageOk : public QPushButton { };
```

**After:**
```cpp
// Single class with property
auto* button = new QPushButton("OK");
button->setObjectName("buttonStyled");
button->setProperty("variant", "messageOk");

// QSS
QPushButton#buttonStyled[variant="messageOk"] {
   /* styling */
}
QPushButton#buttonStyled[variant="cancel"] {
   /* different styling */
}
```

### Pattern 3: Custom Delegate via Setter

For cases where custom delegates are needed:

```cpp
class C_OgeCbxBase : public QComboBox {
   Q_OBJECT
public:
   explicit C_OgeCbxBase(QWidget * parent = nullptr) : QComboBox(parent) {}
   
   void setIconDelegate() {
      setItemDelegate(new C_OgeCbxIconDelegate());
   }
   
   void setStyledDelegate() {
      setItemDelegate(new QStyledItemDelegate());
   }
};

// Usage
auto* comboBox = new C_OgeCbxBase();
comboBox->setObjectName("comboBoxTable");
comboBox->setIconDelegate();
```

---

## QSS Migration Guide

### Current QSS Pattern (Class-Based)

```css
/* ComboBox.qss */
stw--opensyde_gui_elements--C_CamOgeCbxDark QListView::item
{
   padding: 5px;
}

stw--opensyde_gui_elements--C_CamOgeCbxDark::down-arrow
{
   border-image:url(://images/IconArrowDown.svg);
}
```

### Target QSS Pattern (ObjectName-Based)

```css
/* ComboBox.qss */
QComboBox#comboBoxDark QListView::item
{
   padding: 5px;
}

QComboBox#comboBoxDark::down-arrow
{
   border-image:url(://images/IconArrowDown.svg);
}

QComboBox#comboBoxWhite::down-arrow
{
   border-image:url(://images/IconArrowDownBright.svg);
}

QComboBox#comboBoxTable QAbstractItemView::item
{
   padding-top: 5px;
   padding-bottom: 5px;
}
```

### Naming Convention

| Element Type | Prefix | Example |
|--------------|--------|---------|
| ComboBox | `comboBox` | `comboBoxDark`, `comboBoxTable` |
| PushButton | `button` | `buttonAdd`, `buttonBrowse`, `buttonAbout` |
| LineEdit | `lineEdit` | `lineEditFilePath`, `lineEditSearch` |
| SpinBox | `spinBox` | `spinBoxProperties`, `spinBoxFactor` |
| Label | `label` | `labelStatusBar`, `labelHeading` |
| CheckBox | `checkBox` | `checkBoxSettings` |
| GroupBox | `groupBox` | `groupBoxMessageSignals` |

---

## Risk Assessment

| Risk | Impact | Mitigation |
|------|--------|------------|
| **Breaking Changes** | High | Extensive testing, phased approach |
| **UI Regression** | Medium | Screenshot comparison, user acceptance testing |
| **Performance Impact** | Low | Profile before/after, Qt is optimized for this pattern |
| **Migration Complexity** | Medium | Start with simplest cases, build confidence |
| **Team Adoption** | Medium | Documentation, code reviews, examples |

---

## Success Metrics

- [ ] **70% reduction** in custom GUI classes (200+ → 60)
- [ ] **Zero styling-only classes** remaining
- [ ] **Consistent QSS patterns** across all components
- [ ] **Improved code maintainability** (measured by PR review time)
- [ ] **No functional regressions** (100% test pass rate)

---

## Implementation Checklist

### Preparation
- [ ] Read and understand all current GUI element classes
- [ ] Create backup/branch for experimentation
- [ ] Set up test environment with screenshot comparison tools

### Phase 1: Foundation
- [ ] Update Qt_Native_Coding_Standards.md with GUI guidelines
- [ ] Consolidate 4 ComboBox classes (pilot)
- [ ] Update relevant QSS files
- [ ] Test pilot changes thoroughly

### Phase 2: High-Impact
- [ ] Consolidate 15-20 button classes
- [ ] Consolidate 10-15 label classes
- [ ] Update all usage sites
- [ ] Remove obsolete files

### Phase 3: Moderate Complexity
- [ ] Consolidate spin box variants
- [ ] Consolidate line edit variants
- [ ] Consolidate check box variants
- [ ] Document patterns used

### Phase 4: Cleanup
- [ ] Review remaining complex widgets
- [ ] Final cleanup of obsolete code
- [ ] Complete documentation
- [ ] Performance testing

### Post-Implementation
- [ ] Update AGENTS.md with guidelines
- [ ] Add to code review checklist
- [ ] Team training/documentation session

---

## Resources Needed

- **Time**: 40-60 hours over 2-3 weeks
- **Access**: Full codebase, build system, test environments
- **Tools**: Qt Creator, screenshot comparison tools, git
- **Review**: Code review from 1-2 team members

---

## References

- [Qt Native Coding Standards](./02_FUTURE/Qt_Native_Coding_Standards.md) - Section 4: GUI Standards
- [Qt Style Sheets Documentation](https://doc.qt.io/qt-6/stylesheet.html)
- [Qt Object Name Property](https://doc.qt.io/qt-6/qobject.html#objectName-prop)

---

**Document Owner**: Development Team  
**Last Updated**: 2026-03-12  
**Next Review**: After Phase 1 completion
