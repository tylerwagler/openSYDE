# Push Button Migration Progress Report

## Overview

This document tracks the migration of specialized push button classes to the unified `C_OgePubUnified` class system.

## Migration Strategy

Instead of deleting old button classes immediately, we're creating thin wrapper classes that inherit from `C_OgePubUnified`. This approach:
1. Maintains backward compatibility
2. Allows gradual migration of usage sites
3. Preserves existing stylesheet targeting via objectName
4. Makes it easy to identify and remove old code later

## Completed Wrappers

### 1. C_OgePubStandard ✅
**Purpose**: Standard material design button without borders

**Implementation**:
- Inherits from `C_OgePubUnified`
- Constructor calls `C_OgePubUnified(opc_Parent, C_OgePubUnified::eSTANDARD)`
- Removed specialized `m_SetPenColorForFont()` method (handled by QSS now)

**Files Modified**:
- `C_OgePubStandard.hpp` - Changed base class from `C_OgePubToolTipBase` to `C_OgePubUnified`
- `C_OgePubStandard.cpp` - Simplified constructor, removed paint logic

**Previous**: 74 lines → **Now**: 15 lines (80% reduction)

### 2. C_OgePubIconText ✅
**Purpose**: Button with icon and text

**Implementation**:
- Inherits from `C_OgePubUnified`
- Constructor calls `C_OgePubUnified(opc_Parent, C_OgePubUnified::eICON_TEXT)`
- Wraps `SetIconSize()` from unified class
- Default icon size: 36px

**Files Created**:
- `C_OgePubIconText.hpp` - New wrapper header
- `C_OgePubIconText.cpp` - New wrapper implementation

**Previous**: 98 lines (custom paint) → **Now**: 25 lines (75% reduction)

### 3. C_OgePubIconOnly ✅
**Purpose**: Icon-only button with hover states

**Implementation**:
- Inherits from `C_OgePubUnified`
- Constructor calls `C_OgePubUnified(opc_Parent, C_OgePubUnified::eICON_ONLY)`
- Wraps `SetPixmapIcon()` method for setting custom icons
- Hover handling delegated to unified class

**Files Created**:
- `C_OgePubIconOnly.hpp` - New wrapper header
- `C_OgePubIconOnly.cpp` - New wrapper implementation

**Previous**: 152 lines (custom event/paint handling) → **Now**: 35 lines (77% reduction)

## Pending Wrappers

### 4. C_OgePubSvgIconOnly ⏳
**Purpose**: SVG icon-only button

**Planned Implementation**:
- Inherit from `C_OgePubUnified`
- Use `eSVG_ICON` button type
- Wrap `SetSvgIcon()` method

### 5. C_OgePubSvgIconWithText ⏳
**Purpose**: SVG icon with text button

**Planned Implementation**:
- Inherit from `C_OgePubUnified`
- Use `eSVG_ICON_TEXT` button type
- Handle border rendering (complex, may need partial implementation)

### 6. Navigation Buttons ⏳
**Classes**:
- `C_OgePubBackNavigation`
- `C_OgePubPrevNavigation`
- `C_OgePubUseCase`
- `C_OgePubUseCaseNav`

**Planned Implementation**:
- Use `eNAVIGATION` button type
- May need custom hover behavior

### 7. Specialized Buttons ⏳
**Classes**:
- `C_OgePubPathVariables` - Has menu functionality
- `C_OgePubTabClose` - Tab close button
- `C_OgePubColor` - Color picker
- `C_OgePubGenericTextWithBackground` - Colored background
- `C_OgePubStyled` - Style-based button
- `C_OgePubDashboard` - Dashboard button
- `C_OgePubOpen`, `C_OgePubOptions`, `C_OgePubUpdate`, etc.

**Strategy**:
- Keep as separate classes if they have unique functionality
- Convert to wrappers if they're just styling variants

## Migration Benefits

### Code Reduction
- **Before**: 30 button classes
- **After**: ~10 classes (1 unified + 9 specialized wrappers)
- **Reduction**: 65-70%

### Maintainability
- Single source of truth for button behavior
- Easier to add new button types
- Simplified testing

### Flexibility
- Configuration-based customization instead of inheritance
- Runtime button type changes possible
- Consistent API across all button types

## Next Steps

1. **Complete remaining wrappers**
   - Create SVG button wrappers
   - Create navigation button wrappers
   - Assess which specialized buttons need to remain separate

2. **Update usage sites**
   - Identify files using old button classes
   - Update to use new wrappers (or unified class directly)
   - Test functionality

3. **Remove old implementations**
   - Delete original specialized button files
   - Remove duplicate code
   - Update documentation

4. **Testing**
   - Verify all button types render correctly
   - Test hover states and interactions
   - Confirm stylesheet application

## Technical Notes

### ObjectName-Based Styling
The unified button uses objectName for stylesheet targeting:
```cpp
this->setObjectName("C_OgePubUnified");
```

Wrappers can set custom objectNames to maintain existing QSS:
```cpp
// In wrapper constructor:
this->setObjectName("C_OgePubStandard");  // For backward compatibility
```

### Icon Handling
Icons are managed through the unified class:
```cpp
// Set pixmap icons
this->SetPixmapIcon(normal, hovered, clicked, disabled);

// Set SVG icons
this->SetSvgIcon(pathEnable, pathDisable);
```

### Hover Support
Hover events are automatically handled for icon-only and navigation buttons:
```cpp
this->setAttribute(Qt::WA_Hover, true);
```

## Conclusion

The migration strategy is working well. We've successfully:
1. Created a comprehensive unified button class
2. Demonstrated the wrapper pattern with 3 examples
3. Achieved significant code reduction (75-80%)

The remaining work involves creating similar wrappers for other button types and updating usage sites across the codebase.
