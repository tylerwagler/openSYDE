# openSYDE Build Script (PowerShell)
# Unified build script for all openSYDE components with Qt deployment.
#
# Usage: .\build.ps1 [-Component <name>] [-BuildType <Release|Debug>] [-Clean] [-SkipDeploy]
#
# Examples:
#   .\build.ps1                              # Build main GUI (Release) + deploy Qt DLLs
#   .\build.ps1 -Component CANMonitor        # Build CAN Monitor
#   .\build.ps1 -Component SYDEflash         # Build SYDEflash
#   .\build.ps1 -Component All               # Build all components
#   .\build.ps1 -Clean                       # Clean and rebuild
#   .\build.ps1 -BuildType Debug             # Debug build
#   .\build.ps1 -SkipDeploy                  # Build without deploying Qt DLLs

param(
    [Parameter(Mandatory = $false)]
    [ValidateSet("GUI", "Core", "CANMonitor", "SYDEflash", "All")]
    [string]$Component = "GUI",

    [Parameter(Mandatory = $false)]
    [ValidateSet("Release", "Debug")]
    [string]$BuildType = "Release",

    [Parameter(Mandatory = $false)]
    [switch]$Clean,

    [Parameter(Mandatory = $false)]
    [switch]$SkipDeploy
)

# Configuration
$ErrorActionPreference = "Stop"
$QtPath = "C:\Qt\6.10.1\mingw_64"
$QtToolsPath = "C:\Qt\Tools"
$WinDeployQt = "$QtPath\bin\windeployqt6.exe"
$ScriptDir = $PSScriptRoot
$ToolchainFile = (Resolve-Path "$ScriptDir\..\pjt\toolchain_windows.cmake").Path
$ResultDir = (Resolve-Path "$ScriptDir\..\result").Path

# Build configurations
$BuildConfigs = @{
    "GUI"        = @{
        ProjectFolder = "..\pjt\openSYDE"
        TempFolderBase = "..\temp_openSYDE"
        Name          = "openSYDE GUI"
        Target        = "all"
        ExeName       = "openSYDE.exe"
        InstallDir    = "result\tool"
    }
    "Core"       = @{
        ProjectFolder = "..\pjt\openSYDE"
        TempFolderBase = "..\temp_openSYDE"
        Name          = "opensyde_core library"
        Target        = "opensyde_core"
        SkipInstall   = $true
        SkipDeploy    = $true
    }
    "CANMonitor" = @{
        ProjectFolder = "..\pjt\openSYDE_CAN_Monitor"
        TempFolderBase = "..\temp_openSYDE_CAN_Monitor"
        Name          = "CAN Monitor"
        Target        = "all"
        ExeName       = "openSYDE_CAN_Monitor.exe"
        InstallDir    = "result\tool\CAN_Monitor"
    }
    "SYDEflash"  = @{
        ProjectFolder = "..\pjt\SYDEflash"
        TempFolderBase = "..\temp_SYDEflash"
        Name          = "SYDEflash"
        Target        = "all"
        ExeName       = "SYDEflash.exe"
        InstallDir    = "result\utilities\SYDEflash"
    }
}

# ---- Helper Functions ----

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

    if (-not (Test-Path $QtPath)) {
        Write-BuildError "Qt 6.10.1 MinGW not found at: $QtPath"
        Write-Host "Please install Qt 6.10.1 with MinGW 64-bit component" -ForegroundColor Yellow
        exit 1
    }

    if (-not (Test-Path "$QtToolsPath\CMake_64\bin\cmake.exe")) {
        Write-BuildError "CMake not found at: $QtToolsPath\CMake_64\bin\cmake.exe"
        exit 1
    }

    if (-not (Test-Path "$QtToolsPath\Ninja\ninja.exe")) {
        Write-BuildError "Ninja not found at: $QtToolsPath\Ninja\ninja.exe"
        exit 1
    }

    if (-not (Test-Path "$QtToolsPath\mingw1310_64\bin\g++.exe")) {
        Write-BuildError "MinGW 13.1.0 not found at: $QtToolsPath\mingw1310_64"
        exit 1
    }

    if (-not $SkipDeploy -and -not (Test-Path $WinDeployQt)) {
        Write-BuildError "windeployqt6 not found at: $WinDeployQt"
        exit 1
    }

    Write-Host "  Qt 6.10.1  : $QtPath" -ForegroundColor DarkGray
    Write-Host "  CMake      : $QtToolsPath\CMake_64\bin\cmake.exe" -ForegroundColor DarkGray
    Write-Host "  Ninja      : $QtToolsPath\Ninja\ninja.exe" -ForegroundColor DarkGray
    Write-Host "  MinGW      : $QtToolsPath\mingw1310_64" -ForegroundColor DarkGray
    Write-Host "  windeployqt: $WinDeployQt" -ForegroundColor DarkGray
    Write-Host ""
}

