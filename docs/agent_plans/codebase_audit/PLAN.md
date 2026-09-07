# Plan: openSYDE Codebase Remediation

## Goal

Systematically address the 20 issues identified in the codebase audit
(`docs/agent_plans/codebase_audit/analysis.md`), prioritized by risk and
dependency order. Each phase produces independently shippable improvements
that do not regress existing functionality.

## Guiding Principles

1. **Fix root causes, not symptoms.** Where a class is fundamentally flawed
   (`C_SclDynamicArray`, `C_SclString`), replace it rather than patch it.
2. **No regressions.** Every phase must maintain API compatibility for
   downstream consumers or provide a documented migration path.
3. **Test before and after.** Add tests for the affected area before changing
   it, then verify the change passes.
4. **Ship small.** Each phase is sized to land in 1-3 PRs. No mega-refactors.

---

## Status Summary (as of 2026-09-06)

| Phase | Status | Notes |
|---|---|---|
| 0 — Tests & CI | ✅ **Complete** | GTest + CTest under `libraries/opensyde_core/tests/`, GitHub Actions workflow. CI was red from introduction until 2026-09-06; see the CI note below. |
| 1 — Correctness bugs | ✅ **Complete** | |
| 2 — Remove `C_SclDynamicArray` | ✅ **Complete** | Zero references remain. |
| 3 — Retire `C_SclString` | ✅ **Complete** | Class deleted; `C_SclStringCompat.hpp` helpers remain, ~94 files still call them. `C_SclStringList` / `C_SclIniFile` still exist. See `PHASE3_PLAN.md`. |
| 4 — Replace homegrown AES | ✅ **Complete for files; wire protocol out of scope** | File encryption is now AES-256-GCM + PBKDF2, with a versioned header and key wiping. The protocol sub-layer is deliberately unchanged — see below. |
| 5 — Error handling | 🔶 **Started** | `C_OscErrorCategory` (`Errc` + `STWErrorCategory`) exists; the security API returns `std::error_code`. The bulk of the int32_t call sites are unmigrated. 16 sites bridge with the wrong idiom — see below. |
| 6.1 — Singletons | ✅ **Complete, deviating from plan** | Meyer's singleton **rejected** — see below. Race fixed with `std::call_once`; `h_Destroy()` and teardown ordering kept. |
| 6.2 — Standard mutex | ✅ **Complete** | `C_TglCriticalSection` and all four `TglTasks` files deleted; 52 call sites on `std::mutex`. |
| 6.3 — Smart pointers | 🚫 **Closed, no defect found** | Exit criterion is wrong as written, and the hazards it implies do not exist here — see below. |
| 7 — Performance | 🚫 **Blocked on its own criteria** | Prescribes `std::format` (C++20) in a C++17 codebase, and its exit criteria require benchmarks that do not exist — see below. |
| 8 — Build system | 🔶 **Partial** | CMake minimum raised to 3.25 across the Vector submodules; CI matrix reworked; ccache added. The unified root build remains open — see below. |

### Phase 5 — two bridging idioms are in the tree, only one is right

Migrated code has to call unmigrated code, so a legacy `int32_t` return has to
become a `std::error_code` somewhere. There are two spellings of that in the tree
and they are not equivalent:

```cpp
c_Retval = make_error_code_from_stw(Legacy());   // correct
c_Retval = static_cast<Errc>(Legacy());          // wrong idiom
```

`static_cast<Errc>` names no conversion and asserts "this integer is an STW error
code" without checking. Any value outside the 13-member set becomes an enumerator
that does not exist, and a foreign status code — one where 0 also means success,
so the mistake is invisible in testing — is silently relabelled as an STW code.
That exact conflation has already been introduced and fixed twice in this
migration (`TglRemoveDirectory`, `mz_compress`).

**16 sites still use the wrong idiom**, all left by wave 2 in
`libraries/opensyde_core/system_update_package/`:

| File | Lines |
|---|---|
| `C_OscSupNodeDefinitionFiler.cpp` | 154, 257, 380, 461, 465, 469, 475 |
| `C_OscSupServiceUpdatePackageLoad.cpp` | 253, 572 |
| `C_OscSupServiceUpdatePackageCreate.cpp` | 146, 263 |
| `C_OscSupSignatureFiler.cpp` | 84, 111 |
| `C_OscSupDefinitionFiler.cpp` | 109, 152, 158 |

