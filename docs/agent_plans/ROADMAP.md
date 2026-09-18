# openSYDE Roadmap — open work

**This is the one place to look for what is open.** Everything else in
`docs/agent_plans/` is either the record of a finished effort (`archive/`) or the
durable findings from the sweeps (`FINDINGS.md`).

Read `FINDINGS.md` before starting anything in here. Several of the most obvious
"improvements" in this tree are declined there with measurements, and a few of the
scans worth re-running are written up with the traps that made their first versions
wrong.

Last consolidated: 2026-09-18.

---

## Release readiness

The code is in better shape than the machinery around it. There is no release
machinery at all.

| Gap | State |
|-----|-------|
| Release build in CI | `release.yml` (2026-09-18): Release on all three platforms on every `v*` tag, and on PRs that touch the build |
| Artifacts | one archive per platform with a `.sha256`, attached to the GitHub Release; workflow artifacts on PRs |
| Packaging | Windows zip runs as is (import walk deploys Qt, OpenSSL and the runtime); Linux/macOS tarballs need the platform's Qt 6 -- AppImage and `.app` bundles still open |
| Release workflow | tag `v<date>` -> build, package, GitHub Release with generated notes (`docs/releasing.md`) |
| Versioning | Seven hand-edited `version_config.hpp` files with unrelated numbers (tool `1.90.1`, CAN Monitor `1.86.0`, SYDEsup `1.15.0`, flash tool `1.0.1`). Root `project()` has no `VERSION`. Every binary now carries `git describe` + commit date (`C_OscBuildInfo`, shown in banners, logs and About) -- that is what ties a build to a tag; the seven hand-edited numbers are still unrelated to it |

Release **was** built manually on 2026-09-16: all eight tools clean, and the core
suite passes in Release (one locale test skips where the host has no comma locale).
So nothing is known to be broken — it is simply unguarded.

### The functional-coverage question

Worth stating plainly before anyone calls a build a release candidate. The defects
found in the September sweep were not subtle:

- parameter set files **could not be loaded at all** — the CRC never matched *and*
  the version was rejected
- `C_SclIniFile` recognised **no sections whatsoever**, taking out CANopen EDS/DCF
  import
- float attributes were written with a comma separator under a German or French
  locale, silently truncating every factor, offset and float value when the project
  was opened elsewhere

None of those are edge cases; each is "the feature does not work". That they
survived means those paths had not been exercised in this fork since the
migrations. The unit suite is good (320 tests, three platforms) but every one of
those bugs lived in an **integration path** the unit tests do not reach.

**Update, 2026-09-17.** The round-trip technique was then run over every file
format, package, generator and security class in the core (#37–#42; 41 suites,
363 tests). It found five more features that had never worked on this branch since
the migrations, each "the feature does not work" rather than an edge case:

| Never worked | Since | Fixed in |
|---|---|---|
| loading **any** HALC definition with a use-case | phase 3 | #37 |
| CANopen EDS import numbers (`254` read as `54`, hex limits as `0`) | phase 3 | #37 |
| **creating** a service update package, and loading one | phase 3b | #39 |
| telling a COM datapool's Tx list from its Rx list — every message↔element mapping | phase 3 | #40 |
| opening an encrypted session with an ECU (double free at ECDH key creation) | phase 3b | #41 |

