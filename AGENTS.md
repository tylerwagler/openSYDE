# Project Context for AI Agents

## Project Overview
This repository contains the openSYDE project, which appears to be a software development environment or framework. The project includes various components and tools for system development, with documentation and planning documents.

## Directory Structure
- `.claude/` - Claude AI configuration files
- `.git/` - Git version control repository
- `getting_started.txt` - Getting started documentation
- `gpl-3.0.txt` - GNU General Public License v3.0
- `ImageMagick_license.txt` - ImageMagick license information
- `libraries/` - Library files
- `license.txt` - Project license information
- `opensyde_cmd_line_flash_tool/` - Command line flash tool for openSYDE
- `openSYDE_Core_documentation.chm` - Core documentation in CHM format
- `opensyde_syde_coder_c/` - Coder component for openSYDE
- `opensyde_syde_sup/` - Support component for openSYDE
- `opensyde_syde_x_gen/` - X generator component for openSYDE
- `opensyde_tool/` - Main openSYDE tool
- `plans/` - Planning documents (contains .md files)
- `scripts/` - Various scripts

## Key Components
- **openSYDE Core**: Main software development framework
- **Command Line Flash Tool**: Tool for flashing firmware
- **Syde Coder C**: C programming component
- **Syde Support**: Support infrastructure
- **Syde X Gen**: X generation tools
- **Syde Tool**: Main tool for the system

## Documentation
The project includes both CHM documentation and markdown planning documents. The `plans/` directory contains various replacement plans:
- FlexLexer_Replacement_Plan.md
- OpenSSL_Replacement_Plan.md
- QCustomPlot_Replacement_Plan.md
- Qt_Native_Replacement_Plan.md

## License
The project is licensed under GPL v3.0, with additional license information in the `license.txt` and `gpl-3.0.txt` files.

## AI Agent Workspace Rules

**All AI agents working on this repository MUST follow these rules:**

1. **Use the `plans/` directory** for all persistent artifacts:
   - Implementation plans → `plans/implementation_plan.md`
   - Task checklists → `plans/task.md`
   - Walkthroughs → `plans/walkthrough.md`
   - Scripts and utilities → `plans/scripts/`
   - Research notes and documentation → `plans/`

2. **Never use temporary directories** for work products. All artifacts should be saved to `plans/` so they persist across sessions and are available to other agents.

3. **Naming conventions** for plan files:
   - Use descriptive names: `<Feature>_<Type>.md` (e.g., `DBC_Export_Implementation_Plan.md`)
   - Prefix active work items with dates if needed: `YYYY-MM-DD_<name>.md`

4. **Cleanup responsibility**: When work is complete, consolidate or archive old plans rather than leaving orphaned files.

---

## Development Context
This appears to be a complex software development project with multiple components, documentation, and planning for potential replacements of various libraries and tools (such as FlexLexer, OpenSSL, QCustomPlot, and Qt).