None currently produces a wrong value: every bridged callee returns either an STW
code or, in the case of `TglCreateDirectory` (`Load.cpp:572`), only 0 and -1,
which happen to map onto `success` and `unknown_err`. So this is idiom debt, not
a live defect — but it is the shape the two real bugs took, and it should be
converted when that directory is next touched.

Five of them (`C_OscSupNodeDefinitionFiler.cpp:380,461,465,469,475`) wrap
`C_OscXmlParserBase`'s `*Error` methods, which still return `int32_t`. If those
are ever migrated to return `std::error_code`, these sites are part of the blast
radius, and the parser is shared by every tree — so that is its own wave, not a
rider on someone else's.

### Phase 6.1 — why Meyer's singleton was rejected

The plan says "convert to Meyer's singleton, remove `h_Destroy()`". Applying that
literally would introduce two defects:

1. **User settings would be lost on exit.** `C_UsHandler::~C_UsHandler()` calls
   `Save()`, and `h_Destroy()` is invoked from the main-window destructor of all
   three apps — a controlled shutdown while `QApplication` is still alive. A
   function-local static destructs after `main()` returns, once Qt is gone.
2. **An explicit teardown ordering would be discarded.** `~C_PuiProject()`
   destroys `C_PuiSvHandler` *then* `C_PuiSdHandler`. Both are `QObject`s.
   Meyer's singletons destruct in reverse *construction* order, which depends on
   runtime call order and is not guaranteed to preserve that.

The real defect in these singletons was different from what the plan describes:
`h_GetInstance()` did an unguarded check-then-`new`, a data race if ever reached
from two threads. That is now guarded with `std::call_once` while `h_Destroy()`
and the ordering stay exactly as they were.

Known consequence: a `std::once_flag` cannot be reset, so calling
`h_GetInstance()` *after* `h_Destroy()` now returns `nullptr` instead of silently
constructing a fresh instance. The known shutdown paths were checked and none do
this — the widget tree is deleted before the `h_Destroy()` calls,
`C_TblTreDataElementModel::h_CleanUp()` touches only its own statics, and
`C_UsHandler::Save()` reaches no other singleton. Silent resurrection during
shutdown was arguably a latent bug being hidden, not a feature.

`C_CamDbHandler`, `C_CamProHandler` and `C_HeHandler` were already race-free
(static instance, no lazy init) and were left alone.

### Phase 4 — what was done, and what is deliberately not

**Done (file encryption).** The path protecting service-mode projects and
service update packages was AES-128-ECB with a single unsalted MD5 as the KDF
and no authentication. It is now AES-256-GCM with PBKDF2-HMAC-SHA256 (600000
iterations), a random per-file salt and nonce, `OPENSSL_cleanse` on all key
material, and a versioned header the format previously lacked entirely. Eight
tests cover round-trip, wrong password, tamper detection, header layout,
rejection of legacy input, ECB's block-repetition property, nonce uniqueness
across files, and empty input.

**Not done, deliberately: the protocol security sub-layer.**
`C_OscProtocolSecuritySubLayer` encrypts traffic to the ECU with an
ECDH-negotiated key, so the device firmware implements the matching half.
Inspecting the ESX-4CS gateway TSP (`esx_4cs_gw_c_tsp_24_0B`) settles it: across
all eleven precompiled archives and the 207 KB flashloader image there is no AES
symbol, no S-box, no inverse S-box and no Rcon table. The device crypto is RSA
PKCS#1 signature verification via BearSSL (`osy_udc_trg_sec_verify_rsa_signature`,
`bearssl_rsa.c.obj`) plus an RNG for UDS SecurityAccess seeds — matched on the
tool side by `C_OscSecurityRsa::h_SignSignature`.

So traffic encryption is not merely risky to change, it is unusable with this
hardware. The supported device-level security mechanism is RSA-based
SecurityAccess. Changing the sub-layer would gain nothing and would break
compatibility with any future STW device whose TSP does implement it.

**Related, found while scoping Phase 4:** the `C_SclString` → `std::string`
migration replaced hex-aware `ToInt()` with base-10 `std::stoi()`, which zeroed
the AES key (the password was ignored entirely), ECDSA signature bytes, device
serial numbers, and every CANopen EDS object index. Fixed with regression tests;
see the commit "Fix hex string parsing regression that zeroed AES keys and ECDSA
signatures". Any file encrypted before that fix should be treated as
unprotected.

### Phase 6.3 — closed: the criterion is wrong and the hazards are absent

