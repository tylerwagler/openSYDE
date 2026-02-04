# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

openSYDE is an open-source software development environment by STW (Sensor-Technik Wiedemann GmbH) for implementing, commissioning, analyzing, and maintaining control systems for mobile machines. It's a Qt6 C++ application with multiple tools sharing a common core library.

## Build System

All builds use **CMake** with **Ninja** generator and **MinGW 13.1.0** compiler.

### Quick Start (For Agents/Automation)

**Recommended: PowerShell script** (better error handling, works in all shells)

```powershell
cd opensyde_tool/bat

# Build main GUI application
.\build.ps1

# Build CAN Monitor
.\build.ps1 -Component CANMonitor

# Build SYDEflash
.\build.ps1 -Component SYDEflash

# Build everything
.\build.ps1 -Component All

# Clean build (reconfigure CMake)
.\build.ps1 -Clean

# Debug build
.\build.ps1 -BuildType Debug
```

**Alternative: Batch files**

```batch
cd opensyde_tool\bat

# Build all components
build_all.bat

# Or build individual components
build_release.bat              # Main GUI (openSYDE)
build_can_monitor_release.bat  # CAN Monitor
build_syde_flash_release.bat   # SYDEflash
```

### Prerequisites

**Required Qt Installation:**
- **Qt 6.10.1** with MinGW 64-bit at: `C:\Qt\6.10.1\mingw_64\`
- **Qt Tools** at: `C:\Qt\Tools\`
  - CMake (`C:\Qt\Tools\CMake_64\bin\cmake.exe`)
  - Ninja (`C:\Qt\Tools\Ninja\ninja.exe`)
  - MinGW 13.1.0 (`C:\Qt\Tools\mingw1310_64\bin\`)

**Install Qt:**
1. Download Qt Online Installer: https://www.qt.io/download
2. Install Qt 6.10.1 for desktop development
3. Select components:
   - `Qt 6.10.1` → `MinGW 64-bit`
   - `Developer and Designer Tools` → `CMake`
   - `Developer and Designer Tools` → `Ninja`
   - `Developer and Designer Tools` → `MinGW 13.1.0 64-bit`

### Build Process

The build system follows these steps:

1. **Configure** (CMake): Generates Ninja build files
   - Only runs on first build or if `build.ninja` doesn't exist
   - Uses toolchain file: `opensyde_tool/pjt/toolchain_windows.cmake`
   - To reconfigure: Delete temp folder or use `build.ps1 -Clean`

2. **Build** (Ninja): Compiles source code
   - Uses 24 parallel jobs (`-j24`)
   - Build directory: `opensyde_tool/temp_<Component>_<BuildType>/`

3. **Install** (CMake): Copies binaries to result folder
   - Output: `opensyde_tool/result/`
   - Executables:
     - `openSYDE.exe` (main GUI)
     - `openSYDE_CAN_Monitor.exe`
     - `SYDEflash.exe`

### Build Outputs

All build artifacts go to: **`opensyde_tool/result/`**

| Component | Executable | Purpose |
|-----------|------------|---------|
| openSYDE GUI | `openSYDE.exe` | Main system definition tool |
| CAN Monitor | `openSYDE_CAN_Monitor.exe` | CAN message monitoring |
| SYDEflash | `SYDEflash.exe` | Firmware flashing tool |

### Troubleshooting Builds

#### CMake Configuration Errors

**Symptom:** `CMake Error: ...` during configure step

**Solutions:**
1. Delete temp folder and rebuild:
   ```batch
   rmdir /S /Q opensyde_tool\temp_openSYDE_Release
   cd opensyde_tool\bat
   build_release.bat
   ```

2. Verify Qt installation paths exist:
   ```batch
   dir C:\Qt\6.10.1\mingw_64
   dir C:\Qt\Tools\CMake_64\bin\cmake.exe
   dir C:\Qt\Tools\Ninja\ninja.exe
   ```

#### Compilation Errors

**Symptom:** C++ compiler errors (not CMake errors)

**Common causes:**
- **QString migration incomplete**: Legacy `C_SclString` code not yet migrated
  - Example: `error: no member named 'c_str' in 'QString'`
  - See "Current Migration Work" section below
  - Check Phase 1 plans in `plans/02_FUTURE/`

- **Missing dependencies**: Core library compilation failures
  - Files like `C_OscIpDispatcherWinSock.cpp` or `CXFLFlashWrite.cpp`
  - These are part of ongoing QString migration work

**Not a build system issue** - these are code-level problems being addressed in Phase 1.

#### Build Script Issues

**Symptom:** `'build_with_cmake' is not recognized as an internal or external command`

**Solution:** Use PowerShell script instead:
```powershell
.\build.ps1 -Component <name>
```

Or run cmake commands directly (see `opensyde_tool/bat/BUILD_README.md` for details).

### Build Times

Approximate build times (24 parallel jobs, modern hardware):

- Main GUI: 3-5 minutes
- CAN Monitor: 2-3 minutes
- SYDEflash: 2-3 minutes
- **All components**: 7-11 minutes (clean build)

### For AI Agents

**Always use the PowerShell script** for automated builds:

```powershell
# Recommended approach for agents
cd opensyde_tool/bat

