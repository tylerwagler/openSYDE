# Scope: Removing `stwtypes.hpp`

Status: scoping (not yet started implementation). Owner: Tyler. Date: 2026-09-09.

## Goal

Retire `libraries/opensyde_core/stwtypes/stwtypes.hpp` and its C variant `stwtypes.h`,
moving the codebase onto the standard fixed-width/float types instead of STW-wide custom
aliases. Motivation: the aliases collide with C++23 standard types (`std::float64_t` from
`<stdfloat>`, already biting the GUI build — see the `float64_t` collision fix in `5ddff28de`),
and `uint32_t` / `float` / `double` are simply standard.

## What the header provides

`stwtypes.hpp` defines **only types** (the sole `#define` is the `STWTYPESHPP` include guard):

| Alias | Resolves to | Standard equivalent |
|-------|-------------|---------------------|
| `int8_t` … `uint64_t` | `using std::…` dragged into the **global** namespace from `<cstdint>` | `std::intN_t`/module `<cstdint>` |
| `float32_t` | `float` | `std::float32_t` (`<stdfloat>`, C++23) or `float` |
| `float64_t` | `double` | `std::float64_t` (`<stdfloat>`, C++23) or `double` |
| `char_t` | `char` | `char` |

Under C++11+ the integer aliases are `using std::…;` (the header just guarantees they are in
the **global** namespace — see comment at `stwtypes.hpp:46-59`). `float32_t`/`float64_t`/`char_t`
are genuinely custom and have no binary equivalent midway.

## Usage metrics (whole repo, 2026-09-09)

- **~1,045 files** reference `stwtypes` — **1,036** via `#include "stwtypes.hpp"`, **1** C file
  via `#include "stwtypes.h"` (`libraries/opensyde_core/stw_compid/stwcompid.c`), plus a handful of
  non-source references (CMake include dirs, `CLAUDE.md`, docs).
- Type occurrence counts: `uint32_t` 17,249 · `int32_t` 9,912 · `uint16_t` 1,881 · `float64_t`
  1,584 · `uint64_t` 979 · `float32_t` 472 · `int64_t` 356 · `char_t` 247 · ...
- **Segmentation of the 1,036 `.hpp`-includers:**
  - **828 files** use *only* integer types → dropping `#include "stwtypes.hpp"` and adding
    `#include <cstdint>` suffices (mechanical).
  - **208 files** use `float32_t` / `float64_t` / `char_t` → need a real **type change**:
    - `float64_t`: 146 files (opensyde_tool/src 109, opensyde_core 18, opensyde_gui 17, +2)
    - `char_t`: 58 files
    - `float32_t`: 35 files

## Approach

Two workstreams, done **type-first, remove-header-last** so the tree stays compiling at every step:

1. **Type replacement (the only logically-difficult part).**
   - `char_t` → `char` (trivial, no namespace concern).
   - `float32_t`/`float64_t` → pick one policy (see below) across the 208 files.
2. **Include swap (mechanical).** Replace `#include "stwtypes.hpp"` with `#include <cstdint>`
   (and `<stdfloat>` if the std float aliases are chosen) in every includer.
3. **Delete** `stwtypes.hpp` + `stwtypes.h`, update the CMake include-dir entries and any docs,
   and drop them from the repo.

## Generated controller code is C on an embedded target — this splits the work in two

The code generator (`exports/code_generation/C_OscExport*`) produces **C** source for an
embedded controller (Infineon TriCore/AURIX-class). It emits `#include "stwtypes.h"` (the **C**
header, *not* `.hpp`) into the generated output, alongside `<stddef.h>` and the `osco_*.h`
controller headers.

Critically, the per-field types it emits are STW's **no-`_t`** names — `uint8`, `sint16`,
`float32`, `float64` (see `C_OscExportDataPool::mh_GetType`, lines 1330-1369, producing
declarations like `float64 f64_Value;`) — which are **not** the `uint8_t`/`float64_t` aliases that
`stwtypes.hpp`/`.h` define. They come from the controller-side headers.

Consequence: **the embedded target is a separate, constrained domain.**
- The controller compiles **C**, not C++23. It has **no `<stdfloat>` / `std::float32_t` /
  `std::float64_t`**, and possibly no C++ standard library at all. C++23 types are **off the table**
  for generated code.
- So `stwtypes.hpp` (host C++) and `stwtypes.h` (generated/embedded C) are **different contracts**
  and must be handled separately:
  - **Host build (the 1,036 `.hpp`-includers):** free to migrate to `<cstdint>`, and to
    `std::float32_t/float64_t` (C++23) or `float`/`double`, `char`.
  - **Generated controller C:** if it sheds `stwtypes.h`, it must use C99 `<stdint.h>` + native
    `float`/`double`/`char` **only** — never C++23 stdfloat types. The controller-side `uint8`/`float64`
    no-`_t` naming is upstream of us and unchanged.