Plus the X-config missing-node check reading past the end (#38), the HALC magician
mislaying channel values when a domain has both domain and channel parameters (#40),
and hex INI values read as 0 (#39). `FINDINGS.md` carries the six shapes of the
1-based residue with the scanner that finds five of them.

What the suite still cannot reach is everything that talks to hardware: the
protocol drivers, the data dealer, device configuration and the update sequences.
A manual functional pass over those — run a system update through a gateway, CAN
Monitor against a real bus, a secure session with an ECU — remains the highest-value
pre-release activity, and it needs hardware.

---

## Open engineering work

Ranked by value, with enough context to start without opening the archive.

### 1. UDS negative-response-code table is duplicated and has already drifted

`cmon_protocols/C_CanMonProtocolUds.cpp:412` versus
`C_CanMonProtocolOpenSyde.cpp:1043`. Two copies of the NRC→text mapping that
**already differ in wording** (`positiveResponse` versus `positive response`).

The duplication is the ordinary kind; the drift is the reason this is first. Any
further divergence is a user-visible inconsistency nobody will notice until a
support call. Consolidating changes the displayed text for one of the two, so it
needs a wording decision first.

### 2. Tests for the hardware-facing classes, with a mocked protocol

Every file format, package, generator and security class in `opensyde_core` has a
test now (#30, #37–#42). What has none is the layer that talks to devices:
`C_OscDataDealer` / `C_OscDataDealerNvm` (datapool read/write over a protocol),
`C_OscComDriverProtocol`, `C_OscSuSequences` (the update sequences),
`C_OscDcDeviceInformation`, the routing execution (the calculation is tested).

`C_OscDataDealerNvm` is the tractable one: it drives an abstract
`C_OscDiagProtocolBase`, so a mock protocol that answers from a byte array would
let the NVM read/write/list-CRC logic be exercised without a device. The rest need
a loopback CAN or hardware. Given what the round-trips found in every other layer,
assume this one is not clean either.

### 3. Filer error-API migration

~130 remaining call sites, roughly 270 lines. Mechanical, and the shape is settled
by the `std::expected` pilot recorded in `FINDINGS.md`: **start from the
value-or-default callers and leave the bridge sites alone** until the conventions
are unified. Converting a bridge site costs about four lines and buys nothing.

### 4. 135 range-for conversions

Blocked on naming judgement rather than on safety. Mechanical singularisation
produces `Entrie` and `SubNodeIndexe`, so each needs a human to pick the name.

### 5. Phase 7.3 — move semantics

Move constructors and assignment on large classes, `std::move` in hot paths. Worth
doing only where a benchmark shows it matters — 7.1 is the cautionary tale: the
plan's prescribed fix was 1.4x and the actual win came from somewhere the plan had
not looked.

### 6. Standalone GUI-tool configure on macOS

`cmake -S opensyde_can_monitor/pjt` (or `opensyde_tool/pjt`) on a Mac fails with
`Failed to find required Qt component "Svg"`. Homebrew ships `qtbase` and `qtsvg` as
separate kegs, and `find_package(Qt6 COMPONENTS Svg)` looks for `Qt6Svg` relative to
`Qt6_DIR`, where it is not. The unified `build.sh` build is unaffected, and CI uses
`install-qt-action` rather than Homebrew, so nothing shipped is at risk -- but the
standalone path `CLAUDE.md` promises does not work on this platform.

Verified pre-existing against an untouched `develop` worktree on 2026-09-17. Likely fix:
add the `qtsvg` keg to `QT_ADDITIONAL_PACKAGES_PREFIX_PATH` in `toolchain_macos.cmake`,
or make the standalone configure go through whatever `build.sh` does. Small, but it
needs a Mac to verify.

### 7. Low / cosmetic bucket

`std::endl` → `"\n"` in console logging; ~208 `#define` wire-constants →
`constexpr`; god-functions with 16–18 parameter signatures; `== true` / `== false`;
`osc_write_log_*` macros → `std::source_location`.

---

## Larger features

### Dark mode

The big one, and scoped honestly in the archived TODO: full dark mode means
re-tagging the palette by semantic role, providing dark variants, adding a runtime
theme switch (`QStyleHints::colorScheme()` on Qt 6.5+ plus an in-app override), and
auditing **9,741 lines of `.qss` across 63 files**, ~50 hardcoded `QColor`
constants with no role tagging, and 89 C++ files that reference them.
**2–4 weeks**, multi-thousand-line diff, manual screen-by-screen visual QA.

The existing `SetDarkTheme` / `mq_DarkMode` hooks are **not** a partial
implementation — they are a per-dashboard-widget render toggle for runtime
visualisation. The application chrome does not honour the flag at all.

Three smaller fallbacks if the night-brightness pain is real but a multi-week
project is not:

1. **System palette inversion only** (~1 day) — `QApplication::setPalette()` from
   the system palette, so the bits Qt styles itself stop clashing. Most chrome
   stays bright.
2. **Chrome only, leave dashboards** (~1 week) — re-skin main window, nav and
   dialogs; leave dashboard rendering on its existing per-widget toggle.
3. **A less-bright default palette** (~half day) — drop the `#FFFFFF` / `#FAFAFA`
   backgrounds to something readable in daylight and less harsh at night. Never
   matches OS dark mode, but needs no per-screen audit.

### Show actually-linked library versions in About

The dependency *list* is now accurate. What remains is *versions*: only Qt reports
one (`qVersion()`). OpenSSL would have to be conditional (`opensyde_tool` links it,
SYDEflash does not); TinyXML-2 and zlib mean adding an include purely to print a
version; Miniz and Vector::DBC are vendored and would need CMake to inject one.
Small, self-contained, needs a judgement call on how much is worth the includes.

### Audit "low-value" UX features for removal

Scoped in the archived TODO. Unstarted.

---

## Decisions parked on a human

These are not blocked on effort. Each needs a product call.

| Item | The question |
|------|--------------|
| **EDS `LoadFromIni` strictness** | `C_OscCanOpenEdsDeviceInfoBlock::LoadFromIni` and its `FileInfoBlock` twin document `Errc::config` for a missing value and never return it. Four validating `h_Load*ValueFromIniFile` helpers sit **unused** in the same file behind a `//lint -e{8062} Kept for later error reporting` comment. Wiring them up makes EDS import stricter and may reject files that work today — and the original author's own "Maybe mandatory values" comment says they were unsure which keys are mandatory |
| **5 deferred `[[nodiscard]]` callees** | `SetNodeIdentifiers`, `SendCanMessageDirect`, `HandleCanMessage`, `Cycle`, `m_HandleAsyncResponse` (12 declarations in `protocol_drivers`). Whether a caller should abort on one failed send is a protocol decision. Reasons are recorded at each declaration; one line each once decided |
| **`C_SyvDaDashboardsWidget::m_InitOsyDriver`** | Discards `StartLogging`. On failure, CAN signal interpretation silently does not start on the dashboard. It sits inside a `switch (s32_Retval)`, so propagating is a restructure |
| **UDS wording** | See open item 1 — consolidating the two NRC tables changes displayed text for one of them |
| **CAN interfaces hash an IP the filer never persists** | `C_IpAddress()` seeds a default, the node filer writes it only for Ethernet, the loader zeros it for the rest, `CalcHash` covers it on every type. Tidy fix is for the loader to keep the constructor default; that changes what a loaded Ethernet interface without an `ip-address` node looks like, so it needs a look at the GUI first (FINDINGS, filer wave 2) |
| **`CalcCRC32C` has no caller** | Phase 7.2 added a hardware CRC-32C (`C_SclChecksums::CalcCRC32C`, SSE4.2 at runtime, 4 GiB/s) before noticing that every persisted checksum uses the IEEE polynomial, which the instruction cannot compute. It stays tested and benchmarked but nothing calls it. Keep it for a future format, or delete it: a product call. The IEEE `CalcCRC32` got slicing-by-8 instead (4.2x, bit-identical; FINDINGS) |
| **Report the fork's findings upstream?** | The ECDH double free is the fork's own (upstream frees once), but `h_ListIsComTx`'s second-to-last-letter check and the `[i + 1]` availability parsing are correct upstream only because their string class is 1-based — anyone else porting to `std::string` will hit the same six shapes. Whether to write that up for the openSYDE project is a call for a person |

---

## Where everything else lives

| Path | What it is |
|------|-----------|
| `FINDINGS.md` | Durable findings from the consolidation sweep: defect classes and the scans that find them, rejected ideas **with the counts that decided it**, methodology traps. Read before starting work here |
| `archive/` | Completed efforts, kept for their rationale — particularly the won't-fix decisions |
| `../code-comment-todos.md` | Catalogue of in-code `TODO` / `FIXME` / `HACK` markers |
| `../remote-build.md` | Build host and the git-bundle push workflow |

### A note on archived plans

Two archived plans described work as unstarted that had in fact been completed
(`qsettings_migration`, `stwtypes_removal`). Both were found during this
consolidation, and both had been done some time earlier. If a plan here says
something is open, **check the code before believing it** — and update this file
when you find otherwise.
