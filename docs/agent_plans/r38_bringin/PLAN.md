# Bringing upstream R38 (26-0C) changes into the fork — plan

Companion to `AUDIT.md` (the findings). This plan sequences the bring-in of the
upstream R37→R38 delta into the fork's `develop` branch. Active work — update this
file as it proceeds; archive it the day it stops being the working document.

Reference SHAs: R37 = `26-0B` (`1d3ac3395`), R38 = `26-0C` (`f4f195a44`). The fork
branched at exactly R37, so the delta is fully new to `develop`.

## Goal

Bring in the subset of R38 that is worth having, in a way that compiles and passes
the existing suite on all three platforms, without dragging in the parts we reject.

## Scope decision

The audit (AUDIT.md) splits R38 into independent and coupled work. **Recommended
default scope**: Tier 1 + Tier 2 core fixes/features, the self-contained GUI fixes,
and the SYDEsup config-file support — **defer the Crypto Agent as a gated, separate
decision** (phases 4–6 below). The Crypto Agent is not a cherry-pick; it is a port of
a coupled set with several fork-specific bridges (missing `C_OscConfFileHandler`,
tgl functions, dispatcher port/error-model drift) and a new daemon executable. It
should be its own effort with its own product gate.

| Scope | Include? | Rationale |
|---|---|---|
| Tier 1 core bugfixes | **Yes** | Real defects; self-contained; clean picks. |
| Tier 2 independent core features | **Yes** | miniz, DoIP/IP, logging knob, utils, `user_devices.ini`. |
| GUI self-contained fixes | **Yes** (with string de-wrapping) | DBC signal matching, device paths, trigger null-safety. |
| SYDEsup config-file support | **Yes** | Mostly portable; strip crypto fields. |
| Crypto Agent (daemon + core flow) | **Gated** | Big, coupled, fork-bridges; separate decision. |
| Cast-cleanup / reflow / rc bumps | **No** | Zero value, max conflict. |

## Branch and workflow strategy

- Work on the fork's `develop` line. Because the delta is one squashed upstream commit
  and `develop` has diverged, **do not attempt `git cherry-pick` of the squashed
  commit** — it would conflict on nearly every hunk. Instead **port each subsystem by
  diff and manual apply**, pulling the concrete patch with
  `git diff 26-0B 26-0C -- <path>` and adapting to the fork's code (consolidated core,
  `std::string`, `std::error_code`, no i18n).
- **Never merge/upstream-track from `upstream/master` straight into `develop`.** The
  fork's consolidation (single core, i18n stripped, Qt6) means a merge would bring in
  the per-tool vendored core copies and the `C_GtGetText` strings wholesale.
- Keep each phase as its own commit (or small PR) so a rejected part is easy to drop
  and the diff stays reviewable. Prefer PRs through CI: anything touching a core
  signature must be validated on all three platforms (see CLAUDE.md "Verifying
  tool-tree changes").
- Note: pushes route through the 48-core build host; the Mac has no push credentials
  (see `docs/remote-build.md`).

## Verification baseline (do this before phase 1)

Confirm the fork currently builds clean before bringing anything in, so any new
failure is attributable to the port, not pre-existing drift:

```bash
git submodule update --init --recursive
./build.sh -b Debug all        # all eight tools; ~3 min on the 48-core host
cmake -S libraries/opensyde_core -B build/core -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug -DOPENSYDE_CORE_BUILD_TESTS=ON \
  -DOPENSYDE_CORE_SKIP_WINDOWS_DRIVERS=ON -DOPENSYDE_CORE_SKIP_WINDOWS_TARGET=ON
cmake --build build/core -j $(nproc) && ctest --test-dir build/core --output-on-failure
```

Check the toolchain a build directory actually uses before trusting a result:
`grep CMAKE_CXX_COMPILER_AR build/Debug/CMakeCache.txt` (see CLAUDE.md gotchas — the
host's `build/Debug` once silently stayed on GCC 14).

## Phase 1 — Tier 1 core bugfixes (clean picks)

**Status: done on 2026-09-22.** Applicability differed from the audit because the
fork's core has diverged structurally. Landed (each committed-ready, tested in the
core suite on clang):
- fix #1 `ip_dispatcher` `m_ConnectTcp` SO_ERROR + timeout-as-error (Linux + Windows)
- fix #3 `C_HexFile` block sorting (`C_HexDataDumpBlock::operator<` +
  `C_HexDataDump::SortBlocksByAddressOffset`; `CreateHexFile` sorts a copy) —
  adapted to the fork's `std::vector` (no `C_SclDynamicArray`)