function Deploy-QtDlls {
    param(
        [string]$ExePath,
        [string]$ComponentName
    )

    Write-BuildStep "Deploying Qt DLLs for $ComponentName..."

    if (-not (Test-Path $ExePath)) {
        Write-BuildError "Executable not found: $ExePath"
        throw "Cannot deploy: executable not found at $ExePath"
    }

    $deployDir = Split-Path $ExePath -Parent

    # windeployqt needs MinGW in PATH to find gcc runtime DLLs
    $env:PATH = "$script:QtToolsPath\mingw1310_64\bin;$script:QtPath\bin;$env:PATH"

    & $script:WinDeployQt `
        --dir $deployDir `
        --no-translations `
        --no-system-d3d-compiler `
        --no-opengl-sw `
        $ExePath

    if ($LASTEXITCODE -ne 0) {
        throw "windeployqt failed for $ExePath with exit code $LASTEXITCODE"
    }

    Write-Host "  Deployed to: $deployDir" -ForegroundColor DarkGray
}

function Build-Component {
    param(
        [string]$ProjectFolder,
        [string]$TempFolder,
        [string]$ComponentName,
        [string]$Target = "all",
        [bool]$SkipInstallStep = $false,
        [bool]$SkipDeployStep = $false,
        [string]$ExeName = "",
        [string]$InstallDir = ""
    )

    Write-BuildHeader "Building $ComponentName ($BuildType)"

    # Setup environment
    $env:PATH = "$script:QtToolsPath\mingw1310_64\bin;$script:QtToolsPath\CMake_64\bin;$script:QtToolsPath\Ninja;$script:QtPath\bin;$env:PATH"

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

    Push-Location $TempFolder

    try {
        # Step 1: CMake configure
        if (-not (Test-Path "build.ninja")) {
            Write-BuildStep "Step 1/3 - CMake configure..."
            & cmake.exe -S $ProjectFolder -B . -GNinja "-DCMAKE_BUILD_TYPE=$BuildType" -DCMAKE_TOOLCHAIN_FILE="$script:ToolchainFile"
            if ($LASTEXITCODE -ne 0) {
                throw "CMake configure failed with exit code $LASTEXITCODE"
            }
        }
        else {
            Write-BuildStep "Step 1/3 - Using existing CMake configuration (use -Clean to reconfigure)"
        }

        # Step 2: Build
        Write-BuildStep "Step 2/3 - Building target '$Target' (parallel jobs: 8)..."
        $buildStart = Get-Date
        & cmake.exe --build . --target $Target -- -j8
        if ($LASTEXITCODE -ne 0) {
            throw "Build failed with exit code $LASTEXITCODE"
        }
        $buildTime = (Get-Date) - $buildStart
        Write-Host "  Build completed in $($buildTime.TotalSeconds.ToString('F1'))s" -ForegroundColor DarkGray

        # Step 3: Install
        if (-not $SkipInstallStep) {
            Write-BuildStep "Step 3/3 - Installing to result folder..."
            & cmake.exe --build . --target install
            if ($LASTEXITCODE -ne 0) {
                throw "Install failed with exit code $LASTEXITCODE"
            }
        }
        else {
            Write-BuildStep "Step 3/3 - Install skipped (library-only build)"
        }

        Write-Host "`n  $ComponentName build SUCCESS" -ForegroundColor Green -BackgroundColor DarkGreen

    }
    catch {
        Write-BuildError $_.Exception.Message
        Write-Host "`n  $ComponentName build FAILED" -ForegroundColor Red -BackgroundColor DarkRed
        Pop-Location
        exit 1
    }

    Pop-Location

    # Deploy Qt DLLs
    if (-not $SkipDeployStep -and -not $script:SkipDeploy -and $ExeName -ne "" -and $InstallDir -ne "") {
        $exeFullPath = Join-Path $ScriptDir "..\$InstallDir\$ExeName"
        $exeFullPath = [System.IO.Path]::GetFullPath($exeFullPath)
        Deploy-QtDlls -ExePath $exeFullPath -ComponentName $ComponentName
    }
}

# ---- Main ----

Write-BuildHeader "openSYDE Build System"
Write-Host "  Component : $Component" -ForegroundColor White
Write-Host "  Build Type: $BuildType" -ForegroundColor White
Write-Host "  Clean     : $Clean" -ForegroundColor White
Write-Host "  Deploy Qt : $(-not $SkipDeploy)" -ForegroundColor White

Test-Prerequisites

if ($Component -eq "All") {
    $components = @("GUI", "CANMonitor", "SYDEflash")
    $totalStart = Get-Date

    foreach ($comp in $components) {
        $config = $BuildConfigs[$comp]
        $tempFolder = "$($config.TempFolderBase)_$BuildType"
        Build-Component -ProjectFolder $config.ProjectFolder `
            -TempFolder $tempFolder `
            -ComponentName $config.Name `
            -Target $config.Target `
            -SkipInstallStep ($config.SkipInstall -eq $true) `
            -SkipDeployStep ($config.SkipDeploy -eq $true) `
            -ExeName $config.ExeName `
            -InstallDir $config.InstallDir
    }

    $totalTime = (Get-Date) - $totalStart
    Write-BuildHeader "All Components Built Successfully"
    Write-Host "Total time: $($totalTime.TotalMinutes.ToString('F1')) minutes" -ForegroundColor Green
}
else {
    $config = $BuildConfigs[$Component]
    $tempFolder = "$($config.TempFolderBase)_$BuildType"
    Build-Component -ProjectFolder $config.ProjectFolder `
        -TempFolder $tempFolder `
        -ComponentName $config.Name `
        -Target $config.Target `
        -SkipInstallStep ($config.SkipInstall -eq $true) `
        -SkipDeployStep ($config.SkipDeploy -eq $true) `
        -ExeName $config.ExeName `
        -InstallDir $config.InstallDir
}

Write-Host "`nBuild artifacts: opensyde_tool\result\" -ForegroundColor Cyan
