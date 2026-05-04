# openSYDE TODO

Cross-cutting follow-ups that aren't tied to a specific consolidation phase.
For phase-specific work, see `docs/agent_plans/`.

## Dark Mode

openSYDE, CAN Monitor, and SYDEflash all render with their built-in light
palette regardless of the system theme. When the host desktop is in dark
mode the apps stay bright, which is jarring at night.

### Scout findings (so we don't re-discover next time)

Existing `SetDarkTheme` / `mq_DarkMode` hooks are **not** a partial
implementation of app-wide dark mode — they're a per-dashboard-widget
render toggle for runtime visualisation (gauges, charts, tables in a
running dashboard). 30 of the 52 `mq_DarkMode` uses live in
`libraries/opensyde_gui/src/system_views/dashboards/`; the rest are
support for that. The application chrome (menus, dialogs, navigation,
toolbars) doesn't honour this flag at all.

The chrome lives in:

- **9,741 lines of `.qss`** across 63 files (split: 2,462 in
  `opensyde_tool/src/styles/Color.qss`, ~815 in each of CAN Monitor /
  SYDEflash, plus per-widget-class qss).
- **~50 named `QColor` constants** in `libraries/opensyde_gui/src/constants.hpp`,
  defined as concrete RGB tuples (no semantic role tagging — `COLOR_4`
  is `(57,57,109)` always).
- **89 files** referencing those constants in C++ (paint events, dynamic
  widget construction).

### Realistic scope

Full dark mode = re-tag palette by semantic role + provide dark
variants + add runtime theme switch (`QStyleHints::colorScheme()` in
Qt 6.5+, plus in-app override) + audit all 9,741 qss lines + every
`paintEvent` and `setBackgroundColor(QColor(...))` call in C++.
Multi-thousand-line diff, manual screen-by-screen visual QA, **2-4
weeks of focused work**.

### Less-awful intermediate options

If the night-bright pain is real but a multi-week project is out of
scope, three smaller fallbacks:

1. **System palette inversion only** (~1 day): set
   `QApplication::setPalette()` from the system palette so the
   un-styled bits Qt manages itself (some menus, scrollbars where qss
   doesn't override) follow the OS. Most chrome stays bright but the
   unstyled gaps stop clashing.
2. **Chrome only, leave dashboards** (~1 week): re-skin the main
   window / nav / dialogs but leave the dashboard widget rendering on
   its existing per-widget OPENSYDE_BRIGHT/DARK toggle. Smaller surface
   than full dark mode, still significant.
3. **Pick a less-bright default palette** (~half day): drop the
   #FFFFFF / #FAFAFA backgrounds to something readable in daylight but
   less harsh at night. Compromise — never matches OS dark mode, but
   no per-screen audit needed.

## Audit "low-value" UX features for removal

Done so far (commit `ebe6c6b0`): the Ctrl+Enter→accept handler from
`C_OgePopUpContentBase` and 28 subclass overrides got ripped out. The
handler was actively bad — it swallowed plain Enter, suppressing Qt's
standard "Enter activates the default button" and replacing it with an
undiscoverable shortcut. Stock Qt behaviour now works on all 60
migrated popups.

A quick `grep` of the rest of the codebase didn't surface another case
with the same pathology (taking away a default Qt behaviour and
replacing it with worse). The 8 other files that use the same
`q_CallOrg`/`q_CallOrig` pattern are all *adding* keyboard shortcuts to
table/tree views (Delete, `+`, arrows, Enter-to-edit) — feature
additions, not regressions.

Lower-priority code-simplification candidates from the audit:

- `C_TblViewToolTipBase::keyPressEvent` reimplements "Enter to begin
  editing" and "Enter to toggle checkbox" by hand. Both are expressible
  declaratively via `setEditTriggers(QAbstractItemView::EditKeyPressed)`
  and Qt's built-in checkable-item handling — would simplify the code
  without changing behaviour.
- The bespoke title-bar machinery (Phase 4 of the GUI consolidation
  plan) is the next big candidate for "does this earn its keep?"
  scrutiny.

## Fix application version on Linux ("V?.??r?" placeholder)

The About dialog of all three GUI apps (openSYDE, CAN Monitor,
SYDEflash) shows the literal string `V?.??r?` on Linux instead of a
real version. Root cause:
`libraries/opensyde_gui/src/util/C_Uti.cpp::h_GetApplicationVersion`
initialises the local string to that placeholder and only overwrites
it inside `#ifdef _WIN32`, where it reads `VS_FIXEDFILEINFO` from the
Win32 file version resource. The Linux branch was never wired up.

The CLI tools (`opensyde_syde_sup`, `opensyde_syde_x_gen`) already use a
different pattern: a hand-edited `version_config.hpp` with
`PROJECT_VERSION_MAJOR/MINOR/RELEASE/BUILD` macros, included directly.

Two viable fixes:

1. **Adopt the `version_config.hpp` pattern for the GUI apps too** — add
   the file (or share one across apps), reference its macros from
   `C_Uti::h_GetApplicationVersion` in a `#else` branch. Closest to the
   existing pattern; manual bumps on each release.
2. **Inject via CMake** — `project(... VERSION x.y.z)` plus
   `target_compile_definitions` in each app's CMakeLists, then
   `QApplication::setApplicationVersion(...)` in `main()` and read
   `QApplication::applicationVersion()` from `C_Uti`. Cleaner, single
   source of truth, but a wider edit.

Either way, also update each app's `*resources.rc` so the Windows
build keeps the same string the Linux build emits.

## Show actually-linked library versions in About

The About body in `C_NagAboutDialog::InitDynamicNames` originally
listed open-source dependencies as hardcoded strings, including
`"Qt 6.8.3 by The Qt Company"`. The Qt line is now dynamic
(commit `45c8b9be` — uses `qVersion()`). The remaining lines
(`gettext`, `TinyXML-2`, `The MinGW Runtime`) currently show no
version, so there's no rot risk today, but adding versions would make
the dialog more useful.

Future passes if/when they become valuable:

- **OpenSSL** — would have to be conditional. `opensyde_tool` links
  OpenSSL for security features; CAN Monitor and SYDEflash do not
  (`OPENSYDE_CORE_SKIP_SECURITY` for SYDEflash). The OpenSSL line in
  `C_NagAboutDialog.cpp` is currently commented out for the same
  reason. If exposed, query via `OpenSSL_version(OPENSSL_VERSION)`.
- **TinyXML-2** — `tinyxml2.h` lives in `libraries/opensyde_core/`
  and isn't currently included by `opensyde_gui`. Adding the include
  would create a new (small) dependency just to print a version.
  `TIXML2_MAJOR_VERSION` etc. macros are header-only.
- **gettext** — runtime version API isn't portable across all
  gettext implementations. Easiest path is a CMake-injected version
  macro from `pkg-config` / `find_package(Intl)`.
- **MinGW Runtime** — Windows-only. `__MINGW32_MAJOR_VERSION` is
  available at compile time inside `#ifdef __MINGW32__`. Linux builds
  shouldn't show this line at all.
