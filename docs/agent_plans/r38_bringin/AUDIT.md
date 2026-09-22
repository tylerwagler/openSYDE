# R37 → R38 upstream bring-in — audit findings

Companion to `PLAN.md`. Records what changed between Release 37 (`26-0B`,
`1d3ac3395`) and Release 38 (`26-0C`, `f4f195a44`) on the upstream `master` line,
and what is worth bringing into this fork. Last audited 2026-09-22.

## Ground rules

- Upstream ships **no granular commit history** on `master`: the whole release is a
  single squashed commit (`f4f195a44 "Release 38 / 26-0C"`). Everything below was
  derived from the aggregate diff, not from per-feature commit messages. There is no
  upstream `develop`/release branch to decompose against (verified via
  `git ls-remote`).
- Upstream **vendors the core library inside each tool** (`*/libs/opensyde_core/`),
  so all "core" work shows up as per-tool edits. This fork consolidated that into a
  single `libraries/opensyde_core`; the audit normalizes to "the core delta" from the
  `opensyde_tool/libs/opensyde_core/` copy (byte-identical to the `syde_x_gen` copy at
  R38, verified by blob hash).
- The fork branched at **exactly R37** (`1d3ac3395` is the merge-base of
  `origin/develop` and `upstream/master`), has **no** crypto agent, and shares only
  **94** of R38's 1,223 changed filenames. Most of R38 is genuinely new to us.

## Scope of the release

This is a **tool-level release**. Zero changes to `opensyde_core`/`opensyde_gui` as
separate directories (they are vendored upstream), and zero changes to CAN Monitor,
SYDEflash, or TSP convert.

