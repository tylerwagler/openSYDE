# Consolidation sweep — 2026-09-13

A whole-tree audit for duplication, dead code and consolidation opportunities,
run across four parallel surveys (cross-tool duplication, `opensyde_tool`
internals, `opensyde_core`, and modernization) plus independent whole-repo
duplicate detection.

**This document exists mostly to stop the next sweep from re-deriving the same
conclusions — including the several that say "do not do this".**

---

## The headline is a negative result

The premise was "there's a ton of code and it doesn't do that much, so there
must be a lot to consolidate". Measured four ways, the duplication hypothesis
largely fails:

| Measure | Method | Result |
|---|---|---|
| Byte-identical files | md5 over 2,282 tracked sources, comments/whitespace stripped | **121 lines** |
| Near-duplicate files | line-set Jaccard over all 831 substantial `.cpp` | **1 pair** ≥ 0.45 |
| Block clones | 8-line sliding windows, whole tree | ~2,500 of 207K = **1.2%** |
| Dead classes in `opensyde_tool` | all 695 grepped repo-wide incl. `.ui`/`.qrc`/CMake | **0** |
| Unused `.qrc` resources | all 710 checked, including dynamically assembled paths | **0** |

Of ~628K tracked lines, only ~383K is code: 20% doc comments, 14% blank, and
5.2% (32,576 lines) pure `//-----` ruler bars.

**openSYDE is verbose, not redundant.** The volume is the STW comment
convention and a large number of small classes (1,487 total, 910 in
`opensyde_tool`), not copy-paste.

---

## Rejected: the largest line-count "wins"

Two surveys independently ranked comment stripping as the #1 opportunity —
~20,500 lines of banner rules, ~7,900 empty section markers, ~23,800 lines of
Doxygen blocks that only restate the function name. Together ~52,000 lines, 14%
of the tree, at "zero risk".

**Do not do this.** This fork actively merges from upstream STW — the history
carries `Upstream merge B5a…B9` commits and Release 37 (2026-05) and Release 38
merges. Rewriting comments in every file makes every future upstream merge
conflict everywhere and destroys `git blame`, for no behavioural gain. Line
count is a vanity metric here.

**Also rejected: sharing the CAN Monitor / SYDEflash stylesheets.** 8 `.qss`
files are byte-identical between the two tools (318 lines), but **11 of the 19
already differ**. That is per-tool ownership of styling that currently
coincides, not accidental duplication; sharing it would couple two tools against
their design and make the next intentional divergence a refactor.

---

## Done

| PR | Change | Net |
|---|---|---|
| #5 | Dead core classes (`C_OscZipData`, `C_OscConfFileHandler`, `C_OscUtilChecksummedIniFile`, `C_OscNodeDataPoolFilerV2`) + 4 unused TGL functions | −885 |
| #6 | Characterisation tests for the XML parser `*Error()` API + one filer migrated | +tests |
| #7 | Five byte-identical table delegates → `C_TblHoveredRowDelegate` | −728 |
| #8 | `QVector` → `QList` (same type in Qt 6) | clarity |

### Kept deliberately, though uncalled

`C_OscXcoLoad` and `C_OscXceLoad` have no callers, but they are the unpack
halves of the X-config / X-certificates packages and the pattern is load-bearing
elsewhere: `C_OscSupServiceUpdatePackage` has the identical Create/Load pair and
its Load half **is** consumed by SYDEsup. Both are 2025 additions whose Create
siblings are live. They read as consumer-side API awaiting a consumer, not rot.

---

## Traps found the hard way

**Deleting a function can orphan its private helpers, and only one platform will
tell you.** `TglFileAgeString` was the sole caller of the file-local
`m_FileAgeDosTime`, which exists only in the Windows TGL variant. Nothing but the
35-minute Windows CI job compiles that file, so neither the local core build nor
the Linux and macOS jobs could catch it. `-Wunused-function` under `-Werror`.

**The filer boilerplate is worth far less than a block-level estimate suggests.**
Converting `if (SelectNodeChild(x) == x) {...} else {c_Retval = Errc::config;}`
to `SelectNodeChildError` saves ~2 lines per site, not ~15 — only the four-line
`else` collapses, the body is real work, and some sites need a guard *added*.
Across the ~134 convertible sites that is roughly **−270 lines, not −1080**.
The value is that errors stop vanishing, not the subtraction.

**A blind filer sweep would break three separate things.** Repo-wide the
`SelectNodeChild` population splits into **134 convertible**, **179 optional**
(no `else` — converting turns an absent-but-legal node into a hard error), and
**137 other shapes**, plus a separate population of **238
`tgl_assert(SelectNodeChild...)`** sites that are assertions rather than error
handling.

**The sharpest filer edge:** where a preceding call already assigned
`c_Retval`, assigning the select result unconditionally reports success and
*erases that failure*. `C_OscHalcConfigStandaloneFiler`'s `domain-id` site
needed an explicit `!c_Retval` guard.

**The `*Error()` helpers do not log on their own.** `C_OscXmlParserBase` returns
the code silently; only `C_OscXmlParserLog` calls `ReportErrorForNodeMissing`,
and only when a heading has been set. Replacing a filer's hand-written
`osc_write_log_error` with the helper drops the message unless that caller was
passed a logging parser. This is invisible from the signature — check per call
site. Twelve filers already use `C_OscXmlParserLog`; those are the safe ones.

---

## Open, ranked by value

1. **C++23 idioms.** The tree compiles as C++23 and uses almost none of it:
   **0** `std::optional`, **0** `std::expected`, **0** `std::span`, **0**
   `std::ranges`, **0** `[[nodiscard]]`, **0** structured bindings, 4
   `std::move`, 11 `string_view`, 26 `constexpr` — and **1,761 raw index loops
   against 35 range-for**. Phases 2/3/5/6 modernized the *types* and left the
   *idioms* at C++98. `std::expected<T, std::error_code>` is the natural
   completion of phase 5; PR #6 laid the test foundation. Pilot one filer class
   before committing to the 2,788 out-parameters in headers.
2. **Tooltip `event()` family** — 16 files, ~2,389 lines, 8 variants of one
   handler; each wraps a *different* Qt widget so they cannot share by
   inheritance (this is the mixin case). Note the `SigHideOtherToolTips`
   divergence is **not** a bug: the signal is connected in exactly one feature
   (the Datapool selector) and the 5 emitters are precisely the widgets used
   there.
3. **`C_UsHandler` / `C_UsFiler` triplicated** across `opensyde_tool`,
   CAN Monitor and SYDEflash — 358 verbatim lines in one, 237 in the other.
   Needs a shared base with per-tool ini path and singleton plumbing.
4. **Filer error-API migration**, ~133 sites remaining, ~−270 lines, per the
   traps above.
5. **Scrollbar handlers** — 14 copies, 3 distinct bodies, one 5-line
   conditional. ~56 lines if named slots are kept, ~266 if 11 `connect()` sites
   become lambdas. The larger figure costs readability at every call site.

### Explicitly not worth scheduling

Forward-declaration / include-hygiene work: headers average 3.6 includes, only
27 include ≥8 project headers, and PCH is wired on every target. Expect <2%
build time. `NULL` → `nullptr` (616 sites) and `(void)` parameter lists (8,823)
are cosmetic; fold the former into whatever files other passes touch. First-party
C-style casts: **zero** — the 34 found are all in vendored `miniz.c`.

---

## Silently discarded `std::error_code` returns (2026-09-13)

Phase 5 converted every STW `int32_t` error return in `opensyde_core` to
`std::error_code`, but nothing checks that callers actually look at them. There
are **804** distinct `std::error_code`-returning function names in core headers
and **zero** `[[nodiscard]]` anywhere in the tree.

Scanning statement-position calls that neither assign the result nor cast it
away, then resolving each call to its declaring class (name collisions across
filers make an unqualified name lookup useless — `h_SaveData` and
`mh_SaveDataPools` each have both `void` and `std::error_code` overloads in
different classes) and discarding continuation lines of a multi-line assignment,
leaves **19 genuinely dropped error returns**: 7 in `libraries/`, 12 in
`opensyde_tool`.

### Fixed here, because the surrounding code proves the intent

- `C_OscNodeFiler::mh_SaveCanOpenManagers` — the `orc_BasePath.empty()` branch
  dropped `C_OscCanOpenManagerFiler::h_SaveData`'s result while **the sibling
  branch assigns `h_SaveFile`'s**, and the function documents
  `\retval Errc::config file could not be created`. A failed save reported
  success.
- `C_OscHalcMagicianGenerator::m_FillHalcDatapools`, three call sites — the
  enclosing loops are written `for (...; (u32_ItDomain < ...) && (!c_Retval); ...)`,
  so they test `c_Retval` to stop early while the body never assigns it. The
  guard could not fire. `mh_FillHalcDatapoolsDomain` returns `Errc::config` only
  on structural problems, not in normal operation, so propagating is safe.

### Left for triage — they need domain judgement, not a sweep

The remaining 16 may be deliberate. `h_SetValueInMinMaxRange` (6 sites) reports
whether it clamped, which a caller may legitimately not care about;
`h_EthDisconnectNode` is called during teardown. But four sites in
`C_SdClipBoardHelper` drop load and save results, and those look like the same
defect class as the two fixed above.

### Second pass: member calls through `this->`

The first scan only covered qualified static calls (`C_Class::fn(...)`). Extending
it to `this->fn(...)`, which resolves unambiguously to the enclosing class, found
**9 more discarded returns and no new confirmed defects.** Checked individually:

- `C_OscNode::MoveDataPool` discards `DeleteDataPool` and `InsertDataPool`, but
  the enclosing `if` validates both indices and those two only fail with
  `Errc::range` on a bad index. **Provably safe** -- do not "fix" it.
- `C_OscComDriverBase::SendCanMessageDirect` (2 sites) is the cyclic-message
  pump. A silent send failure matters for a flashing tool, but stopping the pump
  on one failure may be worse. No sibling evidence either way; needs domain
  judgement.
- `C_OscIpDispatcher*::CloseUdp` (both platforms) and
  `C_OscProtocolDriverOsy::m_HandleAsyncResponse` are teardown and async paths
  where best-effort is defensible.

That the extension turned up no further confirmed defects raises confidence that
the five fixed are the real ones, rather than the first five of many.

### The durable fix

`[[nodiscard]]` on the error-returning core API would make this a compile error
rather than a scan. The codebase is already shaped for it: it marks intentional
discards with `(void)expr` in 4,439 places as a MISRA convention, so the idiom
for "I meant to ignore this" already exists and is already used everywhere else.
That is a large sweep and a separate decision.

---

## Two more survey items that do not survive inspection (2026-09-13)

### Popup-dialog scaffolding — already done

The `opensyde_tool` survey ranked this ~700 lines across 92 sites. There is a
detailed, user-approved brief for it at
`docs/agent_plans/archive/gui_consolidation/phase3-brief.md` describing a
`C_OgePopUpContentBase` that hoists both the `mrc_ParentDialog` member and a
22-line Ctrl+Enter `keyPressEvent`.

**That base class exists and the migration landed.** Only 5 classes still declare
their own `mrc_ParentDialog`, and searching for the actual Ctrl+Enter → accept
pattern (Key_Enter/Return **and** Alt **and** Shift checks **and** an
`accept()`/`m_OkClicked()` call) finds **4 overrides left, all with distinct
bodies**:

- `C_GiSyBaseWidget` and `C_GiSyColorSelectWidget` interleave colour-picking
  cancel handling with the enter handling.
- `C_ImpCodeGenerationReportWidget` is a straggler but small.
- `C_PopPasswordDialogWidget` is on the brief's own exclusion list (inherits
  `QDialog`, not `QWidget`).

