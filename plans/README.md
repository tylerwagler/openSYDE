# openSYDE Plans & Documentation

**Last Updated**: 2026-02-03
**Maintained By**: Project team & AI agents

---

## Quick Start

### 🚀 For Agents Starting New Work

**START HERE**: [`00_ACTIVE/QString_Migration_README.md`](00_ACTIVE/QString_Migration_README.md)

This is your navigation guide. It will direct you to the current work document with discrete, ready-to-execute tasks.

**Current Active Work**: QString Migration Phase 3
- **Work Document**: [`00_ACTIVE/QString_Migration_Phase3_Agent_Tasks.md`](00_ACTIVE/QString_Migration_Phase3_Agent_Tasks.md)
- **Strategic Plan**: [`00_ACTIVE/QString_Migration_Master_Plan.md`](00_ACTIVE/QString_Migration_Master_Plan.md)

### 📊 For Project Coordinators

Review the active work folder to see:
- Current phase status
- Task assignments and progress
- Sprint priorities
- Blockers and dependencies

---

## Directory Structure

```
plans/
├── README.md                    ← You are here
├── 00_ACTIVE/                   ← CURRENT WORK - Point agents here
│   ├── QString_Migration_README.md
│   ├── QString_Migration_Phase3_Agent_Tasks.md
│   └── QString_Migration_Master_Plan.md
├── 01_COMPLETED/                ← Historical reference
│   ├── QString_Migration_Phase2_*.md
│   ├── C_OscDataLoggerJobFiler_Fix_Summary.md
│   └── implementation_plan_c_osutils.md
├── 02_FUTURE/                   ← Pending projects
│   ├── FlexLexer_Replacement_Plan.md
│   ├── OpenSSL_Replacement_Plan.md
│   ├── QCustomPlot_Replacement_Plan.md
│   └── Qt_Native_Replacement_Plan*.md
└── 03_TOOLS/                    ← Migration scripts & reports
    ├── qstring_migration.py
    ├── analyze_sclstring_members.py
    └── *.txt (generated reports)
```

---

## 00_ACTIVE - Current Work

**Purpose**: Contains all currently active implementation work.

**Current Projects**:
1. **QString Migration - Phase 3**
   - Eliminate C_SclStringList (QStringList migration)
   - Complete deferred Phase 2 tasks
   - Strategic cleanup planning

**Agent Instructions**:
- Always check [`QString_Migration_README.md`](00_ACTIVE/QString_Migration_README.md) first
- Claim tasks in [`QString_Migration_Phase3_Agent_Tasks.md`](00_ACTIVE/QString_Migration_Phase3_Agent_Tasks.md)
- Update task status as you work ([ ] → [IN PROGRESS] → [✓])
- Follow verification steps before marking complete

**Coordination**:
- Reference [`QString_Migration_Master_Plan.md`](00_ACTIVE/QString_Migration_Master_Plan.md) for strategic context
- Report blockers immediately to project coordinator
- Use task numbers in commit messages: `feat: [Phase 3 Task X.Y] Description`

---

## 01_COMPLETED - Historical Reference

**Purpose**: Archived completed work for reference and lessons learned.

**Contents**:
- **QString Migration Phase 2**: Agent tasks, status docs, completion summaries
- **Bug Fixes**: C_OscDataLoggerJobFiler fix summary
- **C_OscUtils Work**: Implementation plans and test cases

**Use Cases**:
- Reference migration patterns from Phase 2
- Review lessons learned
- Check what issues were already addressed
- Understand decision rationale

**Note**: These documents are read-only. Do not modify unless updating with final metrics.

---

## 02_FUTURE - Planned Projects

**Purpose**: Strategic plans for future work, not yet started.

**Pending Projects**:

### 1. FlexLexer Replacement
**File**: [`FlexLexer_Replacement_Plan.md`](02_FUTURE/FlexLexer_Replacement_Plan.md)
**Goal**: Replace FlexLexer dependency with Qt-native solution
**Status**: NOT STARTED
**Priority**: TBD

### 2. OpenSSL Replacement
**File**: [`OpenSSL_Replacement_Plan.md`](02_FUTURE/OpenSSL_Replacement_Plan.md)
**Goal**: Consider Qt's crypto APIs vs OpenSSL
**Status**: NOT STARTED
**Priority**: TBD

