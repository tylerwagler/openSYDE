# QString Migration Status - SYDEflash Build

Last Updated: 2026-01-22 20:15

## Build Status: In Progress - Iterative Fixes

### Data Classes Migrated

| Class | File | Members Changed |
|-------|------|-----------------|
| [x] C_OscParamSetInterpretedFileInfoData | .hpp | 7 members |
| [x] C_OscParamSetDataPoolInfo | .hpp | c_Name |
| [x] C_OscParamSetRawNode | .hpp | c_Name |
| [x] C_OscDataLoggerJobProperties | .hpp/.cpp | c_Name, c_Comment, c_LogDestinationDirectory + CalcHash |
| [x] C_OscParamSetInterpretedNode | .hpp | c_Name |
| [x] C_OscParamSetInterpretedElement | .hpp | c_Name |
| [x] C_OscNodeApplication | .hpp/.cpp | 7 members + CalcHash + 2 helpers |

### Filer/Handler Files Fixed

| File | Changes |
|------|---------|
| [x] C_OscXappPropertiesFiler | 6 changes |
| [x] C_OscHalcMagicianGenerator | 8 changes |
| [x] C_OscParamSetFilerBase | 9 changes |
| [x] C_OscParamSetHandler | 8 changes |
| [x] C_OscNodeSquadFiler | 5 changes |
| [x] C_OscParamSetRawNodeFiler | 7 changes |
| [x] C_OscDataLoggerJobFiler | 23 changes |
| [x] C_OscNodeFiler | ~100 changes (header + impl) |
| [x] C_OscSystemDefinitionFiler | ~15 changes |
| [x] C_OscCanOpenManagerFiler | header + impl migrated |

### Total Changes: 65+ edits across 20+ files

---

## Known Remaining Work

- [x] `C_OscNodeFilerV2.cpp` - implementation migrated
- [x] `C_OscNodeDataPoolFilerV2.hpp` - migrated
- [x] `C_OscNodeDataPoolFilerV2.cpp` - migrated
- [x] `C_OscSystemBusFilerV2.hpp / .cpp` - migrated
- [x] `C_OscSystemDefinitionFilerV2.hpp / .cpp` - migrated
- [x] `C_OscCanOpenManagerFiler.cpp` - file path parameters migrated
- [x] `C_OscDeviceDefinitionFiler.hpp / .cpp` - migrated
- [x] `C_SclResourceStrings.hpp` - migrated
- [x] `C_OscUtils.hpp` - removed deprecated C_SclString methods
- [ ] Various other filer files with `ToQString()` removal needed
- [ ] Additional CalcHash updates as data classes are migrated

## Pattern Summary

Key conversion patterns used:
- `c_str() / Length()` → `toUtf8().constData() / length()`
- `C_SclString::IntToStr()` → `QString::number()`
- `C_SclString::IntToHex()` → `QString::number(..., 16).rightJustified()`
- `.ToQString()` removal on already-QString members
- `Tokenize("x", list)` → `split("x")`
