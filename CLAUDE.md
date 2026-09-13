# CLAUDE.md

Guidance for Claude Code (claude.ai/code) when working in this repository.

**Branch note:** this file describes the `develop` branch. The `dev` branch is a
separate, divergent line of work (Qt-centric: `C_SclString` → `QString`, STL → Qt
containers, a JSON-only filer rewrite, Windows/PowerShell builds). The two branches
share only `Release 35` as a merge base — do not carry conventions, plans, or build
instructions between them.

## Project Overview

openSYDE is an open-source development environment by STW (Sensor-Technik Wiedemann
GmbH) for implementing, commissioning, analyzing, and maintaining control systems for
mobile machines. Qt6 C++ application; several tools share a common core library.

This fork is an **internal English-only fork** targeting **Linux first**. The gettext /
i18n infrastructure has been stripped.

## Build System

CMake + Ninja, driven by `build.sh` at the repo root. **Clang on all three platforms**
— the project targets Linux, macOS and Windows and we ship binaries for each.

```bash
./build.sh                      # Build all tools (Release)
./build.sh -b Debug all         # Debug build of everything
./build.sh opensyde canmonitor  # Build specific tools
./build.sh -b Debug sydesup     # Debug build of one tool
./build.sh -j 8 all             # Limit parallelism
./build.sh -d opensyde          # Build and deploy to $INSTALL_DIR
./build.sh -h                   # Full option list
```

Tool names: `opensyde`, `canmonitor`, `sydeflash`, `sydesup`, `syde_x_gen`,
`syde_coder_c`, `flash_tool`, `tsp_convert`, `all`.

`build.sh` requires plain `cmake`, `ninja` and `clang++` on `PATH` — it never pins a
compiler version. It picks the toolchain file by `uname`: `cmake/toolchain_macos.cmake`
on Darwin, `cmake/toolchain_windows.cmake` under MinGW/MSYS/Cygwin, otherwise
`cmake/toolchain_linux.cmake`. It also uses `getconf _NPROCESSORS_ONLN` rather than
`nproc` so job detection works on macOS, which has no `nproc`.

### One compiler, three platforms

All three toolchain files select `clang`/`clang++`. The standard library does **not**
match across them, and that is deliberate rather than an oversight:

| Platform | Compiler | Standard library |
|----------|----------|------------------|
| Linux | clang 19 | **libstdc++** |
| macOS | Apple clang 21 | libc++ |
| Windows | LLVM-MinGW clang 17 | libc++ |

Linux stays on libstdc++ because the system Qt6 is built against it. `opensyde_core`
is one static archive shared by all eight tools, so it has to match whatever Qt the
GUI tools link — mixing libc++ core with libstdc++ Qt is an ABI mismatch at every
`std::string` that crosses the boundary. "libc++ for the CLI tools, libstdc++ for the
GUI" is not available for the same reason: there is only one core.

The consequence to remember: **libc++ and libstdc++ diverge, and CI is where you find
out.** The one that has already bitten this tree is floating-point `std::from_chars`,
which libc++ 17 and 19 declare but delete. Code that needs it must guard on
`__cpp_lib_to_chars` and fall back to `strtod_l` / `_strtod_l` against an explicit
`"C"` locale — plain `strtod` is locale-dependent and fails the round-trip tests.

The root `CMakeLists.txt` builds everything as one project: `opensyde_core` once,
then each tool linking that single archive. `build.sh` drives it — one configure,
then one `--target` per requested tool.

This replaced eight separate per-tool builds, each compiling its own core copy.
A clean `all` went from **1,139 core object files across eight archives** to 220 in
one, and from roughly three minutes to **2m23s** on the 48-core host.

It is safe because `OPENSYDE_CORE_SKIP_*` are pure source selection — no
preprocessor defines, no `#ifdef` on them anywhere — and core is a static library,
so each tool still links only what it references. Verified by comparing `ldd` for
all eight binaries before and after: every tool links exactly the same libraries,
and CAN Monitor still has no `libcrypto` despite core now compiling with security
enabled. **That is the invariant to re-check if this changes.**

Each tool's own `pjt/CMakeLists.txt` still works standalone: it guards its core
`add_subdirectory` with `if(NOT TARGET opensyde_core)`, so configuring a tool
directly takes the old path with its own `SKIP_*` set.