A first count of "30 remaining" was wrong: the filter matched any
`ControlModifier` handler, so it swept up things like `C_CamMetTreeView`'s
Ctrl+C copy. Worth repeating only with the narrow predicate.

Note the base as built is narrower than the brief — it hoists the member but not
`keyPressEvent`/`m_OnEnterAccept`. Given only 4 sites remain and each has extra
logic, finishing that design now would cost more than it saves.

### Cam/Fla `PubPathVariables` — already consolidated, and the rest is domain logic

`C_CamOgePubPathVariables` and `C_FlaOgePubPathVariables` already share
`C_CamOgePubPathVariablesBase` in `libraries/opensyde_gui/`. Stripping comments,
the only differences left are the three menu entries ("CAN Monitor Binary" vs
"SYDExsh Binary" and so on) — per-tool by design. The prior GUI-consolidation
effort reached the same conclusion independently and recorded it in
`phase5-progress.md`: *"ctor calls m_AddHeading / m_AddEntry (real domain logic,
not styling)"*.

The one real wart: that shared base is named `C_Cam*` and lives under a
`can_monitor/` subtree of the shared library, while SYDEflash compiles and
depends on it. Renaming is cosmetic and touches both apps' CMakeLists and lint
lists.

`C_CamOgeLeFilePath` vs `C_FlaOgeLeFilePath` **are** functionally identical —
13 vs 14 code lines differing only by an extra `#include <cstdint>` — so that
pair alone could still be merged, for about 60 lines.

---

## `[[nodiscard]]` blast radius, measured (2026-09-14)

The dropped-error scan above raised `[[nodiscard]]` as the durable fix but left
it as an open decision. This measures it so the decision is cheap.

**Experiment:** annotate every `std::error_code`-returning declaration in
`opensyde_core` headers — **981 declarations across 122 headers** — then build.

**Result: 60 call sites**, across 22 files.

| Subsystem | Sites |
|---|---|
| `protocol_drivers/communication` | 25 |
| `protocol_drivers/*` (update, config, base) | 14 |
| `md5` | 4 |
| `ip_dispatcher/target_linux_sock` | 3 |
| `project/system*` | 5 |
| `halc/*` | 6 |
| `data_dealer` | 2 |

### Why this is worth doing

**The compiler finds roughly three times what a hand-written scan does.** The
scan earlier in this document resolved qualified static calls and `this->`
member calls and found 19 sites repo-wide; `[[nodiscard]]` finds 60 in core
alone, because it also catches calls in expression and argument position that no
line-oriented heuristic will match.

The codebase is already shaped for the annotation: intentional discards are
written `(void)expr` in **4,439** places as a MISRA convention, so the "I meant
to ignore this" idiom exists, is already used everywhere else, and reads as
deliberate rather than as noise.

### Why it was not landed here

60 sites each need a judgement — propagate, or mark `(void)` — and 39 of them
are in `protocol_drivers`, where best-effort on a CAN send or a teardown
disconnect may well be correct. Making those calls in bulk would be exactly the
"behaviour change made on a guess" that the rest of this sweep avoided. The
annotation is cheap; the triage is the work, and it wants someone who knows the
protocol layer.

### Correction: 60 was also a core-only number

The 60 above counts callers inside `opensyde_core`. The same headers are consumed
by the GUI trees, and those add more. Annotating only `halc/` and `data_dealer/`
gives **14 sites in core plus 6 in `opensyde_tool`**; adding `project/` brings a
further **17 GUI sites** (`C_PuiSdHandler*`, `C_PuiSvHandler`, `C_SdClipBoardHelper`
and others dropping results from `DeleteBus`, `SetNodeName`, `InsertDataPool`,
`SetNodeUpdateInformation`).

So the "easy three subsystems" are roughly **37 sites**, not 15. Measure with a
full `./build.sh all`, not a core build.

A sensible staging if this is taken up: `halc/` + `data_dealer/` first (20 sites),
then `project/` (a further ~17, all GUI handler paths), then `protocol_drivers`
behind domain review.

### A defect the annotation surfaced

`C_OscHalcConfigDomain::CheckChannelLinked` assigns its `orq_IsLinked` out
parameter **only on its success paths**. When the channel index is out of range it
returns `Errc::range` without touching it.

`C_SdNdeHalcConfigImportModel.cpp` (around line 370) declares

    bool q_IsLinkedOld;
    bool q_IsLinkedNew;

uninitialised, calls `CheckChannelLinked` twice **discarding both results**, and
then reads both bools. If that call ever fails, this reads uninitialised memory.

Whether it can fail in practice depends on `u32_ChannelCounter` always being in
range, which was not traced. The shape is unsafe regardless: an out parameter
consumed without checking the status that says whether it was written. Either the
callee should initialise `orq_IsLinked` before its range check, or the caller
should check. This is the clearest argument in this document for the annotation —
no scan in this sweep would have found it.

### Measuring it again

Use `ninja -C <build> -k 0`. A plain `cmake --build` stops at the first failing
translation unit and reports **4** sites, not 60 — the truncated number looks
like a decisive answer and is not.

---

## `-Wconditional-uninitialized` is not worth enabling here (2026-09-14)

Having found two uninitialised-out-parameter bugs by hand, the obvious next move
was to let the compiler do it. Clang has a warning aimed squarely at that class
and it is **not** part of `-Wall -Wextra`.

**Result: 9 sites in `opensyde_core`, all of them false positives — and neither
of the two real bugs is among them.**

The nine split into two shapes the warning cannot see through:

- **Pointer out-parameters.** `C_OscSystemDefinition::CheckErrorNode` takes
  `bool *` parameters, passes them to `CheckErrorManager`, and reads the locals
  afterwards. `CheckErrorManager` writes through every non-null pointer
  unconditionally, so the locals are always written — but that is in another
  translation unit's function body, which the warning does not analyse. 4 sites.
- **A boolean flag correlated with initialisation.** `C_CanMonProtocolOpenSyde`
  sets `u32_SnrSignStartIndex` in the branches where it also leaves
  `q_DlcCorrect` true, and reads it only under `if (q_DlcCorrect == true)`.
  `C_OscCanMessage` does the same with `q_NoCheckNecessary`. Clang does not
  correlate the two. 3 sites. The remaining 2 are the `h_GetComListIndex`
  short-circuit already described above.

**Why the real bugs are invisible to it.** Both `CheckChannelLinked` and
`CalcFileChecksum` are *cross-function*: the callee returns before writing its
out parameter and the caller reads it anyway. Nothing in the caller's own body
looks wrong, so an intraprocedural warning has nothing to flag. That is exactly
why `[[nodiscard]]` found them — it does not analyse flow, it just makes the
ignored status a compile error and lets a human look at the call site.

So: do not enable this warning. It would cost 9 pointless `= 0` initialisations,
each of which makes the code marginally less honest about what it knows, and
would catch none of the real instances.

### Measuring warnings in this tree

`-DCMAKE_CXX_FLAGS=...` on the configure line **does not work** — the toolchain
files set `CMAKE_CXX_FLAGS` outright and win. Append the flag to
`cmake/toolchain_*.cmake` temporarily instead, and verify it arrived:

    python3 -c "import json;print('conditional-uninitialized' in json.load(open('build/warn/compile_commands.json'))[0]['command'])"

Without that check the first run of this experiment reported **0 findings**, from
a build that never had the flag, and also silently tried to compile the Windows
TGL sources because `OPENSYDE_CORE_SKIP_WINDOWS_*` had been left off.

---

## The uninitialised-out-parameter class cannot be closed by scanning (2026-09-14)

Two real bugs of this shape were found and fixed (`CheckChannelLinked`,
`CalcFileChecksum`). The obvious follow-up was to prove there are no more. Three
methods were tried. **None of them can establish absence, and the reasons are
worth recording so the next attempt does not repeat them.**

### 1. Textual scanning — 50% false negatives, and its one hit was a false positive

A scanner was written that finds callees which "can return an error before
writing a scalar out parameter", then callers that declare the variable
uninitialised, discard the status and read it anyway.

Validated by temporarily undoing both known fixes and re-running: **it found 1 of
the 2.** A rewrite using line-based brace counting instead of regex (the
multi-line function signature had defeated the first version) still found 1 of 2,
plus one new candidate that turned out to be a false positive —
`C_OscComDriverFlash::SendOsyBroadcastRequestProgramming` writes
`orq_NotAccepted = false` unconditionally on its first line.

The flaw is structural, not a bug in the script. The heuristic asks *"is there a
write before the first error, textually?"* In `CheckChannelLinked` the first
write sits at relative line 30 and the outermost range return at line 87 — later
in the text, but on a different path. **Textual ordering is not path analysis**,
and no amount of regex fixes that.

### 2. `-Wconditional-uninitialized` — 9 findings, all false, both real bugs invisible

Covered in the section above. It is intraprocedural, so a callee that fails to
write its out parameter is simply not something it can see.

### 3. What actually worked

`[[nodiscard]]`. It performs no analysis at all — it makes the ignored status a
compile error and puts a human in front of the call site. Both bugs were found
that way, and the second was found by looking for the shape of the first.

### Recommendation

For this class specifically, the right tool is the **clang static analyzer**
(`scan-build`, or `clang-tidy` with the `clang-analyzer-*` checks), which is
path-sensitive and does cross-function reasoning. Neither `clang-tidy` nor
`scan-build` is installed on the dev machine or the build host, so this could not
be tried here. Adding it — even as a non-gating CI job — is likely worth more
than any further scanning, and would also cover the other analyser checks this
tree has never been run through.

Until then: **do not treat a zero result from a textual scan as evidence that
this class is clear.** The scan in this document demonstrably misses half of the
known instances.

---

## clang static analyser: installed, run, and wired into CI (2026-09-14)

`clang-tidy-19` and `clang-tools-19` are installed on the build host, version
matched to its clang 19.1.7 — a mismatched clang-tidy parses C++23
inconsistently. A non-gating `Static Analysis (advisory)` job in
`.github/workflows/build.yml` now runs it over `opensyde_core` on every push and
writes the findings to the job summary.

It is **advisory and must stay that way**: it has a real false-positive rate on
this codebase, documented below.

### Baseline over all 220 core sources: 9 findings

| Finding | Verdict |
|---|---|
| `C_HexFile.cpp:375` garbage value in `&` | **Real.** Fixed — the S-record address was read before checking `m_GetSRecordAddress` |
| `C_HexFile.cpp:686` null deref of `mpt_HexData` | Real hazard, **no reproducer**. Hardened; see below |
| `C_HexFile.cpp:183` `errno` may be overwritten by `fclose` | Minor, unreviewed |
| `C_Md5Checksum.cpp:358` stream read at EOF / indeterminate position | Unreviewed |
| `C_OscSuSequences.cpp:1035` same stream pattern | Unreviewed |
| `C_OscSuSequences.cpp:1088` "opened stream never closed" | **Not reproduced.** The file is closed on both the failure path (line 975, guarded by `c_Return != success`) and the success path (line 1082), and nothing changes `c_Return` between them. Left alone rather than restructured on an unproven path |
| `C_SclStringUtil.hpp:567` "memory allocated with size zero" | Looks false: `new char[x_FileSize + 1U]` is at least 1, and `buffer[x_FileSize]` is in range |

### What it does and does not catch

It found the S-record defect, which no scan in this sweep did. It does **not**
catch either of the out-parameter bugs (`CheckChannelLinked`,
`CalcFileChecksum`), because those are cross translation unit: the callee is in
core, the caller in a GUI tree, and the analyser is intra-TU by default. It will
not inline a callee it cannot see and conservatively assumes the out parameter
was written.

