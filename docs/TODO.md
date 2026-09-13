# openSYDE TODO

Cross-cutting follow-ups that aren't tied to a specific consolidation phase.
For phase-specific work, see `docs/agent_plans/`.
For in-code `TODO` / `FIXME` / `HACK` comments tracked individually,
see `docs/code-comment-todos.md`.

## Reimplement the data-logger trigger-expression validator

`C_SdNdeDalTriggerCheckHelper` (data logger trigger conditions in the System
Definition editor) is **stubbed on every platform**: `h_Check` always reports
the expression as valid and `h_ParseDataElements` returns nothing. The target
device still validates trigger conditions at runtime — this only removes the
editor-time syntax/variable feedback.

It was stubbed because its real implementation depended on
`libraries/osy_git_data_model_monitor/`, which is **a prebuilt `.a` plus headers
with no `.cpp` source in the tree** (the archive contains a compiled
`lexer.cpp.obj`; the rest is header-only templates). A committed prebuilt binary
can't be part of a cross-platform build and can't be rebuilt for a new
compiler/ABI, so it was wired Windows-only and is now dropped from the build.
The directory is kept **only as reference material** for the rework.

To restore the feature properly (all three platforms, nothing vendored), either:

- **Reimplement from scratch** against the existing headers in
  `libraries/osy_git_data_model_monitor/includes/` — the missing piece is
  essentially the regex-based lexer (`data::monitor::Lexer`, token set in
  `data/monitor/lexer.hpp`); the expression AST (`expression.hpp/.tpp`,
  `operand.*`) is already header-only. Build it as a normal CMake library.
- **Or recover the original source** (`lexer.cpp` et al.) from wherever the
  `.a` was built and add it as a source-built library.
- **Or decompile** `libosy_git_data_model_monitor.a` to recover the lexer logic.

The pre-stub implementation is in git history (the `#else // _WIN32` arm of
`C_SdNdeDalTriggerCheckHelper.cpp` before the Windows port) and shows the exact
API surface the validator consumed (`Lexer::lex`, `ChannelDataContainer`,
`BooleanExpression::evaluate`).

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

## Show actually-linked library versions in About

The dependency *list* in `C_NagAboutDialog::InitDynamicNames` is now accurate:
gettext was removed (this fork is English-only and the i18n infrastructure is
stripped), the MinGW Runtime line is guarded by `__MINGW32__` so it never shows on
this Linux-first fork, and the libraries genuinely linked through `opensyde_core`
— TinyXML-2, Miniz, Vector::DBC/BLF, OpenSSL, zlib — are listed rather than left
commented out. Vector::DBC/BLF is GPL-3, so omitting it was the more serious
direction of error for an attribution dialog.

What remains is *versions*, not names. Only Qt reports one (`qVersion()`):

- **OpenSSL** — would have to be conditional. `opensyde_tool` links it; SYDEflash
  does not (`OPENSYDE_CORE_SKIP_SECURITY`). Query via
  `OpenSSL_version(OPENSSL_VERSION)`.
- **TinyXML-2** — `TIXML2_MAJOR_VERSION` etc. are header-only macros, but
  `tinyxml2.h` is not currently included by `opensyde_gui`, so this adds an
  include purely to print a version.
- **zlib** — `ZLIB_VERSION` from `zlib.h`, same tradeoff.
- **Miniz / Vector::DBC** — vendored; a version would have to be injected by
  CMake from the submodule.

## Remove redundant `.toStdString().c_str()`

`qstring.toStdString().c_str()` appears **445 times** (re-counted; an earlier note said 472 before `libraries/` was done, and 434 after, both of which are wrong now). It is not
a bug — no site stores the resulting pointer, so there is no dangling-pointer UB
— but it is a redundant round trip:

```
QString -> std::string   (alloc + copy)
        -> const char*   (free)
        -> std::string   (strlen + alloc + copy again)
```

### Why it is there

Upstream has the same pattern and **it was correct there**. The destinations used
to be `C_SclString`, which had an implicit constructor from `const char*` and
none from `std::string`, so `.c_str()` was the only way to assign. Phase 3
replaced `C_SclString` with `std::string`, which made the `.c_str()` redundant —
but a mechanical migration has no reason to notice, so it stayed.

There is also a latent correctness wrinkle: `.c_str()` truncates at an embedded
NUL. Harmless for paths and names; not something you want on a data field.

### Distribution

| Tree | Occurrences |
|---|---|
| `opensyde_tool` | 381 |
| `opensyde_can_monitor` | 46 |
| `libraries` | 38 — **done**, see below |
| `opensyde_syde_flash` | 7 |

### How to do it — do NOT blanket-sed

`libraries/` was completed as a trial: 38 removed, and **4 had to be restored**
because those sites genuinely need a `const char*` (`QString::replace(int, int,
const char *)` and two `QByteArray` overloads). That is roughly **1 in 10**, so a
blind sweep across the remaining 434 would break something like 40 call sites.

The compiler has to be in the loop. Two options:

1. Per-file syntax check, which is fast and needs no full build:
   ```
   QT=$(pkg-config --cflags Qt6Widgets Qt6Core Qt6Gui Qt6Svg)
   INC=$(for d in $(find libraries/opensyde_core libraries/opensyde_gui opensyde_tool/src \
         -maxdepth 4 -type d -not -path '*miniz*' -not -path '*temp_*'); do echo -n "-I$d "; done)
   g++ -fsyntax-only -std=c++17 -fPIC $QT $INC <file>
   ```
   Remove all `.c_str()` in a file, compile, restore only the lines that fail.
