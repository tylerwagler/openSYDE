# Core Sweep — Modernization / Optimization / Dedup / Smells

A four-dimension audit of `libraries/opensyde_core` (run 2026-09-09): modernization,
optimization, deduplication, and design smells / correctness. Findings are verified
against source, ranked by impact. This is the tracking document for the fixes.

**Approach:** one sub-project at a time. `opensyde_core` first; other tool trees are
out of scope until the core findings are reviewed and addressed. Every fix must keep
the core build `-Wall -Wextra -Werror` clean and `ctest` green.

Audit dimensions (each an independent sweep of the whole core):
1. C++ modernization — raw `new`/`delete`, C casts, manual buffers, macros, `NULL`.
2. Performance — copies, string building, container scans, hot-path allocations.
3. Deduplication — copy-pasted helpers across files, type-twin functions, duplicated constants.
4. Smells & correctness — global state, missing `const`, error-code misuse, dead code.

Status legend: `[ ]` open · `[~]` in progress · `[x]` done · `[wontfix]` rejected.

---

## Tier 1 — Real bugs

- [x] **1. Memory leak: `C_OscComDriverProtocol::mc_OsyProtocols` never freed**
      `protocol_drivers/communication/C_OscComDriverProtocol.{hpp,cpp}` + `C_OscComDriverFlash.cpp`.
      Every `C_OscProtocolDriverOsy` allocated in `Init()` leaks on destruction. Convert the
      owning raw-pointer vectors/members to `std::unique_ptr`. **Fixed** as part of item 10.
- [x] **2. Per-instance data clobbered by a `static` local**
      `protocol_drivers/communication/C_OscComDriverBase.cpp:316` — `static uint32_t
      hu32_BusLoadTimeRefresh` shared across all driver instances, not thread-safe, first call
      computes a bogus load over whole uptime. **Fixed:** member `mu32_BusLoadTimeRefresh`, ctor init.
- [x] **3. Unsynchronized global cache dangles live pointers**
      `protocol_drivers/C_OscProtocolSecuritySubLayer.cpp:35` — process-wide
      `static std::map<...> mhc_TheConfig;`, no lock; `h_ClearAll()` clears it while protocol
      objects hold raw pointers from `h_GetConfigByNodeId()`. **Fixed (remove global registry, per
      user decision):** each `C_OscProtocolDriverOsy` now owns its `mc_SecuritySubLayer`; the
      static map and `h_GetConfigByNodeId`/`h_ClearAll` are removed; `C_OscComDriverProtocol`
      gains `IsTrafficEncryptionActive(nodeId)`; GUI call sites (C_SyvComDriverDiag + the
      dashboard encryption-status helper from C_SyvDaDashboardSelectorTabWidget) thread the diag
      driver in. Core + 226 ctest pass; GUI build in progress.
- [x] **4. Dead / nonfunctional `TglSetEnvironmentVariable`**
      `tgl/linux/TglUtils.cpp:194` — stub asserting + returning `-1`, real `putenv` body behind
      `#if 0`. **Fixed:** implemented with `setenv()` (copies both strings, no POS34-C hazard).

## Tier 2 — High-value inefficiency

- [x] **5. CAN-monitor per-byte formatting = tens of thousands of allocs/sec**
      `scl/C_SclStringUtil.hpp:398` `PrintFormattedCompat` used per header + per data byte in
      `cmon_protocols/C_CanMonProtocol.cpp:96-125` and `C_CanMonProtocolL2.cpp:67-85`. **Fixed:**
      `MessageToStringLog`/L2 `MessageToString` now `reserve` the line once and append each byte
      from a small stack buffer via `snprintf` (no per-byte heap allocation).
- [x] **6. INI lookups allocate two strings per comparison**
      `scl/C_SclIniFile.cpp`, `VectorIndexOf`/`ListIndexOfName`, system-definition/halc/project
      validation. **Fixed:** added allocation-free `EqualsCaseInsensitive`/`FindCaseInsensitive`
      to `scl/C_SclStringUtil.hpp` and replaced ~30 `LowerCaseCompat(a) == LowerCaseCompat(b)`
      comparisons (which allocated two strings each).
- [x] **7. `GetValueArrXxx` return whole arrays by value**
      `project/system/node/C_OscNodeDataPoolContent.{hpp,cpp}`. **Fixed:** dropped the meaningless
      `const` on the value returns. Audit noted the copy concern; the copies were already
      RVO/move-elided, and returning `const&` for the decoded (non-uint8) array types isn't
      feasible without added state, so only the misleading `const` was removed.

## Tier 3 — Mechanical dedup / modernization wins