Cross-TU analysis exists (`clang-extdef-mapping`, `experimental-enable-naive-ctu-analysis`)
and was attempted. It needs pre-built `.ast` files per callee translation unit
and an external definition map; the AST emit step failed on the first attempt
and chasing it further was worth less than simply running the analyser in its
normal mode, which is what found the new defect. If someone wants those two bug
shapes caught automatically, CTU is the route, and it is a project in itself.

### Running it by hand

    cmake -S libraries/opensyde_core -B build/tidy -G Ninja \
      -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/toolchain_linux.cmake \
      -DOPENSYDE_CORE_SKIP_WINDOWS_DRIVERS=ON -DOPENSYDE_CORE_SKIP_WINDOWS_TARGET=ON \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    ninja -C build/tidy            # required: the compile database references a PCH
    clang-tidy-19 -p build/tidy --checks='-*,clang-analyzer-*' \
      --header-filter='libraries/opensyde_core/.*' --quiet <file.cpp>

Two things that will otherwise waste time: without the build step it fails with
"PCH file not found", and without `--header-filter` it silently suppresses
everything in project headers (it reported "Suppressed 25593 warnings" and
printed nothing).

---

## `[[nodiscard]]` rollout: what landed, and what is left (2026-09-16)

Four subsystems are done. **700 declarations carry the attribute**, and every
call site it flagged has been either propagated or marked `(void)` with the
reason written next to it.

| Subsystem | Annotated | Sites resolved | Propagated |
|---|---|---|---|
| `halc/` + `data_dealer/` | 164 | 12 | 3 |
| `project/` | 206 | 22 | 4 |
| `protocol_drivers/` | 318 | 29 | 0 |

