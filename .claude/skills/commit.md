# Commit - QString Migration Task Commit

Commits changes for a QString migration task with proper formatting and verification.

## Usage

```
/commit <task-number> <brief-description>
```

## Examples

```
/commit 1.2 Fix C_OscCanOpenEdsDeviceInfoBlock string operations
/commit 2.1 Migrate C_OscExportDataPool to QString
/commit 3.2 Convert PrintFormatted in CAN Monitor files
```

## Instructions

When this skill is invoked, follow these steps:

### 1. Verify Current State
- Run `git status` to see what files have been modified
- Confirm the changes are related to the specified task
- Check for any unintended changes (like .env files, credentials, build artifacts)

### 2. Stage Files
- Add the specific files that were modified for this task
- **IMPORTANT**: Use `git add <file1> <file2>...` with specific file names
- **DO NOT** use `git add .` or `git add -A` to avoid accidentally staging sensitive files
- If there are many files (>5), list them and ask user to confirm before staging

### 3. Create Commit Message

Use this exact format:
```
feat: [Task <task-number>] <description>

<optional-details-if-needed>

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

**Examples:**
```
feat: [Task 1.2] Fix C_OscCanOpenEdsDeviceInfoBlock string operations

- Migrated all string members to QString
- Fixed CalcHash() to use toUtf8().constData()
- Eliminated all c_str() calls

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

```
feat: [Task 2.1] Migrate C_OscExportDataPool to QString

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

### 4. Execute Commit

Use heredoc format for proper multi-line message formatting:

```bash
git commit -m "$(cat <<'EOF'
feat: [Task <task-number>] <description>

<optional-details>

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
EOF
)"
```

### 5. Verify Commit
- Run `git log -1 --pretty=format:"%h - %s%n%b"` to show the commit
- Run `git status` to confirm working tree is clean (or show remaining uncommitted files)
- Report the commit hash to the user

### 6. Update Task Document

After successful commit:
- Update `plans/QString_Migration_Phase2_Agent_Tasks.md`
- Change task status from `[IN PROGRESS - AgentName]` to `[✓]`
- Optionally add commit hash reference

## Important Notes

### Safety Guidelines
- ❌ **NEVER** commit sensitive files (.env, credentials, API keys, passwords)
- ❌ **NEVER** use `git add -A` or `git add .`
- ✅ **ALWAYS** stage files by name
- ✅ **ALWAYS** verify `git status` before committing
- ❌ **NEVER** use `--no-verify` flag (respect pre-commit hooks)

### If Pre-commit Hook Fails
1. **DO NOT** use `--amend` (this would modify the previous commit)
2. Fix the issue reported by the hook
3. Re-stage the fixed files
4. Create a **NEW** commit (not amend)

### Commit Message Types
- Use `feat:` for new functionality or migrations
- Use `fix:` for bug fixes
- Use `docs:` for documentation-only changes
- Use `refactor:` for code restructuring without behavior change

## Edge Cases

### Multiple Files Changed
If more than 10 files were modified:
```bash
# List files first
git diff --name-only

# Stage specific files related to the task
git add opensyde_tool/libs/opensyde_core/imports/C_OscCanOpen*.cpp
git add opensyde_tool/libs/opensyde_core/imports/C_OscCanOpen*.hpp
```

### Uncommitted Work in Other Files
If `git status` shows files unrelated to current task:
- Only stage files for the current task
- Leave other modified files uncommitted
- Inform user about remaining uncommitted changes

### Build Verification Before Commit
For critical tasks (Sprint 1), consider suggesting:
```bash
# Optional: Quick build check before commit
cd opensyde_tool/bat && build_syde_flash_release.bat
```
But note that builds take 15-45 minutes, so this is optional.

## Output Format

After committing, provide this summary:

```
✅ Committed: [Task X.Y] Description

📝 Commit: abc1234
📂 Files: 3 files changed
   - opensyde_tool/libs/opensyde_core/imports/C_OscFoo.cpp
   - opensyde_tool/libs/opensyde_core/imports/C_OscFoo.hpp
   - plans/QString_Migration_Phase2_Agent_Tasks.md (updated)

🎯 Next:
   - Task X.Y marked as [✓] in agent tasks document
   - Ready to claim next task or verify build
```

## Troubleshooting

**"nothing to commit, working tree clean"**
→ All changes already committed or no files staged

**"pre-commit hook failed"**
→ Fix the issue, re-stage, create NEW commit (don't amend)

**"pathspec 'file' did not match any files"**
→ File path is incorrect or file doesn't exist

**Accidentally staged wrong file**
→ Use `git reset HEAD <file>` to unstage
