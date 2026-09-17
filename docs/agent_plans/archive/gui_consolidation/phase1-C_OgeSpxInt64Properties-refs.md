# Reference scan — C_OgeSpxInt64Properties

## .ui files (0)
- (none)

## .qss files (3 files, 24 selectors)
- opensyde_tool/src/styles/SpinBox.qss (lines: 5, 31, 63, 99, 132, 191, 207, 223, 240, 257, 286)
- opensyde_tool/src/styles/Color.qss (lines: 447, 451, 634, 817, 1040, 1176, 1293, 1305, 1471, 1498, 1502, 1633, 1678, 1738)
- opensyde_tool/src/styles/Font.qss (line: 135)

## .cpp/.hpp files (1 external reference)
- libraries/opensyde_gui/src/system_definition/bus_edit/C_SdBueSignalPropertiesWidget.hpp (line 21) — `#include "C_OgeSpxInt64Properties.hpp"` (dead include — class is not referenced anywhere in the .cpp, .hpp, or generated ui_*.h)

## CMakeLists.txt files
- opensyde_tool/pjt/openSYDE/CMakeLists.txt (lines 498, 1325 — .cpp and .hpp entries)

## Lint sources files
- build_test/pclint_config/openSYDE/openSYDE_lint_sources.txt (line 330)

## Other
- No references in C_UtiStyleSheets.cpp
- No references in opensyde_can_monitor or opensyde_syde_flash CMakeLists

## Summary

| Category | Files | Lines |
|---|---|---|
| .ui | 0 | 0 |
| .qss | 3 | 26 |
| .cpp/.hpp (external) | 1 | 1 |
| CMakeLists.txt | 1 | 2 |
| Lint sources | 1 | 1 |
| **Total (excl. class's own files)** | **6** | **30** |

## Migration plan

- Replace `stw--opensyde_gui_elements--C_OgeSpxInt64Properties` with
  `stw--opensyde_gui_elements--C_OgeSpxInt64ToolTipBase[styleRole="spx-int64-properties"]`
  in all 26 .qss selectors.
- Remove the dead `#include "C_OgeSpxInt64Properties.hpp"` from
  `C_SdBueSignalPropertiesWidget.hpp:21`.
- Task C: delete class files, remove 2 CMakeLists entries, remove lint sources
  line 330.

No .ui files to update.
