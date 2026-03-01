# Qt Serialization Framework Migration Strategy

**Date**: 2026-03-01  
**Status**: Phase 1 - Active Development

---

## Migration Strategy Overview

We are following a **phased, incremental migration approach** to minimize risk and ensure system stability.

### Current Phase: Phase 1 - Add New Serialization
✅ **IN PROGRESS** - Adding new serialization methods while keeping legacy code

**Activities:**
- Add 6 serialization methods to data classes (ToQDataStream, FromQDataStream, ToJsonObject, FromJsonObject, ToQDomDocument, FromQDomElement)
- Create new Filer classes with `_New` suffix (e.g., `C_OscDataLoggerJobFiler_New`)
- Keep all legacy Filer classes intact
- Build and test both old and new systems in parallel

**Goal**: Complete all additions before any removal

---

## Complete Migration Phases

### Phase 1: Add New Serialization (Current)
**Status**: ✅ **IN PROGRESS** (7/31 classes complete)

**What We're Doing:**
- Adding serialization methods to existing data classes
- Creating new Filer classes with `_New` suffix
- **NOT removing any legacy code**
- Maintaining 100% backward compatibility

**Benefits:**
- Zero risk to existing functionality
- Can test new and old systems in parallel
- Easy rollback if issues discovered
- No disruption to ongoing development

---

### Phase 2: Validation & Testing
**Status**: ⏳ **PENDING** (After Phase 1 complete)

**Activities:**
- Run comprehensive tests comparing old vs new serialization
- Performance benchmarking (binary vs JSON vs XML vs legacy XML)
- File format compatibility testing
- Round-trip testing (save → load → compare)
- Integration testing with dependent systems

**Success Criteria:**
- All tests pass
- Performance improvements validated
- No data corruption or loss
- Backward compatibility confirmed

---

### Phase 3: Gradual Migration
**Status**: ⏳ **PENDING** (After Phase 2 validation)

**Activities:**
- Switch individual components to use new Filer classes
- Start with low-risk components
- Keep legacy Filer classes as fallback
- Monitor for issues in production/staging

**Migration Order (Recommended):**
1. Low-impact components (configuration files, non-critical data)
2. Medium-impact components (user preferences, view data)
3. High-impact components (system definitions, node configurations)

**Benefits:**
- Limited blast radius if issues occur
- Can fix problems before scaling up
- Gradual confidence building

---

### Phase 4: Deprecation
**Status**: ⏳ **PENDING** (After successful migration of most components)

**Activities:**
- Mark old Filer classes as `[[deprecated]]`
- Add compiler warnings for legacy Filer usage
- Update documentation to recommend new Filer classes
- Create migration guide for developers

**Example:**
```cpp
/**
 * @deprecated Use C_OscDataLoggerJobFiler_New instead.
 * This class will be removed in version 2.0.
 */
[[deprecated("Use C_OscDataLoggerJobFiler_New instead")]]
class C_OscDataLoggerJobFiler {
    // ...
};
```

---

### Phase 5: Removal
**Status**: ⏳ **PENDING** (After deprecation period)

**Activities:**
- Remove deprecated Filer classes
- Clean up unused dependencies
- Update all internal references
- Final documentation updates
- Version bump (e.g., 1.x → 2.0)

**Prerequisites:**
- All components migrated to new Filer classes
- Deprecation period completed (recommended 3-6 months)
- No reported issues with new serialization
- Stakeholder approval

---

## File Organization Strategy

### Current Structure (Phase 1)
```
opensyde_tool/libs/opensyde_core/project/system/node/data_logger/
├── C_OscDataLoggerJob.hpp                 # Has old + new methods
├── C_OscDataLoggerJob.cpp                 # Has old + new methods
├── C_OscDataLoggerJobFiler.hpp            # Legacy (keep for now)
├── C_OscDataLoggerJobFiler.cpp            # Legacy (keep for now)
└── C_OscDataLoggerJobFiler_New.hpp        # New (recommended)
└── C_OscDataLoggerJobFiler_New.cpp        # New (recommended)
```

### Final Structure (Phase 5)
```
opensyde_tool/libs/opensyde_core/project/system/node/data_logger/
├── C_OscDataLoggerJob.hpp                 # Has serialization methods
├── C_OscDataLoggerJob.cpp                 # Has serialization methods
└── C_OscDataLoggerJobFiler.hpp            # Removed (or kept as wrapper)
└── C_OscDataLoggerJobFiler.cpp            # Removed (or kept as wrapper)
```

**Note**: We may keep legacy Filer classes as thin wrappers that delegate to new Filer classes for backward compatibility.

---

## Risk Mitigation

### Risk 1: Breaking Changes
**Mitigation:**
- Keep legacy code throughout Phases 1-3
- Extensive testing in Phase 2
- Gradual migration in Phase 3

### Risk 2: Performance Regression
**Mitigation:**
- Benchmark in Phase 2
- Optimize before Phase 3
- Keep legacy as fallback

### Risk 3: Data Corruption
**Mitigation:**
- Round-trip testing (save → load → compare)
- File format validation
- Backup before migration

### Risk 4: Developer Confusion
**Mitigation:**
- Clear documentation
- Deprecation warnings
- Migration guides
- Training sessions

---

## Timeline Estimate

| Phase | Duration | Dependencies |
|-------|----------|--------------|
| Phase 1: Add New Serialization | Current (2-3 weeks) | None |
| Phase 2: Validation & Testing | 2-3 weeks | Phase 1 complete |
| Phase 3: Gradual Migration | 4-6 weeks | Phase 2 validation |
| Phase 4: Deprecation | 1 week | Phase 3 complete |
| Phase 5: Removal | 1-2 weeks | Phase 4 complete + 3-6 month waiting period |

**Total Estimated Timeline**: 3-6 months (including deprecation waiting period)

---

## Current Status

**Phase 1 Progress**: 7/31 classes migrated (~23%)

**Completed:**
1. C_OscXcoManifest ✅
2. C_OscXceManifest ✅
3. C_OscXappProperties ✅
4. C_OscDataLoggerJob ✅
5. C_OscParamSetRawNode ✅
6. C_OscTargetSupportPackage ✅
7. C_OscViewData ✅

**Remaining**: 24+ classes

---

## Decision Points

### When to Start Phase 2?
- When all 31+ Filer classes have new serialization methods
- When team is confident in code quality

### When to Start Phase 3?
- When Phase 2 testing shows no critical issues
- When performance benchmarks meet targets

### When to Start Phase 4?
- When 90%+ of components use new Filer classes
- When no critical bugs reported for 30+ days

### When to Start Phase 5?
- After 3-6 month deprecation period
- After stakeholder approval
- When legacy code is no longer referenced

---

## Recommendations

1. **Continue Phase 1** until all classes have new serialization
2. **Document everything** - patterns, decisions, issues
3. **Test early and often** - don't wait until Phase 2
4. **Communicate clearly** - inform team about timeline and expectations
5. **Measure performance** - establish baselines before changes
6. **Keep rollback capability** - don't delete legacy code until ready

---

**Last Updated**: 2026-03-01  
**Next Review**: After completing Phase 1 (all 31+ classes)
