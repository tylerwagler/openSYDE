# Running CI on the internal GitLab

`.gitlab-ci.yml` mirrors `.github/workflows/build.yml` job for job. This file
covers what the pipeline assumes of its runners — the part that lives on the
machines, not in the repo.

## Runners

Registered as **project runners**, one per platform, each locked to the project
and tagged:

| Tag | Machine | Executor | Jobs |
|-----|---------|----------|------|
| `docker` | Build host (Debian, 48 cores) | `docker` (image per job) | all Linux jobs, `plans`, `release:publish` |
| `macos` | Intel Mac | `shell` | `core:macos`, `tools:macos`, `pcan:macos` |
| `windows` | N01-WIN11 VM | `shell` (PowerShell) | `core:windows`, `tools:windows` |

Jobs on one runner serialise (`max_builds = 1`). The Windows and Mac jobs are
the long poles; parallel capacity means a second runner with the same tag.

Registration uses a runner authentication token from **Settings → CI/CD →
Runners**, so tags/description/locked live server-side — `gitlab-runner
register` must **not** pass `--tag-list` with such a token (GitLab rejects it).

## Nested submodules

GitLab's `GIT_SUBMODULE_STRATEGY: recursive` initialises **one level only**. This
repo nests `third_party/Vector_DBC|BLF|ASC` under `libraries/can-libraries`, so
every job runs `git submodule sync --recursive && git submodule update --init
--recursive` before configuring. Without it bison/flex have no `Parser.yy` or
`Scanner.ll` and the build dies in Vector_DBC.

## What each runner carries

**macOS (`macos`)** — a plain CommandLineTools Mac, no Homebrew:

- `ninja` (1.13.2) and GNU `bison` (3.8.2) in `~/.local/bin`; Apple's bison is
  2.3 and Vector_DBC needs ≥ 3.3
- flex: Apple's `/usr/bin/flex` is fine, but `FlexLexer.h` lives in
  `/Library/Developer/CommandLineTools/usr/include` — `toolchain_macos.cmake`
  points `FLEX_INCLUDE_DIR` there when Homebrew's keg is absent
- Qt 6.12.0 (universal, x86_64+arm64) unpacked under `~/Qt/6.12.0/macos`;
  `toolchain_macos.cmake` appends it to the **cached** `CMAKE_PREFIX_PATH`
- PCBUSB in `/usr/local` for `pcan:macos` (<https://www.mac-can.com>)

Both the bison and Qt paths are pinned with `CACHE ... FORCE`: a reused build
directory otherwise keeps the first configure's answer forever.

The runner currently: binary in `/tmp/gitlab-runner-darwin`, started manually,
no launchd unit yet — it dies on reboot.

**Windows (`windows`)** — as on a dev box:

- Qt 6.12.0 `win64_llvm_mingw` in `C:\Qt\6.12.0\llvm-mingw_64`
- LLVM-MinGW 22.1.7 (Qt's tool package) in `C:\Qt\Tools\llvm-mingw2217`
- CMake 4.4.4, Ninja 1.13.2, Python 3.12 (WinGet), OpenSSL 3.0.16
  (`C:\Qt\Tools\OpenSSLv3\Win_x64\Win_x64`), winflexbison (`C:\tools\winflexbison`)
- Git Bash on the machine PATH; jobs call `bash -lc '…'` with **single** quotes
  (double quotes let PowerShell eat the escaped paths)
- machine environment: `CMAKE_PREFIX_PATH`, `OPENSSL_ROOT_DIR`

**`builds_dir` must not live under `C:\Windows\System32`** (the shell executor's
default). Ninja fails there with `failed recompaction: Permission denied`; the
config sets `builds_dir = "C:\\ci\\builds"` (TOML needs the doubled backslashes).

The runner currently starts from a scheduled task as SYSTEM; converting it to the
`ci` account is a service install with the account password.

**Linux (`docker`)** — nothing: each job's `before_script` installs its
dependencies. The host needs Docker and GitHub reachability (the Vector submodule
URLs on the `develop` line are public GitHub repositories).

## Releases

Tagging `v*` builds Release on all three platforms and publishes per-platform
archives as release assets. **If a release leaves the organisation, attach a
source archive too** — the code is GPLv3, and binary distribution requires
offering the corresponding source to every recipient.