`build.sh` configures GUI tools only when one is requested, so `./build.sh sydesup`
still works on a machine without Qt6. Switching between GUI and non-GUI tool sets
re-runs CMake, which is why an alternating sequence reconfigures each time.

### Building the core library directly

```bash
cmake -S libraries/opensyde_core -B build/core -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DOPENSYDE_CORE_BUILD_TESTS=ON \
  -DOPENSYDE_CORE_SKIP_WINDOWS_DRIVERS=ON \
  -DOPENSYDE_CORE_SKIP_WINDOWS_TARGET=ON
cmake --build build/core -j $(nproc)
ctest --test-dir build/core --output-on-failure
```

`OPENSYDE_CORE_SKIP_*` options gate individual subsystems; the two above are required
on Linux because those subsystems are Windows-only.

### Prerequisites

Submodules are required — `libraries/can-libraries` (which itself vendors
`Vector_ASC`, `Vector_BLF`, `Vector_DBC`):

```bash
git submodule update --init --recursive
```

On Ubuntu 24.04 / 26.04:

```
cmake ninja-build clang qt6-base-dev qt6-svg-dev qt6-tools-dev
libgl1-mesa-dev libssl-dev flex bison libfl-dev
```

QCustomPlot is **not** a system package — it is vendored and built from source on
every platform (see the QCustomPlot gotcha below), so `libqcustomplot-dev` is no
longer required.

On macOS (Homebrew), the keg-only deps are pointed at by `cmake/toolchain_macos.cmake`:

```
brew install cmake ninja bison flex openssl@3 qtbase qtsvg
```

On Windows, run from Git Bash / MSYS so `uname` reports `MINGW*`, with LLVM-MinGW
(clang + libc++), Ninja, Qt6 for `win64_llvm_mingw`, OpenSSL v3, and winflexbison
on `PATH`. Nothing is vendored: zlib is fetched from source at configure time
(`FetchContent` with `OVERRIDE_FIND_PACKAGE`) because there is no system copy.
See `.github/workflows/build.yml` for the exact versions CI installs.

Notes that have bitten builds before:

- **`flex` alone is not enough.** `FlexLexer.h` ships in `libfl-dev` on Ubuntu and in
  the `flex` keg on macOS. Without it CMake leaves `FLEX_INCLUDE_DIR` unset and
  `Vector_DBC` fails to generate its scanner. `opensyde_core` links
  `CanLibraries::dbc`, so this is not optional. `toolchain_macos.cmake` sets
  `FLEX_INCLUDE_DIR` at the brew keg for this reason.
- **macOS `bison` is too old.** Apple ships bison 2.3; `Vector_DBC` needs ≥ 3.3.
  `toolchain_macos.cmake` points `BISON_EXECUTABLE` at brew's bison 3.8.
- **The `qt` meta-formula can fail on Intel Macs**, but the component kegs (`qtbase`,
  `qtsvg`) install fine and provide everything openSYDE needs; the toolchain adds all
  three to `CMAKE_PREFIX_PATH`.
- **Package names differ across Ubuntu releases.** Qt6 SVG dev is `qt6-svg-dev` from
  noble (24.04) onward, but `libqt6svg6-dev` on jammy (22.04). Jammy also has no
  `g++-13`. CI does not target jammy for these reasons.

### Verifying tool-tree changes

`opensyde_core` builds on its own and does **not** compile the seven sibling tool
trees, so a clean core build proves nothing about them. Any change to a signature
in core must be checked with a full eight-tool build. Grep is not a substitute:
callers reach migrated classes through base-class pointers, share method names
with unmigrated classes, and compare rather than assign.

Two ways to get that, and they answer different questions:

**CI** covers all three platforms and is the only thing that proves a change builds
on Windows and macOS, so anything touching platform-conditional code or a core
signature goes through a PR rather than a local build. Push and read the result.

**The 48-core build host** is for a fast single-platform answer while iterating —
~3 minutes for all eight tools against ~18 on a laptop. See `docs/remote-build.md`.

```bash
ssh claude@claude 'cd ~/Projects/openSYDE && git fetch origin && git checkout develop \
  && git pull --ff-only origin develop && git submodule update --init --recursive \
  && ./build.sh -b Debug -j 48 all'
```

The Mac has no push credentials; pushes route through that host. See
`docs/remote-build.md` for the bundle workflow that preserves interdependent
submodule SHAs.

### Build outputs

Per-tool build directories under `build/`; deploy target defaults to
`~/.local/opt/openSYDE` (override with `INSTALL_DIR`).