- [x] **8. `mh_IntToHex` copy-pasted → use shared `stw::scl::IntToHexCompat`**
      The audit under-scoped this: **~20** duplicate `mh_IntToHex` helpers exist, not 5. Consolidated
      all of them (cmon_protocols ×3, security, protocol_drivers ×3, imports ×2, exports ×5,
      xml_parser, data_dealer) to `stw::scl::IntToHexCompat`; removed every private copy.
- [x] **9. `h_SetValueInContent` ~306 lines of 16 hand-duplicated type dispatches**
      `project/system/node/C_OscNodeDataPoolContentUtil.cpp:146-451` — 8-way clamp + 8-way plain
      assignment. **Fixed:** replaced with a file-local `mh_SetIntegerValueInContent<T>` template
      + a compact 8-case switch (~300 lines → ~110). Behavior preserved.
- [x] **10. Raw owning pointer containers → `std::unique_ptr`**
      `C_OscSuSequences.cpp` (`vector<C_OscHexFile*>`), `C_OscComDriverBase` +
      `C_OscComMessageLogger` (`mpc_AutoSupportProtocol`), `C_OscComMessageLogger.{hpp,cpp}` +
      can_monitor (`mc_LoggingFiles` map), `C_OscComDriverProtocol` (`mc_OsyProtocols`,
      `mc_TransportProtocols`, `mc_LegacyRouterDispatchers`, both broadcast transports) plus the
      derived `C_OscComDriverFlash` / `C_SyvComDriverDiag` adapters. Collapses destructor
      bookkeeping and fixes the item-1 leak. Also subsumes item 1.
- [x] **11. Manual heap buffers → `std::vector`/`std::string`**
      `C_OscHexFile`, `C_HexFile` (OptimizeLinear), `C_OscZipFile`, `C_OscSecurityEcdsa` →
      `std::vector`; `C_OscApplicationInfoBlock::m_GetNonTerminatedString` → `std::string`
      (truncates at the first embedded null to preserve the historical C-string semantics).

## Tier 4 — Lower priority / larger scope

- [ ] **12. `C_HexFile` error-code migration incomplete**
      `hex_file/C_HexFile.cpp:2225/2338` `GetDataByAddress`/`FindPattern` still return raw
      `int32_t` despite the class's `C_HexFileErrorCategory`. → migrate to `std::error_code`
      via `m_MakeError`, return data by value/span.
- [ ] **13. Security AES file — duplicated I/O + hand-coded header**
      `security/C_OscSecurityAesFile.cpp` — read/write blocks duplicated between encrypt/decrypt;
      56-byte header encoded/decoded by hand with literal offsets. → shared helpers.
- [x] **14. `h_FloatToStrGe` float/double twins**
      `exports/code_generation/C_OscExportUti.cpp:398/448`. **Fixed:** both overloads now forward to a
      single private `mh_FloatToStrGeFormat(double, precision, bool*)`.
- [x] **15. `C_OscCanOpenEdsDeviceInfoBlock` four ini-load twins**
      `imports/C_OscCanOpenEdsDeviceInfoBlock.cpp`. **Fixed:** added private member template
      `mh_LoadValueFromIniFile`; the four overloads are thin wrappers.
- [ ] **16. UDS negative-response-code → text duplicated (already drifted)**
      `cmon_protocols/C_CanMonProtocolUds.cpp:412` vs `C_CanMonProtocolOpenSyde.cpp:1043`.
      → single shared `UdsNrcToText`. **Note:** the two already differ in wording
      (`positiveResponse` vs `positive response`), so consolidating changes displayed text for one —
      needs a wording decision.
- [x] **17. Commented-out constants in retired `uint32`/`uint8` spelling**
      `cmon_protocols/C_CanMonProtocolCanOpen.cpp`, `hex_file/C_HexFile.cpp`. **Fixed:** deleted the
      commented-out dead constants (compile hazards if revived).
- [ ] **18. Endian reassembly hand-inlined at ~25 sites**
      No shared `h_GetU32Big/Little` utility across protocol drivers. → add one.
- [ ] **19. Low / cosmetic**
      `std::endl`→`"\n"` in console logging; ~208 `#define` wire-constants → `constexpr`;
      god-functions (16-18 param signatures); `== true`/`== false`; `osc_write_log_*` macros →
      `std::source_location`.

## Confirmed clean (not to revisit)

- No `static_cast<Errc>` misuse — error-code migration complete on that axis.
- No C-style casts, no `NULL`, no dangling `C_SclString`/`C_SclDynamicArray` references.
- CRC computation already centralized (`scl/C_SclChecksums`).
