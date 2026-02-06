# openSYDE Build Script

Single PowerShell script (`build.ps1`) that builds all openSYDE components and deploys the required Qt DLLs so each executable is self-contained and runnable from its output folder.

## Quick Start

```powershell
cd opensyde_tool/bat

.\build.ps1                          # Build main GUI (Release) + deploy Qt DLLs
.\build.ps1 -Component CANMonitor    # Build CAN Monitor
.\build.ps1 -Component SYDEflash     # Build SYDEflash
.\build.ps1 -Component All           # Build everything
.\build.ps1 -Clean                   # Clean and rebuild
.\build.ps1 -BuildType Debug         # Debug build
.\build.ps1 -SkipDeploy              # Build without deploying Qt DLLs
```

## Parameters

| Parameter | Values | Default | Description |
|-----------|--------|---------|-------------|
| `-Component` | `GUI`, `Core`, `CANMonitor`, `SYDEflash`, `All` | `GUI` | What to build |
| `-BuildType` | `Release`, `Debug` | `Release` | Build configuration |
| `-Clean` | switch | off | Delete build cache and reconfigure CMake |
| `-SkipDeploy` | switch | off | Skip windeployqt Qt DLL deployment |

## Prerequisites

- **Qt 6.10.1** with MinGW 64-bit at `C:\Qt\6.10.1\mingw_64\`
- **Qt Tools** at `C:\Qt\Tools\`:
  - CMake (`C:\Qt\Tools\CMake_64\bin\cmake.exe`)
  - Ninja (`C:\Qt\Tools\Ninja\ninja.exe`)
  - MinGW 13.1.0 (`C:\Qt\Tools\mingw1310_64\bin\`)
- **windeployqt6** ships with Qt at `C:\Qt\6.10.1\mingw_64\bin\windeployqt6.exe`

The script validates all prerequisites before building and will report what's missing.

## Build Process

1. **Configure** - CMake generates Ninja build files (only on first build or after `-Clean`)
2. **Build** - Ninja compiles with 8 parallel jobs
3. **Install** - CMake copies executables to the result folder
4. **Deploy** - `windeployqt6` copies Qt DLLs, plugins, and platform files alongside each executable

## Build Outputs

All artifacts go to `opensyde_tool/result/`. Each folder is self-contained with all DLLs needed to run:

| Component | Location |
|-----------|----------|
| openSYDE GUI | `result/tool/openSYDE.exe` |
| CAN Monitor | `result/tool/CAN_Monitor/openSYDE_CAN_Monitor.exe` |
| SYDEflash | `result/utilities/SYDEflash/SYDEflash.exe` |

## Troubleshooting

### CMake configuration errors

```powershell
.\build.ps1 -Clean    # Deletes temp folder and reconfigures
```

### Qt not found

Verify paths exist:
```powershell
Test-Path C:\Qt\6.10.1\mingw_64
Test-Path C:\Qt\Tools\CMake_64\bin\cmake.exe
Test-Path C:\Qt\Tools\Ninja\ninja.exe
```

### windeployqt fails

Ensure MinGW is installed - windeployqt needs `g++.exe` in PATH to locate gcc runtime DLLs. The build script handles PATH setup automatically.