## Testing

Google Test + CTest, in `libraries/opensyde_core/tests/`. Enable with
`-DOPENSYDE_CORE_BUILD_TESTS=ON`, run with `ctest`.

21 suites, 265 tests: `test_application_info_block`, `test_checksums`,
`test_cstdint`, `test_data_logger_trigger_parser`, `test_datapool_content_util`,
`test_dynamic_array`, `test_hex_file`, `test_hex_string_parsing`, `test_logging`,
`test_node_datapool_content`, `test_osc_error_category`, `test_project_metadata`,
`test_protocol_driver_base`, `test_protocol_serial_number`, `test_scl_string`,
`test_security_aes_file`, `test_stwerrors`, `test_tgl_file`, `test_tgl_time`,
`test_view_security_options`, `test_xml_parser`.

The suite runs on all three platforms, so anything it touches must avoid
platform-specific assumptions — `XmlParser.SaveAndLoadFile` hardcoded `/tmp/` and
had to move to `std::filesystem::temp_directory_path()` when Windows joined CI.

### Benchmarks

Google Benchmark, in `libraries/opensyde_core/bench/`. Enable with
`-DOPENSYDE_CORE_BUILD_BENCHMARKS=ON`, which builds a `bench_opensyde` target.

**Always measure in Release.** Phase 7.1 recorded "`std::format` is slower than
`std::stringstream`" as a finding and nearly abandoned the change on it. That was a
Debug artifact: unoptimized, `std::format`'s template machinery looks terrible and
the conclusion inverts. In Release it is 2.8x–12.2x faster. A Debug benchmark number
is not a slow measurement of the right thing, it is a measurement of a different
program.

Measure attribution before optimizing, and A/B by keeping both binaries and running
them alternately — these machines drift enough between runs (~15%) that a before
number taken an hour earlier is not comparable to an after number taken now.

Several of these are regression pins for defects the phase 2/3 migrations
introduced and CI did not catch, so prefer extending them over replacing them.

## CI

`.github/workflows/build.yml`, on push/PR to `main`, `develop`, `master`. **All
three target platforms build on every push:**

| Job | Runners | What it does | Wall clock |
|-----|---------|--------------|------------|
| **Core Library** | `ubuntu-26.04`, `macos-14` | Full subsystem coverage, builds, `ctest` | ~1.5 min |
| **GUI Tools** | `ubuntu-26.04`, `macos-14` | Smoke-builds all eight tools (`./build.sh -b Debug all`), then checks one tool still configures standalone | 12–20 min |
| **Tools + Core Tests** | `windows-2022` | All eight tools under LLVM-MinGW clang, plus the core unit tests | ~35 min |

No job is `continue-on-error`: they are the only targets that have to gate, or CI
stops meaning anything.

macOS Qt comes from `install-qt-action`, not Homebrew. `brew install qt` drags
`qtwebengine` into a source build, and the split `qtbase`/`qtsvg` kegs hid Svg from
`find_package`. Windows likewise installs Qt (`win64_llvm_mingw`), the toolchain
(`tools_llvm_mingw1706`), Ninja and OpenSSL through the same action.

`.github/workflows/bench.yml` is separate and `workflow_dispatch`-only: the Google
Benchmark harness across the same three OSes, in **Release**. GitHub only offers
`workflow_dispatch` for workflows present on the **default branch**, so a new
benchmark workflow has to reach `master` before it can be dispatched.

Keep CI green. The core job was red from the moment it was introduced until
2026-09-06, and that gap is precisely how phase-2 and phase-3 migration residue
(`GetLength()` on `std::vector`, mangled `std::stoi` artifacts) survived unnoticed.
A job that does not run is not a quality gate.

The Windows job exists because `#ifdef _WIN32` code used to be compiled by nothing
we ran — 28 files carry `_WIN32` conditionals. Bringing them under a compiler
immediately turned up a lost `.` in `TglFile`, a dead `TglTasks.hpp` include, a wide
`WCHAR*` used as a `std::string`, an ill-formed `reinterpret_cast<HWND>(nullptr)`
and a committed merge-conflict marker in a `.rc` file. See `docs/TODO.md`.

## Repository Layout

