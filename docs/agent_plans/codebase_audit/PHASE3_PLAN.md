# Phase 3 — Retire C_SclString

## Goal

Replace the custom `C_SclString` class with idiomatic `std::string` + C++17
standard facilities across the entire codebase.

## Status — 2026-08-24 (working tree, uncommitted)

**Core migration is complete.** Verified state of the working tree:

- `C_SclString.{cpp,hpp}` deleted; `C_SclString.hpp` (and `C_SclStringCompat`)
  includes removed from every file. **Zero** `#include "C_SclString.hpp"` and zero
  raw `C_SclString` references remain (the only substring hits are the still-live
  `C_SclStringList` / `C_SclIniFile` / `C_SclChecksums` / `C_SclDateTime` classes).
- New `C_SclStringCompat.hpp` provides ~21 mapping helpers;
  `C_OscErrorCategory.{hpp,cpp}` adds `Errc` + `STWErrorCategory` so the legacy
  int32 STW error convention works through `std::error_code`.
- Security API (`C_OscSecurityAesFile`, `C_OscSecurityAesCbc`,
  `C_OscSecurityEcdhAes`, `C_OscProtocolSecuritySubLayer`) converted to
  `std::error_code`; all external callers use `.value()`.

### Verification performed (2026-08-24)

- `opensyde_core` builds with the CI option set; **112/112 ctest cases pass**.
- All 8 binaries build & link on Linux (Debug): `openSYDE`, `CAN Monitor`,
  `SYDEflash`, `SYDEsup`, `syde_x_gen`, `syde_coder_c`, `flash_tool`,
  `tsp_convert`.
- Fixes applied during verification (residue the migration left behind):
  - `C_PuiProject.cpp`: 2 security calls still assigned `std::error_code` to
    `int32_t` → added `.value()`.
  - canmonitor: missing `C_SclStringCompat.hpp` includes (4 files);
    12 mangled `Xstd::stoi()/stoll()` artifacts and 1 `c_Statusc_str()` glue in
    message_trace/project; removed dead `using namespace stw::scl;` (BLF + 1).
  - sydeflash: `std::vector<...>::GetLength()` → `.size()` (5 sites, pre-existing
    since phase 2 — file was never compiled because sydeflash is not in CI);
    removed dead `using namespace stw::scl;` (2 files).
  - `C_SyvUpNodePropertiesDialog.cpp`: loop `int32_t < .size()` sign-compare
    warning → `uint32_t`.
  - Removed 56 dead `#include "C_SclStringCompat.hpp"` (no compat helper used)
    and 32 dead `using namespace stw::scl;` directives.

### Remaining / notes

- `C_SclStringList` and `C_SclIniFile` still exist (internally migrated to
  `std::string`); retiring them would be follow-up work.
- ~94 files still call `*Compat()` helpers from the compat header; full
  idiomatic conversion (helpers → `std::to_string`/`substr`/`find`…) is optional
  follow-up.
- Some migrated files carry mixed CRLF/LF lines where the compat include was
  injected (cosmetic; gcc tolerates it).
- **CI gap found:** `.github/workflows/build.yml` only builds `opensyde_core`
  with ~13 subsystems skipped plus the `openSYDE` tool. `canmonitor`, `sydeflash`,
  `sydesup`, `syde_x_gen`, `syde_coder_c`, `flash_tool`, `tsp_convert` and the
  skipped core subsystems are not compiled by CI — this is how phase-2 residue
  (`GetLength()` on `std::vector`) and phase-3 residue stayed hidden. Recommend
  a CI job that runs `./build.sh -b Debug <every tool>`.
- `CMakePresets.json` is untracked at repo root but there is no root
  `CMakeLists.txt`, so it currently configures nothing. Decide: delete, or keep
  as groundwork for a unified root build.

## Original plan (for reference)

## Scope

| Metric | Count |
|--------|-------|
| `#include "C_SclString.hpp"` | ~225 files |
| Usages in `.cpp` | ~3,761 |
| Usages in `.hpp` | ~1,954 |
| **Total usage sites** | **~5,700+** |
| Files to touch | 300–500 |

## Strategy: Compatibility-first, migrate incrementally

**Do NOT attempt a find-and-replace.** The 1-based indexing and method name
differences make mechanical replacement dangerous. Instead:

