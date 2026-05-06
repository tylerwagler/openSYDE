# In-code TODO / FIXME / HACK / XXX inventory

Tracker for `TODO` / `FIXME` / `HACK` / `XXX` comments living inside the
GUI source. Snapshot taken from a fresh `grep -rEn 'TODO|FIXME|HACK|XXX'`
on `2026-05-06` against `libraries/opensyde_gui/`, `opensyde_tool/src/`,
`opensyde_can_monitor/src/`, `opensyde_syde_flash/src/`.

For cross-cutting follow-ups that aren't tied to a code-line marker,
see `docs/TODO.md`.

**Counts at snapshot:** 62 markers — 60 `TODO`, 1 `HACK`, 0 `FIXME`, 0 `XXX`.

---

## High-priority themes

These are clusters where multiple markers point at the same gap.

### Linux SocketCAN parity (8 sites)

The PEAK / Vector DLL configuration UI was wired for Windows-only PCAN
DLLs and never replaced for Linux's SocketCAN. PC-icon configuration in
the topology has the same gap.

- `libraries/opensyde_gui/src/system_views/system_setup/C_SyvSeDllConfigurationDialog.cpp:266` — DLL_Open
- `libraries/opensyde_gui/src/system_views/system_setup/C_SyvSeDllConfigurationDialog.cpp:273` — CAN_InteractiveSetup
- `libraries/opensyde_gui/src/system_views/system_setup/C_SyvSeDllConfigurationDialog.cpp:287` — DLL_Close
- `libraries/opensyde_gui/src/system_views/system_setup/C_SyvSeDllConfigurationDialog.cpp:290` — DLL configuration
- `libraries/opensyde_gui/src/system_views/system_setup/C_SyvSeDllConfigurationDialog.cpp:319` — DLL_Open
- `libraries/opensyde_gui/src/system_views/system_setup/C_SyvSeDllConfigurationDialog.cpp:354` — DLL_Close
- `libraries/opensyde_gui/src/system_views/system_setup/C_SyvSeDllConfigurationDialog.cpp:357` — DLL configuration
- `opensyde_tool/src/graphic_items/system_view_items/C_GiSvPc.cpp:482` — Linux SocketCAN configuration UI

### Data Logger placeholders (8 sites)

Three widgets in the data-logger tree carry paired `//lint -e{9175}` +
bare `//TODO` comments. Looks like recently-added scaffold code that
never got filled in.

- `C_SdNdeDalLogJobDataSelectionWidget.cpp:102,105,112,115`
- `C_SdNdeDalLogJobGeneralSettingsWidget.cpp:155,158`
- `C_SdNdeDalLogJobPropertiesWidget.cpp:167,170`

### Generic "TODO: Error handling" (7 sites)

Placeholder for cases where a return code is checked but no real
recovery / user message is wired in. Each will need its own judgment
on what the right response is.

- `libraries/opensyde_gui/src/system_views/communication/C_SyvComDriverDiag.cpp:1446,2579`
- `libraries/opensyde_gui/src/system_views/dashboards/items/param/C_SyvDaItPaWidgetNew.cpp:1321,1351,1363`
- `libraries/opensyde_gui/src/system_views/device_configuration/C_SyvDcWidget.cpp:1950`
- `libraries/opensyde_gui/src/system_views/system_update/update_package/C_SyvUpPacListWidget.cpp:581`

### Bare `//TODO ?` markers (7 sites)

Lone question-mark TODOs with no context. Probably author-uncertainty
that needs a code archaeologist to interpret. Each one is a few-minute
investigation; collectively a half-day cleanup.

- `libraries/opensyde_gui/src/table_base/tree_base/C_TblTreDataElementModel.cpp:2374,2382,2495,2503,2511`
- `libraries/opensyde_gui/src/system_views/dashboards/items/param/C_SyvDaItPaTreeView.cpp:884`
- `opensyde_can_monitor/src/can_monitor/C_CamMainWindow.cpp:234` (`// TODO`)

### "Watch if parent necessary" (3 sites)

