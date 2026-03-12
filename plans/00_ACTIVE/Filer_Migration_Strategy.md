# Filer Migration Status & Strategy

**Date**: 2026-03-12  
**Status**: ✅ Core Data Classes Complete | ⚠️ Call Site Migration Requires Careful Planning

---

## 📊 Current State

### ✅ What's Complete
- **30+ Core Data Classes** have Qt-native serialization methods (ToQDataStream, ToJsonObject, ToQDomElement)
- **30+ `_New` Filer Classes** created with multi-format support (Binary, JSON, XML)
- **Build Success**: All new serialization code compiles correctly

### ⚠️ What's NOT Complete
The `_New` filers **only contain multi-format file I/O methods**, not the XML-specific parsing logic that legacy filers contain.

**Missing from `_New` filers:**
- XML parsing helpers (e.g., `h_LoadData`, `h_SaveData`)
- Utility methods (e.g., `h_GetFileName`, `h_GetTypeString`)
- Internal XML structure manipulation methods

---

## 🎯 Why Simple Replacement Doesn't Work

### Example: `C_OscNodeFiler`
**Legacy version contains:**
```cpp
// File I/O (can be replaced)
h_LoadNodeFile()
h_SaveNodeFile()

// XML parsing logic (NEEDED internally)
h_LoadNode()
h_SaveNode()
h_LoadCommInterfaceId()
h_SaveCommInterfaceId()
h_GetFileName()
// ... 50+ more XML-specific methods
```

**_New version contains:**
```cpp
// Only multi-format file I/O
h_LoadNodeFile()  // Auto-detects format
h_SaveNodeFile()  // Binary/JSON/XML
h_LoadBinary()
h_SaveBinary()
h_LoadJson()
h_SaveJson()
h_LoadXml()
h_SaveXml()
```

**Problem**: Other legacy filers (like `C_OscCanOpenManagerFiler`) call methods like `C_OscNodeFiler::h_LoadCommInterfaceId()` which don't exist in `C_OscNodeFiler_New`.

---

## ✅ Recommended Strategy: Hybrid Approach

### Option 1: Keep Legacy Filers for Internal Use (Recommended)

**Keep both versions:**
- **`_New` filers**: For **external** call sites that need multi-format file I/O
- **Legacy filers**: For **internal** XML parsing logic

**Benefits:**
- ✅ No breaking changes
- ✅ Gradual migration possible
- ✅ Legacy filers can be deprecated over time
- ✅ Minimal risk

**Implementation:**
1. Update **external** call sites to use `_New` filers
2. Keep legacy filers for internal XML parsing
3. Mark legacy methods as `[[deprecated]]` with migration notes
4. Over time, migrate XML logic to `_New` versions if needed

### Option 2: Merge Functionality (More Work)

**Move all XML logic from legacy to `_New`:**
- Copy all XML parsing methods from legacy to `_New`
- Remove legacy filers
- Update all call sites

**Benefits:**
- ✅ Single filer per component
- ✅ Cleaner codebase

**Drawbacks:**
- ❌ Significant work (50+ methods per filer)
- ❌ Risk of breaking changes
- ❌ XML parsing logic may not be needed for binary/JSON formats

---

## 📋 Recommended Next Steps

### Step 1: Identify Safe External Call Sites
Update only call sites that use **simple file I/O** methods:

**Safe to update:**
- `C_OscComMessageLogger` - Uses `h_LoadSystemDefinitionFile()`
- `C_OscSpaServicePackageCreateUtil` - Uses `h_SaveSystemDefinitionFile()`
- `C_OscSupServiceUpdatePackageLoad` - Uses `h_LoadSystemDefinitionFile()`
- `C_OscSupServiceUpdatePackageV1` - Uses `h_SaveSystemDefinitionFile()`
- `C_PuiSdHandlerData` - Uses `h_SaveSystemDefinitionFile()`
- `C_NagMainWidget` - Uses `C_OscProjectFiler`
- `C_PuiProject` - Uses `C_OscProjectFiler`
- `C_OscDeviceGroup` / `C_OscDeviceManager` - Uses `h_Load()` (simple file load)
- `C_OscXceCreate` / `C_OscXceLoad` - Uses manifest filers
- `C_OscXcoCreate` / `C_OscXcoLoad` - Uses manifest filers
- `C_SdNdeHalcConfigImportDialog` - Uses `h_LoadFileStandalone()`
- `C_SdNdeHalcWidget` - Uses `h_SaveFileStandalone()`

**Do NOT update (contain internal XML parsing):**
- `C_OscSystemDefinitionFiler` - Internal XML parsing
- `C_OscNodeFiler` - Internal XML parsing
- `C_OscHalcConfigFiler` - Internal XML parsing
- `C_OscHalcDefFiler` - Internal XML parsing
- `C_OscNodeCommFiler` - Internal XML parsing
- `C_OscCanOpenManagerFiler` - Internal XML parsing
- etc.

### Step 2: Add Deprecation Annotations
Mark legacy filer methods that have `_New` equivalents:

```cpp
[[deprecated("Use C_OscXXXFiler_New::h_LoadFile() or h_LoadBinary()/h_LoadJson()/h_LoadXml()")]]
static int32_t h_LoadFile(Type& orc_Data, const QString& orc_Path);
```

### Step 3: Create Migration Documentation
Document the migration path:
- When to use `_New` filers (external file I/O)
- When legacy filers are still needed (internal XML parsing)
- How to migrate XML parsing logic if needed

### Step 4: Optional - Gradual XML Migration
If needed, migrate XML parsing methods from legacy to `_New`:
- Start with simple methods
- Test thoroughly
- Update internal call sites

---

## 🎯 Decision Required

**Which approach do you prefer?**

1. **Hybrid Approach** (Recommended)
   - Keep both legacy and `_New` filers
   - Update only external call sites
   - Gradual migration over time

2. **Full Migration** (More Work)
   - Merge all XML logic into `_New` filers
   - Remove legacy filers
   - Update all call sites

3. **Keep as-Is**
   - No call site updates
   - Document `_New` filers for future use
   - Legacy filers remain the default

---

## 📈 Metrics

- **Total Filers**: ~30
- **Safe to Update**: ~8-10 filers (external call sites only)
- **Need XML Logic**: ~20+ filers (internal parsing)
- **Estimated Work (Option 1)**: 2-4 hours (update safe call sites + documentation)
- **Estimated Work (Option 2)**: 40-80 hours (merge XML logic + test)

---

## ✅ Recommendation

**Go with Option 1 (Hybrid Approach)** because:
- ✅ Low risk
- ✅ Quick wins (update external call sites)
- ✅ Maintains backward compatibility
- ✅ Allows gradual migration
- ✅ Minimal code changes

**Next Action**: Update the ~8-10 external call sites that use simple file I/O only.