"No manual `new`/`delete` in newly-touched files" reads as ~2,972 violations, but
the overwhelming majority are Qt widgets handed to a parent
(`new C_OgeLabel(this)`), where Qt owns the lifetime and a `unique_ptr` would
cause a double free.

The genuine ownership cases were then checked individually and none is defective:

- **`FILE*` sites.** Nine non-vendored call sites (the four in `tinyxml2` are
  third party). Every one pairs with an `fclose`; no leak on an early return.
  The single flagged case in `C_Md5Checksum::GetMD5` returns only when `fopen`
  itself failed, so there is nothing to close.
- **`C_HexFile` binary image.** `new uint16_t[]` at `OptimizeLinear` is matched
  by `delete[]`, and the guard that could strand it (`u32_Error` non-zero on
  entry) is unreachable — it is initialised to `NO_ERR` immediately above.
- **`C_HexFile` `T_HexLine` ring buffer.** A hand-rolled doubly-linked list, and
  the one item with real substance left in this sub-phase. Converting it means
  restructuring 2,561 lines of firmware-image parsing that has **zero test
  coverage**, to fix no observed defect. That is a bad trade in that order. If
  this is ever revisited, characterisation tests for the hex parser come first.

### Phase 7 — blocked on its own terms

Two problems, neither about effort:

1. **The prescribed tool is unavailable.** 7.1 says to replace `std::stringstream`
   with `std::format`, which is C++20. The toolchain files pin `-std=c++17` and
   `CMAKE_CXX_STANDARD 17`. Landing 7.1 as written requires either a language
   standard bump (a real decision with its own blast radius) or a substitute
   such as `snprintf` or fmtlib.
2. **The exit criteria are unmeasurable.** "Logging throughput benchmark shows
   2x+ improvement" and "CRC benchmark shows HW acceleration benefit" both
   require a benchmark harness. There is none in the tree. Optimising without
   one is speculation, and the phase cannot be declared done against its own
   criteria either way.

Prerequisite for Phase 7: add a benchmark harness, then decide on the C++20
question. Neither is performance work as such.

### Phase 8 — what is done and what is not

Done: CMake minimum raised 3.9 → 3.25 across `Vector_DBC` / `Vector_BLF` /
`Vector_ASC` (clearing the "compatibility with CMake < 3.10" deprecation); CI
reduced to resolute with the distro default compiler; ccache wired into
`build.sh` and CI.

Not done: `build.sh` still configures each of the eight tools as its own CMake
project, so `opensyde_core` is compiled once per tool — eight times for `all`.
The tools cannot trivially share one core target because each sets a different
`OPENSYDE_CORE_SKIP_*` set. ccache masks the cost rather than removing it. A
root `CMakeLists.txt` building core once against the union of those options is
the real fix; `CMakePresets.json` already sits at the repo root with nothing to
drive it.

### CI note

The workflow was red from the moment it was added until 2026-09-06 — every job,
every run. Four independent environment defects (a Qt6 SVG package that does not
exist on the targeted release, a compiler absent from a matrix entry, missing
`flex`/`bison`/`libfl-dev`, missing `libssl-dev`). That is how phase-2 and
phase-3 residue survived, and how two `pjt/` cmake files silently missing from
CAN Monitor and SYDEflash went unnoticed. A job that never runs is not a gate.

---

## Phase 0 — Foundation: Tests & Infrastructure

**Goal:** Establish the testing harness and CI so subsequent phases can verify
correctness.

### 0.1 — Test Framework Setup
- Add Google Test (or Catch2) dependency via `FetchContent`
- Create `tests/` directory under `libraries/opensyde_core/`
- Set up CMake `enable_testing()` / `add_test()` in core library
- Configure CTest for `make test` / `ninja test`
- Wire into a GitHub Actions CI workflow (Ubuntu + Qt6)

### 0.2 — Leaf-Utility Tests (no dependencies)
Write tests for:
- `C_SclChecksums` — CRC16, CRC32, CRC32 TriCore, CRC16STW
- `stwtypes` — type sizes and signedness
- `stwerrors` — constant values
- `C_OscXmlParser` — basic round-trip
- `C_OscLoggingHandler::h_StwError` — all error code strings

### 0.3 — CI Pipeline
- GitHub Actions workflow: build (Debug + Release), run unit tests
- Matrix: Ubuntu 22.04 + 24.04, GCC 12 + 13
- Optional: Windows build (no tests initially)
- Lint: `clang-tidy` on new/changed code only (avoid overwhelming existing)

