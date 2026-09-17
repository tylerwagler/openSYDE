# openSYDE Roadmap — open work

**This is the one place to look for what is open.** Everything else in
`docs/agent_plans/` is either the record of a finished effort (`archive/`) or the
durable findings from the sweeps (`FINDINGS.md`).

Read `FINDINGS.md` before starting anything in here. Several of the most obvious
"improvements" in this tree are declined there with measurements, and a few of the
scans worth re-running are written up with the traps that made their first versions
wrong.

Last consolidated: 2026-09-17.

---

## Release readiness

The code is in better shape than the machinery around it. There is no release
machinery at all.

| Gap | State |
|-----|-------|
| Release build in CI | **Never runs.** `build.yml` hardcodes `BUILD_TYPE: Debug` on all three platforms |
| Artifacts | **None.** CI produces nothing downloadable |
| Packaging | **None.** No AppImage, dmg, installer or archive |
| Release workflow | **None.** No tag trigger, no notes |
| Versioning | Seven hand-edited `version_config.hpp` files with unrelated numbers (tool `1.90.1`, CAN Monitor `1.86.0`, SYDEsup `1.15.0`, flash tool `1.0.1`). Root `project()` has no `VERSION`. Nothing ties a release to a tag |

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

A manual functional pass over what the migrations touched — create/save/load a
project, import an EDS, save/load a parameter set, run a system update, CAN Monitor
against a DBC — is the highest-value pre-release activity, and it needs hardware.

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

### 2. Round-trip tests for the remaining filers

`tests/test_filer_roundtrip.cpp` covers the bus, project and data logger filers;
all three are clean. **Twenty more filers have save/load pairs and no round-trip
test.** This is the technique that found the locale bug and the paramset CRC bug,
and it is the cheapest per-bug method in the sweep so far.

Highest-churn targets first: `C_OscNodeDataPoolFiler`, `C_OscNodeCommFiler`,
`C_OscViewFiler`, `C_OscHalcConfigFiler`, `C_OscCanOpenManagerFiler`. Use `CalcHash`
as a deep-equality oracle — 53 classes provide it.

### 3. Phase 7.2 — hardware CRC32

Add an `#ifdef __SSE4_2__` path using `_mm_crc32_u32` / `_mm_crc32_u8`, keep the
software CRC as fallback, and benchmark to confirm the improvement is real.

Well specified, self-contained, and the Google Benchmark harness already exists
(`libraries/opensyde_core/bench/`, `-DOPENSYDE_CORE_BUILD_BENCHMARKS=ON`).
**Measure in Release** — see the benchmark note in `CLAUDE.md`; a Debug number here
is a measurement of a different program.

### 4. Endian reassembly hand-inlined at ~25 sites

No shared `h_GetU32Big` / `h_GetU32Little` helper exists, so protocol drivers
reassemble multi-byte values by hand. Mechanical, low risk, and each site is a
chance for a transposition bug.

### 5. `C_HexFile` error-code migration incomplete

`hex_file/C_HexFile.cpp:2225` and `:2338` — `GetDataByAddress` and `FindPattern`
still return raw `int32_t` despite the class having its own
`C_HexFileErrorCategory`. Migrate via `m_MakeError`, return data by value or span.

`tests/test_hex_file.cpp` already characterises both functions.

### 6. Security AES file — duplicated I/O and a hand-coded header

`security/C_OscSecurityAesFile.cpp` — the read/write blocks are duplicated between
encrypt and decrypt, and the 56-byte header is encoded and decoded by hand with
literal offsets. Extract shared helpers. The literal offsets are the risk: a header
change has to be made correctly in two places.

### 7. Filer error-API migration

~130 remaining call sites, roughly 270 lines. Mechanical, and the shape is settled
by the `std::expected` pilot recorded in `FINDINGS.md`: **start from the
value-or-default callers and leave the bridge sites alone** until the conventions
are unified. Converting a bridge site costs about four lines and buys nothing.

### 8. 135 range-for conversions

Blocked on naming judgement rather than on safety. Mechanical singularisation
produces `Entrie` and `SubNodeIndexe`, so each needs a human to pick the name.

### 9. Phase 7.3 — move semantics

Move constructors and assignment on large classes, `std::move` in hot paths. Worth
doing only where a benchmark shows it matters — 7.1 is the cautionary tale: the
plan's prescribed fix was 1.4x and the actual win came from somewhere the plan had
not looked.

### 10. Low / cosmetic bucket

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
