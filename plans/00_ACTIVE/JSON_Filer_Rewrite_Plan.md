# JSON Filer Rewrite — Comprehensive Plan

**Status:** Approved, not started
**Owner:** Tyler
**Created:** 2026-04-08
**Supersedes:** Every other plan in `plans/00_ACTIVE/` whose name contains "Filer", "Migration", "Serialization", or "FileLoadersV2". Those are agent residue from prior failed attempts. They are *not* the source of truth — this document is.

---

## 1. Goal

Replace the entire openSYDE filer (serialization) subsystem with a single, uniform, Qt-native, JSON-based implementation.

**No** backward compatibility with existing project files.
**No** XML support of any kind.
**No** multi-format dispatch (binary/JSON/XML).
**No** format-version branching (V1/V2/V3).
**No** third-party XML libraries.
**No** `Qt6::Xml` link.

One format. One mechanism. One pattern. Applied uniformly to every filer in the codebase.

## 2. Scope (concrete)

Total filer code being rewritten or deleted:

| Tree | Files | Lines |
|---|---|---|
| `opensyde_tool/libs/opensyde_core/**/*Filer*.{cpp,hpp}` | ~60 | ~17,600 |
| `opensyde_tool/src/**/*Filer*.{cpp,hpp}` | ~24 | ~19,300 |
| `opensyde_tool/libs/opensyde_core/xml_parser/` (entire dir) | 4 | ~1,500 |
| **Total touched** | **~88** | **~38,400** |

Expected post-rewrite line count: ~50–60% of current. Target ~20k lines. The reduction comes from eliminating cursor traversal boilerplate, error-string formatting, attribute-vs-element decisions, and the V1/V2/_New duplication.

Files explicitly in scope for **deletion**:
- The entire `opensyde_tool/libs/opensyde_core/xml_parser/` directory (`C_OscXmlParser.cpp/.hpp`, `C_OscXmlParserLog.cpp/.hpp`)
- The entire `opensyde_tool/libs/opensyde_core/project/system/FileLoadersV2/` directory
- Every `*FilerV1.cpp/.hpp`, `*FilerV2.cpp/.hpp`, `*Filer_New.cpp/.hpp` file
- `C_OscFilerUtil.hpp` (XML helpers, becomes obsolete)
- The `Qt6::Xml` link in `opensyde_tool/libs/opensyde_core/CMakeLists.txt`
- `Qt6Xml.dll` from windeployqt deployment

## 3. The Pattern

### 3.1 Filer class shape

Every filer class is reduced to **two static methods**:

```cpp
class C_OscNodeDataPoolFiler
{
public:
   static QJsonObject save(const C_OscNodeDataPool & orc_DataPool);
   static int32_t     load(const QJsonObject & orc_Json,
                           C_OscNodeDataPool & orc_DataPool);
};
```

That's it. No `LoadFromFile`, no `SaveToFile`, no `LoadFromString`, no `LoadFromMemory`, no `LoadBinary`, no `LoadJson` (the `Json` is in the type), no `LoadXml`. **One save, one load.**

`save()` returns a `QJsonObject` by value. `load()` takes a `const QJsonObject &` and a non-const data reference, returns `C_NO_ERR` or a `stwerrors` code.

### 3.2 Composition

Filers compose by calling each other directly:

```cpp
QJsonObject C_OscNodeDataPoolFiler::save(const C_OscNodeDataPool & orc_DataPool)
{
   QJsonObject c_Json;
   c_Json["name"] = orc_DataPool.c_Name;
   c_Json["nvm_size"] = static_cast<qint64>(orc_DataPool.u32_NvmSize);

   QJsonArray c_Lists;
   for (const auto & rc_List : orc_DataPool.c_Lists) {
      c_Lists.append(C_OscNodeDataPoolListFiler::save(rc_List));
   }
   c_Json["lists"] = c_Lists;
   return c_Json;
}
```

