# openSYDE TODO

Cross-cutting follow-ups that aren't tied to a specific consolidation phase.
For phase-specific work, see `docs/agent_plans/`.
For in-code `TODO` / `FIXME` / `HACK` comments tracked individually,
see `docs/code-comment-todos.md`.

## Data-logger trigger-expression validator — done

`C_SdNdeDalTriggerCheckHelper` (data logger trigger conditions in the System
Definition editor) validates expressions again, on **all three platforms**, with
nothing vendored.

It had been stubbed because the real implementation depended on
`libraries/osy_git_data_model_monitor/` — a prebuilt `.a` plus headers with no
source in the tree, which could not be part of a cross-platform build. That
directory is now **deleted**.

The lexer and grammar were reimplemented from scratch as
`C_OscDataLoggerTriggerParser` in `opensyde_core` (`project/system/node/data_logger/`),
covering the pieces openSYDE actually used — the library's event/dispatcher
framework was never used here and is gone. The grammar was recovered from the old
archive (cast pattern `^\[[A-Za-z0-9]+\]$`, dot-separated channel paths, the
comparison/logical operator set) and from the original consumer code, and is
pinned by unit tests in `tests/test_data_logger_trigger_parser.cpp`. The original
error messages ("Missing left operand for comparison operator", "Missing/Unexpected
closing parenthesis", …) are preserved.

Because the parser lives in core, it is compiled and tested on every platform —
the old implementation sat behind `#ifdef _WIN32` and was never built at all.

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

## Remove redundant `.toStdString().c_str()` — done

**436 occurrences -> 67.** 369 removed; the 63 that remain genuinely need a
`const char*` and are correct as they stand.

### Why it was there

Upstream has the same pattern and **it was correct there**. The destinations used
to be `C_SclString`, which had an implicit constructor from `const char*` and none
from `std::string`, so `.c_str()` was the only way to assign. Phase 3 replaced
`C_SclString` with `std::string`, which made the `.c_str()` redundant -- but a
mechanical migration has no reason to notice, so it stayed.

Removing it also removes a latent wrinkle: `.c_str()` truncates at an embedded NUL.
Harmless for paths and names, not something you want on a data field. Where the
round trip is gone, that truncation is gone with it.

### How it was actually done

The earlier note here proposed hand-built `g++ -fsyntax-only` per file. That was
not needed. The build system already knows the exact compile command for every
file, so the reliable method is:

1. Strip every `.toStdString().c_str()` **as bytes**, not as decoded text.
2. Build all eight tools with `ninja -k 0` -- keep-going, so one round reports
   *every* failure rather than the first.
3. Restore `.c_str()` on exactly the reported lines.
4. Rebuild.

**It converged in one round:** 54 sites restored, then all eight tools clean.

The predicted failure rate from the `libraries/` trial was about 1 in 10; the
actual was 54 of 430, which is 1 in 8 -- close enough that the warning against a
blanket sed was well founded.

### Two traps worth remembering

**Do the replacement on bytes.** The first attempt decoded each file, normalised
line endings, and wrote back. `C_FlaUpSequences.cpp` has *mixed* endings -- 440
CRLF lines and 7 LF -- so that silently converted those 7. Caught by comparing
`git diff --shortstat` against `git diff --ignore-cr-at-eol --shortstat` (431 vs
424). The search pattern contains no newline, so `raw.replace(...)` on bytes is
both simpler and immune.

**The compiler catches types, not intent.** Three sites were `QString::asprintf`,
where the argument really must be a `const char*` and dropping it would have
passed a `std::string` through varargs -- undefined behaviour that still compiles.
Those live in `C_PuiSvDbDataElementDisplayFormatter.cpp` and were excluded by hand
before the sweep rather than left to the build. (With the `printf` format attribute
now on `PrintFormattedCompat`, the equivalent mistake there *would* be caught.)

**And the Linux build is blind to `#ifdef _WIN32`.** All eight tools built clean on
Linux, and the Windows CI job then failed on four sites that pass a string to a
Win32 API (`LPCSTR`, `GetFileVersionInfoSizeA`, `GetFileVersionInfoA`) in
`C_HeHandler.cpp` and `C_Uti.cpp`. The Windows job runs `build.sh`, which does not
pass `-k 0`, so it stops early and cannot be trusted to list them all in one go.

Rather than iterate through 26-minute Windows runs, find them directly: diff the
branch, map each changed line against the `_WIN32` conditional regions of its file,
and check every hit. Validate that scanner against the sites the build already
reported before believing a zero from it.

Of the 67 kept: 54 found by the Linux compiler, 4 by the Windows compiler, 4 the
`asprintf` arguments, 3 feeding `QVariant` (which has no `std::string`
constructor), and 2 that were never this pattern at all -- `.c_str()).toStdString()`,
the reverse.

## Windows-only code paths are never compiled — done

CI now builds and tests on Linux, macOS **and** Windows (LLVM-MinGW clang), so
`#ifdef _WIN32` code is compiled by something we run. The worst offender —
`C_SdNdeDalTriggerCheckHelper.cpp`, of which ~1,124 of 1,167 lines were
Windows-only and had never been compiled — no longer has a platform split at
all: its expression handling moved into `C_OscDataLoggerTriggerParser` in core
and the file builds everywhere.

Bringing those paths under a compiler immediately turned up real latent bugs
(a lost `.` in `TglFile`, a dead `TglTasks.hpp` include, a wide `WCHAR*` used as
`std::string`, `std::string` passed where `LPCSTR` was expected, an ill-formed
`reinterpret_cast<HWND>(nullptr)`, and a committed merge-conflict marker in a
`.rc` file) — which is the argument for keeping the Windows job green.

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
