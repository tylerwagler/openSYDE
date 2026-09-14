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