```cpp
int32_t C_OscNodeDataPoolFiler::load(const QJsonObject & orc_Json,
                                     C_OscNodeDataPool & orc_DataPool)
{
   if (!orc_Json.contains("name") || !orc_Json["name"].isString()) {
      return C_CONFIG;
   }
   orc_DataPool.c_Name = orc_Json["name"].toString();
   orc_DataPool.u32_NvmSize = static_cast<uint32_t>(orc_Json["nvm_size"].toInteger());

   const QJsonArray c_Lists = orc_Json["lists"].toArray();
   orc_DataPool.c_Lists.clear();
   orc_DataPool.c_Lists.reserve(c_Lists.size());
   for (const auto & rc_ListVal : c_Lists) {
      C_OscNodeDataPoolList c_List;
      const int32_t s32_Result = C_OscNodeDataPoolListFiler::load(rc_ListVal.toObject(), c_List);
      if (s32_Result != C_NO_ERR) {
         return s32_Result;
      }
      orc_DataPool.c_Lists.append(c_List);
   }
   return C_NO_ERR;
}
```

### 3.3 File I/O lives only at the root

**Only the top-level filers** (project, system definition, view, CAN Monitor project) have file-reading methods. Lower filers never touch `QFile`. Pattern at the root:

```cpp
int32_t C_OscProjectFiler::loadFile(const QString & orc_Path, C_OscProject & orc_Project)
{
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::ReadOnly)) {
      osc_write_log_error("Loading project", QString("Could not open \"%1\"").arg(orc_Path));
      return C_RD_WR;
   }
   QJsonParseError c_ParseErr;
   const QJsonDocument c_Doc = QJsonDocument::fromJson(c_File.readAll(), &c_ParseErr);
   c_File.close();
   if (c_ParseErr.error != QJsonParseError::NoError) {
      osc_write_log_error("Loading project",
                          QString("JSON parse error: %1").arg(c_ParseErr.errorString()));
      return C_CONFIG;
   }
   return load(c_Doc.object(), orc_Project);
}

int32_t C_OscProjectFiler::saveFile(const QString & orc_Path, const C_OscProject & orc_Project)
{
   const QJsonDocument c_Doc(save(orc_Project));
   QFile c_File(orc_Path);
   if (!c_File.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
      osc_write_log_error("Saving project", QString("Could not open \"%1\"").arg(orc_Path));
      return C_RD_WR;
   }
   c_File.write(c_Doc.toJson(QJsonDocument::Indented));
   c_File.close();
   return C_NO_ERR;
}
```

### 3.4 Conventions (locked in Phase A, then immutable)

