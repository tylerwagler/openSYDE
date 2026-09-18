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

**Check which compiler a build directory is actually using before drawing a
conclusion from it.** CMake records the compiler in `CMakeCache.txt` on the first
configure and keeps it; editing a toolchain file does not move an existing build
directory onto the new compiler, it silently keeps the old one. The host's
`build/Debug` predated the move to clang and was still building with GCC 14 long
after the toolchain said `clang++`, which quietly made every eight-tool check
there a statement about a compiler we do not ship. One line tells you:

```bash
grep CMAKE_CXX_COMPILER_AR build/Debug/CMakeCache.txt   # gcc-ar-14 vs llvm-ar-19
```

Wipe the directory to pick up a toolchain change.

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

47 suites, 454 tests: `test_application_info_block`, `test_build_info`, `test_can_protocol`,
`test_can_transport_virtual_ecu`, `test_canopen_eds`, `test_checksummed_xml`, `test_checksums`, `test_code_export`,
`test_com_message_logger`, `test_cstdint`, `test_data_dealer`,
`test_data_logger_trigger_parser`, `test_datapool_content_util`,
`test_datapool_type_string`, `test_device_manager`, `test_dynamic_array`, `test_endian`,
`test_filer_roundtrip`, `test_halc_magician`, `test_hex_file`,
`test_hex_string_parsing`, `test_logging`, `test_md5`, `test_node_datapool_content`,
`test_osc_error_category`, `test_osc_utils_niceify`, `test_project_metadata`,
`test_protocol_driver_base`, `test_protocol_driver_osy`, `test_protocol_serial_number`,
`test_routing_calculation`,
`test_scl_ini_file`, `test_scl_string`, `test_security_aes_file`,
`test_security_signatures`, `test_string_prefix_parsing`, `test_stwerrors`,
`test_su_sequences_virtual_ecu`, `test_sup_package`, `test_tgl_file`, `test_tgl_time`, `test_tsp_filer`,
`test_view_security_options`, `test_x_packages`, `test_xco_create`,
`test_xml_float_locale`, `test_xml_parser`.

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

| **Static Analysis** | `ubuntu-26.04` | clang analyser over `opensyde_core`, findings in the job summary | ~5 min |

The first three are not `continue-on-error`: they are the targets that have to
gate, or CI stops meaning anything. **Static Analysis is deliberately advisory**
— the clang analyser has a real false-positive rate here, and the known findings
are triaged in `docs/agent_plans/FINDINGS.md`. Read it before
acting on a finding; several are not bugs.

macOS Qt comes from `install-qt-action`, not Homebrew. `brew install qt` drags
`qtwebengine` into a source build, and the split `qtbase`/`qtsvg` kegs hid Svg from
`find_package`. Windows likewise installs Qt (`win64_llvm_mingw`), the toolchain
(`tools_llvm_mingw1706`), Ninja and OpenSSL through the same action.

`.github/workflows/release.yml` builds all eight tools in **Release** on the three
platforms, packages them (Windows with an `llvm-objdump` import walk for Qt, OpenSSL and the llvm-mingw
runtime next to the executables; Linux/macOS as bare binaries with a `RUNTIME.md`) and,
on a `v*` tag, publishes a GitHub Release with the archives. It also runs, without
publishing, on a pull request that touches the workflow, `build.sh`, `cmake/**` or a
tool's `pjt/CMakeLists.txt`. See `docs/releasing.md`.

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
and a committed merge-conflict marker in a `.rc` file. See
`docs/agent_plans/archive/TODO.md`.

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
| `docs/` | Roadmap, findings, archived plans |

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
success on both sides. None remain (the last 16, in `system_update_package/`, went
with phase 5; the phase 5 section of `docs/agent_plans/archive/codebase_audit/PLAN.md`
records how).

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

**Open work is tracked in one place: `docs/agent_plans/ROADMAP.md`.** It carries the
release-readiness picture, the ranked engineering backlog, the larger features
(dark mode and its smaller fallbacks), and the decisions parked on a human.

Two supporting documents:

- `docs/agent_plans/FINDINGS.md` — durable findings from the consolidation sweeps:
  defect classes and the scans that find them, ideas **rejected with the counts
  that decided it**, and the methodology traps that made several scans wrong the
  first time. Read this before starting anything from the roadmap.