- fix #4 (CBC part) `C_OscSecurityAesCbc::h_Decrypt` zero-length guard
- fix #5 data-logger additional-trigger `c_Operation` defaults to `"=="` (ctor + filer)

**Not applicable to the fork (recorded so nobody retries them):**
- fix #2 (`C_Can.cpp` DLC/zero-init/name copy) — the fork has no such file; CAN goes
  through a libcan adapter in `can_dispatcher/adapter/` into the can-libraries
  submodule.
- fix #6 (`C_OscDeviceManager::AddDevice` relative-path) — the fork's device manager
  is a rewritten `std::filesystem` scanner (`LoadFromPaths` on `device.syd`
  manifests) with no upstream-style `AddDevice`.
- fix #4 (file part `C_OscSecurityAesFile`) — the fork's file AES is GCM-based, not
  CBC, so upstream's `%16`/zero-length file guard does not map.

New regression tests added: `HexFile.CreateHexFile_SortsOutOfOrderBlocksByAddress`,
`SecurityAesFile.AesCbcRejectsEmptyInput`,
`DataLoggerTriggerParser.AdditionalTriggerOperationDefaultsToEqual`.

Port each of these from the core delta into `libraries/opensyde_core`, as separate
commits so each is independently droppable:

1. `ip_dispatcher/{linux_sock,windows_win_sock}` `m_ConnectTcp` SO_ERROR fix +
   timeout-as-error.
2. `can_dispatcher/target_linux_socket_can/C_Can.cpp` — DLC clamp, frame zero-init,
   bounded `ifr_name` copy. (Linux-only file; the fork already compiles it.)
3. `hexfile/C_HexFile` + `C_HexDataDump` block sorting — as a unit with the
   `scl/C_SclDynamicArray.hpp` `c_TheVector` public rename and
   `C_HexDataDumpBlock::operator<`.
4. `security/C_OscSecurityAesCbc`/`C_OscSecurityAesFile` empty-input guards.
5. `project/system/node/data_logger/...AdditionalTriggerProperties` ctor + filer
   default `"=="`.
6. `project/system/C_OscDeviceManager.cpp` `AddDevice` relative-path resolution (via
   `C_OscDeviceGroup::h_GetFullDevicePathFromDeviceIniEntry`).

Verify after each: incremental core build + `ctest`; then one full
`./build.sh -b Debug all` for anything touching a header that other tools include.

## Phase 2 — Tier 2 independent core features

**Status: partially done on 2026-09-22.** Done and committed (each verified in the
core suite on clang):
- miniz 2.0.7 → 3.1.0 (vendored; wrapper unchanged except removing the obsolete
  `MINIZ_NO_ZLIB_COMPATIBLE_NAMES` define)
- `C_TglFileSearchRecord::u64_LastWriteTimeUtcSeconds` (Linux via `stat`, Windows via
  FILETIME conversion)
- `C_OscLoggingHandler::h_SetConsoleMinLogType` + `E_LogType`
- `C_OscUtils::h_StringToIp4` / `h_Ip4ToString`

