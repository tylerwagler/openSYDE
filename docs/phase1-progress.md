# Phase 1 Progress — Spin Boxes

## TODO
- [ ] C_OgeSpxTable (inherits C_OgeSpxToolTipBase) -> replace with C_OgeSpxToolTipBase + styleRole="table"
- [ ] C_OgeSpxTableComIf (inherits C_OgeSpxToolTipBase) -> replace with C_OgeSpxToolTipBase + styleRole="table-com-if"
- [ ] C_OgeSpxTableDouble (inherits C_OgeSpxDoubleToolTipBase) -> replace with C_OgeSpxDoubleToolTipBase + styleRole="table-double"
- [ ] C_OgeSpxDoubleProperties (inherits C_OgeSpxDoubleToolTipBase) -> replace with C_OgeSpxDoubleToolTipBase + styleRole="double-properties"
- [ ] C_OgeSpxInt64Properties (inherits C_OgeSpxInt64ToolTipBase) -> replace with C_OgeSpxInt64ToolTipBase + styleRole="int64-properties"
- [ ] C_OgeSpxInt64Table (inherits C_OgeSpxInt64ToolTipBase) -> replace with C_OgeSpxInt64ToolTipBase + styleRole="int64-table"
- [ ] C_OgeSpxFactorProperties (inherits C_OgeSpxFactor) -> replace with C_OgeSpxFactor + styleRole="factor-properties"
- [ ] C_OgeSpxFactorTable (inherits C_OgeSpxFactor) -> replace with C_OgeSpxFactor + styleRole="factor-table"
- [ ] C_OgeSpxInt64FactorTable (inherits C_OgeSpxInt64Factor) -> replace with C_OgeSpxInt64Factor + styleRole="int64-factor-table"

## Skipped (has logic — do NOT migrate)
- C_OgeSpxTableHalc — constructor calls this->m_Resize(24)
- C_OgeSpxAllBase — base class with tooltip functionality
- C_OgeSpxAutoFixBase — base class with auto-fix logic
- C_OgeSpxDoubleAutoFix — has auto-fix implementation
- C_OgeSpxDoubleAutoFixCustomTrigger — has custom trigger logic
- C_OgeSpxDoubleDynamicDecimalsBase — base class with dynamic decimals
- C_OgeSpxDoubleToolTipBase — base class with tooltip and context menu
- C_OgeSpxEditProperties — overrides focusInEvent and mousePressEvent, has member variable
- C_OgeSpxFactor — overrides stepBy() and validate()
- C_OgeSpxInt64 — has int64 handling logic
- C_OgeSpxInt64AutoFix — has auto-fix implementation
- C_OgeSpxInt64Factor — overrides stepBy() and validate()
- C_OgeSpxInt64ToolTipBase — base class with tooltip and context menu
- C_OgeSpxToolTipBase — base class with tooltip and context menu

## Done
- [x] C_OgeSpxNumber (inherits C_OgeSpxToolTipBase) -> replaced with C_OgeSpxToolTipBase + styleRole="number" (commit: ba5ee9e7)
  - QSS selectors updated to use `stw--opensyde_gui_elements--C_OgeSpxToolTipBase[styleRole="number"]`
  - CMakeLists.txt updated (4-space indentation preserved)
  - Partial .ui migration: C_SdBueSignalPropertiesWidget.ui (1 of 16 files)