| Path | Purpose |
|------|---------|
| `libraries/opensyde_core/` | Shared core library (non-GUI logic), tests |
| `libraries/opensyde_gui/` | Shared GUI library |
| `libraries/can-libraries/` | Submodule: CAN backends + Vector DBC/BLF/ASC |
| `libraries/qcustomplot/` | Vendored QCustomPlot |
| `opensyde_tool/` | Main openSYDE GUI application |
| `opensyde_can_monitor/` | CAN Monitor |
| `opensyde_syde_flash/` | SYDEflash |
| `opensyde_syde_sup/` | SYDEsup (CLI system updater) |
| `opensyde_syde_x_gen/`, `opensyde_syde_coder_c/` | Generators |
| `opensyde_cmd_line_flash_tool/`, `opensyde_tsp_convert/` | CLI utilities |
| `docs/` | TODOs and agent plans |

Core library internals: `project/system/` (nodes, buses, definitions, and
`node/data_logger/` with the trigger-expression parser), `halc/`,
`protocol_drivers/`, `data_dealer/`, `exports/`, `xml_parser/`, `scl/`, `security/`,
`logging/`, `stwtypes/` (C header only, for generated controller code),
`stwerrors/`, plus `tests/` and `bench/`.

**Nothing prebuilt is vendored.** `libraries/osy_git_data_model_monitor/` — a `.a`
plus headers with no source — was the last one and is deleted; the functionality
openSYDE used from it was reimplemented as `C_OscDataLoggerTriggerParser`. Vendored
*source* (QCustomPlot, the CAN libraries submodule) is fine; a binary that cannot be
rebuilt for three platforms is not.

## Coding Conventions

### Naming

- **Classes:** prefix `C_` (e.g. `C_OscUtils`, `C_OscSystemDefinition`)
- **GUI classes:** `C_` + feature abbreviation + type (e.g. `C_CamOgeChxSettings` =
  CAN Monitor / OpenSYDE Gui Element / Checkbox)
- **Namespaces:** `stw::opensyde_core`, `stw::opensyde_gui_logic`, `stw::errors`

### Parameter prefixes (STW convention)

- `o` = object/reference, `q` = boolean
- `u8`/`u16`/`u32`/`u64` unsigned, `s8`/`s16`/`s32`/`s64` signed
- `f32`/`f64` float/double
- `c` = container, `pc`/`opc` = pointer to container, `rc`/`orc` = reference to container

Example: `const uint16_t ou16_Length` = "object, unsigned 16-bit, Length".

### Types

Use `<cstdint>` fixed-width types (`int8_t` … `uint64_t`) for integers and the native `float`,
`double`, and `char` for floats/characters. The `stwtypes.hpp` C++ header has been removed;
`stwtypes.h` (C, with the legacy no-`_t` names) remains only as the contract for generated
controller C code, to be retired when that code is converted to C++.

### Strings and containers

`std::string` and `std::vector`. `C_SclString` and `C_SclDynamicArray` have been
**removed** — do not reintroduce them. `C_SclStringUtil.hpp` (formerly `C_SclStringCompat.hpp`, retired from its transitional
framing 2026-09-08) provides the surviving string
helpers; prefer idiomatic `std::string` in new code and do not add new compat helpers.
The compat header's numeric/float helpers are now `std::to_string`/`std::format` and it no
longer transitively provides `<iomanip>`/`<locale>` — consumers must include
what they use directly.
`C_SclStringList` has been **retired** (phase 3 follow-up); its string-list
helpers in `C_SclStringUtil.hpp` (`ListLoadFromFile`, `ListSaveToFile`, `ListGetText`,
`ListIndexOfName`, `ListAddStrings`) operate on `std::vector<std::string>` and are the
surviving API. `C_SclIniFile`, `C_SclChecksums`, and `C_SclDateTime` still exist.

### Error handling

`stwerrors.hpp` in `stw::errors`: `C_NO_ERR` (0), `C_UNKNOWN_ERR` (-1), `C_RANGE` (-5),
`C_RD_WR` (-7), `C_CONFIG` (-10), `C_TIMEOUT` (-12).

`C_OscErrorCategory.hpp` adds an `Errc` enum + `STWErrorCategory` so these integrate
with `std::error_code`. Callers that still need the legacy integer use `.value()`.
`hex_file` has its own category (`C_HexFileErrorCategory`) because its codes are not
STW codes.

**Bridging unmigrated callees.** Migrated code has to call code that still returns
`int32_t`. There is exactly one correct way to convert:

