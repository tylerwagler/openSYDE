# Releasing

There is one release mechanism: push a tag, and `.github/workflows/release.yml`
builds all eight tools in Release on Linux, macOS and Windows, packages them, and
publishes a GitHub Release with generated notes and the three archives attached.

## Cutting a release

```bash
git checkout develop && git pull --ff-only
git tag -a v2026.09.18 -m "openSYDE 2026.09.18"
git push origin v2026.09.18          # from the build host if the Mac has no push credentials
```

Tags are `v` + a date (`v2026.09.18`; add `-2` for a second cut on the same day).
The bundle is versioned by the tag; the per-tool versions in each tool's
`version_config.hpp` are STW's product versions and are not changed by a release.
`git describe` puts the tag into every archive name.

The release appears at `https://github.com/tylerwagler/openSYDE/releases` a little
under an hour later (Windows is the long pole). Nothing is signed.

## What the archives contain

| Archive | Contents | Runtime |
|---|---|---|
| `openSYDE-<tag>-linux-x86_64.tar.gz` | the eight binaries, one folder each | Qt 6 and OpenSSL 3 from the distribution (see `RUNTIME.md` inside) |
| `openSYDE-<tag>-macos-arm64.tar.gz` | the eight binaries | Qt 6.8 and OpenSSL 3 from Homebrew |
| `openSYDE-<tag>-windows-x86_64.zip` | the eight binaries with every DLL they import (Qt, OpenSSL, the llvm-mingw runtime) and the Qt plugins next to them; resolved by an `llvm-objdump` import walk, not `windeployqt`, which misclassifies the llvm-mingw Qt plugins and refuses to deploy | none |

Each archive comes with a `.sha256`.

## Verifying a packaging change before it lands

The workflow also runs on a pull request that touches `release.yml`, `build.sh`,
`cmake/**` or any tool's `pjt/CMakeLists.txt`, building and packaging on all three
platforms without publishing. The archives are the run's artifacts; download one
and look inside. `workflow_dispatch` does the same, but GitHub only offers it once
the workflow exists on the default branch (`master`).

## Identifying a binary

Every tool prints, next to its version and MD5 checksum, a line of the form

```
Build: v2026.09.18, source dated 2026-09-18T12:14:00+02:00
```

in its banner (command-line tools), its start-up log entry (all tools) and the About
dialog (GUI tools). The first value is `git describe --tags --always --dirty` at build
time: the tag on a tagged commit, `<tag>-<n>-g<hash>` after it, a bare short hash when
no tag is reachable, and a `-dirty` suffix when the tree had uncommitted changes. The
second is the committer date of that commit, not the wall-clock build time, so two
builds of the same commit stamp identically and an incremental build does not re-link
every tool. Both read `unknown` in a build from an exported tarball.

The stamp lives in `libraries/opensyde_core/util/`: `osy_build_info.cmake` writes
`osy_build_info.hpp` into the build directory before every build of `opensyde_core`,
and `C_OscBuildInfo` is the only reader. Nothing to edit when cutting a release.

## Not done yet

- Linux AppImage and macOS `.app` bundles with the Qt frameworks inside, so those
  archives run without a Qt installation the way the Windows one does.
- Code signing and notarisation.
