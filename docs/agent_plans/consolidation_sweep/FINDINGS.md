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
`docs/agent_plans/gui_consolidation/phase3-brief.md` describing a
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
accepted by the 48-core build host, which runs clang 19.1.7. The declaration is
annotated, there is no unannotated override, and clearing the precompiled
headers changes nothing -- clang 19 simply does not emit `-Wunused-result`
there.

**So "all eight tools build on the host" does not mean CI will agree.** For
warning-driven work, the host is a fast first pass and CI is the authority.
