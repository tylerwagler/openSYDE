# ComboBox Consolidation Plan

## Current State

15 combo box classes identified:
- 9 thin wrappers that can be consolidated
- 3 specialized classes with unique functionality
- 3 base/helper classes

## Consolidation Strategy

### Create C_OgeCbxUnified

A comprehensive combo box class with 8 types:
- `eSTANDARD` - Standard combo box
- `eICON_ONLY` - Icon-only items
- `eTEXT` - Text items
- `ePARAM` - Parameter combo box
- `eTABLE` - Table combo box
- `eDASHBOARD` - Dashboard combo box
- `eFONT_PROPERTIES` - Font properties
- `eMULTI_SELECT` - Multi-select

### Classes to Eliminate (Thin Wrappers)

1. **C_OgeCbxIconOnly** (60 lines) - Just sets icon delegate
2. **C_OgeCbxTable** (78 lines) - Just applies stylesheet
3. **C_OgeCbxTableHalc** (62 lines) - Empty subclass
4. **C_OgeCbxMultiSelectTableHalc** (50 lines) - Empty subclass
5. **C_OgeCbxFontProperties** (50 lines) - Wraps QFontComboBox
6. **C_OgeCbxResizingView** (79 lines) - Just adds resize method
7. **C_OgeCbxStyled** (63 lines) - Just adds variant property
8. **C_OgeCbxText** - Text line edit (different class type)
9. **C_OgeCbxParam** (83 lines) - Has some validation, but can be unified

**Total: 9 classes to consolidate**

### Classes to Keep (Substantial Functionality)

1. **C_OgeCbxTableBase** (217 lines) - Has initialization logic for tables
2. **C_OgeCbxMultiSelect** (407 lines) - Complex multi-select implementation
3. **C_OgeCbxBase** - Base class with delegate support

### Helper Classes (Keep Separate)

1. **C_OgeCbxIconDelegate** - Item delegate for icons
2. **C_OgeCbxToolTipBase** - Tool tip base class
3. **C_OgeCbxTextLineEdit** - Text line edit (not a combo box)

## Implementation Plan

### Phase 1: Create C_OgeCbxUnified
- Implement unified combo box with all 8 types
- Support all features: delegates, initialization, styling, etc.

### Phase 2: Create Thin Wrapper Classes
- Create simple wrappers for backward compatibility (optional)
- Or update usage sites directly to use C_OgeCbxUnified

### Phase 3: Update Usage Sites
- Find all usages of thin wrapper classes
- Replace with C_OgeCbxUnified with appropriate type

### Phase 4: Remove Old Classes
- Delete eliminated wrapper classes
- Clean up includes

## Expected Results

- **Before**: 15 combo box classes
- **After**: 5-6 classes (unified + 2-3 specialized + helpers)
- **Reduction**: 60-65%

## Next Steps

1. Implement C_OgeCbxUnified
2. Create wrapper classes (optional)
3. Update usage sites
4. Delete obsolete classes
5. Test and verify
