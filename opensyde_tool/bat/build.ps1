# openSYDE Build Script (PowerShell)
# This script provides a unified way to build all openSYDE components
# Usage: .\build.ps1 [-Component <name>] [-BuildType <Release|Debug>] [-Clean]
#
# Examples:
#   .\build.ps1                              # Build main GUI (Release)
#   .\build.ps1 -Component CANMonitor        # Build CAN Monitor
#   .\build.ps1 -Component SYDEflash         # Build SYDEflash
#   .\build.ps1 -Component All               # Build all components
#   .\build.ps1 -Clean                       # Clean and rebuild
#   .\build.ps1 -BuildType Debug             # Debug build

param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("GUI", "CANMonitor", "SYDEflash", "All")]
    [string]$Component = "GUI",

    [Parameter(Mandatory=$false)]
    [ValidateSet("Release", "Debug")]
    [string]$BuildType = "Release",

    [Parameter(Mandatory=$false)]
    [switch]$Clean
)

# Configuration
$ErrorActionPreference = "Stop"
$QtPath = "C:\Qt\6.10.1\mingw_64"
$QtToolsPath = "C:\Qt\Tools"
$ToolchainFile = "..\pjt\toolchain_windows.cmake"
$LogsDir = "logs"

# Create logs directory if it doesn't exist
if (-not (Test-Path $LogsDir)) {
    New-Item -ItemType Directory -Path $LogsDir -Force | Out-Null
}

# Build configurations
$BuildConfigs = @{
    "GUI" = @{
        ProjectFolder = "..\pjt\openSYDE"
        TempFolder = "..\temp_openSYDE_$BuildType"
        Name = "openSYDE GUI"
    }
    "CANMonitor" = @{
        ProjectFolder = "..\pjt\openSYDE_CAN_Monitor"
        TempFolder = "..\temp_openSYDE_CAN_Monitor_$BuildType"
        Name = "CAN Monitor"
    }
    "SYDEflash" = @{
        ProjectFolder = "..\pjt\SYDEflash"
        TempFolder = "..\temp_SYDEflash_$BuildType"
        Name = "SYDEflash"
    }
}

# Functions
function Write-BuildHeader {
    param([string]$Message)
    Write-Host "`n========================================" -ForegroundColor Cyan
    Write-Host $Message -ForegroundColor Cyan
    Write-Host "========================================`n" -ForegroundColor Cyan
}

function Write-BuildStep {
    param([string]$Message)
    Write-Host "[BUILD] $Message" -ForegroundColor Green
}

function Write-BuildError {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

function Test-Prerequisites {
    Write-BuildStep "Checking prerequisites..."

    # Check Qt installation
    if (-not (Test-Path $QtPath)) {
        Write-BuildError "Qt 6.10.1 MinGW not found at: $QtPath"
        Write-Host "Please install Qt 6.10.1 with MinGW 64-bit component" -ForegroundColor Yellow
        exit 1
    }

    # Check Qt Tools
    if (-not (Test-Path "$QtToolsPath\CMake_64\bin\cmake.exe")) {
        Write-BuildError "CMake not found at: $QtToolsPath\CMake_64"
        Write-Host "Please install Qt Tools (CMake, Ninja, MinGW)" -ForegroundColor Yellow
        exit 1
    }

    Write-Host "  ✓ Qt 6.10.1 found" -ForegroundColor Green
    Write-Host "  ✓ CMake found" -ForegroundColor Green
    Write-Host "  ✓ MinGW found" -ForegroundColor Green
}

function Build-Component {
    param(
        [string]$ProjectFolder,
        [string]$TempFolder,
        [string]$ComponentName
    )

    Write-BuildHeader "Building $ComponentName ($BuildType)"

    # Setup environment
    $env:PATH = "$QtToolsPath\mingw1310_64\bin;$QtToolsPath\CMake_64\bin;$QtToolsPath\Ninja;$QtPath;$env:PATH"

    # Create temp folder if needed
    if (-not (Test-Path $TempFolder)) {
        Write-BuildStep "Creating build directory: $TempFolder"
        New-Item -ItemType Directory -Path $TempFolder -Force | Out-Null
    }

    # Clean if requested
    if ($Clean -and (Test-Path "$TempFolder\build.ninja")) {
        Write-BuildStep "Cleaning previous build..."
        Remove-Item "$TempFolder\*" -Recurse -Force
    }

    # Change to temp folder
    Push-Location $TempFolder

    try {
        # CMake configure (only if build.ninja doesn't exist)
        if (-not (Test-Path "build.ninja")) {
            Write-BuildStep "Running CMake configure..."
            & cmake.exe $ProjectFolder -GNinja -DCMAKE_BUILD_TYPE=$BuildType -DCMAKE_TOOLCHAIN_FILE=$ToolchainFile
            if ($LASTEXITCODE -ne 0) {
                throw "CMake configure failed with exit code $LASTEXITCODE"
            }
        } else {
            Write-BuildStep "Using existing CMake configuration (use -Clean to reconfigure)"
        }

        # Build
        Write-BuildStep "Building with Ninja (parallel jobs: 24)..."
        $buildStart = Get-Date
        & cmake.exe --build . --target all -- -j24
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed with exit code $LASTEXITCODE"
        }
        $buildTime = (Get-Date) - $buildStart
        Write-Host "  ✓ Build completed in $($buildTime.TotalSeconds.ToString('F1')) seconds" -ForegroundColor Green

        # Install
        Write-BuildStep "Installing binaries to result folder..."
        & cmake.exe --build . --target install
        if ($LASTEXITCODE -ne 0) {
            throw "Install failed with exit code $LASTEXITCODE"
        }

        Write-Host "`n✓ $ComponentName build SUCCESS" -ForegroundColor Green -BackgroundColor DarkGreen

    } catch {
        Write-BuildError $_.Exception.Message
        Write-Host "`n✗ $ComponentName build FAILED" -ForegroundColor Red -BackgroundColor DarkRed
        Pop-Location
        exit 1
    }

    Pop-Location
}

# Main execution
Write-BuildHeader "openSYDE Build System"
Write-Host "Component:  $Component" -ForegroundColor White
Write-Host "Build Type: $BuildType" -ForegroundColor White
Write-Host "Clean:      $Clean" -ForegroundColor White

Test-Prerequisites

if ($Component -eq "All") {
    $components = @("GUI", "CANMonitor", "SYDEflash")
    $totalStart = Get-Date

    foreach ($comp in $components) {
        $config = $BuildConfigs[$comp]
        Build-Component -ProjectFolder $config.ProjectFolder `
                       -TempFolder $config.TempFolder `
                       -ComponentName $config.Name
    }

    $totalTime = (Get-Date) - $totalStart
    Write-BuildHeader "All Components Built Successfully"
    Write-Host "Total build time: $($totalTime.TotalMinutes.ToString('F1')) minutes" -ForegroundColor Green

} else {
    $config = $BuildConfigs[$Component]
    Build-Component -ProjectFolder $config.ProjectFolder `
                   -TempFolder $config.TempFolder `
                   -ComponentName $config.Name
}

Write-Host "`nBuild artifacts are in: opensyde_tool\result\" -ForegroundColor Cyan