> **P0 finding (2026-09-09):** the generated controller C emits **no** `_t`-suffixed or `char_t`
> data types — its per-field types are the no-`_t` names (`uint8`, `float64`) supplied by the
> controller-side headers, not by `stwtypes.h`. So the generated data types are independent of
> `stwtypes.h`. `stwtypes.h` is still `#include`d by generated output, likely for a small set of
> `_t`/other symbols; confirm exactly which before deciding whether it can be replaced by
> `<stdint.h>` + native types in generated output.

## Decision: `float32_t`/`float64_t` target type

- **Option A — `std::float32_t` / `std::float64_t`** from `<stdfloat>` (C++23). Semantically the
  faithful "IEEE 32/64-bit" type, and it *removes* the collision class that just bit the GUI build
  (the STW global `float64_t` vanishes; only `std::float64_t` remains, no `using namespace std;`
  ambiguity). On GCC/x86-64 `std::float64_t == double` and `std::float32_t == float`, so it is
  type-identical to today. **Valid only for the host C++ build** — never for generated controller
  C (see previous section). Requires C++23 + a libstdc++ with `<stdfloat>` (we have both), and the
  GUI build already pulls `<stdfloat>` via Qt.
- **Option B — `float` / `double`.** Simplest and dependency-free; loses the IEEE-width intent and
  diverges from a "standard `<cstdint>`-style" naming. Type-identical on every relevant target.
  **This is the only option that also works for generated controller C.**

**DECISION (2026-09-09): Option B for the host side.** Verified that on GCC, `std::float64_t`
and `std::float32_t` are **distinct** `_Float64`/`_Float32` types, *not* the same as `double`/`float`
(`std::is_same_v<std::float64_t,double> == false`). Since STW's `float64_t` is literally
`typedef double`, Option A would change the actual type (not just the name) across ~2,000
occurrences — a real risk of signature/template/overload breakage. And Option B is the only form
that also works for generated controller C (native `float`/`double`). So the host aliases map to
**`float`/`double`** (kept native throughout, per the decided convention).

## Risks / gotchas

1. **Transitive-include loss.** Several hundred files use int/float types *without* directly
   including `stwtypes.hpp`, relying on a header that does. Deleting `stwtypes.hpp` may leave those
   TUs without `<cstdint>`. Mitigation: after phase 1+2, scan for TUs using the types but not
   including `<cstdint>`/`<stdfloat>`, and add direct includes.
2. **The old collision re-appears mid-migration.** While both the STW global `float64_t` and
   `std::float64_t` exist, a TU with `using namespace std;` + bare `float64_t` breaks
   (exactly the `5ddff28de` failure). Keep type changes atomic per-TU and never leave bare
   `float64_t` in a `using namespace std;` TU during the transition.
3. **Code-generation subsystem.** `libraries/opensyde_core/exports/code_generation/C_OscExport*`
   use `stwtypes` types internally (already counted) — verify whether the **generated output**
   (consumer C/C++) also emits `stwtypes.hpp` / the aliases; if so, that generated interface must be
   updated or kept compatible. **Verify before planning phase 1.**
4. **C variant `stwtypes.h`.** `stw_compid/stwcompid.c` must switch to `<stdint.h>` (C99 provides
   the fixed-width ints in the global namespace); `float`/`double` and `char` are native.
5. **CMake.** `stwtypes` is named in `libraries/opensyde_core/CMakeLists.txt`,
   `tests/CMakeLists.txt`, `opensyde_syde_sup/pjt/CMakeLists.txt` (include dir / sources). Update
   once the directory is removed.
6. **Diff size / review.** ~1,045 files touched; split into reviewable phases and keep each phase
   CI-green. `CLAUDE.md` "Types" section and the `float64_t`/`char_t` conventions must be rewritten.

## Suggested phasing

- **P0** — Confirm the code-generation output contract (risk #3); decide Option A vs B.
- **P1** — `char_t` → `char` (58 files).
- **P2** — `float32_t`/`float64_t` → chosen standard type (208 files).
- **P3** — Include swap of the 828 int-only files (and the now-float-free 208) to `<cstdint>`;
  fix any transitive-include fallout; migrate `stw_compid.c`.
- **P4** — Delete `stwtypes.hpp`/`.h`, update CMake + docs, run full eight-tool build + CI.