**Not applicable to the fork:** the optional `user_devices.ini` load in the system
definition filer (#6) — it depends on an ini-based `C_OscDeviceManager::LoadFromFile`
that the fork no longer has (its device manager is a rewritten `std::filesystem`
scanner over `device.syd` manifests).

**Remaining (substantial, entangled):** C_OscIpDispatcher port configurability (#5,
port member + `InitTcp` overload) and DoIP-over-IP (#7: `C_OscProtocolDriverOsyTpIp`
node-ID-only broadcast overloads, `C_OscBuSequences`/`C_OscDcBasicSequences` IP
`Init` + `ConfigureDeviceBySerialNumber`). These are the fork's error_code-migrated
protocol drivers and should be ported as one unit, carefully, against the existing
virtual-ECU tests.

**Updated 2026-09-22:** #5 (dispatcher port) is **done and committed**
(`C_OscIpDispatcher` port member + 3-arg `InitTcp` overload; both impls use the
member; default 13400 unchanged; 460 tests pass).

## Phase 3 update (2026-09-22)

**conf_file_handler prerequisite (workstream 1) done** (`7e830e80b`):
`C_OscConfFileHandler` ported into `libraries/opensyde_core/conf_file_handler/`
(COMMON group), adapted to `std::string`/`std::error_code` and the surviving
`C_SclStringUtil` list helpers. Tested by `test_conf_file_handler.cpp`.

**GUI self-contained fixes (workstream 2) done**, committed as two PRs, all
de-wrapped of `C_GtGetText`:
- `d4fd3302d` — `C_OgeSpxInt64::SetSuffix` (capture value before changing suffix);
  `C_GiSvNodeSyvUpdate` security-state guards + stale top-left icon clear;
  `C_SdNdeDalLogJobAdditionalTriggerPropertiesWidget` null-safety in
  `SetNodeDataLoggerJob`/`resizeEvent`/`m_ToggleExpertView`/`m_SetExpertTriggerCondition`
  + `"=="` combobox default.
- `71289fae6` — `C_CieUtil` DBC re-import signal matching by name
  (`mh_HandleSignalsOfMatchingMessage`), which keeps the int32_t STW convention.

**Not applicable to the fork (recorded so nobody retries):**
- `C_SdTopologyToolbox` call-site change (`h_GetExePath` → `h_GetDevicesIniPath`) —
  the fork's topology toolbox has no user-devices-ini drag/drop code (it was rewritten).
- `C_PuiUtil::h_ChangeRelativePathsInUserDevicesIniToDevicesFolder` + the
  `C_NagMainWindow` ctor call — the fork device manager is a `std::filesystem`
  scanner (`LoadFromPaths` over `device.syd` manifests); nothing reads
  `user_devices.ini`. Added-then-reverted to avoid dead code.
- `C_Uti::h_GetDevicesIniPath` — no consumer in the fork (kept `h_GetPemDbPath`).

**SYDEsup config-file support (workstream 3) done** (`3136ed90f`): `C_SupConfig`
(a `C_OscConfFileHandler` subclass), `-c`/`--configfile` repurposed from
`--certificatesdir`, and two-pass `getopt_long` so the config file is found at any
position and CLI args override it. Crypto-agent fields stripped (per plan);
`C_OscSecurityPemDatabase` kept and still passed to the sequence `Init` (no
sydesuplib/pem-db protocol rewiring). The certificates-dir CLI path is removed to
match upstream, so the PEM db is no longer populated from the CLI until the
crypto agent or a config key for it lands. Verified: SYDEsup builds; a config file
with quiet=true suppresses the banner and its package path is honored; a missing
config path reports the load error.

**Phase 3 is now complete** (conf_file_handler + self-contained GUI fixes +
SYDEsup config-file). All eight tools build locally; Phase 1–3 are the full
non-gated R38 bring-in.

#7 (DoIP-over-IP) is **done and committed on 2026-09-22.** Landed in two commits:
- `1ad3be29e` — `C_OscProtocolDriverOsyTpIp` node-ID-only broadcast overloads; the
  full `BroadcastSetIpAddress`/`BroadcastSetIpAddressExtended` bodies now live in
  private `m_Broadcast*` helpers taking an explicit mode flag (0x02 node-ID-only,
  0x03 IP + node). Full public signatures unchanged (in-tree callers unaffected).
- `d4d7d45e7` — DoIP-over-IP in `C_OscDcBasicSequences` and `C_OscBuSequences`:
  IP `Init` overloads, `ConfigureDeviceBySerialNumber`, and `m_Broadcast*` wrappers
  selecting CAN vs IP.

Adaptations from upstream: the fork keeps `mc_TpCan` as a value member and only
allocates `mpc_TpIp` when an IP dispatcher is given (per-class, selected by which
dispatcher `Init` received) rather than upstream's always-pointer `mpc_TpCan`; all
returns are `std::error_code`, strings `std::string`. The CAN-path virtual-ECU
sequence test (`DcBasicSequencesVirtualEcu.ScanConfigureAndResetTwoDevicesOnTheBus`)
still passes; an IP-path test covers `ConfigureDeviceBySerialNumber`'s node-ID-only
broadcast. 465 core tests green. The `opensyde` GUI tool does **not** build on
`develop` at present — a pre-existing `QString == std::string` break in
`C_SyvUpPacSectionNodeDatablockWidget` (from `d56bac7b5`), unrelated to this port;
all seven other tools build.

1. **miniz 2.0.7 → 3.1.0** (drop in new `miniz.c/h`, `ChangeLog.md`, `readme.md`;
   remove the now-unused `#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES` from
   `zip/C_OscZip{Data,File}.cpp`).
2. `C_TglFileSearchRecord::u64_LastWriteTimeUtcSeconds` (linux + windows tgl).
3. `C_OscLoggingHandler::h_SetConsoleMinLogType` + `E_LogType`.
4. `C_OscUtils::h_StringToIp4` / `h_Ip4ToString`.
5. `C_OscIpDispatcher` port overload + `mu16_UdpTcpPort` member, and the
   `C_OscIpDispatcherImpl` typedefs (or reuse the fork's existing platform alias
   pattern instead of adding a new header — preference is to reuse, per CLAUDE.md's
   "no backward-compat shims" rule; decide in review).
6. Optional `user_devices.ini` load in `C_OscSystemDefinitionFiler`.
7. **DoIP over IP**: `C_OscProtocolDriverOsyTpIp` node-ID-only broadcast overloads,
   `C_OscBuSequences` IP `Init`, `C_OscDcBasicSequences` IP `Init` +
   `ConfigureDeviceBySerialNumber`.

These are additive; verify each with the core suite, then a full eight-tool build.

## Phase 3 — GUI self-contained fixes + SYDEsup config file

Bring in `libraries/opensyde_gui` / `opensyde_tool` items **only** the ones that are
self-contained, and de-wrap every `C_GtGetText::h_GetText(...)` string (`AUDIT.md`,
"Systemic gotcha"):

- `com_import_export/C_CieUtil` DBC re-import signal-name matching.
- `C_PuiUtil::h_ChangeRelativePathsInUserDevicesIniToDevicesFolder` +
  `C_NagMainWindow` ctor line + `C_SdTopologyToolbox` call sites + `C_Uti` add
  `h_GetDevicesIniPath` (keep `h_GetPemDbPath` unless the crypto agent is taken).
- `C_SdNdeDalLogJobAdditionalTriggerPropertiesWidget` null-safety + "Equal" default.
- `C_OgeSpxInt64::SetSuffix` fix.
- `C_GiSvNodeSyvUpdate` security-state guards + stale-icon clear.
- **SYDEsup** config-file support: port `C_SupConfig`, the `-c` repurpose, and the
  two-pass `getopt_long`. To stay independent of the crypto agent, **strip the
  crypto-agent fields** (client-auth keys, `C_OscCryptoAgentSettings`,
  auto-start/stop) and keep only the config-file mechanism. This needs
  `C_OscConfFileHandler` + `C_OscUtils::h_StringToIp4` ported to the fork core first
  (add `conf_file_handler/`).

Note the fork's SYDEsup still ships a `sydesuplib` shared library and `std::string`
types; do **not** adopt the `sydesuplib` API removal or the crypto-auth rewiring in
this phase.

## Gated phases — Crypto Agent (only if product decision is "yes")

These are a coupled set and should be treated as one effort, not cherry-picked
piecemeal. Do them after phases 1–3 land. The fork-specific bridges in `AUDIT.md`
("Fork-specific bridges") must all be resolved first.

### Phase 4 — core prerequisites for the agent
- Port `conf_file_handler/` into `libraries/opensyde_core` (add the COMMON group).
- Add the tgl functions the feature needs (`TglTasks` with `TglStartProcessDetached`,
  and `TglFileExists`/`TglExtractFileName`/`TglFileIncludeTrailingDelimiter`/
  `TglChangeFileExtension` if absent).
- Port `C_OscIpDispatcher` port mechanism (3-arg `InitTcp`, `mu16_UdpTcpPort`) onto
  the fork's `std::error_code` API.
- Port `security/crypto_agent/{C_OscCryptoAgentSettings,C_OscCryptoAgentAccessUtil}`.

### Phase 5 — protocol-driver refactor (breaks existing callers)
- `C_OscComDriverProtocol`/`C_OscComDriverFlash`/`C_OscComSequencesBase::Init`
  drop the PEM-db param; `m_SetNodeSecurityAccess` → virtual
  `m_HandleCryptoAgentCommunication`; `ReConnectNode` extra out-param;
  `C_OscSystemDefinition::Get/SetLastLoadedFilePath`;
  `C_OscSecurityPemDatabase` level-7 removal.
- **Update every in-fork caller** of the old `Init(..., pemDb)` signature and any
  `GetLevel7PemInformation`/`AddLevel7PemFile` user. Grep for comparisons, not just
  assignments: `grep -rn "SecurityPemDatabase" --include=*.cpp --include=*.hpp`.

### Phase 6 — the daemon tool
- Port `opensyde_crypto_agent/src/**` (Linux impl first; keep the
  `C_CaServer`/`C_CaServerLogic` split so Windows can be added later), its `pjt`
  CMake adapting to link the **single** consolidated core (do not copy the vendored
  `libs/opensyde_core` `add_subdirectory`), and its `src/test/**` pytest harness as
  the acceptance test. Do **not** copy the upstream `-std=c++03` toolchain — the fork
  uses a modern standard.
- Add the `OPENSYDE_CORE_SKIP_SECURITY_CRYPTO_AGENT_UTILS` group to the fork core
  CMake so tools that don't use the util can skip it (mirror upstream; SYDEflash and
  CAN Monitor set it to 1).

