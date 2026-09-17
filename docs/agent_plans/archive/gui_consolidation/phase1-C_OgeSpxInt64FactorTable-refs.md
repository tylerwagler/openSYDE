# Reference scan — C_OgeSpxInt64FactorTable

Replacement: `C_OgeSpxInt64Factor` + `styleRole="spx-int64-factor-table"`

## .ui files (0)
- (none)

## .qss files (3 files, 22 selectors)
- opensyde_tool/src/styles/SpinBox.qss (lines 27, 71, 93, 138, 185, 200, 215, 230, 259, 287, 416) — 11
- opensyde_tool/src/styles/Color.qss (lines 371, 436, 625, 807, 884, 1279, 1289, 1724) — 8
- opensyde_tool/src/styles/Font.qss (line 212) — 1

## .cpp/.hpp files
- libraries/opensyde_gui/src/system_views/dashboards/properties/C_SyvDaPeUpdateModeTableDelegate.cpp
  - Line 22: `#include "C_OgeSpxInt64FactorTable.hpp"` — replace with `C_OgeSpxInt64Factor.hpp`
  - Line 125: `new C_OgeSpxInt64FactorTable(opc_Parent, false)` — replace with `new C_OgeSpxInt64Factor(...)` + setProperty
  - Line 136: `new C_OgeSpxInt64FactorTable(opc_Parent, true)` — same treatment

## CMakeLists.txt
- opensyde_tool/pjt/openSYDE/CMakeLists.txt (lines 749, 1573)

## Lint sources
- build_test/pclint_config/openSYDE/openSYDE_lint_sources.txt (line 581)

## Note
Parent class `C_OgeSpxInt64Factor` has the same `(QWidget*, const bool&)`
constructor signature, and inherits the same `SetMinimum`/`SetMaximum`/
`SetMinimumCustom`/`SetMaximumCustom` methods via `C_OgeSpxInt64`, so the
type swap preserves all behavior.
