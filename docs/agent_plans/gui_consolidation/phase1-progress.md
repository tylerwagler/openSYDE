# Phase 1 Progress — Check Box

## TODO (migrate in this order; dependency-aware)
- [ ] C_OgeChxPropertiesSmall (inherits C_OgeChxProperties [also stylesheet-only]) -> replace with C_OgeChxToolTipBase + styleRole="chx-properties-small" (MUST be done BEFORE Properties since its .hpp includes Properties.hpp)
- [ ] C_OgeChxDefaultCheckedDisabled (inherits C_OgeChxToolTipBase) -> replace with C_OgeChxToolTipBase + styleRole="chx-default-checked-disabled"
- [ ] C_OgeChxProperties (inherits C_OgeChxToolTipBase) -> replace with C_OgeChxToolTipBase + styleRole="chx-properties"
- [ ] C_OgeChxSystemCommisioningEdit (inherits C_OgeChxToolTipBase) -> replace with C_OgeChxToolTipBase + styleRole="chx-system-commisioning-edit"
- [ ] C_OgeChxTiny (inherits C_OgeChxToolTipBase) -> replace with C_OgeChxToolTipBase + styleRole="chx-tiny"
- [ ] C_OgeChxTristate (inherits C_OgeChxTristateBase) -> replace with C_OgeChxTristateBase + styleRole="chx-tristate"
- [ ] C_OgeChxTristateTransparentError (inherits QCheckBox directly) -> replace with QCheckBox + styleRole="chx-tristate-transparent-error"
- [ ] C_OgeChxTristateTransparentToggle (inherits C_OgeChxTristateBase) -> replace with C_OgeChxTristateBase + styleRole="chx-tristate-transparent-toggle"

## Skipped (has logic — do NOT migrate)
- C_OgeChxDefaultSmall — has Q_SIGNALS, overloaded constructor, member vars (mu32_Index, mu32_SubIndex)
- C_OgeChxToolTipBase — base class with event() override
- C_OgeChxTristateBase — base class with resizeEvent() override

## Done
(none yet)

---

## Survey Summary
- **Total classes surveyed:** 11
- **Migratable (stylesheet-only):** 8
- **Skipped (has logic):** 3

## Notes
- `C_OgeChxPropertiesSmall` inherits from `C_OgeChxProperties`, which is ALSO
  stylesheet-only. When migrating, the replacement for PropertiesSmall uses the
  grandparent `C_OgeChxToolTipBase` (since the direct parent Properties will
  also be deleted). PropertiesSmall MUST be migrated and deleted first to
  avoid a transient build break where PropertiesSmall.hpp includes a missing
  Properties.hpp.
- Per past family conventions (spin boxes used `spx-` prefix, labels used
  context prefixes), I'm prefixing these with `chx-` for QSS readability.