**Exit criteria:** `ninja test` passes, CI green on PRs.

---

## Phase 1 — Correctness Bugs (HIGH priority)

**Goal:** Fix demonstrable bugs that can cause undefined behavior or data loss.

### 1.1 — `C_SclDynamicArray::GetHigh()` (Analysis #3)
- **Bug:** Returns 0 for empty arrays → callers iterate into UB
- **Fix:** Change to return -1 for empty arrays
- **Impact:** Low risk, mechanical change
- **Test:** Add tests for empty, single-element, multi-element arrays
- **PR:** Single commit + test

### 1.2 — `rand()` Timer ID Collision (Analysis #4)
- **Bug:** `h_StartPerformanceTimer` uses `rand()` — not thread-safe, 16-bit
  IDs collide under load
- **Fix:** Use `std::atomic<uint16_t>` static counter (wrap on overflow) or
  `std::mt19937` + `std::uniform_int_distribution`
- **Impact:** Low risk, contained to logging handler
- **Test:** Verify no duplicate IDs in concurrent access (thread-sanitizer)

### 1.3 — `IntToHex` Stream Manipulator (Analysis #5)
- **Bug:** `&std::hex` passed instead of `std::hex`
- **Fix:** Remove address-of operator
- **Impact:** Trivial, single character fix
- **Test:** Verify hex output for edge cases (0, max values, all digits)

### 1.4 — String Literal Exceptions
- **Problem:** `throw "C_SclDynamicArray::Delete at Invalid Position !"`
- **Fix:** Throw `std::out_of_range` instead (or better, eliminate the class —
  see Phase 2)
- **Impact:** Minimal until Phase 2 eliminates the class

**Exit criteria:** All Phase 1 fixes merged, tests passing, no UB in
thread-sanitizer run.

---

## Phase 2 — Eliminate `C_SclDynamicArray` (Analysis #6)

**Goal:** Remove the custom dynamic array class entirely, replacing all ~80+
usage sites with `std::vector`.

### 2.1 — Audit & Inventory
- `grep -r C_SclDynamicArray` to catalog every usage site
- Categorize by pattern:
  - `SetLength(n)` → `resize(n)`
  - `GetLength()` → `size()`
  - `GetHigh()` → `empty() ? -1 : size()-1` (or adjust loop to use `size()`)
  - `IncLength(n)` → `resize(size() + n)`
  - `Delete(i)` → `erase(begin() + i)`
  - `Insert(i, v)` → `insert(begin() + i, v)`
  - `operator[]` → `operator[]` (identical)
- **Key difference:** `std::vector::size()` returns `size_t` (unsigned), not
  `int32_t`. This will cascade into loop variable type changes.

### 2.2 — Mechanical Replacement (per subsystem)
Work through subsystems in dependency order:
1. `C_SclString::Tokenize` — output parameter change
2. `C_HexFile` / `C_HexDataDump` — internal data structures
3. `C_CanRxQueue` — deque already used, but check for dynamic array usage
4. Protocol driver structures
5. Project model classes (node, bus, CAN message, datapool, etc.)
6. GUI model classes (system views, dashboards, etc.)

### 2.3 — Remove the Class
- Delete `C_SclDynamicArray.hpp`
- Remove from all `CMakeLists.txt` include directories
- Update `precomp_headers.hpp` if referenced

### 2.4 — Loop Variable Audit
Many loops use `int32_t` counters because `C_SclDynamicArray` used `int32_t`.
After migration to `std::vector::size_type` (`size_t`), audit for:
- Signed/unsigned comparison warnings
- Loop variables that should be `size_t`
- `for (int32_t i = 0; i < vec.size(); i++)` → `for (size_t i = 0; ...)`

**Exit criteria:** No `C_SclDynamicArray` references remain in any `.cpp` or
`.hpp` file. Builds clean with `-Wsign-conversion`. All existing tests pass.

---

## Phase 3 — Retire `C_SclString` (Analysis #7)

**Goal:** Replace the custom string class with idiomatic `std::string` + C++17
standard facilities. This is the largest single refactor in the plan.

**Strategy:** Not a find-and-replace. Introduce a compatibility layer, migrate
callers incrementally, then remove the class.

### 3.1 — Compatibility Header
Create `C_SclStringCompat.hpp` that maps `C_SclString` methods to `std::string`
equivalents via inline helpers:
```cpp
// Old: str.SubString(1, 5)
// New: str.substr(0, 5)
// Compat helper:
inline std::string SubString(const std::string & s, uint32_t idx, uint32_t cnt) {
    return s.substr(idx - 1, cnt);  // C_SclString is 1-based
}
```

