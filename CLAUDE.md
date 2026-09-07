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

CMake + Ninja + GCC. Driven by `build.sh` at the repo root.

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

`build.sh` requires plain `cmake`, `ninja`, and `g++` on `PATH` — it never pins a
compiler version.

There is **no root `CMakeLists.txt`**. `CMakePresets.json` exists at the root but
therefore configures nothing on its own; each tool is configured from its own `pjt/`
directory, and the core library from `libraries/opensyde_core`.

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
cmake ninja-build g++ qt6-base-dev qt6-svg-dev qt6-tools-dev
libgl1-mesa-dev libssl-dev flex bison libfl-dev libqcustomplot-dev
```

Notes that have bitten CI before:

- **`flex` alone is not enough.** `FlexLexer.h` ships in `libfl-dev`. Without it CMake
  leaves `FLEX_INCLUDE_DIR` unset and `Vector_DBC` fails to generate its scanner.
  `opensyde_core` links `CanLibraries::dbc`, so this is not optional.
- **Package names differ across Ubuntu releases.** Qt6 SVG dev is `qt6-svg-dev` from
  noble (24.04) onward, but `libqt6svg6-dev` on jammy (22.04). Jammy also has no
  `g++-13`. CI does not target jammy for these reasons.
- **QCustomPlot is unresolved.** `libqcustomplot-dev` ships the Qt5-linked library;
  the Qt6 runtime is a separate package (`libqcustomplot2.1-qt6`), and the tree also
  carries a vendored copy at `libraries/qcustomplot/`. Confirm which one the build
  should use before relying on the system package — see "Known Broken" below.

### Verifying tool-tree changes

`opensyde_core` builds on its own and does **not** compile the seven sibling tool
trees, so a clean core build proves nothing about them. Any change to a signature
in core must be checked with a full eight-tool build. Grep is not a substitute:
callers reach migrated classes through base-class pointers, share method names
with unmigrated classes, and compare rather than assign.

A 48-core build host makes this cheap — ~3 minutes for all eight tools against
~18 on a laptop. See `docs/remote-build.md`.

```bash
ssh claude@claude 'cd ~/Projects/openSYDE && git fetch origin && git checkout develop \
  && git pull --ff-only origin develop && git submodule update --init --recursive \
  && ./build.sh -b Debug -j 48 all'
```

### Build outputs

Per-tool build directories under `build/`; deploy target defaults to
`~/.local/opt/openSYDE` (override with `INSTALL_DIR`).

## Testing

Google Test + CTest, in `libraries/opensyde_core/tests/`. Enable with
`-DOPENSYDE_CORE_BUILD_TESTS=ON`, run with `ctest`.

13 suites, 175 tests: `test_application_info_block`, `test_checksums`,
`test_dynamic_array`, `test_hex_file`, `test_hex_string_parsing`, `test_logging`,
`test_osc_error_category`, `test_protocol_serial_number`, `test_scl_string`,
`test_security_aes_file`, `test_stwerrors`, `test_stwtypes`, `test_xml_parser`.

Several of these are regression pins for defects the phase 2/3 migrations
introduced and CI did not catch, so prefer extending them over replacing them.

## CI

`.github/workflows/build.yml`, on push/PR to `main`, `develop`, `master`.

- **Core Library** — `ubuntu-26.04`. Configures with full subsystem coverage,
  builds, runs `ctest`. ~1.5 min.
- **GUI Tools** — `ubuntu-26.04`, smoke-builds all eight tools via
  `./build.sh -b Debug all`. ~13-18 min.

One runner, one compiler — the distro default `g++`, unpinned, which is what
resolute users actually get. Neither job is `continue-on-error`: as the only
targets they have to gate, or CI stops meaning anything.

Keep CI green. The core job was red from the moment it was introduced until
2026-09-06, and that gap is precisely how phase-2 and phase-3 migration residue
(`GetLength()` on `std::vector`, mangled `std::stoi` artifacts) survived unnoticed.
A job that does not run is not a quality gate.

**CI does not compile anything behind `#ifdef _WIN32`.** Both runners are Linux
and so is the remote build host. 23 files carry `_WIN32` conditionals; most guard
a few lines, but `C_SdNdeDalTriggerCheckHelper.cpp` is stubbed wholesale on Linux
and roughly 1,124 of its lines are never built. See `docs/TODO.md`.

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