- `docs/agent_plans/archive/` — completed efforts, kept for their rationale and
  especially their won't-fix decisions. Frozen; do not trust a status in there
  without checking the code. Two archived plans described finished work as
  unstarted, which is what prompted the consolidation.

The 8-phase codebase audit that drove most of 2026 is complete except for 7.2
(hardware CRC32) and 7.3 (move semantics), both of which have moved to the roadmap.
Phase 7.1 is the cautionary tale worth knowing before doing 7.3: the logging hot
path came out **4.2x** faster, but not from the change the plan prescribed — that
one was 1.4x, and the real win was somewhere the plan had not looked. Measure
first, in Release.

In-code `TODO` / `FIXME` markers are catalogued in `docs/code-comment-todos.md`.

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

**Every tool's CMakeLists has the same shape, and the shared parts live in `cmake/`.**
Each tool is `<tool>/pjt/CMakeLists.txt` (including `opensyde_tool`, whose file used to
sit one level deeper at `pjt/openSYDE/` and was special-cased in the root CMakeLists and
`build.sh`). Each one opens the same way:

```cmake
cmake_minimum_required(VERSION 3.25)
project(<Name> LANGUAGES CXX)
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/osy_tool_common.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/lint_config.cmake)
set(PROJECT_ROOT ${PROJECT_SOURCE_DIR}/..)
```

`osy_tool_common.cmake` holds what is genuinely identical across all eight: the C++23
standard and the `osy_tool_werror(<targets>)` function that turns warnings into errors
on the targets we own. `lint_config.cmake` is the PC-lint generator. Source lists,
include directories and the `OPENSYDE_CORE_SKIP_*` selection stay in each tool -- that
is what makes each tool a different tool.

Before 2026-09-17 every tool carried its own copy of both, and they had drifted: two
tools had an older `lint_config.cmake` missing a fix the other six had, three named the
core build directory differently, two did not pin the C++ standard, and the `-Werror`
block existed in three spellings. It was also why splitting CAN Monitor and SYDEflash
out of `opensyde_tool/` once left them unable to build at all -- the per-tool copy did
not come across. A shared file cannot be forgotten that way.

Paths in a tool's CMakeLists are written from `${PROJECT_ROOT}`, never as bare `../`.
Core is always `add_subdirectory(${PROJECT_ROOT}/../libraries/opensyde_core
${CMAKE_BINARY_DIR}/opensyde_core)` behind the `if(NOT TARGET opensyde_core)` guard.

`build.sh` passes a single `-DCMAKE_TOOLCHAIN_FILE=cmake/toolchain_{linux,macos,windows}.cmake`
(selected by `uname`) for the whole root build. The per-tool `pjt/toolchain_*.cmake`
files that used to sit beside some CMakeLists were vestigial leftovers from standalone
configures, referenced by nothing, and are gone.

**`tgl_assert` on Windows is a message box.** `TglReportAssertion` writes the report
to stderr and then shows a `MessageBoxA`; on a headless machine that dialog waits
forever. `OSY_ASSERT_NO_DIALOG=1` keeps it on stderr only, and the Windows CI test step
sets it -- a fired assertion is a log line there, not a six-hour hang. If a Windows job
ever sits in "Build + run core unit tests" far past its usual time, that is the first
thing to check.

**A standalone GUI-tool configure on macOS needs more than the toolchain file.** The
Homebrew `qtbase` and `qtsvg` kegs are split, and `find_package(Qt6 COMPONENTS Svg)`
looks for `Qt6Svg` relative to `Qt6_DIR`, where it is not. The unified `build.sh` build
works; `cmake -S opensyde_can_monitor/pjt` on its own does not. This predates the shared
scaffolding (verified against an untouched `develop` worktree) and is on the roadmap.

## Agent Workspace Rules

- Persistent artifacts (plans, checklists, notes) go in `docs/agent_plans/`. There is
  no `plans/` directory on this branch.
- Do not reference plan documents from source comments — plans get archived and the
  references rot. `CLAUDE.md` is fine; `.cpp`/`.hpp` are not.
- Fix root causes. No `Q_UNUSED` to silence warnings, no backward-compat shims or
  aliases, no `_New`-suffixed parallel classes.
- Prefer incremental builds; `build.sh -c` (clean) is slow and rarely necessary.