2. Remove in bulk and let CI find the failures. Cheaper in effort, noisier in
   history, and `opensyde_tool` at 381 sites would likely need several rounds.

Option 1 is preferred, done a directory at a time.

### Status

- `libraries/` — done (34 removed, 4 correctly kept). One file,
  `C_CieImportDbc.cpp`, could not be syntax-checked locally due to an include
  path and rests on CI.
- `opensyde_tool`, `opensyde_can_monitor`, `opensyde_syde_flash` — outstanding,
  445 sites as of the phase 5 completion.

## Windows-only code paths are never compiled

CI runs `ubuntu-26.04` only, and the remote build host is Debian, so anything
behind `#ifdef _WIN32` is not compiled by anything we run. 23 files in our own
trees carry `_WIN32` conditionals. Most guard a few lines (a Win32 API call with
a POSIX sibling), which is low risk.

One is not:

- `opensyde_tool/src/system_definition/node_edit/data_logger/C_SdNdeDalTriggerCheckHelper.cpp`
  is `#ifndef _WIN32` → a stub returning `true`, `#else` → the real
  implementation. About **1,124 of its 1,167 lines are Windows-only** and have
  never been compiled here. The Linux stub comment says the
  `osy_git_data_model_monitor` library is unavailable, so data logger trigger
  validation does not run on Linux at all — the expression is only checked on
  the device.

This bit during the `ToDouble` locale fix: the `std::stod` call and its exception
handler both live in that branch, so the edit went in unverified by any compiler.

Options, cheapest first:

1. **Accept and flag it.** Note in review that edits to that file are unbuilt.
   Free, and unreliable.
2. **Add a Windows CI job.** `windows-latest` with the MinGW toolchain the tool
   targets. Real coverage, but the Qt install is the slow part and the job would
   dominate CI time.
3. **Make the Linux stub compile the parseable parts.** Split the file so the
   pure logic (token parsing, constant conversion, syntax checks) builds
   everywhere and only the `osy_git_data_model_monitor` calls stay guarded.
   Best coverage per minute of CI, but it is a real refactor of a 1,167-line
   file.

Option 3 is the one worth doing if that file gets touched again.

## Core build warnings — done

`libraries/opensyde_core/CMakeLists.txt` now sets `-Wall -Wextra -Werror`. It
previously set no warning flags at all, while the eight tool trees had used
`-Wall -Wextra -Wpedantic` all along — so the most safety-critical code in the
repository was the only part compiled without them.

That gap hid a real defect: `C_OscViewFiler`'s nine inverted comparisons, which
swapped the debugger / authentication / traffic-encryption settings read from a
project file. GCC reports it as `-Wparentheses`, part of `-Wall`. Verified after
the fact — compiling the pre-fix file with `-Wall` emits the warning at all nine
lines.

All 45 warnings the flags surfaced are cleared:

| Was | Warning | Resolution |
|---|---|---|
| 18 | `-Winvalid-pch` | The PCH was `PUBLIC` but `_LARGEFILE64_SOURCE` was `PRIVATE`, so targets reusing the PCH did not define it and GCC discarded it — each one recompiling the whole header set. Made the define `PUBLIC` to match. |
| 26 | `-Wsign-compare` | `int32_t` loop counters against `.size()`. Cast the size rather than retyping counters. Two were `size() - 1` comparisons that also underflowed to `SIZE_MAX` on an empty container; casting before subtracting fixes that too. |
| 1 | `-Wunused-variable` | A shadowed outer declaration, never read. |

`-Werror` is on deliberately. If it ever blocks legitimate work, silence the
specific warning at its site with a comment explaining why, rather than dropping
the flag.

## Warnings as errors — done everywhere

`opensyde_core` and all eight tool trees build with `-Werror` and zero
diagnostics in code we own.

This closes the gap that hid `C_OscViewFiler`'s nine inverted security-option
comparisons: GCC reported them as `-Wparentheses`, part of `-Wall`, and the tool
builds *were* emitting that warning. Nobody read it, and core wasn't compiled
with warnings at all.

The tools apply `-Werror` at **target scope**, not through `CMAKE_CXX_FLAGS` or
`add_compile_options`:

```cmake
if(NOT MSVC)
   foreach(OSY_WERROR_TARGET openSYDE)
      if(TARGET ${OSY_WERROR_TARGET})
         target_compile_options(${OSY_WERROR_TARGET} PRIVATE -Werror)
      endif()
   endforeach()
endif()
```

Directory-scoped flags propagate into `add_subdirectory` targets, including the
vendored `can-libraries` submodule, whose flex-generated
`Vector_DBC/Scanner.cpp` emits two `-Woverloaded-virtual` warnings we neither own
nor can fix. Verified at the compile-command level that the split holds:

| | flags |
|---|---|
| `Vector_DBC/Scanner.cpp` | `-Wall -Wextra -Wpedantic` |
| openSYDE sources | `-Wall -Wextra -Wpedantic -Werror` |

**Do not "simplify" this into `CMAKE_CXX_FLAGS`** — it will break the submodule
build. `sydesuplib` is covered as well as `SYDEsup`; a couple of targets are
defined conditionally, hence the `if(TARGET ...)` guard.
