# Reference scan — C_CamOgeMuRecentProjects

**Class:** `stw::opensyde_gui_elements::C_CamOgeMuRecentProjects`
**Parent:** `QMenu` (Qt builtin — no app-side base)
**Replacement:** `QMenu` + `styleRole="cam-mu-recent-projects"`
**Stylesheet-only:** ✅ confirmed (empty ctor body, no overrides, no members, only public method is the ctor)

## Counts

| File type | Files | Refs |
|---|---|---|
| `.ui` | 0 | none (instance is created programmatically in `C_CamTitleBarWidget` ctor) |
| `.qss` | 6 | 18 selector lines (`Color.qss` × 6 + `Font.qss` × 1 + `Menu.qss` × 2 in each app) |
| `.cpp` / `.hpp` (excluding the class itself) | 2 | 3 lines (`C_CamTitleBarWidget.hpp` `#include` + member type; `.cpp` `new` expression) |
| Build / lint | 2 | 4 lines (both CMakeLists × .cpp + .hpp) |

## .ui files

(none — instance is constructed in `C_CamTitleBarWidget::C_CamTitleBarWidget` and never declared in a `.ui`)

## .qss files (6 files, 18 lines)

- `Color.qss` — 6 selectors per app (background, border in multiple states)
- `Font.qss` — 1 line per app
- `Menu.qss` — 2 selectors per app (root menu + `::item` selector)

Migration pattern:
```css
/* before */
stw--opensyde_gui_elements--C_CamOgeMuRecentProjects
{ ... }
stw--opensyde_gui_elements--C_CamOgeMuRecentProjects::item
{ ... }
/* after */
QMenu[styleRole="cam-mu-recent-projects"]
{ ... }
QMenu[styleRole="cam-mu-recent-projects"]::item
{ ... }
```

## .cpp / .hpp consumers

- `opensyde_can_monitor/src/can_monitor/C_CamTitleBarWidget.hpp`:
  - Replaced `#include "C_CamOgeMuRecentProjects.hpp"` with `#include <QMenu>`
  - Changed member type: `C_CamOgeMuRecentProjects * mpc_Menu;` → `QMenu * mpc_Menu;`
- `opensyde_can_monitor/src/can_monitor/C_CamTitleBarWidget.cpp`:
  - Changed initializer `mpc_Menu(new C_CamOgeMuRecentProjects)` → `mpc_Menu(new QMenu)`
  - Added `this->mpc_Menu->setProperty("styleRole", "cam-mu-recent-projects");` in the ctor body so the qss styling still applies.

## Build / lint

- `opensyde_can_monitor/pjt/CMakeLists.txt` — .cpp + .hpp entries removed
- `opensyde_syde_flash/pjt/CMakeLists.txt` — .cpp + .hpp entries removed