```cpp
c_Retval = make_error_code_from_stw(Legacy());   // yes
c_Retval = static_cast<Errc>(Legacy());          // no
```

`static_cast<Errc>` names no conversion and asserts "this integer is an STW error
code" without checking. Any value outside the 13-member set becomes an enumerator
that does not exist, and a foreign status code is silently relabelled as an STW one.
Two such conflations have already been introduced and fixed here
(`TglRemoveDirectory`, `mz_compress`) — both invisible in testing, because 0 means
success on both sides. 16 sites still use the wrong idiom, all in
`system_update_package/`; see the phase 5 section of
`docs/agent_plans/codebase_audit/PLAN.md`.

**Before bridging, read the callee's own `\return` block.** Not every function
returning `int32_t` returns an STW code — TGL file helpers return plain 0/non-zero,
miniz returns its own codes, and OpenSSL returns 1 for success, the opposite
polarity.

**Grep for comparisons, not just assignments.** `if (Call() == C_NO_ERR)` matches no
assignment-shaped search, and callers frequently reach a migrated class through a
base-class pointer without ever naming it.

### File header

New files carry **both** copyright lines — STW and Elytron Defense:

```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Brief description

   \copyright   Copyright [YEAR] Sensor-Technik Wiedemann GmbH. All rights reserved.
                Copyright [YEAR] Elytron Defense. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
/* -- Namespace ----------------------------------------------------------------------------------------------------- */
/* -- Types --------------------------------------------------------------------------------------------------------- */
/* -- Implementation ------------------------------------------------------------------------------------------------ */
```

Files inherited from upstream STW keep their original single-line copyright. Some
net-new files added during phases 0–3 (e.g. `stwerrors/C_OscErrorCategory.hpp`,
`scl/C_SclStringUtil.hpp`) carry only the STW line — that is an oversight in those
files, not a convention to copy.

## Current Work

Tracked in `docs/agent_plans/codebase_audit/` — `PLAN.md` is the master plan (8
phases), `analysis.md` the underlying audit, `PHASE3_PLAN.md` the string migration
detail.