## Risks / watch-items

- **`std::error_code` vs int32 STW codes.** Several upstream core functions return
  plain `int32_t`; the fork migrates to `std::error_code`/`Errc`. When porting, bridge
  with `make_error_code_from_stw(...)`, never `static_cast<Errc>(...)`
  (CLAUDE.md). Check each callee's own `\return` block — not every `int32_t` is an STW
  code (TGL helpers, miniz, OpenSSL).
- **libc++ vs libstdc++ (Linux vs macOS/Windows).** Anything ported that uses
  `std::to_chars`/`std::from_chars` must guard on `__cpp_lib_to_chars` and fall back
  to `strtod_l`/`_strtod_l` under an explicit `"C"` locale — libc++ 17/19 declares but
  deletes `std::from_chars` for floating point (CLAUDE.md; round-trip tests fail
  otherwise).
- **miniz 3.1.0** is a vendored-library replacement; re-run the zip/filer round-trip
  tests.
- **Core signatures are shared.** Any signature change in core requires a full
  eight-tool verification, not just the core build. Grep is not a substitute for a
  build (CLAIM: callers reach migrated classes via base pointers and share method
  names).
- **CI is the only platform-proof.** Touch platform-conditional code or a core
  signature → go through a PR and read the CI result (all three platforms) before
  drawing conclusions.

## Definition of done

For each phase: the tools affected build on all three platforms, the core suite
(`ctest`) is green, no `Q_UNUSED`/back-compat shims introduced, and any ported string
is merged as plain literal (no `C_GtGetText`). A debug-clean full `./build.sh -b Debug
all` plus a passing `ctest` on the host signals the phase is shippable; a PR through
CI is the gate for anything touching core.