| Concern | Convention |
|---|---|
| JSON key naming | `snake_case`, ASCII only |
| Field naming | Match the C++ field name minus the STW prefix (`u32_NvmSize` → `nvm_size`, `c_Name` → `name`, `q_DiagEventCall` → `diag_event_call`) |
| Enum serialization | **String, not integer.** Robust to enum reordering. Conversion via free helper functions per enum (not enum classes — STW doesn't use them). |
| `uint64_t` fields | Serialize as **string**, not number. JSON numbers are doubles internally, lose precision above 2^53. Cheap insurance. |
| `uint8`/`uint16`/`uint32`/`int*` | Serialize as JSON number via `qint64` cast |
| `float32`/`float64` | Serialize as JSON number via `double` cast. NaN/Inf are not legal JSON; reject on save with `C_RANGE` if encountered. |
| `QString` | Direct assignment (`c_Json["name"] = orc.c_Name`) — Qt handles UTF-8 conversion |
| `QByteArray` (binary blobs) | Base64 string via `QJsonValue::fromVariant(QString::fromLatin1(blob.toBase64()))` |
| Optional fields | `if (orc_Json.contains(key))` check before reading |
| Arrays | Always present (possibly empty), never optional |
| Top-level document | A `QJsonObject` with at minimum a `format_version` field (integer) and a `type` field (string identifying which root filer wrote it). No other version branching exists. |
| File extension | Append `.json` to existing extensions: `.syde` → `.syde.json`, `.syde_def` → `.syde_def.json`, `.syde_v` → `.syde_v.json`, `.syde_cam` → `.syde_cam.json`, etc. File dialogs and recent-files lists updated with the root filers in Phase C/D. |
| Indentation | `QJsonDocument::Indented` always. Keep diffs reviewable. |
| Field order | Insertion order is preserved by `QJsonObject` in Qt 5.14+ — write fields in a stable order so git diffs are minimal |

### 3.5 What the new utility header looks like

A small `C_OscJsonUtil.hpp` (~150 lines) provides only what cannot be expressed cleanly inline:

```cpp
namespace stw::opensyde_core::json_util
{
   // Type-safe getters that return C_CONFIG on missing/wrong type
   int32_t getString(const QJsonObject & json, const QString & key, QString & out);
   int32_t getU32(const QJsonObject & json, const QString & key, uint32_t & out);
   int32_t getU64(const QJsonObject & json, const QString & key, uint64_t & out);
   int32_t getBool(const QJsonObject & json, const QString & key, bool & out);
   int32_t getF64(const QJsonObject & json, const QString & key, float64_t & out);
   int32_t getObject(const QJsonObject & json, const QString & key, QJsonObject & out);
   int32_t getArray(const QJsonObject & json, const QString & key, QJsonArray & out);

   // u64 helpers (string-encoded)
   void setU64(QJsonObject & json, const QString & key, uint64_t value);

   // Optional variants
   void getStringOr(const QJsonObject & json, const QString & key,
                    const QString & defaultValue, QString & out);
   // ... etc
}
```

These are *helpers*, not a framework. Filers may use them or write the access inline; both are fine.

## 4. Phasing

This is sequenced bottom-up so each phase ends on a green build. **No phase merges to dev unless the affected components compile and the GUI launches.**

### Phase A — Foundation (1 day)

**Goal:** Establish the pattern. No production filer code touched yet.

Tasks:
1. Lock the conventions in section 3.4. If anything in that table is wrong, this is the time to challenge it.
2. Create `C_OscJsonUtil.hpp/.cpp` in `opensyde_tool/libs/opensyde_core/serialization/`.
3. Update CLAUDE.md with a short "Filer Pattern" section pointing at this plan.
4. Move all the prior `Filer*`/`Migration*`/`Serialization*`/`FileLoadersV2*` plan files from `00_ACTIVE/` to `99_ARCHIVE/` so future agents stop mining them for context.

Exit criteria: utility header compiles, plan documents archived, conventions agreed.

### Phase B — Leaf filers in `opensyde_core` (3–5 days)

**Goal:** Rewrite the filer types with the fewest dependencies. Each one ends on a green core library build.

Order, smallest dependency footprint first:

1. **`C_OscNodeDataPoolContent`** — primitive value type with type tag + payload. Prove the pattern handles polymorphic content cleanly.
2. **`C_OscNodeDataPoolListElement`** — uses content. Tests array handling.
3. **`C_OscNodeDataPoolDataSet`** — small.
4. **`C_OscNodeDataPoolList`** — uses element + dataset.
5. **`C_OscNodeDataPool`** — uses list. **This is where we rejoin the immediately broken file.**
6. `C_OscCanMessage`, `C_OscCanMessageContainer`, `C_OscCanProtocol`
7. `C_OscCanOpenManagerInfo` (and the CANopen filer)
8. `C_OscHalcDef*` (definition) and `C_OscHalcConfig*` (configuration). Both become JSON-only filers like everything else. The main tool no longer reads vendor XML HALC files directly — those are converted ahead of time by a separate tool (section 9).
9. `C_OscParamSet*` filers
10. `C_OscDataLoggerJob`, `C_OscXappProperties`
11. `C_OscDeviceDefinition` (drop the V1)
12. `C_OscNodeComm`, `C_OscNodeSquad`, `C_OscNodeProperties`
13. `C_OscSystemBus`
14. `C_OscNode` (composes most of the above)

For each filer:
- Write the new `.cpp/.hpp` from scratch. Do *not* port the old code; refer to it only to confirm the data shape.
- Delete the old filer files (including any `*V1`, `*V2`, `*_New` siblings).
- Update every caller to use the new `save()`/`load()` signature. Most callers are simple.
- Build `Core` component (`build.ps1 -Component Core -SkipDeploy`) before moving on.

**Strip serialization methods off data classes after the matching filer is wired up.** Several core data classes (`C_OscNodeDataPoolContent`, `C_OscHalcConfigChannel`, `C_OscCanOpenManagerInfo`, etc.) currently carry their own `ToJsonObject` / `FromJsonObject` / `ToQDomDocument` / `FromQDomDocument` / `ToQDataStream` / `FromQDataStream` methods, sometimes split out into `*_Serialization.cpp` files. The new pattern is "data class is data only; serialization lives in the filer." Once a data class's new filer is in place AND every caller has been migrated to the filer, the on-class serialization methods (and any sibling `*_Serialization.cpp`) get deleted in the same step. **Do not leave these methods around as zombies.** Track each one in the per-filer subtask in this phase.

Exit criteria: all of `opensyde_core` filers rewritten. Core library compiles. `xml_parser/` directory is no longer included by anything in the rewritten code (but is not yet deleted — Phase E does that).

### Phase C — Root filers in `opensyde_core` (1–2 days)

**Goal:** Add `loadFile()`/`saveFile()` to the top-level filers and verify a full project round-trip works in core.

Tasks:
1. **`C_OscSystemDefinitionFiler`** — composes nodes + buses. Add `loadFile`/`saveFile`.
2. **`C_OscViewFiler`** — composes views.
3. **`C_OscProjectFiler`** — top-level container. Add `loadFile`/`saveFile`.
4. **`C_OscSupDefinitionFiler`** / **`C_OscSupNodeDefinitionFiler`** / **`C_OscSupSignatureFiler`** — system update package family. Drop their multi-format scaffold (it was the same cargo cult that broke the data pool filer).
5. **`C_OscXce*ManifestFiler`** / **`C_OscXco*ManifestFiler`** / **`C_OscTargetSupportPackageFiler`** — same treatment, drop the V2.
6. **File extension rename.** Update file dialog filters, default save extensions, and recent-files lookup wherever the root filers are invoked. The new extensions are `.syde.json`, `.syde_def.json`, `.syde_v.json`, etc. Grep for the old extension strings to find every site.

Write a small CLI test harness (`tests/json_filer_roundtrip.cpp`) that loads a known JSON project file, saves it back, and diffs. This becomes the regression check for the rest of the migration. (Wired into QtTest later — out of scope for this rewrite.)

Exit criteria: Core builds. CLI roundtrip test passes on a hand-written sample project file. New file extensions used by all root save/load paths.

### Phase D — GUI filers in `opensyde_tool/src` (3–5 days)

**Goal:** Same treatment for the GUI-side filer subsystem (`C_PuiSd*`, `C_PuiSv*`, `C_PuiBs*`, `C_CamProHandlerFiler`).

Order:
1. `C_PuiBsElementsFiler` (bus structure base — leaf)
2. `C_PuiSdHandlerFiler` (system definition handler — drop `C_PuiSdHandlerFilerV2`)
3. `C_PuiSvDashboardFiler`
4. `C_PuiSvHandlerFiler` (drop `C_PuiSvHandlerFilerV1`)
5. `C_CamProHandlerFiler` (CAN Monitor projects)
6. `C_SyvUpPacConfigFiler` (drop the `_New` sibling)
7. `C_PuiTargetSupportPackageFiler`

`C_UsFiler` (user settings) is **out of scope for this rewrite** — it uses `QSettings`, not the filer pattern. Leave it alone.

Exit criteria: Full GUI build green (`build.ps1 -Component All`). openSYDE GUI launches. CAN Monitor launches. SYDEflash launches.

### Phase E — Cleanup (1 day)

Tasks:
1. Delete `opensyde_tool/libs/opensyde_core/xml_parser/` entirely.
2. Delete `opensyde_tool/libs/opensyde_core/project/system/FileLoadersV2/` entirely.
3. Delete `C_OscFilerUtil.hpp`.
4. Grep the entire tree for any remaining references and clean them up.
5. Remove `Qt6::Xml` from `CMakeLists.txt` files.
6. Remove `Qt6Xml.dll` from `windeployqt` invocations / verify it's no longer copied.
7. Update the misleading comment on `C_OscXmlParser.hpp` line 9 — well, the file is gone, so this resolves itself.
8. Update `CLAUDE.md` migration section: remove "QString migration" Phase 1 references that no longer apply, add a "Filer pattern" section pointing at this plan as the canonical spec.

Exit criteria: `grep -ri "QDom\|QXmlStream\|tinyxml\|XmlParser\|FilerV2\|FilerV1\|Filer_New" opensyde_tool/` returns zero hits in production code.

### Phase F — Validation (1 day)

Tasks:
1. Build all three components Release and Debug.
2. Run the GUI-level smoke tests: create a new project, add a node, define a data pool with several lists and elements (mix of all primitive types), save, close, reopen, verify identical state. Repeat for views, dashboards, CANopen managers, HALC configs.
3. Verify `Qt6Xml.dll` is no longer in any deploy folder.
4. Verify the deployed binary size dropped (sanity check).
5. Snapshot the new file format with a short example saved to `docs/file_format_example.json` so anyone reviewing project diffs in git understands what they're looking at.
6. Move this plan to `01_COMPLETED/` with a `## Outcome` section appended.

## 5. Risks and how we'll handle them

| Risk | Mitigation |
|---|---|
| Scope discovery during Phase B uncovers types we didn't know about | The bottom-up order means we'll hit these as compile errors rather than late surprises. Each one is a new filer class added to the same pattern — no surprise architecture. |
| Some data type can't be cleanly expressed in JSON | Unlikely for openSYDE's data model (it's all structured records and arrays). If we hit one, document the encoding choice in `C_OscJsonUtil.hpp` comments and move on. Don't invent a framework. |
| Round-trip produces non-identical bytes (key reordering, float formatting) | Use stable insertion order in `save()`. For floats, use `QString::number(value, 'g', 17)` precision when round-tripping is critical. Test with the roundtrip harness. |
| Field rename causes silent data loss in load() | Loaders should return `C_CONFIG` on missing required fields, not silently default. Section 3.4 specifies this. |
| An agent attempts to "improve" the pattern mid-migration by adding helpers, abstractions, base classes | **Hard rule:** the only abstraction in the new system is `C_OscJsonUtil`. No filer base classes. No CRTP. No visitors. No reflection helpers. If a future change wants one, it argues for it in a separate plan and gets approved separately. |
| User project file loss due to format break | Already accepted by user. No mitigation. The first run of the new build cannot open old projects. Document this in release notes. |
| Mid-migration interruption leaves codebase in a half-state | Each phase ends on a green build. Commit at every phase boundary. Worst case if abandoned mid-Phase-B: we have a partially-rewritten core lib that still compiles, and a clear inventory of what's left in this document. |

## 6a. HALC vendor XML → JSON converter (separate deliverable)

The main openSYDE tool reads only JSON. To handle the existing world of vendor-supplied XML HALC definitions, a **separate converter tool** is built alongside this rewrite — but tracked outside the main filer rewrite scope.

Properties of the converter:

- **Standalone executable.** Lives in `opensyde_tool_halc_converter/` or similar (decide name during Phase A). Not built into the main openSYDE binary.
- **Single command-line interface.** `osy_halc_convert input.syde_halc_def output.syde_halc_def.json`. Optionally bulk mode for a directory.
- **Reads XML using `QXmlStreamReader`** from `QtCore`. Can also keep using QDom internally if convenient — this is the *only* place in the codebase that's allowed to link `Qt6::Xml`, and only because it's an isolated tool that nothing else depends on. **Recommendation: still use `QXmlStreamReader`** to keep the tool lightweight and avoid `Qt6Xml.dll` deployment for it.
- **Writes JSON** using the same `C_OscHalcDefFiler::save()` from the main tool. The converter shares the data model headers from `opensyde_core` but only links the JSON-writing path.
- **Out of scope for Phases A–F.** Built as a follow-up after the main rewrite is green. May be planned in its own document.

The converter exists so device vendors can hand us XML and we can preprocess it once during device package preparation. Users of openSYDE never see XML.

## 7. What's explicitly out of scope

To prevent agents from drifting:

- **User settings (`C_UsFiler`).** Uses `QSettings`. Different mechanism. Leave alone.
- **DBC import/export.** Uses Vector DBC library, not the filer subsystem.
- **BLF import.** Already removed.
- **Code generation output.** Already produces source files, not project files.
- **HALC definition file format conversion.** HALC XML files come from external tools — we still need to *read* third-party HALC XML at import time, but that import becomes a one-off XML→internal-model parser, not a "filer." Live in `halc/import/` not `halc/configuration/`. Decide in Phase B step 8 whether to keep `Qt6::Xml` linked just for this, or rewrite the import to use `QXmlStreamReader` from `QtCore`. **Recommendation: rewrite to `QXmlStreamReader`** so `Qt6::Xml` truly goes away.
- **Project file migration tooling.** No XML→JSON converter is in scope. Users start fresh.

## 8. Decision log

Decisions made during the conversation that produced this plan:

1. **Format = JSON.** Chosen over XML (verbose, cursor-state heavy), CBOR (binary, no diff), QDataStream (binary, version-fragile).
2. **Library = `QJsonDocument`.** Lives in QtCore, no extra module.
3. **Mechanism = per-type filer class with two static methods.** Chosen over free functions (less consistent), methods on data classes (couples data to serialization), visitor pattern (over-abstracted for this codebase).
4. **No backward compatibility.** Existing project files become unreadable. User accepted this explicitly.
5. **No format-version branching.** A `format_version` field exists in the root document for future use, but no V1/V2/V3 code paths.
6. **`uint64` serialized as string.** Defensive against JSON-number-as-double precision loss.
7. **Enums serialized as string.** Robust to enum reordering.
8. **Stable insertion order on save.** For minimal git diffs.
9. **Plan supersedes the entire `00_ACTIVE/` graveyard.** Old plans get archived in Phase A so they don't keep misleading future agents.

## 9. Resolved decisions (locked 2026-04-08)

1. **Field naming convention:** `snake_case`. Confirmed.
2. **HALC definition import:** the main openSYDE tool reads JSON HALC definition files only. **Vendor XML HALC definitions are converted to JSON by a separate offline tool**, not by openSYDE itself. `C_OscHalcDefFiler` becomes JSON-only like every other filer. The XML→JSON converter is its own small utility (see section 9).
3. **CAN Monitor project files (`.syde_cam`):** same treatment, break compatibility. Confirmed.
4. **File extensions:** **rename to `.json`-suffixed extensions.** A JSON file should be called `.json`. Existing extensions (`.syde`, `.syde_def`, `.syde_v`, `.syde_cam`, etc.) get extended to `.syde.json`, `.syde_def.json`, etc. File dialogs, recent-files lists, and any external integrations that match on extension need to be updated as part of the root filers' phase.
5. **Roundtrip test harness:** standalone CLI executable for now, no test framework. Will be wired into QtTest later as part of a separate testing initiative — out of scope for this rewrite.

Phase A may begin.