Core library internals: `project/system/` (nodes, buses, definitions), `halc/`,
`protocol_drivers/`, `data_dealer/`, `exports/`, `xml_parser/`, `scl/`, `security/`,
`stwtypes/`, `stwerrors/`.

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

Use `stwtypes.hpp`: `int8_t` … `uint64_t`, `float32_t`, `float64_t`, `char_t`.

### Strings and containers

`std::string` and `std::vector`. `C_SclString` and `C_SclDynamicArray` have been
**removed** — do not reintroduce them. `C_SclStringCompat.hpp` provides transitional
helpers; prefer idiomatic `std::string` in new code and do not add new compat helpers.
`C_SclStringList`, `C_SclIniFile`, `C_SclChecksums`, and `C_SclDateTime` still exist.

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
`scl/C_SclStringCompat.hpp`) carry only the STW line — that is an oversight in those
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
| 4 — Replace homegrown AES | Done for files; wire protocol deliberately out of scope. `C_OscSecurityAesFile` is AES-256-GCM + PBKDF2 (600k) with a versioned 56-byte header and key wiping. `C_OscSecurityAesCbc` remains AES-128-CBC and is still used by `C_OscProtocolSecuritySubLayer` — changing that is an ECU-side protocol change, not a PC-side one |
| 5 — Error handling modernization | In progress — `C_OscErrorCategory` (`Errc` + `STWErrorCategory`), `hex_file` has its own category. Waves done: security, imports, data_dealer, zip, cmon_protocols, system_package_handling, halc. Remaining: `protocol_drivers` (~292) and `project` (~204, ~640 caller files) — each large enough to run alone |
| 6 — Concurrency & singletons | 6.1 done (`std::call_once`; Meyer's singleton rejected). 6.2 done (`C_TglCriticalSection` and `TglTasks` deleted, 52 sites on `std::mutex`). 6.3 closed — no defect found |
| 7 — Performance | Not started |
| 8 — Build system modernization | Partial — CMake minimum 3.25, CI reworked, ccache added. Unified root build still open |

Cross-cutting follow-ups (dark mode, Linux version string, About dialog) live in
`docs/TODO.md`; in-code `TODO`/`FIXME` markers are catalogued in
`docs/code-comment-todos.md`.

## Gotchas

**QCustomPlot is a system dependency on Linux, not vendored.** `libraries/qcustomplot/`
holds only `qcustomplot.h`, a compatibility wrapper that re-defines the Qt keywords
(`signals`, `slots`, `foreach`) around the *system* header, because the project builds
with `QT_NO_KEYWORDS` for DBC library compatibility. There is no vendored
`libqcustomplot.a` on Linux. The tool's CMake tries pkg-config `qcustomplot-qt6` first
(openSUSE) and falls back to `find_library(NAMES QCustomPlotQt6 QCustomPlot)` (Debian
/ Ubuntu, where `libqcustomplot-dev` ships the capitalized `libQCustomPlotQt6.so`).

**Each GUI tool needs its own `pjt/toolchain_linux.cmake` and `pjt/lint_config.cmake`.**
`build.sh` passes `-DCMAKE_TOOLCHAIN_FILE` per tool, and each tool's `CMakeLists.txt`
does `include(lint_config.cmake)` from its own `pjt/` directory. When CAN Monitor and
SYDEflash were split out of `opensyde_tool/` into their own top-level trees, the
`CMakeLists.txt` files came across but these two siblings did not, so neither tool
could build on Linux at all. Restored 2026-09-06. If a new tool is split out the same
way, check for both files.

Both problems were invisible until the GUI CI job was repaired on 2026-09-06 — before
that it died at dependency installation and never reached a build.

## Agent Workspace Rules

- Persistent artifacts (plans, checklists, notes) go in `docs/agent_plans/`. There is
  no `plans/` directory on this branch.
- Do not reference plan documents from source comments — plans get archived and the
  references rot. `CLAUDE.md` is fine; `.cpp`/`.hpp` are not.
- Fix root causes. No `Q_UNUSED` to silence warnings, no backward-compat shims or
  aliases, no `_New`-suffixed parallel classes.
- Prefer incremental builds; `build.sh -c` (clean) is slow and rarely necessary.
