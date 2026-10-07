# Phase 2: Paginated message-layout grid for J1939 multi-packet (TP) messages

**Archived 2026-10-07. Done.** Phase 1 shipped as `942c632a9` (the multi-packet flag on
`C_OscCanMessage`), this phase as `72c91d655` (the paginated grid), and the DBC import/export
propagation that followed as `43e1e7672`. Kept for the design decision below: the grid stays 64
bits wide and pages through the payload, rather than growing with it.

## Context

Phase 1 (done, `942c632a9`) added `bool q_IsMultipacket` to `C_OscCanMessage`; `u16_Dlc`
carries the payload byte size (1785 for the L6T's `ECUIdentificationInformation`, 0–8 for
single-frame). The message-layout grid (`C_SdBueMlvGraphicsScene`) is hard-coded to 64 bits and
currently clamps to avoid crashing, so a TP message shows only its first 8 bytes. Phase 2 makes
the grid render a multi-packet message's full payload via **paginated 8-byte pages** (the user's
chosen UX), with signals shown correctly across pages.

## Design decision

`mu16_MaximumCountBits` becomes **per-page** (always ≤ 64, the grid width). A new absolute
offset `mu32_PageStartBit` locates the page in the payload; `mu32_TotalBits` (payload bits =
`u16_Dlc*8`) and `mu32_PageCount` describe the message. Pagination is a **no-op for
non-multipacket** messages (`q_IsMultipacket == false` → `mu32_PageStartBit = 0`,
`mu32_PageCount = 1`). This keeps every index into `mac_SetGridState[64]` /
`mc_VecEmptyItems` in `[0,64)` so no array sizes change.

## Changes

### 1. `C_SdBueMlvGraphicsScene.{hpp,cpp}` — page state + API
- Add members: `uint32_t mu32_TotalBits`, `uint32_t mu32_PageStartBit`, `uint32_t mu32_PageCount`,
  `uint16_t mu16_MessageDlc`. Init in ctor (`TotalBits 0`, `PageStartBit 0`, `PageCount 1`, `MessageDlc 8`).
- Add public `void SetPage(uint32_t)` / `uint32_t GetPageCount() const` / `uint32_t GetPageStartBit() const`
  / `bool GetIsMultipacket() const`; signal `void SigPageChanged(uint32_t)`; private
  `void m_RebuildGridForCurrentPage()` and `uint16_t m_GetPageLocalBitPos(uint32_t) const`.
- `SetMessage` (cpp:218-345): replace the 64-bit clamp (`min(u16_Dlc*8, 64)`, lines 257-262) with
  page-state computation: `mu16_MessageDlc = u16_Dlc`; `mu32_TotalBits` = ECES 48 / CANopen 64 /
  `u16_Dlc*8`; `mu32_PageCount = (TotalBits==0)?1:ceil(TotalBits/64)`; `mu32_PageStartBit = 0`;
  `mu16_MaximumCountBits = min(64, TotalBits - PageStartBit)`. Replace the inline activation loops
  (lines 267-306) with calls to `m_UpdateBorderItems()`/`m_UpdateEmptyItems()` (now page-aware).
- `SetPage(p)`: if `PageCount <= 1` return; `PageStartBit = min(p, PageCount-1)*64`;
  `m_RebuildGridForCurrentPage()`; emit `SigPageChanged`.
- `m_RebuildGridForCurrentPage()`: recompute `mu16_MaximumCountBits`; clear `mac_SetGridState`,
  reset empty items; call `m_UpdateBorderItems()`/`m_UpdateEmptyItems()`; for each signal
  `SetPageStartBit` then `Update(...)` then `m_AddSignalToGridMapping(...)`.

### 2. Absolute→page-local mapping in the grid functions
- `m_AddSignalToGridMapping` (cpp:1077-1103): switch to the set-based single pass
  (`GetDataBytesBitPositionsOfSignal`); for each absolute pos in `[PageStartBit, PageStartBit+MaximumCountBits)`,
  map to `local = pos - PageStartBit` and index `mac_SetGridState[local]` / `mc_VecEmptyItems[local]`.
- `m_UpdateSignalInGridMapping` (cpp:1106-1150): search the absolute-position set for
  `u16_Counter + PageStartBit`; guard `mu16_MaximumCountBits == 0` (uint16 wrap on `--u16_Counter`).
- `m_RemoveSignalFromGridMapping(Position)`, `m_CheckGridMappingPositionForError`: no change
  (already page-local).

### 3. `C_SdBueMlvSignalManager.{hpp,cpp}` — page-awareness + dynamic length
- Rename `mu16_MaximumCountBits` → `uint32_t mu32_MaximumCountBits` (the **total** payload bits,
  used by `MoveSignal`); add `uint32_t mu32_PageStartBit` + `void SetPageStartBit(uint32_t)`;
  make `ms16_MaximumLength` non-const.