This lets us migrate callers one file at a time without breaking the build.

### 3.2 — Migration by Category (parallelizable)

| Category | Sites | Strategy |
|----------|-------|----------|
| `IntToStr` / `IntToHex` | ~200 | Replace with `std::to_string` / `std::format` / `<charconv>` |
| `FloatToStr` | ~50 | Replace with `std::to_string` / `std::format` |
| `SubString` / `Pos` / `Trim` | ~400 | Replace with `std::string::substr` / `find` / manual trim |
| `UpperCase` / `LowerCase` | ~100 | `<algorithm>` + `::toupper` / `::tolower` |
| `ToInt` / `ToDouble` | ~150 | `std::from_chars` / `std::stoi` / `std::stod` |
| `printf` / `StringPrintFormatted` | ~50 | `std::format` (C++20) or `fmt::format` |
| `Tokenize` | ~30 | `std::stringstream` + `std::getline` or manual loop |
| `c_str()` | ~1000+ | Already returns `const char*` — mostly compatible |
| Operator overloads | ~500 | `std::string` operators are identical |
| Constructor from int/float | ~100 | Replace with explicit `std::to_string` calls |

### 3.3 — Remove the Class
- Delete `C_SclString.hpp`, `C_SclString.cpp`
- Remove from CMakeLists.txt
- Remove `#include "C_SclString.hpp"` from precompiled headers
- Clean up `SCL_DEPRECATED_*` macros

**Exit criteria:** No `C_SclString` references remain. Builds clean. String
operations produce identical logical results (formatting differences accepted
and documented).

---

## Phase 4 — Security: Replace Homegrown AES (Analysis #1)

**Goal:** Remove the vendored AES implementation and use a well-audited crypto
library.

### 4.1 — Audit Crypto Usage
- Find all callers of `AES::EncryptECB`, `EncryptCBC`, `EncryptCFB`, etc.
- Determine actual requirements: key sizes, modes, IV handling, authentication
- Check if OpenSSL is already a dependency (it may be for TLS)

### 4.2 — Implement OpenSSL EVP Wrapper
- Create `C_OscAes256Gcm` class wrapping `EVP_EncryptInit_ex` / `EVP_DecryptInit_ex`
- Use AES-256-GCM (authenticated encryption)
- Constant-time comparison for tags (`CRYPTO_memcmp`)
- Clear key material with `OPENSSL_cleanse`

### 4.3 — Migrate Callers
- Replace each call site with the new API
- Update key derivation if applicable
- Add integration tests with known-answer tests (KATs)

### 4.4 — Remove Old Code
- Delete `libraries/opensyde_core/security/aes/`
- Remove from CMakeLists.txt

**Exit criteria:** No references to old AES class. New implementation passes
NIST CAVP test vectors for AES-256-GCM. All existing crypto-dependent features
work identically.

---

## Phase 5 — Error Handling Modernization (Analysis #8)

**Goal:** Move from integer error codes to type-safe error handling.

### 5.1 — Define Error Types
```cpp
enum class OscErrc : int32_t {
    Success = 0,
    Unknown = -1,
    Warning = -2,
    Busy    = -4,
    Range   = -5,
    // ...
};

class OscErrorCategory : public std::error_category { ... };
inline std::error_code make_error_code(OscErrc e);
```

### 5.2 — Migrate Hot Paths First
- Start with `C_OscLoggingHandler::h_StwError` — make it accept `std::error_code`
- Migrate `C_OscProtocolDriverOsy` return codes
- Migrate `C_HexFile::LoadFromFile` return codes
- Each migration is a self-contained PR

### 5.3 — `std::expected` Adoption (C++23 or tl::expected)
For functions that return either a value or an error:
```cpp
// Before: int32_t ReadValue(int32_t & out_val);
// After:  std::expected<int32_t, OscErrc> ReadValue();
```

**Exit criteria:** No new code uses bare `int32_t` error codes. Old codes
remain for backward compat but are deprecated.

---

## Phase 6 — Concurrency & Singleton Fixes (Analysis #9, #10, #11)

### 6.1 — Meyer's Singleton Migration
- `C_PuiSdHandler` — convert to Meyer's singleton
- Search for other manual singletons (`C_PuiProject`, etc.)
- Remove `h_Destroy()` methods

