# Remote build machine

A full eight-tool build takes **~18 minutes** on the dev laptop (8 cores) and
**~3 minutes** on the build machine (48 cores). That difference is what makes it
practical to verify every tool before pushing, instead of discovering breakage in
CI.

```
host    claude@claude
os      Debian 13 (trixie)
cpu     48 cores
ram     125 GB
repo    ~/Projects/openSYDE
```

## Why this matters

`opensyde_core` is built by `libraries/opensyde_core/CMakeLists.txt`, which does
**not** compile the seven sibling tool trees. Any change to a signature in core
ripples into `opensyde_tool`, `opensyde_can_monitor`, `opensyde_syde_flash`,
`opensyde_syde_sup`, `opensyde_syde_x_gen`, `opensyde_syde_coder_c`,
`opensyde_cmd_line_flash_tool` and `opensyde_tsp_convert` — and a core-only build
proves nothing about them.

Grep is not a substitute. During the std::error_code migration, callers were
missed repeatedly because they:

- reach the migrated class through a base-class pointer, never naming it
  (`C_SyvComPollingThreadDiag.cpp` via `C_SyvComDataDealer *`)
- share a method name with unmigrated classes (`LoadFromFile` exists on
  `C_OscXmlParser`, `C_PuiSdHandler`, `C_OscSecurityPem`, `C_CamProHandler`, …)
- **compare** rather than assign — `if (call(...) == C_NO_ERR)` matches no
  assignment-shaped search

Build all eight tools. It is three minutes.

## Setup

```bash
sudo apt-get update
sudo apt-get install -y \
  cmake ninja-build ccache g++ pkg-config \
  qt6-base-dev qt6-svg-dev qt6-tools-dev \
  libqcustomplot-dev libgl1-mesa-dev \
  libssl-dev zlib1g-dev libboost-dev \
  flex bison libfl-dev
```

Do **not** derive this list from a machine that already builds. Three packages
were missed that way — `zlib1g-dev` (needed by `Vector_BLF`), `pkg-config`
(needed by the two Qt GUI tools to locate QCustomPlot), and the qcustomplot/flex
set — because the dev laptop already had them, so they never produced an error.
The authoritative list comes from the build itself:

```bash
grep -rhoE 'find_package\([A-Za-z0-9_]+' --include=CMakeLists.txt . | sort -u
# BISON Boost CanLibraries FLEX OpenSSL PkgConfig Qt6 Threads ZLIB
# (plus optional tooling: CCCC Cppcheck Doxygen Graphviz LCOV HTMLHelp)
```

`FlexLexer.h` ships in `libfl-dev`, not `flex`. Without it CMake leaves
`FLEX_INCLUDE_DIR` unset and `Vector_DBC` fails to generate its scanner.

## Clone

```bash
mkdir -p ~/Projects && cd ~/Projects
git clone https://github.com/tylerwagler/openSYDE.git
cd openSYDE && git checkout develop
git submodule update --init --recursive     # can-libraries + the three Vector trees
```

## Verify a change before pushing

```bash
# push the branch, then on the build host:
ssh claude@claude 'cd ~/Projects/openSYDE && git fetch origin && git checkout develop \
  && git pull --ff-only origin develop \
  && git submodule update --init --recursive \
  && ./build.sh -b Debug -j 48 all'
```

To test uncommitted work, rsync the tree instead of pulling:

```bash
rsync -az --delete --exclude .git --exclude build --exclude result \
  ~/Projects/openSYDE/ claude@claude:~/Projects/openSYDE/
ssh claude@claude 'cd ~/Projects/openSYDE && ./build.sh -b Debug -j 48 all'
```

Collect *every* error rather than stopping at the first — the difference between
one round and four:

```bash
ssh claude@claude 'cd ~/Projects/openSYDE && cmake --build build/Debug/opensyde -j 48 -- -k 0'
```

## Timings

| | |
|---|---|
| Dev laptop, 8 cores, `build.sh all` | ~18 min |
| Build host, 48 cores, `build.sh -j 48 all` | **3 min 1 s** (`user` 73 min) |
| Core library only, either machine | ~1–2 min |
| CI (GitHub Actions) | core ~1.5 min, GUI tools ~13–18 min |