### 3. QCustomPlot Replacement
**File**: [`QCustomPlot_Replacement_Plan.md`](02_FUTURE/QCustomPlot_Replacement_Plan.md)
**Goal**: Evaluate QCustomPlot alternatives
**Status**: NOT STARTED
**Priority**: TBD

### 4. Qt Native Replacement (Comprehensive)
**Files**:
- [`Qt_Native_Replacement_Plan.md`](02_FUTURE/Qt_Native_Replacement_Plan.md)
- [`Qt_Native_Replacement_Plan_Phase2_Comprehensive_Plan.md`](02_FUTURE/Qt_Native_Replacement_Plan_Phase2_Comprehensive_Plan.md)

**Goal**: Replace all legacy STW libraries with Qt equivalents
**Status**: NOT STARTED - SUPERSEDED by QString migration work
**Priority**: LOW (partially addressed by current QString work)

**Note**: These plans may need revision based on lessons from QString migration.

### 5. std::vector to QList Investigation
**File**: [`std_vector_to_QList_Investigation.md`](02_FUTURE/std_vector_to_QList_Investigation.md)
**Goal**: Investigate whether to migrate from std::vector to QList/QVector
**Status**: NOT STARTED - Investigation Required
**Priority**: HIGH - Blocks Phase 3 Sprint 1 tasks (Task 1.2, 3.1)
**Created**: 2026-02-03

**⚠️ Decision Needed**: Should std::vector<QString> conversions use QList instead?
This decision affects current Phase 3 work and should be resolved ASAP to avoid rework.

### 6. Code Reduction Strategy
**File**: [`Code_Reduction_Strategy.md`](02_FUTURE/Code_Reduction_Strategy.md)
**Goal**: Strategic plan to reduce codebase size by 35-45% (~75,000 lines)
**Status**: PLANNED - Analysis Complete
**Priority**: MEDIUM - Long-term architectural improvements
**Created**: 2026-02-03

**Overview**: Comprehensive analysis identifying major code reduction opportunities:
- Legacy KEFEX library removal (31,730 lines)
- CAN Monitor protocol consolidation (11,000 lines)
- XML Filer framework (26,789 lines)
- Custom GUI element consolidation (30,000 lines)
- Complete QString migration (3,140 lines)
- Table delegates, UI files, and more

**Implementation**: Phased roadmap over 6-12 months with quick wins, architectural improvements, and strategic decisions.

---

## 03_TOOLS - Automation Scripts

**Purpose**: Python scripts and generated reports for migration automation.

**Scripts**:

### qstring_migration.py
Automated QString migration tool
- Finds C_SclString usage patterns
- Generates migration suggestions
- Creates usage reports

### analyze_sclstring_members.py
Analyzes C_SclString member variables in classes
- Scans header files
- Identifies member variables to migrate
- Generates analysis reports

### migrate_sclstring_members.py
Automated member variable migration
- Updates header files
- Migrates C_SclString members to QString
- Creates backup files

### migrate_sclstring_headers.py
Header file migration automation
- Updates include statements
- Migrates type definitions
- Handles forward declarations

**Reports**:
- `qstring_migration_report.txt` - General migration opportunities
- `sclstring_member_analysis.txt` - Member variable analysis
- `sclstring_migration_report.txt` - Migration execution logs

**Usage**: These tools were used during Phase 2. Review for Phase 3+ automation needs.

---

## Project Status Summary

### QString Migration Progress

| Phase | Status | Completion |
|-------|--------|------------|
| Phase 1: TGL Layer Elimination | ✅ COMPLETE | 100% |
| Phase 2: C_SclString → QString | ✅ MOSTLY COMPLETE | ~85% |
| Phase 3: C_SclStringList → QStringList | 🔄 IN PROGRESS | 0% |
| Phase 4: Strategic Cleanup | ⏳ PLANNED | 0% |
| Phase 5: C_SclIniFile → QSettings | ✅ COMPLETE | 100% |

**Current Sprint**: Phase 3 Sprint 1 (High-frequency C_SclStringList files)

**Total Estimated Remaining Work**: 14-18 hours across 13 primary tasks

### Other Projects

All other projects (FlexLexer, OpenSSL, QCustomPlot replacements) are on hold pending QString migration completion.

---

## Agent Workflow

### 1. Starting Work
```bash
# Navigate to plans folder
cd C:\Users\tyler\Dev\repos\openSYDE\plans

# Read the active work README
cat 00_ACTIVE/QString_Migration_README.md

# Open task document
cat 00_ACTIVE/QString_Migration_Phase3_Agent_Tasks.md
```

