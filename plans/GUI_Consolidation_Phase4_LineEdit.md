# Phase 4: LineEdit Consolidation

## Goal
Consolidate styling-only LineEdit classes into `C_OgeLeStyled` base class.

## Target Classes to Consolidate

### Styling-only (can be consolidated):
- `C_OgeLeProperties` - Empty constructor, used for general property input
- `C_OgeLePropertiesName` - Empty constructor, used for name input
- `C_OgeLeIpAddress` - Empty constructor, used for IP address input

### Already Consolidated (keep as-is):
- `C_OgeLeStyled` - Already has variant support
- `C_OgeLeContextMenuBase` - Base with context menu functionality
- `C_OgeLeToolTipBase` - Base with tooltip functionality

### Keep (have functionality):
- `C_OgeLeComboBox` - Has custom popup behavior
- `C_OgeLeFilePathBase` - Has path management logic
- `C_OgeLeListHeader` - Has focus tracking
- `C_OgeLeNavigation` - Has navigation functionality
- `C_OgeLePlaceholderVar` - Has cursor position tracking
- `C_OgeLeSearch` - Has search functionality
- `C_OgeLeTableHalc` - Has table-specific logic

## Implementation Plan

### Step 1: Update QSS
Update `LineEdit.qss` to use `QLineEdit#objectName` selectors instead of class-based selectors.

### Step 2: Update UI Files
Replace styling-only classes with `C_OgeLeStyled` and add appropriate `objectName` properties:
- `C_OgeLeProperties` → `C_OgeLeStyled` with `objectName="lineEditProperties"`
- `C_OgeLePropertiesName` → `C_OgeLeStyled` with `objectName="lineEditPropertiesName"`
- `C_OgeLeIpAddress` → `C_OgeLeStyled` with `objectName="lineEditIpAddress"`

### Step 3: Update C++ Code
Update includes and type references in C++ files.

### Step 4: Delete Obsolete Files
Remove:
- `C_OgeLeProperties.hpp/.cpp`
- `C_OgeLePropertiesName.hpp/.cpp`
- `C_OgeLeIpAddress.hpp/.cpp`

### Step 5: Update CMakeLists.txt
Remove references to deleted files from CMakeLists.txt files.

## Files to Update

### UI Files (C_OgeLeProperties):
- `can_monitor/can_monitor_settings/C_CamMosDatabaseSelectionPopup.ui`
- `com_import_export/C_RtfExportWidget.ui`
- `project_operations/C_PopCreateServiceProjDialogWidget.ui`
- `project_operations/C_PopPasswordDialogWidget.ui`
- `project_operations/C_PopSaveAsDialogWidget.ui`
- `system_definition/C_SdNodeToNodeConnectionSetupWidget.ui`
- (More files to be discovered)

### UI Files (C_OgeLePropertiesName):
- `can_monitor/can_monitor_settings/C_CamMosFilterPopup.ui`
- (More files to be discovered)

### UI Files (C_OgeLeIpAddress):
- (To be discovered)

## Expected Reduction
- 3 styling-only classes consolidated into `C_OgeLeStyled`
- Estimated 6+ files deleted (3 .hpp + 3 .cpp)