| Phase | Status |
|-------|--------|
| 0 — Test framework & CI | Done |
| 1 — Correctness bugs | Done |
| 2 — Remove `C_SclDynamicArray` → `std::vector` | Done |
| 3 — Retire `C_SclString` → `std::string` | Done |
| 3x — Retire `C_SclStringList` → `std::vector` (phase 3 follow-up) | Done |
| 4 — Replace homegrown AES | Done for files; wire protocol deliberately out of scope. `C_OscSecurityAesFile` is AES-256-GCM + PBKDF2 (600k) with a versioned 56-byte header and key wiping. `C_OscSecurityAesCbc` remains AES-128-CBC and is still used by `C_OscProtocolSecuritySubLayer` — changing that is an ECU-side protocol change, not a PC-side one |
| 5 — Error handling modernization | Done — every STW `int32_t` error return in `opensyde_core` is `std::error_code` (9 waves: security, imports, data_dealer, zip, cmon_protocols, system_package_handling, halc, protocol_drivers, dispatchers/xml_parser/project plus a final six). Bridging scaffolding fell 238 → 8; 19 functions stay `int32_t` on purpose (foreign conventions). No `static_cast<Errc>` misuse remains |
| 6 — Concurrency & singletons | 6.1 done (`std::call_once`; Meyer's singleton rejected). 6.2 done (`C_TglCriticalSection` and `TglTasks` deleted, 52 sites on `std::mutex`). 6.3 closed — no defect found |
| 7 — Performance | 7.1 done — logging hot path **4.2x** (`BM_WriteLogInfo` ~2.6µs → ~0.62µs, Release). The win was allocation, a hand-written fixed-width timestamp, and a per-second `localtime_r` cache in `TglGetDateTimeNow` (366 → 60 ns) — **not** the `std::format` swap the plan prescribed, which alone was only 1.4x. 7.2 and 7.3 open — both need a decision before code, see `PLAN.md` |
| 8 — Build system modernization | Done — CMake minimum 3.25, CI reworked, ccache added, unified root build (one opensyde_core, all eight tools). C++23 adopted tree-wide (root + core + tool toolchains) on 2026-09-08 |

A whole-tree consolidation sweep ran 2026-09-13; findings, what was rejected and
why, and the ranked remainder are in `docs/agent_plans/consolidation_sweep/FINDINGS.md`.
Read that before starting any dedup work — several of the largest apparent wins
are deliberately declined there.

Cross-cutting follow-ups (dark mode, Linux version string, About dialog) live in
`docs/TODO.md`; in-code `TODO`/`FIXME` markers are catalogued in
`docs/code-comment-todos.md`.

## Gotchas

**QCustomPlot is vendored and built from source on every platform.** This replaced
three inconsistent resolution paths (Linux system package, a committed Windows `.a`
whose path did not even exist, nothing on macOS) with one source build. The layout:

- `libraries/qcustomplot/qcustomplot.h` — a compatibility wrapper that redefines the
  Qt keywords (`signals`, `slots`, `foreach`) around `#include_next <qcustomplot.h>`,
  because the project builds with `QT_NO_KEYWORDS` for DBC library compatibility.
- `libraries/qcustomplot/upstream/` — the vendored v2.1.1 source (`qcustomplot.{cpp,h}`,
  GPLv3) plus a `CMakeLists.txt` that builds a `qcustomplot` static library.

The tool's CMake does `add_subdirectory(.../upstream)` guarded by
`if(NOT TARGET qcustomplot)` and links `qcustomplot`. Two non-obvious things make it
compile: the subdir's CMakeLists calls `remove_definitions(-DQT_NO_KEYWORDS)` —
per-target `-UQT_NO_KEYWORDS` is not enough because **AUTOMOC reads the directory's
COMPILE_DEFINITIONS**, and moc must see the keywords — and it sets
`AUTOMOC ON AUTOUIC OFF AUTORCC OFF` so the plain library doesn't inherit the tool's
`.ui`/`.qrc` rules and collide on generated headers.

**Platform-specific CMake predicates must test `WIN32`, not `else()`.** A recurring
bug in this tree was `if(CMAKE_SYSTEM_NAME STREQUAL "Linux") … else() …` where the
`else()` branch silently meant *Windows* — which breaks macOS, since macOS is neither.
For anything that is vendored on Windows but resolved from the system elsewhere
(OpenSSL, zlib, QCustomPlot), the correct shape is `if(WIN32) vendored / else()
system` — macOS then takes the same path as Linux. Clang-only and GCC-only warning
flags (`-Wunused-private-field` is clang-only; `-Wmaybe-uninitialized`, `-Wlogical-op`,
`-static-libgcc/-static-libstdc++` are GCC-only) must likewise be gated on
`CMAKE_CXX_COMPILER_ID` or they are fatal under `-Werror` on the other compiler.

**Do not set `CMAKE_SYSTEM_NAME` in a toolchain file for a native host.** Setting it
at all makes CMake declare `CMAKE_CROSSCOMPILING`, which changes how `find_package`
searches — on Windows this produced a `ZLIB NOTFOUND` that looked for all the world
like a missing dependency and was not. Only `toolchain_linux.cmake` sets it (and only
because it is the historical cross-compile lane); `toolchain_macos.cmake` and
`toolchain_windows.cmake` deliberately do not, and both carry a comment saying so.
If a native build starts failing to find a package it should obviously have, check
this before hunting the package.

**The build uses one shared toolchain file, plus per-tool `pjt/lint_config.cmake`.**
`build.sh` passes a single `-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain_{linux,macos,windows}.cmake`
(selected by `uname`) for the whole root build; the older per-tool
`pjt/toolchain_linux.cmake` files are vestigial standalone-config leftovers, not what
the unified build uses. Each tool's `CMakeLists.txt` still does
`include(lint_config.cmake)` from its own `pjt/` directory, so that file is per-tool.
When CAN Monitor and SYDEflash were split out of `opensyde_tool/` into their own
top-level trees, their `lint_config.cmake` did not come across and neither tool could
build on Linux at all (restored 2026-09-06, invisible until the GUI CI job was
repaired the same day). If a new tool is split out the same way, check for that file.

## Agent Workspace Rules

- Persistent artifacts (plans, checklists, notes) go in `docs/agent_plans/`. There is
  no `plans/` directory on this branch.
- Do not reference plan documents from source comments — plans get archived and the
  references rot. `CLAUDE.md` is fine; `.cpp`/`.hpp` are not.
- Fix root causes. No `Q_UNUSED` to silence warnings, no backward-compat shims or
  aliases, no `_New`-suffixed parallel classes.
- Prefer incremental builds; `build.sh -c` (clean) is slow and rarely necessary.