### 2. Claiming a Task
- Edit `00_ACTIVE/QString_Migration_Phase3_Agent_Tasks.md`
- Change `[ ]` to `[IN PROGRESS - YourName]`
- Commit change: `docs: Claim Phase 3 Task X.Y - Brief description`

### 3. Executing Work
- Follow task-specific instructions
- Use provided code patterns
- Make incremental commits if helpful
- Run verification commands

### 4. Completing Task
- Update status to `[✓]`
- Commit: `feat: [Phase 3 Task X.Y] Migrate C_SclStringList in FileX`
- Include statistics in commit message when applicable

### 5. Reporting Issues
- Document blockers directly in task document
- Notify project coordinator for critical issues
- Update task with sub-issues discovered

---

## Common Questions

### Q: Where do I start?
**A**: Read [`00_ACTIVE/QString_Migration_README.md`](00_ACTIVE/QString_Migration_README.md) - it's your entry point.

### Q: Which task should I work on?
**A**: Check [`00_ACTIVE/QString_Migration_Phase3_Agent_Tasks.md`](00_ACTIVE/QString_Migration_Phase3_Agent_Tasks.md) for unclaimed tasks (marked `[ ]`). Start with Sprint 1 tasks for highest priority work.

### Q: How do I verify my changes?
**A**: Each task includes specific build commands. Typically:
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
```

### Q: What if I find a bug or blocker?
**A**: Document it in the task document and notify the project coordinator. Add sub-issues to the task if needed.

### Q: Can I work on future projects?
**A**: Not yet. Complete Phase 3 of QString migration first. Future projects may need revision based on current work.

### Q: Where are the migration patterns?
**A**: All common patterns are documented in the task documents. For Phase 3:
- C_SclStringList → QStringList patterns
- std::vector<C_SclString> → std::vector<QString> patterns
- Method name conversions (Add→append, Count→count, etc.)

---

## Build Commands Reference

### Clean Build
```bash
cd opensyde_tool
cmd /c "rmdir /s /q result\build 2>nul"
```

### SYDEflash (Most Common)
```bash
cd opensyde_tool/bat
build_syde_flash_release.bat
# Expected: Exit code 0
# Output: opensyde_tool/result/SYDEflash_win.exe
# Time: ~15-25 minutes
```

### CAN Monitor
```bash
cd opensyde_tool/bat
build_can_monitor_release.bat
# Expected: Exit code 0
# Output: opensyde_tool/result/openSYDE_CAN_Monitor_win.exe
# Time: ~20-30 minutes
```

### Main openSYDE (Final Verification)
```bash
cd opensyde_tool/bat
build_release.bat
# Expected: Exit code 0
# Output: opensyde_tool/result/openSYDE_win.exe
# Time: ~30-45 minutes
```

---

## Document Maintenance

### When to Update This README
- New active project added to 00_ACTIVE/
- Major phase completion (move docs to 01_COMPLETED/)
- New future project plans created
- New automation tools added to 03_TOOLS/
- Significant changes to agent workflow

### How to Update
1. Keep "Quick Start" section current with active work
2. Update project status table when phases complete
3. Add new sections as needed for new project types
4. Maintain directory structure documentation
5. Update "Last Updated" date at top

### Ownership
This README is maintained by both human coordinators and AI agents. All updates should be committed with clear descriptions.

---

## Related Documentation

- **Main Project README**: See repository root `README.md`
- **CLAUDE.md**: Project-level AI agent instructions in repository root
- **Build Documentation**: See `opensyde_tool/bat/` folder
- **Architecture Overview**: See `CLAUDE.md` in repository root

---

## Feedback & Issues

### For Coordinators
- Review task status regularly in `00_ACTIVE/` documents
- Update phase completion percentages
- Maintain project status summary section
- Archive completed work to `01_COMPLETED/`

### For Agents
- Report unclear instructions in task documents
- Suggest improvements to patterns and workflows
- Document discovered issues
- Update task status in real-time

### For Contributors
- Follow agent workflow above
- Use consistent commit message format
- Include task numbers for traceability
- Document lessons learned in completion summaries

---

**Document Version**: 1.0
**Created**: 2026-02-03
**Last Updated**: 2026-02-03
**Maintained By**: openSYDE Development Team

**PRIMARY ENTRY POINT**: [`00_ACTIVE/QString_Migration_README.md`](00_ACTIVE/QString_Migration_README.md) ⭐
