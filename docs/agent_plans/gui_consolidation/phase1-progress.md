# Phase 1 Progress — Spin Box

## TODO
- [ ] C_OgeSpxNumber (inherits C_OgeSpxToolTipBase) -> replace with C_OgeSpxToolTipBase + styleRole="spx-number"
- [ ] C_OgeSpxInt64Properties (inherits C_OgeSpxInt64ToolTipBase) -> replace with C_OgeSpxInt64ToolTipBase + styleRole="spx-int64-properties"
- [ ] C_OgeSpxFactorProperties (inherits C_OgeSpxFactor) -> replace with C_OgeSpxFactor + styleRole="spx-factor-properties"
- [ ] C_OgeSpxInt64Table (inherits C_OgeSpxInt64ToolTipBase) -> replace with C_OgeSpxInt64ToolTipBase + styleRole="spx-int64-table"
- [ ] C_OgeSpxInt64FactorTable (inherits C_OgeSpxInt64Factor) -> replace with C_OgeSpxInt64Factor + styleRole="spx-int64-factor-table"
- [ ] C_OgeSpxDoubleProperties (inherits C_OgeSpxDoubleToolTipBase) -> replace with C_OgeSpxDoubleToolTipBase + styleRole="spx-double-properties"
- [ ] C_OgeSpxTableDouble (inherits C_OgeSpxDoubleToolTipBase) -> replace with C_OgeSpxDoubleToolTipBase + styleRole="spx-table-double"
- [ ] C_OgeSpxTableComIf (inherits C_OgeSpxToolTipBase) -> replace with C_OgeSpxToolTipBase + styleRole="spx-table-com-if"
- [ ] C_OgeSpxTable (inherits C_OgeSpxToolTipBase) -> replace with C_OgeSpxToolTipBase + styleRole="spx-table"
- [ ] C_OgeSpxFactorTable (inherits C_OgeSpxFactor) -> replace with C_OgeSpxFactor + styleRole="spx-factor-table"

## Skipped (has logic — do NOT migrate)
- C_OgeSpxEditProperties — has Q_SIGNALS, overrides focusInEvent and mousePressEvent, has private member mq_JustGotFocus
- C_OgeSpxTableHalc — constructor calls this->m_Resize(24)
- C_OgeSpxInt64AutoFix — has overrides (fixup, validate), member variables, extensive validation logic
- C_OgeSpxInt64 — has overrides (stepBy, validate, stepEnabled), member variables (mq_IsUnsigned, mu64_StepWidth, etc.)
- C_OgeSpxToolTipBase — has overrides (textFromValue, keyPressEvent, event), member variables (mpc_ContextMenu, mq_ShowSpecialMin, etc.)
- C_OgeSpxInt64ToolTipBase — has overrides (event, m_IsEnabled, etc.), member variable (mpc_ContextMenu)
- C_OgeSpxInt64Factor — has overrides (stepBy, validate), method InterpretValue to skip zero values
- C_OgeSpxFactor — has overrides (stepBy, validate, stepEnabled), static constant mhf64_FACTOR_MIN
- C_OgeSpxDoubleToolTipBase — has overrides (event, m_IsEnabled, etc.), member variable (mpc_ContextMenu)
- C_OgeSpxDoubleDynamicDecimalsBase — constructor sets decimals via this->setDecimals()
- C_OgeSpxDoubleAutoFixCustomTrigger — has Q_SIGNALS, override stepBy, method SetSuffix
- C_OgeSpxDoubleAutoFix — has overrides (fixup, validate, m_Init), member variables (mf64_ScaledMin, mf64_ScaledMax, mf64_StepWidth)
- C_OgeSpxAutoFixBase — abstract base class with virtual destructor, member variables, pure virtual m_Init
- C_OgeSpxAllBase — has overrides (m_IsEnabled, m_GetMinimumString, etc.), member variables (mc_UserMinValString, mc_UserMaxValString, etc.)

## Done
(none yet)

---

## Survey Summary
- **Total classes surveyed:** 24
- **Migratable (stylesheet-only):** 10
- **Skipped (has logic):** 14
