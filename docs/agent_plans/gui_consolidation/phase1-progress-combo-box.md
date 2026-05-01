# Phase 1 Progress — Combo Box

## TODO
(none)

## Skipped (has logic — do NOT migrate)

### Functional bases (overrides, public API, member state)
- `C_OgeCbxToolTipBase` — `event` + `keyPressEvent` overrides; `SetItemState` / `SetItemVisible` API; multiple-inheritance with `C_OgeToolTipBase`.
- `C_OgeCbxIconDelegate` — `QStyledItemDelegate` subclass with `paint` override and `ms16_PaddingLeft` member; not a widget.
- `C_OgeCbxMultiSelect` — `paintEvent` / `mousePressEvent` / `showPopup` / `hidePopup` overrides; signals (`SigValueChanged`); popup frame + list widget members; rich public API.
- `C_OgeCbxResizingView` — empty ctor body, but exposes public `ResizeViewToContents` method.
- `C_OgeCbxTableBase` — public API (`InitFromStringList`, `InitMinMaxAndScaling`, `GetValue`, `SetValue`); six member variables.
- `C_OgeCbxTable` — `keyPressEvent` override + ctor installs `C_OgeCbxIconDelegate`.
- `C_OgeCbxParam` — `keyPressEvent` override + ctor installs `C_OgeCbxIconDelegate` (padding 0).
- `C_OgeCbxText` (and sibling `C_OgeCbxTextLineEdit` in same TU) — `showPopup` / `hidePopup` overrides; `SigErrorFixed` signal; temporary-text member state; public `SetTemporaryText` / `SuppressHide` API.

### Classes with ctor side-effects (would be candidates if ctor body were empty)
- `C_OgeCbxFontProperties` — ctor sets `Qt::NoContextMenu` on the view's scroll bars.
- `C_OgeCbxIconOnly` — ctor creates a `C_OgeCbxIconDelegate` and calls `setItemDelegate(...)`.
- `C_OgeCbxMultiSelectTableHalc` — ctor pins `setMinimumHeight(24)` / `setMaximumHeight(24)`.
- `C_OgeCbxTableHalc` — ctor pins min/max height **and** installs an icon delegate.

## Done
(none — no migration candidates)

## Notes
All 12 classes fail the playbook bar (ctor body must be `{}`, no overrides/members, only public method is the ctor). Stylesheet selectors that target these by class name continue to work as-is — no `.ui`, `.qss`, or build changes were made for this family.

The four ctor-side-effect classes (`FontProperties`, `IconOnly`, `MultiSelectTableHalc`, `TableHalc`) could in principle be migrated by hoisting the ctor calls to the call sites (or to a small helper), but that is a behavioural refactor rather than a stylesheet-only migration and is out of scope for Phase 1.
