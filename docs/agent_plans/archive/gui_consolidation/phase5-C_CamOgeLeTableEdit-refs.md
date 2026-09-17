# Reference scan — C_CamOgeLeTableEdit

**Class:** `stw::opensyde_gui_elements::C_CamOgeLeTableEdit`
**Parent:** `C_TblEditLineEditBase`
**Replacement:** `C_TblEditLineEditBase` + `styleRole="cam-le-table-edit"`
**Stylesheet-only:** ✅ confirmed (empty ctor body, no overrides, no members, only public method is the ctor)

## Counts

| File type | Files | Refs |
|---|---|---|
| `.ui` | 0 | none (instances are created programmatically by `C_CamTblDelegate::m_CreateLineEdit`) |
| `.qss` | 6 | 14 selector lines (`Color.qss` × 5 + `Font.qss` × 1 + `LineEdit.qss` × 1 in each app) |
| `.cpp` / `.hpp` (excluding the class itself) | 1 | 2 lines in `C_CamTblDelegate.cpp` (`#include` + `new` call) |
| Build / lint | 2 | 4 lines (both CMakeLists × .cpp + .hpp) |

## .ui files

(none — class is only instantiated programmatically as a delegate-created editor inside `C_CamTblDelegate`)

## .qss files (6 files, 14 lines)

Same shape as the other line-edit migrations:
- `Color.qss` — 5 selectors per app
- `Font.qss` — 1 line per app (bundled font selector)
- `LineEdit.qss` — 1 selector per app (border, padding)

Migration pattern:
```css
/* before */
stw--opensyde_gui_elements--C_CamOgeLeTableEdit,
/* after */
stw--opensyde_gui--C_TblEditLineEditBase[styleRole="cam-le-table-edit"],
```

## .cpp / .hpp consumers

- `opensyde_can_monitor/src/can_monitor/table_base/C_CamTblDelegate.cpp` — replaced `#include "C_CamOgeLeTableEdit.hpp"` with `#include "C_TblEditLineEditBase.hpp"`, and rewrote `m_CreateLineEdit` from `return new C_CamOgeLeTableEdit(opc_Parent);` to:
  ```cpp
  C_TblEditLineEditBase * const pc_LineEdit = new C_TblEditLineEditBase(opc_Parent);
  pc_LineEdit->setProperty("styleRole", "cam-le-table-edit");
  return pc_LineEdit;
  ```
  The `setProperty` call replaces what the old subclass got "for free" via class-name qss matching.

## Build / lint

- `opensyde_can_monitor/pjt/CMakeLists.txt` — .cpp + .hpp entries removed
- `opensyde_syde_flash/pjt/CMakeLists.txt` — .cpp + .hpp entries removed