1. **Create `C_SclStringCompat.hpp`** — inline helpers mapping `C_SclString`
   methods to `std::string` equivalents, so individual files can migrate
   independently.
2. **Add `[[deprecated]]` + conversion operators** to `C_SclString` so the
   compiler warns at every old-style usage.
3. **Migrate by category** (parallelizable across sub-agents).
4. **Remove the class** once all callers are gone.

## Migration Categories (ordered by difficulty)

| # | Category | Sites | Strategy | Risk |
|---|----------|-------|----------|------|
| 1 | `IntToStr` / `IntToHex` | ~524 | `std::to_string` / `<charconv>` / `std::stringstream` | Low |
| 2 | `c_str()` / `data()` | ~1000+ | Already compatible — just remove `.c_str()` calls where redundant | Low |
| 3 | `operator+` / `operator==` etc. | ~500 | Identical `std::string` semantics | Low |
| 4 | `ToInt` / `ToDouble` / `ToIntDef` / `ToInt64` | ~150 | `std::from_chars` / `std::stoi` / `std::stod` | Low |
| 5 | `SubString` / `Pos` / `LastPos` / `LastDelimiter` | ~400 | `substr` / `find` / `rfind` — **watch 1-based→0-based** | Medium |
| 6 | `Trim` / `TrimLeft` / `TrimRight` | ~100 | Manual `find_first_not_of` / `find_last_not_of` | Low |
| 7 | `UpperCase` / `LowerCase` | ~100 | `<algorithm>` + `::toupper` / `::tolower` | Low |
| 8 | `Delete` / `Insert` / `ReplaceAll` / `SetLength` | ~200 | `erase` / `insert` / `replace` / `resize` — **watch 1-based** | Medium |
| 9 | `FloatToStr` | ~50 | `std::to_string` / `std::format` / `std::stringstream` | Low |
| 10 | `Tokenize` | ~30 | `std::stringstream` + `std::getline` or manual loop | Low |
| 11 | `printf` / `StringPrintFormatted` | ~50 | `std::format` (C++20) or `fmt::format` | Low |
| 12 | Constructor from int/float | ~100 | Replace with explicit `std::to_string` calls | Low |
| 13 | `AnsiCompare` / `AnsiCompareIc` | ~30 | `operator<` / `operator==` / case-insensitive compare | Low |
| 14 | `StringOfChar` | ~5 | `std::string(count, char)` | Low |
| 15 | `operator[]` (1-based) | ~200 | `operator[]` (0-based) — **HIGH RISK, audit every site** | High |
| 16 | `AsStdString()` | ~100 | Remove — just use the string directly | Low |

## Migration Order (within opensyde_core)

```
Phase 3a: Compatibility header + deprecation attributes
Phase 3b: IntToStr/IntToHex → std::to_string / <charconv>
Phase 3c: c_str/data cleanup
Phase 3d: ToInt/ToDouble family → <charconv>
Phase 3e: SubString/Pos/Trim family → substr/find
Phase 3f: UpperCase/LowerCase → <algorithm>
Phase 3g: Delete/Insert/ReplaceAll → erase/insert/replace
Phase 3h: FloatToStr → std::to_string
Phase 3i: Tokenize → manual loop
Phase 3j: printf/StringPrintFormatted → std::format
Phase 3k: Constructor cleanup
Phase 3l: operator[] audit (1-based→0-based)
Phase 3m: Remove C_SclString
```

## Files to Modify

### Primary target
- `libraries/opensyde_core/scl/C_SclString.hpp` — add deprecation, conversion ops
- `libraries/opensyde_core/scl/C_SclString.cpp` — simplify implementation
- `libraries/opensyde_core/scl/C_SclStringList.hpp` — uses C_SclString
- `libraries/opensyde_core/scl/C_SclIniFile.hpp` — uses C_SclString

### Compatibility header (NEW)
- `libraries/opensyde_core/scl/C_SclStringCompat.hpp`

### Test file
- `libraries/opensyde_core/tests/test_scl_string.cpp` — already expanded to 103 tests

## Verification

1. All 103 SclString tests pass after each migration step
2. Full `ctest` suite passes
3. Build clean with no warnings (after deprecation migration)
4. `lsp_diagnostics` clean on changed files

## Effort

Estimated 20-40 commits across 13 sub-phases. Each sub-phase is independently
shippable.
