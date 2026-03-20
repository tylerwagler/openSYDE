# TextEdit Consolidation Analysis

## Overview

TextEdit classes have a more complex hierarchy than LineEdit/CheckBox due to specialized functionality. This analysis identifies consolidation opportunities.

## Current Classes Analysis

### Classes to KEEP (Unique Functionality)

| Class | Lines | Key Functionality |
|-------|-------|-------------------|
| `C_OgeTedContextMenuBase` | 150 | Custom context menu with undo/redo/cut/paste |
| `C_OgeTedElided` | 142 | Custom paint event for text eliding |
| `C_OgeTedTable` | 83 | Tab/Enter key handling, select-all on focus |
| `C_OgeTedPropertiesComment` | 91 | Focus out clears selection, confirm on Enter |
| `C_OgeTedToolTipContent` | 42 | Tooltip content display with size adjustment |

### Base Classes (Already Consolidated)

| Class | Lines | Notes |
|-------|-------|-------|
| `C_OgeTedBase` | 45 | Already has variant-based styling (consolidated in past refactoring) |
| `C_OgeTedToolTipBase` | 42 | Tooltip support, inherits from ContextMenuBase |

## Consolidation Opportunities

### Thin Wrapper Candidates

**`C_OgeTedPropertiesComment`** - **THIN WRAPPER**
- Comment states: "This class does not contain any functionality, but needs to exist, to have a unique group, to apply a specific stylesheet for"
- Has some keypress/focus handling but could be parameterized
- **Recommendation**: Consolidate into unified class with mode/variant

**`C_OgeTedTable`** - **NOT A THIN WRAPPER**
- Has unique Tab/Enter key handling
- Emits `SigConfirmed()` signal
- Deactivates scroll bar context menus
- **Recommendation**: Keep as-is

**`C_OgeTedElided`** - **NOT A THIN WRAPPER**
- Has complex custom paint event
- Implements text eliding logic
- **Recommendation**: Keep as-is

## Proposed Consolidation Strategy

### Option 1: Consolidate Only PropertiesComment
- Create `C_OgeTedUnified` with variant support
- Replace `C_OgeTedPropertiesComment` with `C_OgeTedUnified`
- Minimal impact, low risk

### Option 2: Full Consolidation (Recommended)
- Create `C_OgeTedUnified` with all common features
- Keep specialized classes that have unique functionality
- Pattern similar to LineEdit consolidation

### Unified Class Features

```cpp
class C_OgeTedUnified : public C_OgeTedToolTipBase
{
    Q_PROPERTY(QString textVariant READ GetTextVariant WRITE SetTextVariant)
    Q_PROPERTY(E_Mode textEditMode READ GetTextMode WRITE SetTextMode)

public:
    enum class E_Mode
    {
        STANDARD,           // Default behavior
        COMMENT,            // Property comment mode (replaces C_OgeTedPropertiesComment)
        TABLE,              // Table mode (replaces C_OgeTedTable)
        ELIDED,             // Elided text mode (replaces C_OgeTedElided)
        TOOLTIP_CONTENT     // Tooltip content mode
    };

    // Variant styling
    void SetTextVariant(const QString & orc_Variant);
    QString GetTextVariant(void) const;

    // Mode-specific functionality
    void SetEditMode(const E_Mode e_Mode);
    E_Mode GetEditMode(void) const;

    // Signals for mode-specific behavior
    void SigCommentConfirmed(void) const;  // For COMMENT mode
    void SigConfirmed(void) const;         // For TABLE mode

protected:
    void focusOutEvent(QFocusEvent * const opc_Event) override;
    void keyPressEvent(QKeyEvent * const opc_KeyEvent) override;
    void paintEvent(QPaintEvent * const opc_Event) override;
};
```

## Impact Assessment

### If Consolidating Only PropertiesComment
- **Files Deleted**: 2 (C_OgeTedPropertiesComment.hpp/.cpp)
- **Files Created**: 2 (C_OgeTedUnified.hpp/.cpp)
- **Files Modified**: 10-15 usage sites
- **Risk**: Low - isolated change

### If Full Consolidation
- **Not Recommended** - Too many specialized features that shouldn't be merged
- Eliding logic is complex and unique
- Table mode has specific key handling
- Better to keep these separate

## Recommendation

**Consolidate only `C_OgeTedPropertiesComment`**

Reasons:
1. It explicitly states it exists only for styling
2. Its functionality (focus out, key press) can be parameterized
3. Minimal risk
4. Follows pattern of other consolidations

**Keep the following separate:**
- `C_OgeTedElided` - Complex paint logic
- `C_OgeTedTable` - Specialized key handling
- `C_OgeTedToolTipContent` - Simple but separate use case

## Implementation Plan

1. Create `C_OgeTedUnified` with COMMENT mode support
2. Delete `C_OgeTedPropertiesComment`
3. Update all usage sites
4. Update stylesheets
5. Update UI files
