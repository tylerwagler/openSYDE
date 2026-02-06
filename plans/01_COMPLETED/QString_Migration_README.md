# QString Migration Documentation - Navigation Guide

This directory contains comprehensive documentation for the QString migration project (replacing legacy `C_SclString` with Qt's `QString`).

**Last Updated**: 2026-02-03
**Last Updated By**: Claude Code
**Coordinator**: tyler
**Questions/Blockers**: Report in task document or create GitHub issue

---

## 🔗 Quick Links (Primary Documents)

| Document | Purpose | For |
|----------|---------|-----|
| [QString_Migration_Phase3_Agent_Tasks.md](QString_Migration_Phase3_Agent_Tasks.md) | ⭐ **WORK HERE** - Phase 3 discrete tasks ready for agents | Agents |
| [QString_Migration_Master_Plan.md](QString_Migration_Master_Plan.md) | Strategic overview & progress log | Everyone |
| [Phase2_Agent_Tasks.md](../01_COMPLETED/QString_Migration_Phase2_Agent_Tasks.md) | Phase 2 tasks (COMPLETED - reference only) | Reference |
| [Phase2_Status.md](../01_COMPLETED/QString_Migration_Phase2_Comprehensive_Status.md) | Phase 2 detailed analysis (COMPLETED) | Reference |

---

## 📋 What's Changed Recently

### 2026-02-03 - Plans Folder Reorganization
- ✅ Reorganized plans folder into 00_ACTIVE, 01_COMPLETED, 02_FUTURE, 03_TOOLS
- ✅ Phase 2 documents moved to 01_COMPLETED (reference only)
- ✅ Phase 3 is now the PRIMARY ACTIVE WORK
- ✅ Created comprehensive plans/README.md for navigation
- **Updated Primary Work Document**: `QString_Migration_Phase3_Agent_Tasks.md` ⭐

### 2026-02-02 - Critical Indexing Bug Warning Added
- ⚠️ **CRITICAL**: Added comprehensive warnings about `indexOf()` vs `Pos()` indexing differences (0-based vs 1-based)
- Added Pattern 8 (SubString → mid) and Pattern 9 (Pos → indexOf) to agent tasks
- Real-world example from C_OscCanOpenObjectDictionary.cpp bug included
- **Action Required**: All agents must review indexing warning section before claiming tasks

### 2026-02-02 - Initial Documentation Created
- Created comprehensive status analysis (2,010 C_SclString occurrences in 179 files)
- Created 23 discrete agent tasks organized into 5 sprints
- Estimated 19-26 hours of remaining work

---

## Quick Start for Agents

### New Agent Starting Work:
1. **Read**: `QString_Migration_Phase3_Agent_Tasks.md` ⭐ (START HERE - CURRENT WORK)
2. **⚠️ CRITICAL**: Review the indexing warning section (SubString/mid and Pos/indexOf differences)
3. **Claim a task**: Find an unclaimed `[ ]` task matching your skill level
4. **Execute**: Follow the task's specific instructions and patterns
5. **Verify**: Run the build command specified in the task
6. **Commit**: Use `/commit <task-number> <description>` skill to commit changes with proper formatting

### Project Coordinator:
1. **Current Tasks**: `QString_Migration_Phase3_Agent_Tasks.md` (Phase 3 - ACTIVE)
2. **Planning**: `QString_Migration_Master_Plan.md`
3. **Phase 2 Reference**: `../01_COMPLETED/QString_Migration_Phase2_*.md` (completed work)
4. **Risk Tracking**: See Risk Assessment section below

---

## Document Index

### 📋 Task Execution (Agent Work)
- **`QString_Migration_Phase3_Agent_Tasks.md`** ⭐ PRIMARY WORK DOCUMENT (CURRENT)
  - 13 discrete tasks for Phase 3 (C_SclStringList migration)
  - Organized into 5 sprints
  - Specific file paths and patterns
  - Verification steps for each task
  - Estimated time: 14-18 hours total
  - Clear before/after code examples

### 📊 Phase 2 Reference (COMPLETED)
Located in `../01_COMPLETED/`:
- **`QString_Migration_Phase2_Agent_Tasks.md`**
  - 23 completed Phase 2 tasks (reference for patterns)
  - Archived for reference

- **`QString_Migration_Phase2_Comprehensive_Status.md`**
  - Detailed Phase 2 codebase analysis
  - File-by-file breakdown with issue counts
  - Risk assessment and success metrics

- **`QString_Migration_Phase2_Complete_Summary.md`**
  - Phase 2 completion summary
  - Final statistics and lessons learned

- **`QString_Migration_Status.md`**
  - Historical status tracking
  - Data classes migrated
  - Pattern summary

### 🗺️ Strategic Planning
- **`QString_Migration_Master_Plan.md`** (in 00_ACTIVE)
  - Overall phase structure (Phases 1-5)
  - Current blockers and action items
  - Migration patterns reference
  - Progress log (session-by-session)
  - File-by-file migration checklist

---

## Document Relationships

```
plans/
├── README.md (Navigation for all plans)
├── 00_ACTIVE/ (CURRENT WORK)
│   ├── QString_Migration_README.md (You are here)
│   ├── QString_Migration_Phase3_Agent_Tasks.md ⭐ WORK HERE
│   └── QString_Migration_Master_Plan.md (Strategic)
├── 01_COMPLETED/ (Phase 2 - Reference)
│   ├── QString_Migration_Phase2_Agent_Tasks.md
│   ├── QString_Migration_Phase2_Comprehensive_Status.md
│   └── QString_Migration_Phase2_Complete_Summary.md
├── 02_FUTURE/ (Pending projects)
└── 03_TOOLS/ (Automation scripts)
```

---

## Work Status Summary

### Phase Status
- ✅ **Phase 1**: TGL Layer Elimination - COMPLETE
- ✅ **Phase 4**: C_SclDynamicArray → QList - COMPLETE
- ✅ **Phase 5**: C_SclIniFile → QSettings - COMPLETE
- ✅ **Phase 2**: C_SclString → QString - **~85% COMPLETE** (core work done)
- 🔄 **Phase 3**: C_SclStringList → QStringList - **IN PROGRESS** (current work)

### Current Sprint Priorities (Phase 3)
1. **Sprint 1: High-Frequency C_SclStringList Files** (4 tasks, ~4-5 hours) - HIGHEST PRIORITY
2. **Sprint 2: Remaining C_SclStringList Files** (3 tasks, ~2-3 hours) - HIGH PRIORITY
3. **Sprint 3: Deferred Phase 2 Tasks** (2 tasks, ~3-4 hours) - MEDIUM PRIORITY (API coordination)
4. **Sprint 4: Strategic Cleanup Planning** (3 tasks, ~2-3 hours) - PLANNING
5. **Sprint 5: Final Verification** (3 tasks, ~2 hours) - VERIFICATION

**Total Remaining Work**: 14-18 hours across 13 Phase 3 tasks

---

## Key Statistics (Current State)

### Phase 3 Targets
| Metric | Count | Status |
|--------|-------|--------|
| C_SclStringList occurrences | 90 | 🔄 Target for Phase 3 |
| Files with C_SclStringList | 31 | 🔄 To be migrated |
| std::vector<C_SclString> files | 4 | 🔄 To be migrated |
| Deferred Phase 2 tasks | 2 | 🔄 API coordination needed |

### Phase 2 Status (For Reference)
| Metric | Value | Status |
|--------|-------|--------|
| Total C_SclString occurrences | ~2,010 → ~300 | ✅ ~85% migrated |
| Files affected | 179 → 131 | ✅ Core work complete |
| QStringList already in use | 137 files | ✅ Good adoption |

---

## How Tasks Are Structured

Each task in `QString_Migration_Phase3_Agent_Tasks.md` includes:

✅ **Task Number** - Unique identifier (e.g., Task 1.1, Task 2.1)
✅ **File Path** - Exact location of files to modify
✅ **Estimated Time** - Expected completion time
✅ **Status Checkbox** - `[ ]` unclaimed, `[IP-agent]` in progress, `[✓]` done
✅ **Dependencies** - What must be done first
✅ **Issues Breakdown** - Specific problems to fix (Issue A, B, C...)
✅ **Code Patterns** - Before/after examples for C_SclStringList → QStringList
✅ **Verification Steps** - Build commands to verify success
✅ **Deliverables** - Checklist of what "done" means

---

## ⚠️ Known Issues & Common Migration Pitfalls

### Critical Indexing Issues (TOP PRIORITY)
1. **SubString() → mid() Offset Error**
   - C_SclString uses 1-based indexing, QString uses 0-based
   - `SubString(1, 4)` → `mid(0, 4)` (subtract 1 from start position!)
   - **Real bug found**: Line 226 had `mid(1, 4)` instead of `mid(0, 4)`

2. **Pos() → indexOf() Comparison Error**
   - C_SclString Pos() returns 1-based position, QString indexOf() returns 0-based
   - `if (Pos("sub") == 5)` → `if (indexOf("sub") == 4)` (subtract 1!)
   - **Real bug found**: Line 221 had `indexOf() == 5` instead of `== 4`

3. **"Not Found" Sentinel Value**
   - C_SclString Pos() returns 0 when not found
   - QString indexOf() returns -1 when not found
   - `if (Pos("x") == 0)` → `if (indexOf("x") == -1)`

### Other Common Errors
4. **Back-Conversion Anti-Pattern**
   - Never do: `QString → toStdString() → c_str() → C_SclString`
   - Use: Direct QString operations or `.ToQString()` when needed

5. **Case Conversion Assignment**
   - QString's `toUpper()`/`toLower()` return NEW strings
   - Must assign: `c_String = c_String.toUpper()` (not just `c_String.toUpper()`)

6. **PrintFormatted() Replacement**
   - `c_Text.PrintFormatted("fmt", args)` → `c_Text = QString::asprintf("fmt", args)`
   - Note the `=` assignment!

---

## Build Verification Commands

### Test SYDEflash (most tasks)
```batch
cd opensyde_tool/bat
build_syde_flash_release.bat
```
**Expected**: Exit code 0 (success)
**Build Time**: ~15-25 minutes
**Output**: `opensyde_tool/result/SYDEflash_win.exe`
**Logs**: Timestamped `.log` file in `opensyde_tool/bat/`

### Test CAN Monitor
```batch
cd opensyde_tool/bat
build_can_monitor_release.bat
```
**Expected**: Exit code 0 (success)
**Build Time**: ~20-30 minutes
**Output**: `opensyde_tool/result/openSYDE_CAN_Monitor_win.exe`

### Test Main openSYDE (final verification)
```batch
cd opensyde_tool/bat
build_release.bat
```
**Expected**: Exit code 0 (success)
**Build Time**: ~30-45 minutes
**Output**: `opensyde_tool/result/openSYDE_win.exe`

### Clean Build (when needed)
```batch
cd opensyde_tool
cmd /c "rmdir /s /q result\build 2>nul"
```

**Note**: Builds are LONG - avoid running unnecessarily. Most tasks can be verified by checking compilation errors in build output.

---

## Common Migration Patterns

### Phase 3 Patterns (C_SclStringList → QStringList)
All patterns are documented in detail in `QString_Migration_Phase3_Agent_Tasks.md` under "COMMON MIGRATION PATTERNS"

Quick reference for Phase 3:
- `C_SclStringList` → `QStringList`
- `.Add("item")` → `.append("item")`
- `.Count()` → `.count()` or `.size()`
- `.Strings[i]` → `[i]` or `.at(i)`
- `.Delete(i)` → `.removeAt(i)`
- `.Clear()` → `.clear()`
- `std::vector<C_SclString>` → `std::vector<QString>`

### Phase 2 Patterns (Reference)
See `../01_COMPLETED/QString_Migration_Phase2_Agent_Tasks.md` for:
- `Length()` → `length()`
- `ToInt()` → `toInt()`
- `c_str()` → `toUtf8().constData()` or eliminate
- `PrintFormatted(...)` → `= QString::asprintf(...)`
- `UpperCase()` → `toUpper()` (returns new QString)
- `LowerCase()` → `toLower()` (returns new QString)

---

## Agent Workflow

1. **Claim Task**
   - Edit `QString_Migration_Phase3_Agent_Tasks.md`
   - Change `[ ]` to `[IN PROGRESS - YourName]`
   - Commit: `docs: Claim Phase 3 Task X.Y - Brief description`

2. **Execute Task**
   - Follow task instructions exactly
   - Use provided code patterns (C_SclStringList → QStringList)
   - Make incremental commits if helpful

3. **Verify**
   - Run build command specified in task
   - Fix any compilation errors
   - Ensure task deliverables met

4. **Complete Task**
   - Edit `QString_Migration_Phase3_Agent_Tasks.md`
   - Change `[IN PROGRESS - YourName]` to `[✓]`
   - Note commit hash if helpful
   - Commit: `feat: [Phase 3 Task X.Y] Migrate C_SclStringList in FileX`

5. **Report/Continue**
   - If blocked: Document blocker and notify coordinator
   - If successful: Pick next task or report completion

---

## Questions or Issues?

### If you encounter:
- **Build errors not mentioned in task**: Document in task as sub-issue, continue if possible
- **Missing dependencies**: Check if earlier tasks need completion first
- **Unclear patterns**: Refer to comprehensive status doc for more examples
- **New issues discovered**: Add to task notes or create new task entry

### Coordination:
- Update task status in real-time in the agent tasks document
- Use git commit messages with task numbers for traceability
- Major blockers should be escalated to project coordinator

---

## 🎯 Success Criteria (Phase 3 Complete)

**Track completion in [QString_Migration_Master_Plan.md](QString_Migration_Master_Plan.md)**

### Code Quality
- [ ] All 13 Phase 3 tasks in agent tasks document marked `[✓]`
- [ ] C_SclStringList occurrences reduced to near-zero (target: <10 from current 90)
- [ ] std::vector<C_SclString> eliminated (target: 0 from current 4 files)
- [ ] Deferred Phase 2 tasks completed (Tasks 3.1, 3.2)
- [ ] Phase 4 cleanup strategy documented

### Build Verification
- [ ] SYDEflash builds cleanly (exit code 0)
- [ ] CAN Monitor builds cleanly (exit code 0)
- [ ] Main openSYDE builds cleanly (exit code 0)
- [ ] No regressions in Phase 2 work

### Documentation
- [ ] Phase 3 completion summary created
- [ ] Progress log updated in Master Plan
- [ ] Phase 4 agent tasks document created
- [ ] Migration priority matrix complete

### Readiness
- [ ] Ready to begin Phase 4 (strategic cleanup)
- [ ] No critical bugs reported
- [ ] All agents released from Phase 3 work

**Current Status**: See [Work Status Summary](#work-status-summary) above

---

## ⚠️ Risk Assessment Summary

| Risk | Likelihood | Impact | Mitigation | Status |
|------|-----------|--------|------------|--------|
| **Indexing off-by-one bugs** | HIGH | HIGH | Added comprehensive warnings & patterns | MITIGATED |
| **Encoding issues (Latin-1 vs UTF-16)** | MEDIUM | HIGH | Test file I/O with international chars | MONITORING |
| **Binary incompatibility** | HIGH | MEDIUM | Full rebuild addresses this | ACCEPTED |
| **Cascading GUI changes** | LOW | LOW | SYDEflash/CANMonitor isolated | LOW RISK |
| **Runtime behavior changes** | MEDIUM | MEDIUM | Comprehensive testing of string ops | MONITORING |
| **Build time increase** | HIGH | LOW | QString operations slightly slower | ACCEPTED |

**See [Phase 2 Comprehensive Status](../01_COMPLETED/QString_Migration_Phase2_Comprehensive_Status.md) for detailed risk assessment from Phase 2.**

---

## 📞 Coordination & Support

### Questions or Blockers?
1. **Check Known Issues** section above first
2. **Review comprehensive status** for detailed context
3. **Document blockers** directly in the task (add sub-issue)
4. **Escalate critical issues** to project coordinator (tyler)
5. **Create GitHub issue** for bugs or unclear requirements

### Agent Workflow Support
- **Git commits**: Use `/commit <task-number> <description>` skill for automated proper formatting
- **Task claiming**: Edit task document directly, mark as `[IN PROGRESS - YourName]`
- **Build verification**: See build commands section above
- **Pattern reference**: All patterns in agent tasks document

### Document Updates
- This README should be updated when major changes occur
- Add entries to "What's Changed" section when updating
- Increment version number for significant revisions
- Update "Last Updated By" field

---

## 📚 Version History

| Version | Date | Updated By | Changes |
|---------|------|------------|---------|
| 2.0 | 2026-02-03 | Claude Code | Major update: Plans folder reorganized; Phase 3 now active work; Updated all paths and references; Phase 2 moved to 01_COMPLETED |
| 1.1 | 2026-02-02 | Claude Code | Added: Known Issues, Risk Assessment, Quick Links, Version History, Improved build commands with expected outputs |
| 1.0 | 2026-02-02 | Claude Code | Initial creation with document index, navigation guide, and work status summary |

---

**Document Version**: 2.0
**Created**: 2026-02-02
**Last Updated**: 2026-02-03
**Last Updated By**: Claude Code

**Primary Work Document**: `QString_Migration_Phase3_Agent_Tasks.md` ⭐

**NOTE**: Phase 2 documents moved to `../01_COMPLETED/` for reference. Phase 3 is now the active work.
