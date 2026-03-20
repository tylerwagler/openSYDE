# GUI Controls Consolidation - Phase 1 Complete: ComboBox Consolidation

**Date**: 2026-03-12  
**Status**: ✅ **COMPLETE** - Phase 1 Pilot Successful  
**Classes Consolidated**: 4 → 1 (75% reduction)

---

## Summary

Successfully consolidated 4 separate ComboBox classes into a single unified `C_OgeCbxBase` class that uses Qt's `objectName` property for styling differentiation.

### Before (4 separate classes)
```cpp
class C_CamOgeCbxDark : public C_OgeCbxToolTipBase { /* sets QStyledItemDelegate */ };
class C_CamOgeCbxWhite : public C_OgeCbxToolTipBase { /* sets C_OgeCbxIconDelegate */ };
class C_CamOgeCbxTable : public C_OgeCbxToolTipBase { /* sets C_OgeCbxIconDelegate */ };
class C_CamOgeCbxTableSmall : public C_OgeCbxToolTipBase { /* sets C_OgeCbxIconDelegate */ };
```

### After (1 unified class)
```cpp
class C_OgeCbxBase : public C_OgeCbxToolTipBase {
    void SetStyledDelegate();      // For dark theme
    void SetIconDelegate();        // For table with icons
    void SetIconDelegateWithPadding(int16_t); // For table with custom padding
};

// Usage:
auto* cbx = new C_OgeCbxBase();
cbx->setObjectName("comboBoxDark");  // or "comboBoxWhite", "comboBoxTable", "comboBoxTableSmall"
cbx->SetStyledDelegate();
```

---

## Files Created

### New Helper Class
- `opensyde_gui_elements/combo_box/C_OgeCbxBase.hpp` - Base class header
- `opensyde_gui_elements/combo_box/C_OgeCbxBase.cpp` - Base class implementation

---

## Files Modified

### QSS Styling
- `can_monitor/styles/ComboBox.qss` - Updated to use `QComboBox#objectName` selectors

### UI Files (4 files)
- `can_monitor/can_monitor_settings/C_CamMosBitrateWidget.ui` - Changed to `C_OgeCbxBase` with `objectName="comboBoxDark"`
- `can_monitor/can_monitor_settings/C_CamMosLoggingWidget.ui` - Changed 2 comboboxes to `C_OgeCbxBase`
- `can_monitor/message_trace/C_CamMetControlBarWidget.ui` - Changed 2 comboboxes to `C_OgeCbxBase` with `objectName="comboBoxWhite"`
- `syde_flash/settings/C_FlaBitrateWidget.ui` - Changed to `C_OgeCbxBase` with `objectName="comboBoxDark"`

### C++ Code (2 files)
- `can_monitor/table_base/C_CamTblDelegate.cpp` - Updated to create `C_OgeCbxBase` with `SetIconDelegate()`
- `can_monitor/message_generator/signals/C_CamGenSigTableDelegate.cpp` - Updated to create `C_OgeCbxBase` with `SetIconDelegate()`

### Utility Files
- `can_monitor/util/C_CamUtiStyleSheets.cpp` - Updated QSS selector from class-based to objectName-based

---

## Files Deleted

- `can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxDark.hpp`
- `can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxDark.cpp`
- `can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxWhite.hpp`
- `can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxWhite.cpp`
- `can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxTable.hpp`
- `can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxTable.cpp`
- `can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxTableSmall.hpp`
- `can_monitor/can_monitor_gui_elements/combo_box/C_CamOgeCbxTableSmall.cpp`

---

## QSS Migration

### Before (class-based selectors)
```css
stw--opensyde_gui_elements--C_CamOgeCbxDark QListView::item { padding: 5px; }
stw--opensyde_gui_elements--C_CamOgeCbxWhite::down-arrow { border-image: ... }
stw--opensyde_gui_elements--C_CamOgeCbxTable QAbstractItemView::item { padding: 5px; }
```

### After (objectName-based selectors)
```css
QComboBox#comboBoxDark QListView::item { padding: 5px; }
QComboBox#comboBoxWhite::down-arrow { border-image: ... }
QComboBox#comboBoxTable QAbstractItemView::item { padding: 5px; }
QComboBox#comboBoxTableSmall QAbstractItemView::item { padding: 1px; }
```

---

## Benefits Achieved

1. **Code Reduction**: Eliminated 8 files (4 header + 4 implementation)
2. **Maintainability**: Single class to maintain instead of 4 nearly identical classes
3. **Qt Best Practices**: Uses Qt's objectName property system instead of custom subclasses
4. **Styling Flexibility**: QSS can target any widget by objectName without creating new classes
5. **Consistency**: Establishes pattern for consolidating other GUI control types

---

## Next Steps (Future Phases)

Following this successful pattern, similar consolidation can be applied to:

### High Priority (Styling-only classes)
- **Push Buttons**: ~15-20 classes → 2-3 base classes with objectName
- **Labels**: ~10-15 classes → 1-2 base classes
- **Line Edits**: ~5-8 styling variants → 1 base class

### Medium Priority (Some functionality)
- **Spin Boxes**: ~10-15 classes with custom behavior
- **Check Boxes**: ~5-8 classes with tooltip variations

### Implementation Guidelines
1. Identify classes that differ only by styling
2. Create base class with optional delegate/feature methods
3. Update QSS to use `widgetType#objectName` selectors
4. Update all usage sites to set `objectName` property
5. Delete obsolete classes

---

## Verification

All changes have been applied:
- ✅ New `C_OgeCbxBase` class created
- ✅ QSS updated with objectName selectors
- ✅ All UI files updated to use `C_OgeCbxBase`
- ✅ All C++ code updated to use `C_OgeCbxBase`
- ✅ Obsolete class files deleted
- ✅ No remaining references to old classes

---

**Phase 1 Status**: ✅ COMPLETE  
**Ready for**: Build verification and testing
