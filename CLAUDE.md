# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

openSYDE is an open-source software development environment by STW (Sensor-Technik Wiedemann GmbH) for implementing, commissioning, analyzing, and maintaining control systems for mobile machines. It's a Qt6 C++ application with multiple tools sharing a common core library.

## Build System

All builds use **CMake** with **Ninja** generator and **MinGW 13.1.0** compiler.

### Quick Start

```powershell
cd opensyde_tool/bat

# Build main GUI application (includes Qt DLL deployment)
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

# Build without deploying Qt DLLs
.\build.ps1 -SkipDeploy
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
   - Uses 8 parallel jobs (`-j8`)
   - Build directory: `opensyde_tool/temp_<Component>_<BuildType>/`

3. **Install** (CMake): Copies binaries to result folder

4. **Deploy** (windeployqt6): Copies required Qt DLLs and plugins alongside each executable
   - Each executable's folder becomes self-contained and runnable
   - Skip with `-SkipDeploy` if only checking compilation

### Build Outputs

All build artifacts go to: **`opensyde_tool/result/`**

| Component | Location | Purpose |
|-----------|----------|---------|
| openSYDE GUI | `result/tool/openSYDE.exe` | Main system definition tool |
| CAN Monitor | `result/tool/CAN_Monitor/openSYDE_CAN_Monitor.exe` | CAN message monitoring |
| SYDEflash | `result/utilities/SYDEflash/SYDEflash.exe` | Firmware flashing tool |

Each folder also contains the Qt DLLs and plugins needed to run the executable.

### Troubleshooting Builds

#### CMake Configuration Errors

**Symptom:** `CMake Error: ...` during configure step

**Solutions:**
1. Clean rebuild:
   ```powershell
   cd opensyde_tool\bat
   .\build.ps1 -Clean
   ```

2. Verify Qt installation paths exist:
   ```powershell
   Test-Path C:\Qt\6.10.1\mingw_64
   Test-Path C:\Qt\Tools\CMake_64\bin\cmake.exe
   Test-Path C:\Qt\Tools\Ninja\ninja.exe
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

### For AI Agents

```powershell
cd opensyde_tool/bat
.\build.ps1 -Component All
```

The script validates prerequisites, builds, installs, and deploys Qt DLLs automatically. Use `-SkipDeploy` when you only need to verify compilation.

### Build Script Reference

| Parameter | Values | Default | Description |
|-----------|--------|---------|-------------|
| `-Component` | `GUI`, `Core`, `CANMonitor`, `SYDEflash`, `All` | `GUI` | What to build |
| `-BuildType` | `Release`, `Debug` | `Release` | Build configuration |
| `-Clean` | switch | off | Delete temp folder and reconfigure CMake |
| `-SkipDeploy` | switch | off | Skip windeployqt Qt DLL deployment |

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
