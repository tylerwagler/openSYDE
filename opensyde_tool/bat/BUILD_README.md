# openSYDE Build Scripts

This directory contains build scripts for compiling openSYDE components.

## Quick Start

### Option 1: PowerShell (Recommended for agents/automation)

```powershell
# Build main GUI application
.\build.ps1

# Build CAN Monitor
.\build.ps1 -Component CANMonitor

# Build SYDEflash
.\build.ps1 -Component SYDEflash

# Build everything
.\build.ps1 -Component All

# Clean build
.\build.ps1 -Clean

# Debug build
.\build.ps1 -BuildType Debug
```

### Option 2: Batch Files

```batch
# Build all components (Release)
build_all.bat

# Build individual components
build_release.bat              # Main GUI
build_can_monitor_release.bat  # CAN Monitor
build_syde_flash_release.bat   # SYDEflash
```

## Prerequisites

- **Qt 6.10.1** with MinGW 64-bit installed at: `C:\Qt\6.10.1\mingw_64\`
- **Qt Tools** (CMake, Ninja, MinGW 13.1.0) at: `C:\Qt\Tools\`

### Installing Qt

1. Download Qt Online Installer from https://www.qt.io/download
2. Install Qt 6.10.1 for desktop development
3. Select components:
   - Qt 6.10.1 → MinGW 64-bit
   - Developer and Designer Tools → CMake
   - Developer and Designer Tools → Ninja
   - Developer and Designer Tools → MinGW 13.1.0 64-bit

## Build Outputs

**Build artifacts** are placed in: `opensyde_tool\result\`
- `openSYDE.exe` - Main GUI application
- `openSYDE_CAN_Monitor.exe` - CAN Monitor tool
- `SYDEflash.exe` - Firmware flashing tool

**Build logs** are saved in: `opensyde_tool\bat\logs\`
- `build_syde_flash_YYYYMMDD_HHMMSS.log` - SYDEflash build log
- Logs are timestamped and preserved for debugging
- Logs directory is gitignored (not checked into version control)

## Build Process

The build system uses CMake with Ninja generator and MinGW compiler:

1. **Configure**: CMake generates build files (only on first build or when CMakeLists.txt changes)
2. **Build**: Ninja compiles with 24 parallel jobs
3. **Install**: Binaries are copied to `opensyde_tool\result\`

## Troubleshooting

### Build.ninja not found or CMake errors

Delete the temp folder and rebuild:
```batch
rmdir /S /Q ..\temp_openSYDE_Release
build_release.bat
```

### Qt not found errors

Verify Qt installation paths:
- `C:\Qt\6.10.1\mingw_64\` should exist
- `C:\Qt\Tools\CMake_64\bin\cmake.exe` should exist
- `C:\Qt\Tools\Ninja\ninja.exe` should exist

### Compilation errors

If you see C++ compilation errors (not CMake errors), these are usually due to:
- Incomplete QString migration (Phase 1 work in progress)
- Missing dependencies in core library

Check the Phase 1 implementation plan for QString migration status.

## For Agents/Scripts

The PowerShell script `build.ps1` is recommended for automated builds because:
- Better error checking and reporting
- Colored output for easy parsing
- Parameter validation
- Prerequisite checking
- Works in any shell (PowerShell, Git Bash via `pwsh`)

Example for automation:
```powershell
# Build with error handling
try {
    .\build.ps1 -Component All
    Write-Host "Build succeeded"
} catch {
    Write-Host "Build failed: $_"
    exit 1
}
```

## Build Script Reference

| Script | Purpose | Usage |
|--------|---------|-------|
| `build.ps1` | **PowerShell build script** | `.\build.ps1 -Component <name>` |
| `build_all.bat` | Build all components | `build_all.bat [Release\|Debug]` |
| `build_with_cmake.bat` | **Core build logic** | Called by other scripts |
| `build_release.bat` | Build main GUI | `build_release.bat` |
| `build_can_monitor_release.bat` | Build CAN Monitor | `build_can_monitor_release.bat` |
| `build_syde_flash_release.bat` | Build SYDEflash | `build_syde_flash_release.bat` |

## Clean Builds

To force a full rebuild (reconfigure CMake):

**PowerShell:**
```powershell
.\build.ps1 -Clean
```

**Batch:**
```batch
rmdir /S /Q ..\temp_openSYDE_Release
rmdir /S /Q ..\temp_openSYDE_CAN_Monitor_Release
rmdir /S /Q ..\temp_SYDEflash_Release
build_all.bat
```

## Build Times

Approximate build times on modern hardware (parallel jobs: 24):

- Main GUI: 3-5 minutes
- CAN Monitor: 2-3 minutes
- SYDEflash: 2-3 minutes
- **Total (all components)**: 7-11 minutes

## Advanced Usage

### Change parallel job count

Edit `build_with_cmake.bat` line 31:
```batch
cmake.exe --build . --target all -- -j24
```
Change `-j24` to your preferred number (typically CPU cores * 2).

### Custom toolchain

Edit component batch files to use different toolchain:
```batch
set TOOLCHAIN=../pjt/toolchain_custom.cmake
```

### Build specific targets

```batch
cd ..\temp_openSYDE_Release
cmake --build . --target <target_name>
```
