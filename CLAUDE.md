# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

openSYDE is an open-source software development environment by STW (Sensor-Technik Wiedemann GmbH) for implementing, commissioning, analyzing, and maintaining control systems for mobile machines. It's a Qt6 C++ application with multiple tools sharing a common core library.

## Build Commands

All builds use CMake with Ninja and MinGW. Run from `opensyde_tool/bat/`:

```batch
# Build main GUI application (openSYDE)
build_release.bat

# Build CAN Monitor
build_can_monitor_release.bat

# Build SYDEflash (firmware flashing tool)
build_syde_flash_release.bat
```

**Requirements:**
- Qt 6.10.1 (MinGW 64-bit) installed at `C:\Qt\6.10.1\mingw_64`
- Qt Tools (CMake, Ninja, MinGW 13.1.0) at `C:\Qt\Tools\`

Build output goes to `opensyde_tool/result/`.

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

Active QString migration replacing legacy `C_SclString` with Qt's `QString`:
- `C_SclString` → `QString`
- `C_SclDynamicArray` → `QList`
- `c_str() / Length()` → `toUtf8().constData() / length()`
- `C_SclString::IntToStr()` → `QString::number()`

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