| Area | Files | What it is |
|---|---|---|
| `opensyde_crypto_agent` | 836 | **Brand-new tool** — the Crypto Agent daemon |
| `opensyde_tool` | 164 | Main GUI |
| `opensyde_syde_sup` | 83 | SYDEsup CLI |
| `opensyde_syde_x_gen` | 67 | Generator |
| `opensyde_syde_coder_c` | 67 | Generator |
| `opensyde_cmd_line_flash_tool` | 4 | CLI flash (all deletions) |
| — | 2 | `license.txt`, `openSYDE_Core_documentation.chm` |
| — | ~60 | the **core delta** (inside each tool's vendored `libs/opensyde_core/`) |
| — | 94 | `opensyde_tool/src` + `pjt` (GUI, non-core) |

## The dominant theme: the Crypto Agent security re-architecture

R38 moves **RSA-1024 challenge-response authentication out of the client tools and
into an external daemon**. The old model held the private keys in a per-client PEM
database and signed the 8-byte challenge in-process. The new model: a standalone
`osy_crypto_agent` service owns the keys in its own `.pem` folder, and clients ask it
over TCP (default port **50963**, bind **127.0.0.1**) to solve the challenge. The
reference implementation is explicitly meant to be derived/replaced
(`opensyde_crypto_agent/doc/create_your_own/`).

This is a **coupled set** — take it all or none:

- **Core module (client side)** `security/crypto_agent/{C_OscCryptoAgentSettings,
  C_OscCryptoAgentAccessUtil}.{hpp,cpp}` — static client over
  `C_OscIpDispatcher`; no Qt/OpenSSL/boost, only core infra.
- **The daemon** `opensyde_crypto_agent/` — `C_CaApplication`, `C_CaServer`,
  `C_CaServerLogic`, Linux/Windows platform impls (same class name, selected by
  CMake), Python pytest suite with a hardcoded expected RSA signature, vendored
  OpenSSL 3.0.0 headers (Windows only) + full vendored core. Linux + Windows only,
  no macOS.
- **Protocol-driver refactor**: `C_OscComDriverProtocol/Flash` and
  `C_OscComSequencesBase::Init` **drop the `C_OscSecurityPemDatabase*` parameter**;
  `m_SetNodeSecurityAccess` replaced by a new virtual `m_HandleCryptoAgentCommunication`;
  `ReConnectNode` gains an optional `opu32_ErrorActiveNodeIndex` out-param.
- **`C_OscSecurityPemDatabase`** loses its level-7 handling
  (`GetLevel7PemInformation`/`AddLevel7PemFile` deleted).
- **`C_OscSystemDefinition::GetLastLoadedFilePath`** (+ `SetLastLoadedFilePath`) —
  fed to the agent so it can locate the loaded system definition.
- **`TglStartProcessDetached`** (Linux `pipe`+`fork`+`setsid`+`execv`; Windows
  `CreateProcessA`), new `C_OscIpDispatcher` port overload + `mu16_UdpTcpPort`
  member, new `C_OscIpDispatcherImpl` typedef headers, and a new
  `OPENSYDE_CORE_SKIP_SECURITY_CRYPTO_AGENT_UTILS` CMake group.

## Fork-specific bridges (the real cost of the Crypto Agent)

Verified against the fork's `libraries/opensyde_core` on 2026-09-22:

| Gap | Detail |
|---|---|
| `conf_file_handler/` | **Absent** from the fork's consolidated core; upstream has it in the COMMON core group. The daemon cannot build without it. |
| `security/crypto_agent/` | Absent (this is what gets ported for the client side). |
| `C_OscIpDispatcher` API drift | Fork uses `std::error_code`/`Errc` and a 2-arg `InitTcp(ip, handle)` with **no port concept**; upstream uses int32 STW codes + a 3-arg port-aware `InitTcp` (default member 13400). Needs either porting the port mechanism (small, backward-compatible) or a raw-socket client. |
| `C_OscIpDispatcherImpl` | No such typedef in the fork; it already uses a `C_OscIpDispatcherPlatform`-style alias pattern. |
| tgl layer | Fork `tgl/linux/` has only `TglFile`, `TglTime`, `TglUtils` — **no `TglTasks`**. Missing functions used by the feature: `TglStartProcessDetached`, `TglFileExists`, `TglGetExePath`, `TglExtractFileName`, `TglExtractFileExtension`, `TglFileIncludeTrailingDelimiter`, `TglChangeFileExtension`. |
| `C_OscSecurityRsa::h_SignSignature` | Returns `std::error_code` in the fork (int32 in R38) — adapt the daemon's `C_CaServerLogic` call site. |
| `C_OscSecurityPemDatabase` | Fork still has `GetLevel7PemInformation`/`AddLevel7PemFile` and `Init` still takes the PEM-db pointer, i.e. fork matches the R37 client behavior. The full protocol-driver diff must be ported together, and all in-fork callers of the old `Init` signature updated. |
| Core `SKIP_*` groups | Fork core lacks `OPENSYDE_CORE_SKIP_LINUX_TARGET` and `OPENSYDE_CORE_SKIP_SECURITY_CRYPTO_AGENT_UTILS`; group names differ from upstream. |
| Platform defaults | Default executable path is Windows-shaped (`../connectors/crypto_agent/osy_crypto_agent.exe`) in both `C_OscCryptoAgentSettings::SetDefault` and `C_UsFiler`; GUI file-pick filter is `*.exe`. Revisit for Linux-first. |
| Toolchain | Upstream daemon is built with `-std=c++03` toolchain flags but uses C++17 nested namespaces — do not copy the toolchain file verbatim. |

## What to bring in — ranked

### Tier 1 — independent, high-value bugfixes (clean picks)

Self-contained, depend only on APIs already present at R37.

| Fix | Where | Why |
|---|---|---|
| TCP connect `SO_ERROR` detection + timeout-as-error | `ip_dispatcher/{linux_sock,win_sock}::m_ConnectTcp` | Non-blocking connect treated "writable" as success; a timed-out connect was silently accepted. |
| CAN frame `DLC` clamp, zero-init, bounded `ifr_name` copy | `can_dispatcher/target_linux_socket_can/C_Can.cpp` | Sent garbage beyond DLC; unbounded-ish name copy. |
| Hex-file block ordering | `hexfile/C_HexFile` + `C_HexDataDump::SortBlocksByAddressOffset` | Wrong hex if blocks not ascending. Take as a unit with `C_SclDynamicArray::c_TheVector` public rename + `C_HexDataDumpBlock::operator<`. |
| AES/AesFile empty-input `C_CONFIG` guards | `security/C_OscSecurityAesCbc`, `C_OscSecurityAesFile` | Reject zero-length / non-multiple-of-16 input. |
| Data-logger trigger operation default `"=="` | `...AdditionalTriggerProperties` ctor + filer | Undefined comparison on empty op. |
| `C_OscDeviceManager::AddDevice` relative-path resolution | `project/system/C_OscDeviceManager.cpp` | Device files referenced relative to `devices.ini` now load correctly. |
| DBC re-import signal matching by name | GUI `com_import_export/C_CieUtil` | Preserves signal identity across re-imports. Best GUI item. |
| `user_devices.ini` relative-path fix | GUI `C_PuiUtil` + `C_NagMainWindow` + `C_SdTopologyToolbox` + `C_Uti::h_GetDevicesIniPath` | Store new device paths relative to `../devices`. |
| Data-logger trigger null-safety + "Equal" default combo | GUI `...DalLogJobAdditionalTriggerPropertiesWidget` | Crash guard + sensible default. |
| `C_OgeSpxInt64::SetSuffix` fix | GUI sparkline/spin-box | Suffix change no longer mangles the value. |
| Security-state guards / stale-icon clear | GUI `C_GiSvNodeSyvUpdate` | Don't read security state from unconnected sub-nodes. |

### Tier 2 — independent new features (nice-to-haves)

- **miniz 2.0.7 → 3.1.0** (ZIP64, `inflateReset`, ...). `C_OscZip*` wrappers
  untouched — low-risk vendored-library replacement.
- `C_TglFileSearchRecord::u64_LastWriteTimeUtcSeconds` (both platforms).
- `C_OscLoggingHandler::h_SetConsoleMinLogType` — suppress console chatter
  (purely additive).
- `C_OscUtils::h_StringToIp4` / `h_Ip4ToString`.
- `C_OscIpDispatcher` port configurability (used by DoIP + crypto agent).
- Optional `user_devices.ini` load in `C_OscSystemDefinitionFiler`.
- **DoIP over IP**: `BroadcastSetIpAddress` node-ID-only variants (`OsyTpIp`),
  `C_OscBuSequences` IP `Init`, `C_OscDcBasicSequences` IP `Init` +
  **`ConfigureDeviceBySerialNumber`** (assign a node ID over the bus without knowing
  its current ID). Independent of the crypto feature and genuinely useful.
- **SYDEsup config-file support** (`C_SupConfig` + `-c` repurposed to `--configfile`,
  two-pass `getopt`). Mostly portable, but pulls in `C_OscConfFileHandler` +
  `h_StringToIp4` (both absent in the fork). The config-file part is the best partial
  port — the crypto-agent fields are strippable if that feature is deferred.

### Skip

- **All cast-cleanup/reflow** (~half the diff, ~30 files incl. the bus_edit table
  models) — zero value, maximum conflict surface.
- Windows `.rc` version bumps (`resources.rc` ×3, `version_config.hpp`) and the
  flash-tool `.a` deletions — irrelevant to a Linux fork.
- Qt6 focus fixes (`C_SyvDaDashboardSceneWidget`, `C_GiSvDaRectBaseGroup`) — the fork
  is already Qt6; verify before porting.
- `x_gen`/`coder_c` `SKIP_LINUX_DRIVERS` — already present in the fork.
  `SKIP_SECURITY_CRYPTO_AGENT_UTILS` — meaningless until the crypto agent is taken.
- SYDEsup `sydesuplib` shared-library API removal — an **API break** vs the fork,
  which still ships that library. Deliberate decision.
- Node "update disabled" status-logic rework — entangled with the security flow, and
  exactly the kind of logic a mature fork has already reworked.

## Systemic gotcha

Upstream routes **every new string through `C_GtGetText::h_GetText`** (R38 adds ~25
strings in the settings dialog and rewrites ~6 in auth dialogs). The fork is
English-only / gettext-stripped, so **any hunk touching those lines conflicts
regardless of which feature is picked**. Every GUI cherry-pick needs manual
string de-wrapping (`C_GtGetText::h_GetText("...")` → `"..."`).

## Notes on upstream quirks worth recording

- The daemon sources use C++17 nested-namespace syntax
  (`namespace stw::osy_crypto_agent`) while both upstream toolchain files force
  `-std=c++03` and CMakeLists sets no `CMAKE_CXX_STANDARD` — inconsistent as shipped.
- The daemon's TCP protocol is **neither authenticated nor encrypted**
  (`doc/create_your_own/deriving.txt` warns about this for remote deployments).
- Default port `50963`, bind `127.0.0.1`, protocol version `0x0001`, RSA-1024 only,
  STW manufacturer id `0x00`.