**Of 63 call sites, only 7 were genuinely dropped errors.** The rest were
already correct and are now documented as such. `protocol_drivers` in particular
produced **zero** — every one of its 29 was teardown, cleanup on an error path
where assigning would have replaced a real result, or a call whose enclosing
function returns `void`. Several were settled by comments already in the code
("Stop routing always to clean up"; "this session request will return with an
error. This error can be ignored").

### Still unannotated

| Subsystem | `std::error_code` declarations without the attribute |
|---|---|
| `exports/` | 53 |
| `system_update_package/` | 37 |
| `security/` | 35 |
| `imports/` | 32 |
| `xml_parser/` | 25 |
| `protocol_drivers/` | 12 (deliberate, below) |

The 12 in `protocol_drivers` are the five deferred callees --
`SetNodeIdentifiers`, `SendCanMessageDirect`, `HandleCanMessage`, `Cycle`,
`m_HandleAsyncResponse` -- each carrying a comment at its declaration saying why.
Whether a caller should abort on one failed send is a protocol decision.

### Two markings that want a product decision

Both are `(void)` today because there is nowhere sensible to report from, not
because the failure does not matter:

- `C_NagMainWindow` and the device definition load. Now at least reported
  through a dialog, but the underlying `LoadFromPaths` had to be taught to
  report failure at all first -- it returned `Errc::success` unconditionally.
- `C_SyvDaDashboardsWidget::m_InitOsyDriver` discarding `StartLogging`. A
  failure means CAN signal interpretation does not start on the dashboard. It
  sits inside `switch (s32_Retval)`, so propagating is a restructure.

### The build host is a weaker checker than CI

`C_SyvDaDashboardsWidget`'s discard was rejected by all three CI runners and
accepted by the 48-core build host. The declaration is annotated, there is no
unannotated override, and clearing the precompiled headers changes nothing.

**Corrected later in the sweep:** the host was not running clang at all. Its
`build/Debug` directory had been configured before the tree moved to clang, and
CMake keeps the cached compiler across a toolchain change -- so it was building
with **GCC 14** while `toolchain_linux.cmake` said clang. So the compiler that
accepted that discard was GCC 14, not clang 19, and the divergence was between
two different compilers rather than two versions of one. See the section on
stale build directories below.

**So "all eight tools build on the host" does not mean CI will agree.** For
warning-driven work, the host is a fast first pass and CI is the authority.

## Wave 2: the remaining core subsystems

The first rollout covered four subsystems. This wave annotated **everything
else in core** that returns `std::error_code` -- 281 declarations across 53
headers, in `exports`, `system_update_package`, `security`, `imports`,
`xml_parser`, `zip`, `util`, `can_dispatcher`, `ip_dispatcher`,
`cmon_protocols`, `system_package_handling`, `scl`, `data_dealer`, `hex_file`,
`md5`, `tgl`, `stwerrors`, `conf_file_handler` and `stw_compid`.

**281 declarations produced 14 discarded-return sites.** That ratio keeps
falling (wave 1: 700 declarations, 63 sites), which is what you want -- it means
the convention was already being followed by hand in most of the tree.

Of the 14:

| Sites | Disposition |
|-------|-------------|
| 4 | **Real defect** -- MD5 (below) |
| 5 | Callee cannot fail; fixed by changing the callee, not the caller |
| 5 | Legitimate `(void)`: destructors and error-path cleanup |

### The defect: `C_Md5Checksum::GetMD5` hex-encoded uninitialised stack

Both buffer and `FILE *` overloads discarded `mh_Md5Process` and `mh_Md5Done`.
Those return `Errc::config` on a corrupt hash state, and on that path
`au8_Result[16]` is **never written** -- so the function hex-encoded
uninitialised stack and returned it as a valid-looking MD5. The header already
documented "empty string if there are problems"; the code did not implement it.

This is the third instance of the same class (uninitialised out-param read on an
error path) that `[[nodiscard]]` has found, after `C_OscHalcConfigDomain` and
`C_OscHexFile`. A wrong checksum is the worst shape for this bug: the caller
cannot tell a garbage digest from a good one by looking at it.

The failure path needs a corrupted `C_HashState`, so it is not reachable through
the public API today. It was still worth fixing, and worth noting that **MD5 had
no test coverage at all** -- `test_md5.cpp` now pins it against the RFC 1321
appendix A.5 vectors, the 64-byte block boundary, both file entry points and the
empty-string-on-failure contract.

> Both times I wrote an expected digest from memory it was wrong and the
> implementation was right. The vectors in that file are all checked against
> Python `hashlib`, not against the code under test.

### `std::error_code` returns that can never be anything but success

Scanning core for functions whose every `return` is literally `Errc::success`
found **17**. Only **4** are non-virtual:

- `C_CanMonProtocols::SetProtocolMode` / `SetDecimalMode` -- plain setters.
  Converted to `void`. This removes 5 of the 14 discard sites without writing a
  single `(void)`, which is the better fix: a caller should not have to
  acknowledge an error that cannot happen.
- `C_HexFile::Validate` -- a **false positive**. Its error arrives through
  `GetDataDump`'s out-param, which the scan does not model.
- `C_OscCanOpenEdsDeviceInfoBlock::LoadFromIni` / `C_OscCanOpenEdsFileInfoBlock::LoadFromIni`
  -- an INI parser that always reports success is suspicious in its own right.
  Left alone; that is a separate question from this sweep.

The other 13 are virtual overrides of interfaces whose *other* implementations
do fail, so they have to keep the signature.

**The scan's own limits:** the first version reported 66, because
`return SomeCall();` contains no `Errc::` token and so looked infallible. The
tightened version requires every `return` to be literally `Errc::success` or a
local never assigned from a call. It still cannot see errors that arrive by
out-param, as `Validate` shows.

### Windows-only code is invisible to the Linux jobs

`C_OscIpDispatcherWinSock.cpp:643` had the same discarded `CloseUdp()` as its
Linux twin. Neither the local build nor the two Linux CI jobs compile that file
-- only the Windows job does. When a fix lands in a `target_linux_*` file, grep
the `target_windows_*` sibling before assuming a green build means anything.

### What the tool trees added: 22 more sites

Core built clean. The eight-tool build then produced **22** discarded returns
that core-only building structurally cannot see, because core does not compile
the GUI trees at all.

**The first build reported 2 of the 22.** `build.sh` drives ninja without
`-k 0`, so it stops at the first failing target. This is the second time in
this sweep that a keep-going flag changed the size of a result by an order of
magnitude. For any warning-driven sweep, `ninja -C build/Debug -k 0` directly,
not `build.sh`.

| Group | Sites | Real defects |
|-------|-------|--------------|
| `C_OscXmlParser::LoadFromString` in clipboard helpers | 13 | 0 (all correct by accident) |
| `C_OscCanOpenObjectDictionary::Is*Ro` | 7 | 3 |
| `C_OscSecurityPemDatabase::ParseFolder` | 3 | 0 (already documented as optional) |

#### The clipboard group: correct by accident, 13 times

Every one had the same shape -- parse a clipboard string, then read the parser
without checking. All 13 are *currently* correct, because a failed parse leaves
the parser empty, so the following `SelectRoot()` comparison does not match and
the function falls into the `else` that already returns `C_CONFIG` (or
`C_RANGE`). The parse check now returns that same code directly.

Nothing is fixed here in the sense of changed behaviour. What changes is that
correctness no longer depends on a guard fifty lines away continuing to exist.

#### The CANopen group: the uninitialised-out-param class again

`Is*Ro` leaves its `bool &` untouched when the EDS entry is missing. Three of
the seven callers were wrong, each differently:

- `C_SdBueMessageSelectorTreeWidget:2405` declared `bool q_IsPdoRo;`
  **uninitialised** and branched on it. Stack garbage decided whether a PDO
  appeared read-only.
- `C_SdBueMessagePropertiesWidget` declared **one** flag and reused it across
  four queries. A failed query left the previous control's read-only state
  applied to the next one -- a stale value rather than an uninitialised one,
  and invisible to any uninitialised-read analysis.
- `C_SdBueMlvGraphicsScene:833` passed the member `mq_CoFixedMapping` directly,
  so a failed query left another message's value in place.

The other four already used a fresh flag defaulted to `false`, which is why the
fix defaults to `false` everywhere: it matches what most of the tree already
did. Whether a missing EDS entry *should* leave a control editable is a product
question, not something to settle inside a warning sweep.

That single-flag-reused-four-times case is worth dwelling on. It is not an
uninitialised read, so `-Wconditional-uninitialized`, `-Wmaybe-uninitialized`
and the clang analyser would all stay quiet on it forever. `[[nodiscard]]`
found it because it does not look at the variable at all -- it looks at the
call.

### Running total for the attribute

| | Declarations | Sites | Real defects |
|---|---|---|---|
| Wave 1 (4 core subsystems) | 700 | 63 | 7 |
| Wave 2 (rest of core) | 281 | 14 | 1 |
| Wave 2 (tool trees) | -- | 22 | 3 |

**981 declarations, 99 sites, 11 defects.** Nine of the eleven are the same
shape: an error path that leaves an out-param unwritten, and a caller that
reads it anyway.

## The 1-based indexing residue: four silent parser failures

Following the EDS question turned up a defect class the whole sweep had missed,
because none of its tools look for it.

`C_SclString` was **1-based**. `std::string` is 0-based. The phase-3 migration
handled the method calls correctly -- that is what `SubStringCompat`, `PosCompat`
and the rest of `C_SclStringUtil.hpp` exist for, and they carry the conversion in
their names and comments. What it did not handle is **raw `operator[]` with a
literal index**, which changes meaning silently and compiles either way.

Four sites, all found by one grep:

```
grep -rnE "\w+\[[0-9]\]\s*[=!]=\s*'" --include='*.cpp' libraries opensyde_tool
```

| Site | What it was testing | Effect |
|------|--------------------|--------|
| `C_SclIniFile.cpp:161,165` | `[` and `;` at line start | **No INI section or comment was ever recognised** |
| `C_OscXmlParser.cpp:572` | `0x` prefix on an attribute | every hex attribute returned its default |
| `C_OscHalcDefContentBitmaskItem.cpp:132` | `0x` prefix on a bitmask | `0x10` silently parsed as `0` |
| `C_OscUtils.cpp:949` | `//` or `\\` UNC prefix | UNC paths treated as relative, base dir prepended |

The `C_SclIniFile` one is the serious one. `SectionExists` returned false for
every section, so **every** `ReadString`/`ReadInteger`/`ReadBool` fell back to its
default. That is silent by construction: a missing key is not an error, it is a
default. It takes out CANopen EDS/DCF import and the CAN Monitor protocol
parameter files.

Each of the four pairs the wrong index with a **correctly** migrated 1-based
`SubStringCompat(..., 3UL, ...)` right beneath it. That is the signature: the
substring extraction was converted, the guard in front of it was not. A site
where both were converted looks identical to a site where neither was.

`TglFile.cpp:613` matches the same grep and is **correct** -- `"C:\"` genuinely
has `:` at index 1. The pattern finds candidates, not bugs.

### Why nothing caught this

- It compiles. `std::string::operator[]` at `size()` is defined and returns
  `'\0'`, so there is not even UB to trip a sanitiser.
- Every failure mode is a *default*, not an error. No log line, no error code.
- `[[nodiscard]]` cannot see it: the return values are all used.
- **None of these classes had any test at all.** `C_SclIniFile` had none, the
  XML hex path had none, the bitmask parser had none.

That last point is the whole story. `test_scl_ini_file.cpp`,
`test_string_prefix_parsing.cpp` and `test_canopen_eds.cpp` are new, and the
first assertion in each is the one that was failing in production.

### A second bug in the EDS loader

Separately, `C_OscCanOpenObjectDictionary::LoadFromFile` aborted its parse loop on
a malformed object description and then **overwrote the error two lines later**:

```cpp
if (c_Return) { break; }          // "error in EDS file; abort"
}
c_Return = m_CheckForExistingObjects("MandatoryObjects", c_IniFile);   // clobbers it
```

The three following checks were already guarded with `if (!c_Return)`; the first
was not. So a malformed EDS imported as success while `GetLastErrorText()` still
held the real message -- incoherent state the caller had no way to detect.

### Still open here

`C_OscCanOpenEdsDeviceInfoBlock::LoadFromIni` and its `FileInfoBlock` twin
document `Errc::config` for a missing value and **never return it**. Both read
through `ReadString`/`ReadBool` with defaults instead of the validating
`h_Load*ValueFromIniFile` helpers that sit unused in the same file -- four public
functions with **zero callers**, kept behind a `//lint -e{8062} Kept for later
error reporting` comment. The plumbing above them is live and reaches the user.

Wiring it up would make EDS import stricter, which can reject files that work
today. The original author's own "Maybe mandatory values" comment says they were
unsure which keys are mandatory. **That is a product decision, not a sweep
decision**, so it is recorded here rather than made.

## The same migration, the other half: hex parsed as decimal

The `[0]`/`[1]` fixes closed the *indexing* half of the `C_SclString` migration.
The *parsing* half was still open, and it is worse.

`C_SclString::ToInt()` was **hex-aware and threw on bad input**. The migration
replaced it with `std::stoi`, which throws but is **hard-wired to base 10**.
`std::stoi("0x10")` reads the leading `0`, stops at the `x`, and returns **0**
without throwing. Every `try`/`catch` around it stays quiet.

`C_SclStringUtil.hpp` already documents this exact failure -- `ToIntCompat`'s
comment says so in as many words, and `ScanBaseCompat` exists to pick base 16 for
a `"0x"` prefix without treating a leading `0` as octal. The helper was written;
these call sites were never moved onto it.

### Seven of eight XML attribute getters claimed hex and did not do it

Every one carries the line *"Can handle "0x" notation to interpret hex values"*,
and each explains that it avoids `XMLElement::Query` **precisely because Query
cannot parse hex**. Then it calls `std::stoi(c_Text)`.

| Getter | Documented hex | Implemented |
|--------|---------------|-------------|
| `GetAttributeSint32` / `Uint32` / `Sint64` | yes | **no** |
| `GetAttributeUint64` | yes | yes (and it was broken too -- see the indexing section) |
| the four `*Error` variants | yes | **no** -- they delegate to the plain getters |

Fixing three functions fixes all eight, because the `*Error` variants delegate.

### What that actually broke: every parameter set file

Two production writers emit `"0x"`-prefixed values, and both are on the same file
format:

- `C_OscChecksummedXml::SaveToFile` writes the file CRC as `"0x" + hex`, and
  `LoadFromFile` read it back with `GetAttributeUint32` -- so `u16_CrcFromFile`
  was **always 0** and the comparison against the real CRC always failed.
  **The class could not load a file it had just written.**
- `C_OscParamSetFilerBase::h_SaveFileVersion` writes `"0x0001"`, and
  `h_CheckFileVersion` read it with a base-10 `std::stoi`, got 0, and logged
  *"Version defined by 'file-version' is not supported."*

`C_OscChecksummedXml` is used by exactly one thing: the parameter set (`.syde_psi`)
filers. So loading any parameter set failed twice over, on the CRC and on the
version.

This was verified by round trip, not by reading: save a checksummed file, load it
back, and the load returns `C_CHECKSUM`. That probe is now
`ChecksummedXml.RoundTripsItsOwnOutput`.

**Every other filer writes its version with `std::to_string`**, i.e. decimal, so
their `std::stoi` readers are correct. The paramset filer is the only one that
writes hex, which is why this is contained to one file format rather than all of
them.

### The scans that found nothing

Worth recording, because negatives are cheap to re-run and expensive to redo:

- **1-based helper result fed into a 0-based `std::string` API** (`PosCompat` etc.
  into `.substr`/`.erase`/`operator[]`, and `.find()` into `SubStringCompat`):
  **zero hits**, across 45 `PosCompat`, 36 `SubStringCompat`, 7 `DeleteCompat`,
  4 `InsertCompat` uses. The scanner was validated against planted cases first --
  it fires on all three shapes.
- **Raw literal indexing on string-typed variables**: 44 candidates, all benign
  after the earlier fixes -- `&c_Text[0]` buffer idioms, `QStringList`/vector
  indexing, and `TglFile.cpp:613` where `"C:\"` really does have `:` at index 1.

So the migration's remaining residue is **parsing**, not indexing. The 68
remaining `std::sto*` calls without an explicit base were checked against their
writers; all are decimal by contract (file versions via `to_string`, DLC,
counters, dates, and `c_RawValueDec` which says so in its name).

### Negative controls matter here

Both fixes make a check *pass* that previously failed. That is exactly the shape
where a "fix" can be a disabled check, so the tests include the inverse:
`ChecksummedXml.DetectsTamperedContent` and
`ParamSetFileVersion.UnsupportedVersionIsRejected` pass **both** with and without
the fix. Only the round-trip tests change state.

### And the third form: 1-based *loop bounds*

Indexing and parsing were two forms of the same migration residue. Loop bounds
are the third, and the scan for it is one line:

```
for (...; <var> <= <something>.length(); ...)
```

An inclusive bound against a *count* is 1-based thinking. Two sites, both real:

**`C_OscUtils::h_NiceifyStringForCeComment`** ran `1 <= index <= length` while
indexing 0-based. So it **never examined character 0**, and its final iteration
read and then *wrote* `c_Result[size()]` -- undefined for the non-const
`operator[]`.

This function exists to stop a Datapool comment from breaking the C file it is
embedded in. With the old bounds, anything at position 0 passed straight
through. Verified by reverting the fix against the new tests:

| Input | Old output |
|-------|-----------|
| `*/rest` | `*/rest` -- **terminates the generated comment** |
| `\nbc` | `\nbc` -- a raw newline inside a block comment |
| `` `ackquote `` | unchanged |
| `text\` | unchanged -- continues a C++ line comment |

The sibling directly above it, `h_NiceifyStringForFileName`, is correct 0-based.
Same file, same author, same pattern -- one converted, one not. That is the
clearest illustration in the tree of why this class is invisible to review: the
right and wrong versions are adjacent and look alike.

**`opensyde_tsp_convert`'s `mh_Sanitize`** had the same bounds: it dropped the
first character and appended a `_` for the terminator, so `"ESX-4CS-GW"` became
`"SX_4CS_GW_"`. Its own doc comment says the answer should be `"ESX_4CS_GW"`.

### The three forms, and what closes them

| Form | Scan | Found |
|------|------|-------|
| Indexing | `\w+\[[0-9]\]\s*[=!]=\s*'` | 4 (PR #29) |
| Parsing | `std::sto*` with no base, checked against its writer | 2 sites, 8 functions (PR #30) |
| Loop bounds | `<=` against `.size()`/`.length()` | 2 (PR #30) |

All three are silent: wrong value, no error, no log, nothing for `[[nodiscard]]`
or a sanitiser to catch. All three were invisible because the affected classes
had no tests. The scans are cheap and worth re-running after any future
string-handling change.

## Handing a defect class to the compiler instead of to a grep

`PrintFormattedCompat` is the project's `printf`. It is varargs, it had **no
format attribute**, and so **not one of its ~200 call sites was ever checked** by
either compiler. A wrong conversion or a missing argument there is undefined
behaviour that no test reliably catches.

Adding `__attribute__((format(printf, 1, 2)))` closes the class permanently, on
every platform, with no scan to remember. It found **7** defects immediately:

| Sites | Problem | Found by |
|-------|---------|----------|
| 4 | `%02d` passed a 64-bit `size_type` | macOS + Linux |
| 3 | `%llu` passed a `unsigned long` | **Linux only** |

The split in that last column is the argument for the attribute all by itself.
`uint64_t` is `unsigned long` on Linux and `unsigned long long` on macOS and
Windows, so `"%013llu"` is correct on two of the three platforms this project
ships and wrong on the third. The local macOS build was silent; the Linux build
host rejected it. `PRIu64` is the portable spelling.

### What the hand-written scan could and could not do

A scan for **argument-count** mismatches across all 200 call sites found **none**
-- but only after it was taught that C++ concatenates adjacent string literals.
The first version read just the first literal of a wrapped format string and
reported **eleven** false positives, including several that looked completely
convincing (`"...%s...%04X.%02X"` with apparently two arguments, where the third
was simply on the next line).

It was validated against planted cases before its zero was believed, and the one
site it flagged most confidently was checked by hand and found correct.

That scan cannot see a **type** mismatch at all, which is the entire set of what
was actually wrong here. The compiler found 7; the scan would have found 0.

**The general lesson for the rest of this sweep:** where a defect class can be
handed to the compiler, hand it over. A grep has to be remembered, re-run, and
re-validated every time; an attribute runs on every build forever, on all three
platforms, and cannot be forgotten. `[[nodiscard]]` earned its place the same
way.

## The build host was silently building with the wrong compiler

`cmake/toolchain_linux.cmake` sets `CMAKE_CXX_COMPILER clang++`. The build host's
`build/Debug` was building with **GCC 14**.

CMake records the compiler in `CMakeCache.txt` on first configure and keeps it.
Changing the toolchain file does not move an existing build directory onto the
new compiler -- it silently keeps the old one. That directory predates the switch
to clang, so every "all eight tools build on the host" check in this sweep was a
statement about GCC 14, not about the clang that CI and the shipped binaries use.

It is visible in one line and nowhere else:

```bash
grep CMAKE_CXX_COMPILER_AR build/Debug/CMakeCache.txt   # gcc-ar-14 vs llvm-ar-19
```

A fresh configure with the same toolchain gives clang 19 (`llvm-ar-19`,
`clang-scan-deps-19`), confirming the toolchain itself is correct.

**This is not all bad news, and that is worth being precise about.** Two compilers
is more coverage than one, and GCC earned its keep here: it caught the
`%llu`/`unsigned long` mismatch. But nobody knew that was what was happening, and
the earlier note about the host "being a weaker checker than CI" was attributed to
the wrong cause. After wiping the directory, all eight tools build clean under
clang 19 as well, so the tree is good on both.

**When measuring compiler behaviour on the host, check the cache or wipe the
directory first.** A toolchain edit that appears to do nothing is the expected
outcome, not a sign that the flag was wrong.

## Warning flags: measured, then mostly rejected

Following the format-attribute result, fifteen candidate warnings were measured
across the tree -- one build with all of them enabled non-fatally, counted by
category. The counts decide, not taste:

| Flag | Findings | Verdict |
|------|---------:|---------|
| `-Wnon-virtual-dtor` | 0 | **adopted** |
| `-Wimplicit-fallthrough` | 0 in project code (3 in vendored tinyxml2) | **adopted** |
| `-Wcast-qual` | 0 in project code (2 in vendored miniz) | **adopted** |
| `-Wswitch-enum` | 7 | rejected -- all are `default:`-covered by design |
| `-Wdouble-promotion` | 3 | rejected -- lossless `float`→`double` into helpers taking `double` |
| `-Wfloat-equal` | 2 | rejected -- `operator==` on a stored value; already carries `//lint -e{777}` |
| `-Wformat-nonliteral` | 4 | rejected -- all vendored; the format attribute already covers ours |
| `-Wzero-as-null-pointer-constant` | 3047 | rejected -- style, not a defect class |
| `-Wsuggest-override` | 4661 | rejected -- style, not a defect class |
| `-Wold-style-cast` | 10273 | rejected -- style, not a defect class |
| `-Wuseless-cast` | n/a | GCC-only; clang rejects the spelling |

**Zero new defects.** That is the honest headline: unlike the format attribute,
which found 7, this sweep found none. The three adopted flags are adopted
*because* they are at zero -- each covers a class that is silent at runtime
(deleting through a base pointer with no virtual destructor, a missing `break`,
casting away `const`) and nothing else in the build would report it.

Adopting them exposed a second stale-compiler assumption: the vendored-source
warning suppression was gated on `CMAKE_CXX_COMPILER_ID STREQUAL "GNU"` and so
silenced nothing under clang. It now covers `miniz.c` and `tinyxml2.cpp` on any
non-MSVC compiler.

And the test targets were the one place in the tree building without `-Werror`.
They sat at exactly one warning -- a discarded `[[nodiscard]]` return, the very
class the attribute exists to surface. Fixed, and they are held at zero now.

## Round-trip testing, and the locale bug it found

The September sweep's defects all lived in **integration paths**, not in functions:
`C_OscChecksummedXml` could not reload the file it had just written, and a
parameter set's own version did not survive the trip. No unit test over a single
function can see that class. So the systematic version of the technique is a
save/load round trip per filer, with `CalcHash` as a deep-equality oracle where
the class provides one.

`test_filer_roundtrip.cpp` starts that: `C_OscSystemBusFiler` (parser level, with
and without CAN-FD), `C_OscProjectFiler` and `C_OscDataLoggerJobFiler` (both
through a real file, because that is the path the CRC bug lived in). **All pass** --
those three filers are clean.

### Float attributes followed the process locale

Chasing the same thread into *how* values are serialised found a real one.

`SetAttributeFloat32/64` and `GetAttributeFloat32/64` went through tinyxml2, which
formats and parses floating point with `snprintf` and `sscanf`. **Both honour
`LC_NUMERIC`.** openSYDE never calls `setlocale` itself, so it inherits whatever Qt
set from the environment. Under a German or French locale the writer emits:

```xml
<element factor="1,5"/>
```

Still well-formed XML. Read back anywhere else, `sscanf` stops at the comma and the
value becomes **1**. Exposed: every `f64_Factor` and `f64_Offset` on every Datapool
element, and every float Datapool value -- min, max, default and actual.

A same-locale round trip works, so the user who creates the file never sees a
problem. It only appears when the project is shared, and it appears as data that is
simply wrong rather than as an error.

The tell that this was an oversight rather than a decision: `C_SclStringUtil`
already defends its *own* parsing against precisely this, with `strtod_l` against an
explicit `"C"` locale and a comment explaining why. The XML parser was left out.

Fixed in core so the CLI tools benefit too: writing goes through
`std::format("{}", …)`, which is always the C locale and gives the shortest exactly
round-tripping representation; reading goes through `ToDoubleCompat`, which is
locale-independent **and** accepts one `,`, so projects already written with a comma
now read back with their real values instead of being truncated.

### The controls matter, again

Three of the four new tests fail without the fix. The fourth --
`RoundTripsAwkwardValues`, covering `0.0`, `-0.0`, `1e-300`, `1e300` and friends --
**passes in both states**. It is there because changing the write path from
tinyxml2's `%.17g` to `std::format("{}")` could have quietly cost precision, and a
fix that corrupts ordinary values while fixing exotic ones would otherwise look
like a success.

Note that `HexStringParsing.ToDoubleCompatIsLocaleIndependent` **skips** when the
host has no comma locale generated -- as it did on the Release run here. A test
that silently skips is not protecting anything on that machine; the new tests skip
the same way for the same reason, and say so in the skip message.

## GUI tree audit (2026-09-17)

The core sweep's four dimensions -- modernisation, performance, dedup, smells --
applied to the trees it explicitly left out: `opensyde_tool/src` (377K lines),
`libraries/opensyde_gui` (31K), CAN Monitor (41K) and SYDEflash (10K). **Three
times the size of core**, so the method was scan-generated candidates with every
count verified before it became a finding.

That verification step is the headline. **Every large scanner count overstated,
some of them by orders of magnitude:**

| Scanner said | After verification | What it actually was |
|---|---|---|
| 188 `delete this` | **0** | all `delete this->mpc_Ui;` -- the regex matched the prefix |
| 440 `new` without a Qt parent | **26** owning pointers | 115 are parented to `this`; the rest are handed to a layout, which takes ownership |
| 4,931 `== true` / `== false` | **0** defects | the STW house style, applied consistently |
| 11 blocking sleeps in GUI code | **0** defects | worker-thread yields and one deliberate 50 ms bus-settle after a modal dialog |
| 25 mutable statics | **1** bug | the rest are singleton instances and per-import state that is cleared on entry |
| 4 swallowing `catch(...)` | **0** defects | two wrap `std::map::at` on optional lookups, two are shutdown paths |

A scan is a question, not an answer. The cost of asking is seconds; the cost of
acting on an unverified count is a wasted PR at best.

### What the trees already got right

Worth stating, because it narrows where to look next. **Zero** string-based
`SIGNAL`/`SLOT` connects -- every connection is the compile-checked pointer form.
**Zero** C-style numeric casts. **Zero** `foreach`. The GUI is more modern on those
axes than a codebase this age usually is.

### Tier 1 -- a real bug

**`C_CamMosDatabaseItemWidget::m_OnUpdate` kept a per-widget counter in a `static`
local.** `hu8_FileOpenFailCounter` counts consecutive failed opens of *a* database
to suppress a repeated "could not open" dialog. There is one widget per database,
and one counter for all of them -- so database A's failures suppressed database
B's dialog, and B's success reset A's count. Now a member. Identical to core sweep
finding #2, "per-instance data clobbered by a static local".

### Tier 2 -- duplication, measured

Byte-identical function bodies (whitespace-normalised, at least 8 lines) across
different files: **53 groups, 1,154 redundant lines.** The three largest:

- **The scroll-bar show/hide slot, 26 copies.** 17 vertical, 9 horizontal, 10
  lines each, connected to `rangeChanged` on scroll areas, table views, tree views
  and list widgets that share no base but `QAbstractScrollArea`. Now one helper,
  `C_OgeWiUtil::h_ShowHideScrollBar`, and each slot is a one-liner. −167 lines.
- **`mouseMoveEvent`, 6 copies** of 13 lines across table views.
- **User settings, duplicated per application.** `C_UsHandler::GetRecentFolders`
  (30 lines), `C_UsFiler::mh_LoadRecentProjects` (29), `GetMostRecentFolder` (25)
  are identical in `opensyde_tool` and CAN Monitor. The user-settings layer was
  copied per app rather than shared through `opensyde_gui`. Open.

Smaller groups: `Equals` ×4 across data-pool models, `Clear` ×3 across tree
models, `headerData` ×2 at 32 lines, `m_InitButtonIcons` ×2 at 28. All open; the
full list is in the scan output and each is mechanical.

### Tier 3 -- modernisation, mechanical and rule-backed

- **243 `Q_UNUSED` → 0.** `CLAUDE.md` forbids it outright. What the 243 turned out
  to be, once the compiler had adjudicated every one:
  - **188** silenced an unused parameter in a Qt override → parameter name dropped.
  - **10** silenced a parameter used only under `#ifdef` → `[[maybe_unused]]`,
    because dropping the name would break the Windows build Linux cannot see.
  - **44** silenced a variable that *is* used later in the function → pure noise,
    deleted. Almost certainly left behind after the code grew around them.
  - **12** silenced a widget constructed purely for its side effect (it installs
    itself into a parent-owned pop-up) → the idiomatic bare `new` statement.
- **151 `NULL`** → `nullptr`. Mechanical, code only -- strings and comments untouched.

  A false alarm from that pass is worth recording: several diagnostics came back as
  *warnings* rather than errors, which looked like `-Werror` was not applied to part
  of `opensyde_tool` and CAN Monitor. The ninja graph shows every one of those files
  compiled once, with `-Werror`, and the final build has zero diagnostics. The
  warning text was an artifact of that build round -- most plausibly ccache replaying
  stderr from a stale compile. The archived TODO's "warnings as errors, done
  everywhere" stands.
- **26 raw owning pointers with a manual `delete`** → `std::unique_ptr`. Seven are
  `QSvgRenderer`, three `C_NagToolTip`. Same class as core sweep #10. Not 440 --
  see the table above.
- **69 Qt integer typedefs** (`qint32`, `qreal`) mixed with `<cstdint>`. Cosmetic.
- **30 `#define` constants** → `constexpr`. Cosmetic.

### Tier 4 -- design-level, noted not actioned

- **31 `QApplication::processEvents()` calls**, concentrated in the update and
  device-configuration widgets, keeping the UI alive through long operations. The
  re-entrancy hazard is real -- a user can click during the loop -- but the fix is
  moving the work to a thread, which is a redesign per widget, not a sweep item.
- **805 `dynamic_cast` / `qobject_cast`**, hot in the scenes (52 in
  `C_SdTopologyScene` alone). A design property of the graphics-item model.
- **`C_CieImportDbc` holds its warnings and error text in `static` members.**
  Cleared on entry so not a bug, but not re-entrant, and instance state would cost
  nothing.
- **Two `catch(...)` around `std::map::at`** could narrow to `std::out_of_range`.

### What was not scanned

Performance beyond the surface: the 223 "QString built inside a loop" candidates
are almost certainly dominated by cases the compiler already handles, and telling
the real ones apart needs a profiler, not a regex. Phase 7.1 is the precedent --
the plan's diagnosis was wrong and the measurement was right. Deferred until
there is a workload to measure against.

## Tool CMake consistency (2026-09-17)

Eight tools, eight `CMakeLists.txt`, and a survey across twelve axes found drift on
nine of them. None of it was a build break -- everything built -- which is exactly why
it had accumulated. The three axes that were already consistent are worth naming
because they are the ones that *should* differ per tool: source lists, include
directories, and the `OPENSYDE_CORE_SKIP_*` selection, all in one uniform `set(X 0/1)`
style.

| Axis | Before | After |
|------|--------|-------|
| CMakeLists location | `opensyde_tool/pjt/openSYDE/`, everyone else `pjt/` | all `pjt/`; two special cases removed from root CMakeLists and `build.sh` |
| `lint_config.cmake` | 8 copies; **2 were an older version** missing the `NOTFOUND` guard and `REMOVE_DUPLICATES` the other 6 had | one `cmake/lint_config.cmake` |
| `-Werror` block | copied 8×, three spellings | `osy_tool_werror(<targets>)` in `cmake/osy_tool_common.cmake` |
| C++ standard | 6 pinned it, 2 did not | shared include pins it for all |
| `cmake_minimum_required` | root 3.25, **core and all 8 tools 3.24** | 3.25 everywhere; `CLAUDE.md`'s claim is now true |
| core build-dir name | `opensyde_core` ×5, `osy_core` ×3 | `opensyde_core` |
| path style | `${PROJECT_ROOT}/…` ×7, bare `../…` in `coder_c` (and one `.rc` in `flash_tool`) | `${PROJECT_ROOT}` everywhere |
| per-tool `toolchain_*.cmake` | 6 files across 5 tools, one of them `toolchain_ubuntu.cmake` in a single tool, **referenced by nothing** | gone |
| `project()` form | `project(X LANGUAGES CXX)`, `project("x")`, mixed | `project(X LANGUAGES CXX)`; names unchanged |
| version file | 7 tools had one, `tsp_convert` had **no version at all** | `tsp_convert` at 1.0.0, printed in its banner |
| duplicate `SKIP_*` line | `flash_tool` set `PROTOCOL_DRIVERS_MONITOR` twice | once |
| stale comments | "minimum 3.24" above a 3.25 line; "C++17 to match…" above `CMAKE_CXX_STANDARD 23` | fixed |

### Deliberately left alone

- **`project()` names.** `openSYDE`, `SYDEflash`, `console_system_updater_sydesup`,
  `osy_tsp_convert` -- four naming schemes. But the names are also the target names,
  and those are wired into `build.sh`'s deploy table, the CI workflow and the result
  paths. Renaming for tidiness would ripple through all of that for no functional gain.
- **`src/<toolname>/` in CAN Monitor and SYDEflash.** Six tools put sources directly in
  `src/`; these two keep a namespace directory from when they were split out of
  `opensyde_tool`. Their `version_config.hpp` sits at *their* source root, which is
  consistent within that convention. Flattening is hundreds of path edits for cosmetics.
- **`-Wno-deprecated-declarations`.** Five tools add it, three do not. That is per-tool
  need (Qt deprecations in the GUI tools), not scaffolding drift.

### Two near-misses

**`coder_c` did not define `PROJECT_ROOT`.** It was the one tool writing bare `../`
paths, so it never needed the variable. The first pass of the core-path unification
rewrote its `add_subdirectory` to the `${PROJECT_ROOT}` form *before* checking that,
which would have expanded to `/../libraries/opensyde_core` and failed configure.
Caught by grepping for the definition before building, not by the build. A mechanical
"make these lines look alike" pass has to check what each line's spelling *depended
on*.

**The macOS standalone GUI configure fails -- and did before.** With the shared
scaffolding in place, `cmake -S opensyde_can_monitor/pjt` on the Mac failed with
`Failed to find required Qt component "Svg"`. That looked like a regression until the
identical configure was run against an untouched `develop` worktree and failed
identically. It is the split `qtbase`/`qtsvg` Homebrew kegs; the unified `build.sh`
build is unaffected. Pre-existing, recorded on the roadmap, and a reminder that a
verification that fails needs a control before it counts as evidence of anything.

### Verification

Fresh configure (build directory wiped, because the moved CMakeLists and shared
includes change the configure graph and a stale directory could mask exactly the
errors being tested for) and eight-tool build on the Linux host: clean. Standalone
CLI configure on macOS: clean, core present, C++23 in flags. `tsp_convert` rebuilt and
run: prints its version.

## Filer round-trips, wave 2 (2026-09-17)

Continuing the technique that found the CRC and locale bugs: build an object with every
field away from its default, save, load into a fresh object, compare -- `CalcHash` as
the deep-equality oracle where the class offers one, individual fields alongside so a
failure names what was lost.

Covered, all **clean after the fixes below**: system bus, project, data logger job,
node datapool (DIAG and NVM), X-config manifest, X-app properties, X-certificates
manifest, device definition, CAN protocol (J1939 and CANopen), node squads, parameter
set (interpreted and raw), system view core, system definition **file** (which drives
the node, datapool and comm-protocol file filers through the real folder layout),
CANopen manager file (with its EDS copy), HALC definition, HALC configuration and the
HALC standalone export. Twenty-one round trips, nineteen filers. The `Sup*` package
filers stay out: they are zip containers around filers already covered.

### One defect: `C_OscDeviceDefinitionFiler` could not load what it wrote

The saver writes `file-version` as `"0x" + IntToHexCompat(...)`; the loader parsed it
with a bare `std::stoi`, got `0`, and rejected the file as an unsupported version.
**The same defect as the parameter-set filer fixed in #30**, in a second filer -- and
#30's claim that "every other filer writes its version with `std::to_string`" was
**incomplete**, because that search covered only `SetNodeContent` writers and this one
uses `CreateNodeChild`. A writer-versus-reader table across every filer (in the PR)
claimed this was the last hex writer paired with a base-10 reader. **It was not** --
the service update package filers write their minor versions as `"0x000102"` and
`"0x000101"` and read them with a bare `std::stoi`; see the package round-trip below.

**Latent, not live.** Nothing in the product calls `h_Save` -- device definitions are
authored by STW and shipped read-only, spelled in decimal, which is why the base-10
loader has always worked on them. Fixed anyway with the same `ScanBaseCompat` parse,
which accepts both spellings: a filer that cannot read its own output is a defect
waiting for its first caller.

### The oracle has to know the model -- four lessons from the false alarms

Every other failure in this wave was the test being wrong about what the filer
*should* preserve. Each cost a round of investigation before it was recognised, so
they are recorded here to be recognised faster next time:

1. **Persistence is per-type.** The datapool element filer writes `diag-event-call`
   only for `eDIAG` pools and `nvm-start-address` only for `eNVM`; the CAN message and
   signal filers write the CANopen-manager fields only for `eCAN_OPEN` and the J1939
   part only for `eJ1939`. `CalcHash` hashes all of those regardless of type. So a
   source object must set only what its type persists, and the honest test is **one
   round trip per type branch** -- which is also better coverage.
2. **Constructors pre-populate.** `C_OscNodeDataPool()` adds one default list and
   `C_OscNodeDataPoolList()` one default element. A test that pushes "two lists" onto a
   fresh pool has three. The extra list came back faithfully; the count assertion was
   what was wrong.
3. **Loaders reject what savers accept.** The device-definition loader requires at least
   one `sub-device` and at least one `interface`; the saver writes empty lists without
   complaint. A real definition always has both. Not a defect, but worth knowing that
   save does not validate.
4. **Loaders transform.** The device-definition loader runs `TglExpandFileName` on the
   image, toolbox-icon and company-logo paths, resolving them against the file's
   directory and yielding `""` for anything not on disk; and for a definition with at
   most one sub-device it **auto-fills** every interface implied by the bus counts as
   connected, honouring saved `connected` flags only with two or more. So a load
   followed by a save does not reproduce the original file. `c_CompanyLogoLink` is a
   URL by name and a file path by treatment.

Also: my field-listing grep silently dropped every numeric member for most of this
wave, because its type pattern had no digits in it and `uint32_t` has two. It was
noticed when a class "had no fields". Same lesson as every scanner in this sweep.

### The big one: no HALC definition with use-cases could be loaded

`C_OscHalcDefFiler::mh_SplitAvailabilityString` and `mh_ParseAvailabilityStringSubElements`
both read `string[u32_ItChar + 1U]` inside a loop that starts at 0 -- the 1-based
`C_SclString` subscript kept after the phase-3 migration to `std::string`. Every
availability string was parsed shifted by one character: `"0,1,2"` became `",1,2"`
and was rejected as "contains empty section"; `"all"` became `"ll"` plus the
terminator and was rejected as an unexpected character. `availability` is a required
attribute of every `channel-use-case`, so **every HALC definition with a use-case --
which is every real one -- failed to load on `develop`**, and with it every node
that references one. Nothing noticed because nothing on this branch had ever loaded
a HALC file: no test, no CI job, and the GUI only under a human. Found the moment the
round-trip saved a definition and read it back.

The same signature turned up in `C_OscImportEdsDcf::mh_GetIntegerValue` and
`mh_Get64IntegerValue`, the EDS/DCF importer's number parsers: `"254"` was read as
`54` (transmission type), `"1"` as an empty string (mapping count), `"100"` as `0`
(event timer). `"$NODEID+0x180"` survived only because the dropped character was the
one the parser strips anyway. So a CANopen EDS import produced wrong or empty PDOs.
Pinned by `CanOpenEds.ImportReadsNumericFieldsAtTheirRealPosition` through the
public `h_Import`. A scan of every tree for `[<counter> + 1]` string subscripts found
one more, in `C_OscProtocolSerialNumber`, which is correct (it pairs characters).

The same importer had a third defect the pin exposed once the first two were fixed:
`mh_Get64IntegerValue` converted with a bare `std::stoll`, base 10, so the hex
spelling EDS files use for limits and defaults (`LowLimit=0x02`, `HighLimit=0xF0`,
`DefaultValue=0x10`) parsed as `0` -- every imported signal got a 0..0 range and a 0
default whenever the file wrote them in hex. The `std::stoi`-on-hex residue from the
earlier waves, in the one parser the writer-versus-reader table did not cover because
it reads foreign files rather than our own. Now `ScanBaseCompat`, like the rest.

**The lesson is the one from the earlier residue sweeps, sharpened:** the three
known shapes were raw `[1]`, `<= length()` bounds and `std::stoi` on hex. The fourth
shape is `[i + 1]` under a 0-based loop, and it is invisible to every scanner that
looked for the first three. The tree-wide hunt for it is the next section.

### Two smaller filer defects, fixed

* **HALC configuration dropped a parameter struct's comment.** `mh_AddParameters`
  seeds `C_OscHalcConfigParameterStruct::c_Comment` from the definition and `CalcHash`
  covers it, but `mh_SaveIoParameterStruct` wrote only the elements of a struct (a
  single value carried its comment inside `single-value`). Every load returned the
  struct with an empty comment. The saver now writes it next to the elements and the
  loader reads it when present, so older files still load.
* **`C_OscDeviceDefinitionFiler` hex version** -- above.

### Recorded, not changed

* **`C_OscNodeComInterfaceSettings` hashes an IP that CAN interfaces never persist.**
  `C_IpAddress()` seeds a default address on every interface, the node filer writes
  it only for Ethernet, the loader zeros it for everything else, and `CalcHash`
  covers it on all types. A CAN interface therefore hashes differently before and
  after its first round trip. Harmless -- nothing reads a CAN interface's address and
  the GUI computes its change-detection hash from loaded state -- but the round-trip
  test has to zero it explicitly, and any future "did the model change" comparison
  across a save will trip on it. The tidy fix is for the loader to keep the
  constructor default rather than zero; left alone because it changes what a loaded
  Ethernet interface without an `ip-address` node looks like, and that needs a look
  at the GUI first.
* **`C_OscHalcDefDomain::c_Comment` is hashed but has no element in the definition
  format**, in either direction. Real device files never carry one; it is always
  empty in practice. The domain *config* comment is a different field and persists.
* **`C_OscHalcDef::CalcHash` covers the base fields only** -- it does not descend
  into domains (the configuration's does). An oracle that trusts it says nothing
  about the domains, which is how the def round-trip first "passed" while losing the
  domain comment. The test compares domains individually now.

### More oracle lessons, added to the four above

5. **"Base path" means the file being written.** `C_OscNodeFiler::h_SaveNodeFile`
   hands its own file path down as `orc_BasePath`; every side file (datapool, comm
   protocol, CANopen EDS copy) goes next to *that file*. Passing a directory puts the
   side files one level up.
6. **Loaders defer.** The CANopen device loader only remembers the EDS path; the
   dictionary is parsed on first `GetEdsFileContent()`, and `CalcHash` covers the
   parsed dictionary. The target has to be asked for its content before the hashes
   can be compared, or the comparison is between a parsed and an empty dictionary.
7. **Qualify the base hash.** `CalcHash` is virtual; hashing through a base reference
   still hashes the derived object. `obj.Base::CalcHash(h)` is the only way to
   compare just the base part.
8. **Some formats carry half the model on purpose.** The HALC standalone export holds
   the configuration half only (domain id, channel names, parameter ids, channel and
   domain configs), never the definition; its oracle is those pieces, not the
   inherited domain hash.
9. **Generated identifiers are length-checked at load.** A HALC display name plus its
   domain's singular name must fit 31 characters (18 for the domain name after the
   longest constant prefix). The saver does not check; the loader rejects the file.

### Not in the wave

* `C_OscSup*` filers (system update packages): zip containers around the system
  definition and view filers, which are covered.

## The fourth residue shape, hunted tree-wide (2026-09-17)

The HALC and EDS defects above were found by a round-trip, not by a scan, and the
scan that followed was one grep. This is the proper pass: a scanner validated on
planted positives first, then run over every tree we own (`opensyde_core`,
`opensyde_gui`, all eight tools; not `can-libraries`, `qcustomplot`, `tinyxml2`).

### What it looks for

Every shape is index arithmetic that meant one thing on the 1-based `C_SclString`
and means another on `std::string` or `std::vector`. The compat helpers
(`PosCompat`, `SubStringCompat`, `InsertCompat`, `DeleteCompat`, `LastPosCompat`,
`LastDelimiterCompat`) are still **1-based by contract** -- their comments and
bodies say so -- which is what makes mixing them with raw `std::string` calls a
shape of its own.

| Shape | Pattern | Verdict rule |
|---|---|---|
| one ahead | `X[i + 1]` | suspect when `i` starts at 0 and the bound is `< X.length()/size()` without a `- 1` |
| one behind | `X[i - 1]` | suspect when `i` starts at 0 (underflow at the first iteration) |
| pair extract | `std::string(1, X[i + 1])` | as above |
| first skipped | `for (i = 1; i < X.length()/size(); ...)` | review: legitimate when element 0 is handled before the loop |
| overrun | `for (i = 0; i <= X.length()/size(); ...)` | suspect |
| 1-based found test | `.find(...) > 0` or `>= 1` | suspect: `npos > 0` is true |
| 1-based at-start test | `.find(...) == 1` | review |
| from the start | `.substr(1, ...)` | review |
| cross-convention | `p = PosCompat(...)` then `X.substr(p)` / `X[p]` / `X.erase(p)` without `- 1` | suspect |
| cross-convention | `p = X.find(...)` then `SubStringCompat(X, p, ...)` without `+ 1` | suspect |
| compat with literals | `SubStringCompat(X, 1, ...)`, `PosCompat(...) == 5` | review only: 1 and 5 are *correct* under the 1-based contract |
| last character | `X[X.length()]` / `X[X.size()]` | suspect: the 1-based "last character" reads the terminator on `std::string` (found the day after the sweep, in `C_OscSuSequences`; the tree has no other) |
| second-to-last | `X[X.length() - 1]` / `X[X.size() - 1]` | review: on the 1-based class this was the *second-to-last* character; on `std::string` it is the last. Every hit needs its intent read -- 63 in the trees, 62 are "last element" of a container and correct, one (`C_OscCanProtocol::h_ListIsComTx`) meant second-to-last and was wrong |

Array-typed names (`au8_`, `apc_`, `Buffer`, `.data()`) are excluded; the scanner
walks back up to forty lines to find the `for` header that declares the counter so
it can read the start value and the bound.

### What the tree said

| Shape | Hits | Outcome |
|---|---|---|
| `X[i + 1]` | 22 | 1 suspect: `C_OscProtocolSerialNumber` -- correct, steps by two. 21 reviewed: all guarded (`(pos + 1) < len` in the trigger parser and `ScanBaseCompat`, `(i + 1) < argc` in `tsp_convert`, nibble pairs in `C_HexFile`, byte offsets in the protocol driver, `tinyxml2`) |
| `X[i - 1]` | 5 | all guarded by an explicit `> 0` |
| first skipped | 14 | all the "element 0 before the loop, then pairs" idiom |
| overrun | 2 | **1 defect**: `C_OscXcoCreate::mh_CheckParamsToCreatePackage` looped `<= c_Nodes.size()` and dereferenced -- the missing-node case the loop exists to detect read one past the end first, and with no nodes at all it read `c_Nodes[0]` of an empty vector (crashes in the pin). The other is `FindCaseInsensitiveCompat`'s `start + sub.size() <= str.size()`, which is right |
| found / at-start tests, `substr(1` | 0 | |
| cross-convention, both directions | 0 | |
| compat with literals | 56 | all consistent with the 1-based contract |

So after #37 the tree holds exactly one more instance of the class, now fixed and
pinned by `test_xco_create`. The three earlier grep-style scans in this document
would not have found it: it is a container, not a string, and the shape is a
bound, not a subscript. Put differently, the residue is not about strings at all;
it is about *every* loop the migration touched, and the only reliable oracle for
the ones with no test is the shape table above.

### Not done here

The scanner is not checked into the tree: it is thirteen regexes and a forty-line
lookback, and the table above is its specification. If the class turns up a fifth
shape, extend the table first, then the regexes.

## The package filers, round-tripped at last (2026-09-17)

`C_OscSupServiceUpdatePackageCreate` and `...Load` were the one filer family left out
of both waves ("zip containers around filers already covered"). Wrong call: the
container has logic of its own, and none of it worked. `test_sup_package` creates a
package from a two-node system with a device manifest, two applications and a
parameter set, then processes it back. Three defects stood between create and load:

| Where | Defect | Effect |
|---|---|---|
| `C_OscSuSequences::h_CreateTemporaryFolder` | `orc_TargetPath[orc_TargetPath.length()]` -- the 1-based "last character", which reads the terminator on `std::string` | the "path must end in a delimiter" check **always** failed, so **no service update package could be created on `develop`** (SYDEsup, the GUI's package export). Phase 3b, 2026-08-24 |
| `C_OscSupDefinitionFiler::h_LoadUpdatePackageDefFile` | bare `std::stoi` on a version written as `"0x000102"` | the filer's own check passed on a string compare but handed `0` to its caller, which refused the package as an unknown version |
| `C_OscSupNodeDefinitionFiler::mh_CheckFileVersion` | the same on `"0x000101"` | every node definition inside a package rejected. `C_OscSupSignatureFiler` had the same parse; it writes decimal today, fixed alongside |

So a package could neither be written nor, had one been written by older code, read.
The first is the **fifth shape** of the 1-based residue (now in the scanner table; a
grep for `[X.length()]` / `[X.size()]` subscripts finds no other in any tree), the
other two are the `stoi`-on-hex class from wave 1 -- the pair the writer-versus-reader
table missed because these writers spell a two-part version as one hex literal.

Two behaviours the round-trip had to learn, both by design: only the *active* nodes
are loaded back (inactive ones stay `UnloadedNodeWithNodeIndex<n>` without a device),
and copied files carry their update position as a prefix (`1_app_a.hex`).

The process-global `C_OscSystemDefinition::hc_Devices` only loads the package's own
device root if nothing loaded device definitions earlier in the process, which is
why this test is its own binary.

Every remaining single-argument `std::stoi`/`stoll` in core was listed after this.
One more was wrong: `C_SclIniFile::ReadInteger` (and the `ReadUint*` family on top of
it) replaced `C_SclString::ToInt()`, which accepted `0x`, with a base-10 `std::stoi`;
CiA 306 allows hex for every numeric EDS value, so `NrOfRxPDO=0x4` read as `0`.
Fixed with `ScanBaseCompat`, pinned in `test_scl_ini_file`. The rest read values
their own writers spell in decimal (covered by the round-trips) or values whose
format is decimal by definition (DBC raw values, dates, availability indices).

## The last untested families: exporters, magician, TSP (2026-09-17)

After the package filers, the survey of "file-format classes with no test at all"
left three: the C code exporters (`C_OscExport*`, what `syde_coder_c` ships), the
HALC magician (HALC configuration -> datapools) and the target-support-package
loader. All three have tests now, built on the node and HALC models the round-trips
already used (moved into `tests/osy_test_models.hpp` for sharing).

### One defect, and it is the worst kind: `h_ListIsComTx` never said yes

`C_OscCanProtocol::h_ListIsComTx` decides whether a COM datapool list is the Tx or
the Rx list of an interface by its name (`CAN1_TX` / `CAN1_RX`). The comment says
"check second to last letter"; the code did `c_Name[c_Name.length() - 1]`, which on
the 1-based string *was* the second-to-last letter and on `std::string` is the last
-- `'X'` for both. So **no list was ever a Tx list**: `h_GetComListIndex(..., Tx)`
found nothing and left its output untouched, `h_GetComListIndex(..., Rx)` returned
the Tx list. Every mapping of a CAN message to a datapool element goes through these
two -- the bus editor's signal assignment, the DBC sync, the CAN monitor's signal
values and the comm-stack code export, which is where it surfaced ("Datapool does
not exist for specified communication protocol"). Pinned directly in
`test_can_protocol` and indirectly by the export.

This is the **sixth shape**: `[length() - 1]` meaning second-to-last. It cannot be
scanned for -- the same expression is the correct 0-based "last element" everywhere
else (62 other hits, all fine) -- only read for intent, which the comment made easy
here. The scanner table carries it as a review item.

### What the tests pin

* **Code export** (`test_code_export`): a programmable application over the shared
  node model exports ten files (init, three datapools, one comm stack). No golden
  file; the oracle is invariants a migration defect would break (every reported file
  exists, no terminators, balanced braces, tool info present) plus content pins
  chosen from a reviewed export: element indices, scaling defines, min/max/data-set
  values, message and mux indices, signal geometry, CAN ids, the datapool reference.
* **HALC magician** (`test_halc_magician`): a two-level NVM configuration with two
  copies yields four `eHALC_NVM` datapools in the documented order, with the four
  lists per domain, the channel-number/use-case/parameter variables named by
  `C_OscHalcMagicianUtil`, non-safe variables as arrays of the two non-safe
  channels and safe ones as scalars, and -- the part that found a defect -- the
  configuration data set holding each channel's number, use-case value and
  parameter initial values.

  **The magician mislaid channel values.** `m_GetListIndex` computes where a
  variable sits in the generated list. For the domains *before* the one asked
  about it chooses domain values or channel values by "does this domain have
  channels"; for the domain itself it *probed* the domain values first and
  accepted any in-range hit. A domain with channels that also carries domain
  parameters (the format allows it; the model here has one) therefore got every
  channel variable's index computed against the domain parameters: the `flags`
  initial value landed in `mode`'s slot. The probe's misses also went through
  `tgl_assert`, three dialogs' worth on Windows. Now the same branch as for the
  passed domains, and `mh_GetSubElementIndex` reports `range` instead of skipping
  silently when an element index does not fit.

  Whether STW's own device definitions ever combine domain and channel parameters
  in one domain is not known here; if they do, generated HALC datapools on this
  branch had wrong configuration values in exactly that domain.
* **Target support package** (`test_tsp_filer`): version 3 yields every field;
  versions 1 and 2 are answered with `busy`, meaning "use the converter's loader".
  `C_OscTargetSupportPackageV2Filer.hpp` in core was a declaration with no
  definition and no includer (the V2 loader moved to `opensyde_tsp_convert`);
  removed.

### `tgl_assert` on Windows is a message box, and CI has nobody to press OK

The Windows `TglReportAssertion` showed a `MessageBoxA` and nothing else. The
magician's three (then-)spurious asserts made the Windows core-test step wait on a
dialog for as long as GitHub allows (six hours) before the run was cancelled -- no
log, no failure, no verdict. It now writes the report to stderr first, as the Linux
implementation always has, and shows the dialog only when `OSY_ASSERT_NO_DIALOG` is
unset; the Windows CI step sets it. Any assertion on Windows CI is now a visible
line in the log instead of a silent hang.

## Security: signatures, PEM material, key agreement (2026-09-17)

`security/` had one test (the AES file container). `test_security_signatures` now
covers what the secure update flow is built on, with keys and self-signed
certificates generated in the test through OpenSSL: RSA sign -> verify against the
certificate (and a flipped bit, and a stranger's certificate), the PEM loader
(private key as DER PKCS#8, certificate as DER, serial number, key-usage and
extended-key-usage flags, meta text, modulus/exponent), the PEM database's folder
scan and serial-number index (a certificate without its key is skipped, not fatal),
the secure-update PEM's usage rule (digitalSignature + emailProtection or
`overflow`), ECDSA P-256 sign -> verify with the DER round-trip the packages use,
ECDH key agreement between two parties and AES-CBC.

### `C_OscSecurityEcdhAes::CreateEcKeys` freed its key context twice

After a successful `EVP_PKEY_keygen` the context was freed "as not needed any
longer" -- and then freed again on the way out, on the success path and the
fall-through alike. A double free on **every successful key generation**: abort on
macOS, segfault on Linux, confirmed under AddressSanitizer. The only production
caller is `C_OscProtocolSecuritySubLayer`, the ECDH key exchange that opens an
encrypted session with an ECU, so every attempt at encrypted traffic took the PC
tool down at the handshake.

**Not upstream's.** #41's commit message guessed "arrived with the Release 37
port"; `git log -S` says otherwise. The port brought the file with *one* free,
exactly as upstream Release 38 still has it. Phase 3b (2026-08-24, the
`C_SclString` retirement) restructured the function's returns and added the two
extra frees -- the same commit as the `[i + 1]`, `[length()]` and
`[length() - 1]` residue. So the fork's one crash-on-every-call defect is a
side effect of the mass rewrite too, just not a string one.

Two facts the tests had to learn: the PEM loader stores a certificate serial as
the ASN.1 INTEGER's *content* bytes (tag, length and a leading zero pad stripped),
and the ECDH public key travels compressed (33 bytes), not as the 64-byte X||Y the
ECDSA side uses.

### On the way: a wedged build directory

Debugging the abort with `lldb --batch` on macOS left a `debugserver` behind and
the next `ninja` in that build directory went into uninterruptible sleep (`U`),
where no signal reaches it; every later `ninja` there followed. The fix was to
build in a fresh directory. If `ninja` produces no output at all and `ps` shows
`U`, that is what happened; do not spend time on the build itself.

## Four more never-tested surfaces, no defects (2026-09-17)

After the security pass: the X-config and X-certificates packages
(`test_x_packages`, create -> process, including loading the unpacked system
definition against the package's own device root), the routing calculation
(`test_routing_calculation`: a gateway hop between two CAN buses, and the four
ways it can say no -- inactive gateway, routing disabled, target function
disabled, unknown target) and the CAN message logger (`test_com_message_logger`:
a J1939 frame with a multiplexer decoded against a saved system definition, the
Motorola 16-bit value checked bit by bit, a non-selecting multiplexer value, an
unknown id). All four passed on the first complete run. Worth writing down
because it is the first pass since #37 that found nothing: the package and
protocol layers above the filers were built on code the earlier waves had
already exercised, and the residue class lives in string handling, which these
do little of.

One requirement to know: the routing calculation reads the target's flashloader
and diagnostic capabilities from `pc_DeviceDefinition` and asserts it is set.

### The device-facing layer, reached with a mock (2026-09-17)

`test_data_dealer` (#44) carries a `C_OscDiagProtocolBase` mock that answers from
a byte map (NVM) and a keyed store (datapool elements); it is the first test of
anything below the data dealer without hardware. Datapool element write/read in
both endiannesses, wrong-sized answers, negative response codes, the NVM list
read with its CRC (good, flipped bit, unknown list), per-element NVM access at the
element's own address and change notification all behave. No defect. The mock is
the template for reaching `C_OscComDriverProtocol` and the update sequences: those
sit on a CAN dispatcher rather than a diagnostic protocol, so the next mock is one
level lower.

## Byte-order sites: 25 read, 33 write, none transposed (2026-09-18)

The roadmap carried "endian reassembly hand-inlined at ~25 sites" as a risk: every site was
four casts, three shifts and an index, with the index order the only thing saying big from
little endian. The sweep that replaced them with `C_OscEndian` (`util/C_OscEndian.hpp`,
`test_endian`) read every one of them first. **No site was transposed.** The hand-written
code was consistent throughout: the openSYDE protocol, DoIP and Intel hex offsets are big
endian, CANopen SDO payloads, the STW "low/high" CAN protocols and the datapool hash
inputs are little endian, and each site matched its wire format.

So this was a maintainability change, not a bug fix, and the value is in what a future
site looks like: `h_GetU32Big(&data[4])` names the order and reads exactly four bytes,
and a wrong helper is one line to fix rather than 58 to re-audit. The scan that found the
sites is `grep -rn "<< 24" --include='*.cpp'` for reads and `">> 24"` for writes, minus the
MD5, CRC and tinyxml internals, which stay hand-written because they are the algorithm,
not a wire format. `C_CanMonProtocolBase`'s `mh_BytesTo*` helpers were kept as the
CAN-monitor-facing API and now delegate.

Two things the sweep did change while it was there: `C_OscNodeDataPoolContent` carried
its own private copies of all eight conversions (deleted, callers use the shared helper),
and `C_OscExportCanOpenConfig` built a 4-byte payload with four `push_back` calls.

## Phase 7.2, resolved sideways: slicing-by-8 for the IEEE CRC32 (2026-09-18)

The plan for 7.2 was an SSE4.2 `_mm_crc32` path for `C_SclChecksums::CalcCRC32`. That
instruction computes CRC-32C (Castagnoli, 0x1EDC6F41). `CalcCRC32` is CRC-32/ISO-HDLC
(0xEDB88320), and its value is persisted everywhere: every `CalcHash` in the system
definition, the service-update-package signatures, the hex-file checksum. Changing the
polynomial changes every stored hash, so the hardware path cannot serve the function
that has the 497 callers. An earlier pass had already added a separate `CalcCRC32C` with
the hardware path and a benchmark, then stopped at that decision; it has no caller (see
the parked decisions in the roadmap).

What could be done without changing a single output was **slicing-by-8**: fold eight
input bytes per step through eight tables derived from the byte table at compile time.
Measured in Release on the 48-core host, `bench_opensyde --benchmark_filter=CRC32_Software`,
two alternating runs of each binary:

| Buffer | Byte-at-a-time | Slicing-by-8 | Ratio |
|---|---|---|---|
| 16 B | (not in the old range) | 1.41 GiB/s | |
| 64 B | | 1.53 GiB/s | |
| 1 KiB | 378 MiB/s | 1.58 GiB/s | 4.2x |
| 1 MiB | 377 MiB/s | 1.57 GiB/s | 4.2x |

For scale, the unused CRC-32C hardware path does 4.0 GiB/s on the same machine. The
result is bit-identical by construction; `test_checksums` now compares against a bitwise
reference for every length and start alignment up to 64 bytes, over odd chunkings, and
from a non-default start value. Where it matters in practice: the hex-file checksum and
package signature over megabytes, and the system-definition hash, which is thousands of
small calls -- the 16-byte number says those get most of the gain too.

The trap to note for anyone reading the archived plan: "hardware CRC32" and "the CRC32
we use" are different polynomials, and the archive's 17-20x figure compares the two, not
a faster implementation of ours.

## A virtual ECU: the update sequences run without hardware (2026-09-18)

The roadmap said the layer that talks to devices -- `C_OscSuSequences`, the flash com
driver, the UDS driver, the transports -- needed hardware to test. It does not, on
Ethernet. `C_OscIpDispatcher` is an abstract class with `InitTcp` / `SendTcp` / `ReadTcp`
/ `SendUdp` / `ReadUdp`, and the DoIP framing the transport puts on it is eight header
bytes, two logical addresses and the UDS payload. A dispatcher double that frames a
reply from a small UDS server is enough to run the whole stack in-process, and the
server can keep everything it was asked and everything it was given.

`test_su_sequences_virtual_ecu.cpp` does that: one Ethernet bus, one node with a device
definition whose reset waits are 10 ms, a virtual ECU that answers twenty services
(session control, tester present, twelve read/write identifiers, security seed/key,
four routines, download/transfer/exit, reset). The suite activates the flashloader,
reads the device out, flashes a 700-byte image built with `C_HexFile::CreateHexFile`
and resets, and then checks the image arrived byte for byte at its address, in the
right number of `TransferData` pieces, with the signature address on the last exit.
Eleven tests, 1.7 s, most of it the sequence's own 500 ms reset-wait floor.

**What it found on the first run.**

1. `m_FlashNodeOpenSydeHex` builds `std::vector<std::unique_ptr<C_OscHexFile>>` and
   never allocates the elements, then calls `LoadFromFile` on the first. Every flash of
   a hex file through `UpdateSystem` -- SYDEsup's purpose, the GUI's update view --
   segfaulted before the first byte was sent. Introduced by the ownership sweep
   (054deae63a), which turned a raw-pointer vector into unique_ptrs and dropped the
   `new`. The sixth "feature never worked" defect on this branch; one line to fix.
2. `ActivateFlashloader` with the default `oq_FailOnFirstError` broke out of the
   node loop before the line that copies the node's timeout into its connect state, so
   the GUI got an error without the node being marked as timed out. The copy now
   happens before the `break` at both sites.
3. `OsyReadFlashBlockData` parsed the response's optional tagged fields by testing
   `c_ReceiveData[u32_Counter]` for the next tag -- including when the previous field
   was the last byte of the response. Every real device that omits a trailing field
   (the virtual one omits the timestamp and the additional information) made the
   client read one byte past the vector. Harmless on most heaps, invisible on the
   build host and on macOS, and an immediate `operator[]` assertion on Ubuntu 26.04,
   whose GCC 15 libstdc++ checks bounds by default: the CI core job aborted twice
   while the same binary passed everywhere else. The parser is now a loop that checks
   each field's presence and length before reading it and reports `Errc::rd_wr` for a
   field cut short. **The host now keeps an assertion-enabled core build
   (`build/corehard`, `-D_GLIBCXX_ASSERTIONS`) next to the normal one; run both.**

**What it did not find.** The UDS driver itself. `test_protocol_driver_osy.cpp` puts
`C_OscProtocolDriverOsy` over a scripted `C_OscProtocolDriverOsyTpBase` and checks the
encoding and decoding of the services the sequences and the data dealer use, plus
negative responses, response-pending, wrong echoes, wrong lengths, silence and
unsolicited events: 36 tests, all green on the first run. The 1-based residue and the
endian sweep had been through that file already; this is the confirmation.

**Method note.** The defect in (1) is invisible to every scan in this document: it is
a correct use of a modern idiom that happens to be missing a line, in a function no
test called. The only thing that finds it is calling the function. The archived plan's
"needs hardware" was the reason nobody had; the seam was there all along. Before
declaring a layer untestable, look for the abstract class it talks through. And (3)
is the reason the Ubuntu runner is worth more than it looks: its standard library
asserts on the reads the others let through, so a test that passes on the host and
fails there is a real out-of-bounds access, not a flaky runner.