`C_SyvDaDashboardScene` constructs Qt items without a parent and notes
the suspicion. Worth resolving with proper ownership semantics.

- `libraries/opensyde_gui/src/system_views/dashboards/C_SyvDaDashboardScene.cpp:351,619,1844`

### Custom cursor (3 sites in C_SdTopologyScene)

- `libraries/opensyde_gui/src/system_definition/C_SdTopologyScene.cpp:1209,1218,1304`

### "TODO BAY:" — author-initials TODOs (3 sites)

Short notes signed `BAY`. Worth pinging that person if still on the
team to disambiguate.

- `libraries/opensyde_gui/src/opensyde_gui_elements/C_OgeHorizontalListWidget.cpp:182` — drop-event painting won't stop
- `libraries/opensyde_gui/src/system_definition/node_edit/datapools/C_SdNdeDpSelectorListWidget.cpp:1268` — bare
- `libraries/opensyde_gui/src/system_views/system_update/C_SyvUpInformationWidget.cpp:447` — Handle PEM files

### Icon states for icon-only buttons (3 sites)

Deactivated because "we need more time to adapt all icon-only-buttons".

- `libraries/opensyde_gui/src/opensyde_gui_elements/push_button/C_OgePubIconOnly.cpp:108,146`
- `libraries/opensyde_gui/src/system_definition/node_edit/datapools/C_SdNdeDpListDataSetWidget.cpp:324`

---

## Medium-priority — single-site, specific

- `C_SdSearchTreeWidget.cpp:686` — `// TODO: Add handling CANopen Manager with s32_Flag = 5`
- `C_SdNdeNodeEditWidget.cpp:300` — `// TODO: Handle CANopen Manager`
- `C_SdTopologyScene.cpp:924` — `// remove from scene TODO: investigate delete bug`
- `C_SyvDaDashboardScene.cpp:576` — same delete-bug comment
- `C_SyvDaDashboardsWidget.cpp:797` — bitrate not necessary for init / bus load not used here
- `C_NagToolTipWithImage.cpp:235` — provide image size as parameter (currently hard-coded for Node)
- `C_PopUtil.cpp:108` — `// TODO trigger update of window title...`
- `C_NagMainWindow.cpp:1118` — reset SD submode on project load: remove from user settings save
- `C_FlaConNodeConfigPopup.hpp:37` — `//TODO PTH: flashloader reset timeout as param`
- `C_SdManUnoTopologyReconnectBaseCommand.hpp:45` — save initial position if undo path ever needed
- `C_SyvSeContextMenuManager.cpp:97,103` — deactivated because of selection-of-PC / nodes problem
- `C_SyvComMessageMonitor.cpp:895,909` — `// TODO for logging`
- `C_SyvDaItTaDelegate.cpp:187` — `//TODO draw hover border`
- `C_SebGraphicsView.cpp:567` — `// TODO Maybe we need the context menu in this context too`
- `C_CamMetTreeView.cpp:990` — `// TODO: Is there a more efficient way?`
- `C_SdTopologyScene.cpp:3795` — `TODO: evaluate if counter grows too big -> override cursor stack may cause performance problems` (in a doxygen comment, not a code comment)
- `C_SyvDaDashboardsWidget.cpp:609` — bare `// TODO`

---

## HACK (1 site)

- `opensyde_tool/src/graphic_items/style_setup/C_GiSyColorSelectWidget.cpp:1206` — `// HACK: Because mouse grabbing doesn't work across processes, we have to have a dummy,`. Has a stated reason; the workaround is intentional. Document on a real cross-process eyedropper rewrite if/when one happens.

---

## Workflow

When you address a marker:

1. Make the fix on a branch.
2. Remove the marker line in the same change.
3. Strike the entry through here in the same commit so this file stays
   in sync with reality:

```markdown
- ~~`path/file.cpp:123` — description~~ (commit `abc1234`)
```

When you find a new marker that should land here, add it under the
right cluster (or open a new cluster) and re-run the count line at the
top.
