# QSS (Qt Style Sheets) Modernization Investigation Report

## Summary
The openSYDE project currently uses a hybrid styling approach. While an extensive QSS framework exists (`C_UtiStyleSheets` loading ~20 modular `.qss` files), the project still relies on over 70 unique C++ classes for basic label styling. This results in significant code duplication and maintenance overhead.

## Current State

### 1. Existing Infrastructure
- **Location**: `opensyde_tool/src/styles/`
- **Management**: `C_UtiStyleSheets.cpp` loads files like `CheckBox.qss`, `Label.qss`, and a 129KB `Color.qss`.
- **Method**: QSS is compiled into resources (`application.qrc`) and loaded at startup.
- **Missing Features**: No "Hot Reload" support; style changes require a full C++ recompile/relink.

### 2. The "70 Label" Problem
The project has dozens of classes like:
- `C_OgeLabAdaptiveSize`
- `C_OgeLabHeadingGroupBold`
- `C_OgeLabErrorRed` (hypothetical, but typical of the pattern)

These classes hardcode fonts, colors, and margins in C++ constructors, ignoring the existing QSS system.

## Proposed Implementation Plan (Phase 1)

### Task 2: Label Consolidation
Transition from hardcoded C++ classes to a property-driven QSS system.

1.  **Base Class**: Use `C_OgeLabBase` (or standard `QLabel` where possible).
2.  **Property Toggles**: Use Qt dynamic properties to change styles.
    ```cpp
    label->setProperty("styleVariant", "heading");
    label->setProperty("isBold", true);
    ```
3.  **QSS Styling**: Update `styles/Label.qss` to handle these properties.
    ```css
    QLabel[styleVariant="heading"] { font-size: 14pt; }
    QLabel[isBold="true"] { font-weight: bold; }
    ```

### Task 2.5: Developer "Hot Reload" Mode (NEW)
To speed up UI development, implement a style watcher.
- **Action**: Add `QFileSystemWatcher` to `C_UtiStyleSheets`.
- **Behavior**: If a `.qss` file in the source tree is modified, reload the application stylesheet instantly.
- **Benefit**: 10x faster iteration on UI design.

## Recommendation
Proceed with Task 2 as defined in the Phase 1 plan, but expand it to include the **Style Manager** improvements (Hot Reload) to ensure the team can transition away from C++ styling efficiently.

## Decision
**MODERNIZE QSS USAGE**