# Build with error handling
try {
    .\build.ps1 -Component All
    Write-Host "Build succeeded"
} catch {
    Write-Host "Build failed: $_"
    exit 1
}
```

**Why PowerShell?**
- ✅ Better error checking and reporting
- ✅ Colored output for parsing
- ✅ Prerequisite validation (checks Qt paths)
- ✅ Works in Git Bash (via `pwsh`)
- ✅ Clear step-by-step progress
- ✅ Returns proper exit codes

**Batch files** work but have limitations:
- Less error information
- Harder to debug
- May not work in Git Bash

### Build Script Reference

| Script | Purpose | Usage |
|--------|---------|-------|
| **`build.ps1`** | **PowerShell build (recommended)** | `.\build.ps1 -Component <name>` |
| `build_all.bat` | Build all components | `build_all.bat [Release\|Debug]` |
| `build_with_cmake.bat` | Core build logic (called by others) | Internal use |
| `build_release.bat` | Build main GUI | `build_release.bat` |
| `build_can_monitor_release.bat` | Build CAN Monitor | `build_can_monitor_release.bat` |
| `build_syde_flash_release.bat` | Build SYDEflash | `build_syde_flash_release.bat` |

📖 **Detailed documentation**: See `opensyde_tool/bat/BUILD_README.md`

### Advanced Build Options

**Clean build (reconfigure CMake):**
```powershell
.\build.ps1 -Clean
```

**Debug build:**
```powershell
.\build.ps1 -BuildType Debug
```

**Change parallel jobs** (edit `build_with_cmake.bat` line 31):
```batch
cmake.exe --build . --target all -- -j24
```
Change `-j24` to desired number (typically CPU cores × 2).

### Common Build Errors Summary

| Error Type | Symptom | Solution |
|------------|---------|----------|
| **Qt not found** | `CMake Error: Qt6 not found` | Verify `C:\Qt\6.10.1\mingw_64\` exists |
| **CMake not found** | `'cmake' is not recognized` | Verify `C:\Qt\Tools\CMake_64\bin\cmake.exe` exists |
| **Old CMake config** | `CMake Error: ... precompiled_headers ...` | Delete temp folder, rebuild |
| **QString migration** | `error: no member named 'c_str'` | Code issue (Phase 1 work), not build system |
| **C_SclString errors** | `cannot convert C_SclString to QString` | Code issue (Phase 1 work), not build system |

## Architecture

### Main Components

| Component | Location | Purpose |
|-----------|----------|---------|
| openSYDE GUI | `opensyde_tool/src/` | Main system definition and configuration tool |
| CAN Monitor | `opensyde_tool/src/can_monitor/` | CAN message monitoring and analysis |
| SYDEflash | `opensyde_tool/src/syde_flash/` | Firmware flashing tool |
| opensyde_core | `opensyde_tool/libs/opensyde_core/` | Shared core library (non-GUI logic) |

Additional tools: `opensyde_syde_coder_c/`, `opensyde_syde_sup/`, `opensyde_syde_x_gen/`, `opensyde_cmd_line_flash_tool/`

### Core Library Structure

```
opensyde_core/
├── project/system/node/     # Node definitions, CAN protocols
├── project/system/          # System definitions, device definitions
├── project/view/            # View definitions
├── halc/                    # Hardware Abstraction Layer Configuration
├── protocol_drivers/        # OSY protocol, flashing, device config
├── data_dealer/paramset/    # Parameter set handling
├── exports/code_generation/ # Code generation
├── xml_parser/              # XML handling (TinyXML2)
├── scl/                     # Legacy string library (being migrated to Qt)
├── stwtypes/                # Portable type definitions
└── stwerrors/               # Standard error codes
```

## Coding Conventions

### Naming

- **Classes**: Prefix `C_` (e.g., `C_OscUtils`, `C_OscSystemDefinition`)
- **GUI classes**: `C_` + feature abbreviation + type (e.g., `C_CamOgeChxSettings` = CAN Monitor OpenSYDE GUI Element Checkbox)
- **Namespaces**: `stw::opensyde_core`, `stw::opensyde_gui_logic`

### Parameter Prefixes (STW convention)

- `o` = object/reference, `q` = boolean
- `u8/u16/u32/u64` = unsigned integers, `s8/s16/s32/s64` = signed integers
- `f32/f64` = float/double
- `c` = container, `pc/opc` = pointer to container
- `rc/orc` = reference to container

Example: `const uint16_t ou16_Length` = "object unsigned 16-bit Length"

### Types

Use types from `stwtypes.hpp`: `int8_t`, `uint8_t`, `int16_t`, `uint16_t`, `int32_t`, `uint32_t`, `int64_t`, `uint64_t`, `float32_t`, `float64_t`, `char_t`

### Error Codes

From `stwerrors.hpp` in `stw::errors` namespace:
- `C_NO_ERR` (0), `C_UNKNOWN_ERR` (-1), `C_RANGE` (-5), `C_RD_WR` (-7), `C_CONFIG` (-10), `C_TIMEOUT` (-12)

### File Structure

```cpp
//----------------------------------------------------------------------------------------------------------------------
/*!
   \file
   \brief       Brief description
   \copyright   Copyright [YEAR] Sensor-Technik Wiedemann GmbH. All rights reserved.
*/
//----------------------------------------------------------------------------------------------------------------------

