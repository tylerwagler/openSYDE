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

### Build outputs

Per-tool build directories under `build/`; deploy target defaults to
`~/.local/opt/openSYDE` (override with `INSTALL_DIR`).

## Testing

Google Test + CTest, in `libraries/opensyde_core/tests/`. Enable with
`-DOPENSYDE_CORE_BUILD_TESTS=ON`, run with `ctest`.

Current suites: `test_checksums`, `test_dynamic_array`, `test_logging`,
`test_osc_error_category`, `test_scl_string`, `test_stwerrors`, `test_stwtypes`,
`test_xml_parser`.

## CI

`.github/workflows/build.yml`, on push/PR to `main`, `develop`, `master`.

- **Core Library** — matrix of `ubuntu-24.04` × `ubuntu-26.04` × `g++-12` × `g++-13`.
  Configures with full subsystem coverage, builds, runs `ctest`.
- **GUI Tools** — `ubuntu-24.04` and `ubuntu-26.04`, smoke-builds all eight tools via
  `./build.sh -b Debug all`.

`ubuntu-26.04` is still a preview runner image, so resolute jobs are
`continue-on-error` — they report breakage without gating.

Keep CI green. The core job was red from the moment it was introduced until
2026-09-06, and that gap is precisely how phase-2 and phase-3 migration residue
(`GetLength()` on `std::vector`, mangled `std::stoi` artifacts) survived unnoticed.
A job that does not run is not a quality gate.

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
with `std::error_code`. The security API already returns `std::error_code`; callers
that need the legacy integer use `.value()`.

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
| 4 — Replace homegrown AES | Largely done — `security/aes/` sources removed, `C_OscSecurityAesCbc` uses OpenSSL EVP. Note it is AES-128-CBC, not the AES-256-GCM the plan specifies |
| 5 — Error handling modernization | Started — `C_OscErrorCategory` exists, security API converted |
| 6 — Concurrency & singletons | Not started |
| 7 — Performance | Not started |
| 8 — Build system modernization | Not started |

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
