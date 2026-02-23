# AGENTS.md: OpenSYDE Development Guidelines for AI Agents

## Build/Lint/Test Commands

### Build Commands (Execute from /home/tyler/Projects/openSYDE/opensyde_tool/bat/)
- Build main GUI application: `.uild.ps1`
- Build CAN Monitor: `.uild.ps1 -Component CANMonitor`
- Build SYDEflash: `.uild.ps1 -Component SYDEflash`
- Build all components: `.uild.ps1 -Component All`
- Clean build (reconfigure CMake): `.uild.ps1 -Clean`
- Debug build: `.uild.ps1 -BuildType Debug`
- Build without Qt DLL deployment: `.uild.ps1 -SkipDeploy`

### Test Commands
- Run unit tests (if available): `.	est.ps1` (Note: Test scripts may not be fully implemented yet)
- For individual test execution, locate test files in `opensyde_tool/src/*/tests/` and run via CMake test runner
- To run specific test: `cmake --build . --target <test_target_name>` from build directory
- Test output appears in `opensyde_tool/temp_<Component>_Release/` directory

### Linting Commands
- C++ linting: Use Clang-Tidy with the following config:
  - Enable: readability-identifier-naming, modernize-deprecated-headers, performance-inefficient-string-concatenation
  - Disable: readability-magic-numbers (legacy code exception)
- Format code with clang-format (use .clang-format from root)
- Run clang-format: `clang-format -i --style=file <file_path>`
- Check style with: `clang-format -n --style=file <file_path>`

## Code Style Guidelines

### Naming Conventions
- **Classes**: Prefix with `C_` (e.g., `C_OscUtils`, `C_OscSystemDefinition`)
- **GUI classes**: `C_` + feature abbreviation + type (e.g., `C_CamOgeChxSettings` = CAN Monitor OpenSYDE GUI Element Checkbox)
- **Variables**: Use STW parameter prefixes:
  - `o` = object/reference, `q` = boolean
  - `u8/u16/u32/u64` = unsigned integers, `s8/s16/s32/s64` = signed integers
  - `f32/f64` = float/double
  - `c` = container, `pc/opc` = pointer to container
  - `rc/orc` = reference to container
  - Example: `const uint16_t ou16_Length`
- **Namespaces**: Use `stw::opensyde_core`, `stw::opensyde_gui_logic`
- **Functions**: PascalCase for public API, camelCase for internal functions

### Types and Typesafety
- Use types from `stwtypes.hpp`: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`, `float32_t`, `float64_t`, `char_t`
- Prefer Qt-native types: `QString`, `QList`, `QHash`, `QSet`, `QStringList`
- Avoid STL containers: Replace `std::vector<T>` with `QList<T>`, `std::map<QString, T>` with `QHash<QString, T>`, etc.
- Replace `C_SclString` with `QString`

### Error Handling
- Use error codes from `stwerrors.hpp` in `stw::errors` namespace:
  - `C_NO_ERR` (0), `C_UNKNOWN_ERR` (-1), `C_RANGE` (-5), `C_RD_WR` (-7), `C_CONFIG` (-10), `C_TIMEOUT` (-12)
- Functions should return error codes, not throw exceptions
- Check return codes immediately after function calls
- Use `Q_ASSERT` for debug-time assertions, but not for runtime validation

### File Structure
- Use the standard file header template:
```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Brief description
   \copyright   Copyright [YEAR] Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------
```
- Order of sections: Includes → Namespace → Types → Implementation
- Keep files under 2000 lines; split large files logically

### Imports
- Include headers in this order:
  1. Project headers ("...")
  2. Qt headers (<...>)
  3. Standard library headers (<...>)
- Use forward declarations when possible to reduce compilation dependencies
- Avoid `using namespace` in header files; use in cpp files only when necessary

### Qt Migration (Phase 1)
- Migrate all legacy code to Qt-native equivalents:
  - `C_SclString` → `QString`
  - `C_SclDynamicArray` → `QList`
  - `std::vector<T>` → `QList<T>`
  - `std::vector<QString>` → `QStringList`
  - `std::map<QString, T>` → `QHash<QString, T>`
  - `std::set<T>` → `QSet<T>`
  - `std::ifstream/ofstream` → `QFile + QTextStream`
  - `std::filesystem::path` → `QString + QFileInfo`
- Use Qt methods:
  - `c_str()` → `toUtf8().constData()`
  - `Length()` → `length()`
  - `C_SclString::IntToStr()` → `QString::number()`

### Formatting
- Use 4-space indentation (no tabs)
- Maximum line length: 100 characters
- Function declarations: Keep parameters on one line if < 100 chars, otherwise one per line
- Braces: Always use braces for control structures, even for single-line statements
- Space around operators: `a + b`, not `a+b`
- Pointer declarations: `int* ptr`, not `int *ptr`

### Documentation
- Maintain Doxygen-style comments for all public APIs
- Update documentation when modifying code
- File-level documentation must include copyright and brief description
- Function documentation should include parameters, return value, and exceptions (if any)

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

## Additional Notes

- All builds use CMake with Ninja generator and MinGW 13.1.0 compiler
- Qt 6.10.1 with MinGW 64-bit is required at `C:\Qt\6.10.1\mingw_64\`
- Build outputs are located in `opensyde_tool/result/`
- Each executable folder is self-contained after deployment (Qt DLLs copied)
- The migration from legacy C_SclString to Qt-native types (Phase 1) is complete.
- Current priority is migrating STL containers (std::vector, std::map, std::set) to Qt-native equivalents (QList, QHash, QSet) as outlined in plans/02_FUTURE/Qt_Native_Coding_Standards.md
- Always check `plans/02_FUTURE/Qt_Native_Coding_Standards.md` for complete Qt migration guidelines
- Do not create new files with legacy patterns - all new code must use Qt-native equivalents
- When in doubt about code style, follow existing code patterns in the same component
- Use `clang-format` to auto-format code before committing changes
- Always run build after making changes to verify compilation

## Git Commit Guidelines

- Use imperative mood in commit messages: "Fix", "Add", "Update", "Refactor", "Improve"
- Include component name in commit message: "[CANMonitor] Fix memory leak"
- Reference related issue or plan if applicable
- Do not commit changes to build artifacts or deployment files
- Keep commits small and focused on single changes
- If changes affect multiple components, create separate commits

## Important Warnings

- DO NOT use `std::string`, `std::vector`, `std::map`, or other STL containers in new code
- DO NOT use `C_SclString` or `C_SclDynamicArray` in new code
- DO NOT use `std::ifstream` or `std::ofstream` for file I/O
- DO NOT use `std::filesystem::path`
- DO NOT create new files with legacy patterns
- DO NOT commit files containing secrets (.env, credentials.json, etc.)
- DO NOT use force push without explicit user approval
- DO NOT use `git commit --amend` unless specifically instructed
- DO NOT modify Qt installation files or system libraries

## Tooling

- Use the provided build.ps1 script for all builds
- Use clang-format for code formatting
- Use CMake for configuration and build
- Use Ninja for compilation
- Use Qt's windeployqt for deployment
- Use the Qt Creator IDE for development if available

## Qt Native Coding Standards

Refer to `plans/02_FUTURE/Qt_Native_Coding_Standards.md` for complete guidelines on:
- String handling
- Container usage
- File I/O
- Memory management
- Thread safety
- Event handling
- Signal/slot patterns
- Object lifetime management
- Resource management with RAII

All new code must follow these Qt-native standards.

---

This document supersedes the previous agents.md file. All AI agents must follow these guidelines when working on the openSYDE codebase.