/* -- Includes ------------------------------------------------------------------------------------------------------ */
/* -- Namespace ----------------------------------------------------------------------------------------------------- */
/* -- Types --------------------------------------------------------------------------------------------------------- */
/* -- Implementation ------------------------------------------------------------------------------------------------ */
```

## Current Migration Work

### Phase 1: QString & Qt Container Migration (In Progress)

Active migration replacing legacy STL/C_SclString with Qt-native equivalents:

**String Migration:**
- `C_SclString` → `QString`
- `C_SclDynamicArray` → `QList`
- `c_str() / Length()` → `toUtf8().constData() / length()`
- `C_SclString::IntToStr()` → `QString::number()`

**Container Migration:**
- `std::vector<T>` → `QList<T>`
- `std::vector<QString>` → `QStringList`
- `std::map<QString, T>` → `QHash<QString, T>`
- `std::set<T>` → `QSet<T>`

**File I/O Migration:**
- `std::ifstream/ofstream` → `QFile + QTextStream`
- `std::filesystem::path` → `QString + QFileInfo`

📖 **Qt-Native Standards**: See `plans/02_FUTURE/Qt_Native_Coding_Standards.md` for complete guidelines.

📋 **Implementation Plans**: See `plans/02_FUTURE/Phase_*_Implementation_Plan.md` for detailed task breakdowns.

## AI Agent Workspace Rules

From `agents.md`:
- Use `plans/` directory for all persistent artifacts (implementation plans, checklists, scripts, notes)
- Never use temporary directories for work products
- Naming: `<Feature>_<Type>.md` (e.g., `DBC_Export_Implementation_Plan.md`)

## Key Dependencies

- **Qt 6.8.3** (LGPL 3) - GUI framework
- **TinyXML2** (zlib) - XML parsing
- **OpenSSL 3.0.0** (Apache 2.0) - Encryption
- **Vector BLF/DBC** (GPL 3) - CAN file formats
- **Miniz** (MIT) - ZIP compression