- `LoadSignal`: set `ms16_MaximumLength = max(64, min(0x7FFF, mu32_MaximumCountBits))` so a
  14280-bit signal is resizable; non-multipacket stays 64.
- `MoveSignal` (cpp:344-383): bound `s32_NewStartBit < mu32_MaximumCountBits` (absolute — no page
  offset needed).
- `m_UpdateItemConfiguration` (cpp:633-838): filter positions to the current page
  (`abs - PageStartBit` in `[0,64)`), build the page-local set, then run the existing row-grouping
  on page-local positions. `q_AllRowsVisible` = whole signal on this page (else no resize handles).

### 4. Border/empty item labels (`C_SdBueMlvBorderItem`, `C_SdBueMlvEmptyItem`)
- Widen `SetIndex` from `uint8_t` to `uint32_t`.
- `m_UpdateBorderItems` (cpp:965-999): set vertical (byte) labels to `PageStartBit/8 + i`,
  active when `(pageStartByte + i) < MessageDlc || CANopen`; horizontal (bit) labels unchanged.
- `m_UpdateEmptyItems` (cpp:1002-1028): set `SetIndex(PageStartBit + s32_BitPosition)` (absolute
  bit number); `SetActive(s32_BitPosition < mu16_MaximumCountBits)`.

### 5. Scene interaction (`mouseMoveEvent`, add/paste)
- Convert the page-local `s32_ActGridIndex` to absolute once: `+ mu32_PageStartBit`; use it in the
  resize/`SetStartBit`/`SetLastBit` paths. Byte-alignment `% 8` is unchanged (page offset is a
  multiple of 8).
- `m_ActionAdd`/`m_ActionAddMultiplexed`/`m_ActionPaste`: emit `SigAddSignal(..., s32_Counter + PageStartBit)`
  so new signals get absolute start bits.

### 6. Widget navigation (`C_SdBueMlvWidget.{cpp,hpp,ui}`)
- `.ui`: add an HBox `pc_WidgetPageNavigation` (prev button, `pc_LabelPage`, next button) above the
  graphics view; `visible=false` initially.
- `.cpp`: connect prev/next → `m_OnPrevPage()`/`m_OnNextPage()`; scene `SigPageChanged` → `m_OnPageChanged`.
- `m_SelectMessage` (cpp:236-256): after `SetMessage`, `m_UpdatePageNavigation()` — show the bar if
  `GetPageCount() > 1`, set the label, disable prev on page 0 / next on the last page.

### 7. DLC spinbox (`C_SdBueMessagePropertiesWidget.cpp`)
- At load (where `setValue(u16_Dlc)` runs, ~cpp:422): if `q_IsMultipacket`, `SetMaximumCustom(0xFFFF)`
  else `SetMaximumCustom(8)`. Guard the J1939 `SetComProtocol` branch (cpp:2965-2977) that forces
  `setValue(8)` so it doesn't clobber a loaded multipacket payload. Keeps single-frame behavior identical.

## Edge cases
- DLC 0: `TotalBits 0`, `PageCount 1`, all empty items inactive; guard `m_UpdateSignalInGridMapping`
  against the `--u16_Counter` wrap.
- Non-multipacket: page count 1, `PageStartBit 0`, `MaximumCountBits = u16_Dlc*8` — byte-for-byte
  identical to today; nav bar hidden; `SetPage` no-ops.
- Signal spanning pages: each page renders its slice with no resize handles (`q_AllRowsVisible == false`).
- Navigation bounds: `SetPage` clamps to `[0, PageCount-1]`; buttons disabled at the ends.

## Scope guardrails
Do NOT touch the core model/filer/validation (Phase 1). Only the MLV scene, signal manager,
border/empty items, widget, and the DLC spinbox max are in scope. The clamp at cpp:257-262 is
replaced by page-state computation (multipacket reaches the full payload; non-multipacket unchanged).

## Verification (GUI — manual)
1. Add an L6T node; open `ECUIdentificationInformation` (J1939 TP, `u16_Dlc=1785`).
2. Confirm no crash; nav bar shows "Page 1 / N" (N ≈ 224).
3. Page next/prev; confirm byte/border labels advance by 8, empty-cell bit labels by 64, and the
   14280-bit signal renders its slice per page with no crash.
4. Confirm a normal single-frame message renders exactly as before (no nav bar).
5. Confirm the DLC spinbox shows 1785 for the multipacket message and survives save/reload.
6. Confirm move/resize on a single-page message still works.
7. `./build.sh -b Debug opensyde` builds clean; then `./build.sh -d opensyde` to deploy.
