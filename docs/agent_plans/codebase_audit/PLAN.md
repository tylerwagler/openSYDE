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