### 6.2 — Standard Mutex Migration
- Replace `C_TglCriticalSection` with `std::mutex` + `std::lock_guard`
- In logging handler: replace two custom mutexes with `std::mutex`
- Remove `C_TglCriticalSection` from TGL layer if no other consumers

### 6.3 — Smart Pointer Adoption
- Hex file parser: replace `T_HexLine` linked list with `std::vector<T_HexLine>`
  or `std::list<std::unique_ptr<T_HexLine>>`
- Replace `FILE*` with `std::ifstream` / `std::ofstream`
- Audit for `new`/`delete` pairs that should be `std::unique_ptr`

**Exit criteria:** No manual `new`/`delete` in newly-touched files. No custom
mutex wrappers. Singletons use local-static.

---

## Phase 7 — Performance (Analysis #13, #14, #16)

### 7.1 — Logging Hot Path Optimization
- Replace `std::stringstream` in `mh_WriteLog` with `std::format` (or fmtlib)
- Cache date-time string, update only when millisecond changes
- Batch writes with larger buffer

### 7.2 — Hardware CRC32
- Add `#ifdef __SSE4_2__` path using `_mm_crc32_u32` / `_mm_crc32_u8`
- Fall back to software CRC
- Benchmark to confirm improvement

### 7.3 — Move Semantics
- Add move constructors / move assignment to large classes
- Look for `std::vector` / `C_SclString` return values that should be moved
- Use `std::move` in hot paths

**Exit criteria:** Logging throughput benchmark shows 2x+ improvement. CRC
benchmark shows HW acceleration benefit.

---

## Phase 8 — Build System Modernization (Analysis #17, #18, #19)

### 8.1 — CMake Version Bump
- Bump minimum to 3.24
- Adopt `CMAKE_CXX_STANDARD` properly
- Use `FetchContent` for test frameworks

### 8.2 — Source File Management
- Evaluate `target_sources` with `FILE_SET` for header files
- Or use `file(GLOB CONFIGURE_DEPENDS)` with explicit documentation
- Break monolithic CMakeLists.txt into per-module files

### 8.3 — CI/CD
- GitHub Actions: Ubuntu 22.04 + 24.04, GCC 12 + 13
- Build all 8 tools in Release + Debug
- Run unit tests
- Optional: clang-tidy diff analysis

**Exit criteria:** `cmake --build` succeeds with CMake 3.24+. CI green on PRs.

---

## Dependency Graph

```
Phase 0 (Tests + CI)
    ├── Phase 1 (Correctness) — no deps
    ├── Phase 2 (DynamicArray removal) — needs Phase 0 tests
    │       └── Phase 3 (String) — needs Phase 2 (Tokenize uses DynamicArray)
    │               └── Phase 5 (Error handling) — easier with std::string
    ├── Phase 4 (Security) — independent
    ├── Phase 6 (Concurrency) — partially independent
    ├── Phase 7 (Performance) — needs Phase 3 (stringstream→format)
    └── Phase 8 (Build) — independent, can start early
```

Phases 0, 4, 6, and 8 can proceed in parallel. Phases 2 → 3 → 5 → 7 are
sequential (each depends on the previous).

---

## Effort Estimate

| Phase | Description | Files Touched | Estimated Commits | Risk |
|-------|-------------|--------------|-------------------|------|
| 0 | Tests & CI | 20-30 | 3-5 | Low |
| 1 | Correctness bugs | 5-10 | 3-4 | Low |
| 2 | Remove C_SclDynamicArray | 80-120 | 8-15 | Medium |
| 3 | Retire C_SclString | 300-500 | 20-40 | High |
| 4 | Replace AES | 10-20 | 3-5 | Medium |
| 5 | Error handling | 100-200 | 10-20 | Medium |
| 6 | Concurrency | 30-50 | 5-10 | Medium |
| 7 | Performance | 20-40 | 5-8 | Low-Medium |
| 8 | Build system | 15-25 | 3-6 | Low |

**Total estimated effort:** 60-110 commits across 8 phases.

---

## How to Start

1. **Begin with Phase 0** — test framework and CI. No behavioral changes, pure
   infrastructure. Validates that the build system works and establishes the
   quality gate for all subsequent phases.
2. **Phase 1 in parallel** — the correctness bugs are independent and
   low-risk. Ship them early.
3. **Phase 4 (security) independently** — the AES replacement touches no other
   system and is the highest-risk item if left unfixed.
