#!/bin/bash
# openSYDE Build Script (Bash)
# Unified build script for all openSYDE components with Qt deployment on Linux.
#
# Usage: ./build.sh [-Component <name>] [-BuildType <Release|Debug>] [-Clean] [-SkipDeploy]
#
# Examples:
#   ./build.sh                              # Build main GUI (Release) + deploy Qt libraries
#   ./build.sh -Component CANMonitor        # Build CAN Monitor
#   ./build.sh -Component SYDEflash         # Build SYDEflash
#   ./build.sh -Component All               # Build all components
#   ./build.sh -Clean                       # Clean and rebuild
#   ./build.sh -BuildType Debug             # Debug build
#   ./build.sh -SkipDeploy                  # Build without deploying Qt libraries

set -euo pipefail

# Configuration
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TOOLCHAIN_DIR="$SCRIPT_DIR/../pjt"
RESULT_DIR="$SCRIPT_DIR/../result"

# Default values
COMPONENT="GUI"
BUILD_TYPE="Release"
CLEAN=false
SKIP_DEPLOY=false

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -Component)
            COMPONENT="$2"
            shift 2
            ;;
        -BuildType)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -Clean)
            CLEAN=true
            shift
            ;;
        -SkipDeploy)
            SKIP_DEPLOY=true
            shift
            ;;
        *)
            echo "Unknown parameter passed: $1" >&2
            exit 1
            ;;
    esac
done

# Validate component
CASED_COMPONENT=$(echo "$COMPONENT" | tr '[:lower:]' '[:upper:]')
if [[ "$CASED_COMPONENT" != "GUI" && "$CASED_COMPONENT" != "CORE" && "$CASED_COMPONENT" != "CANMONITOR" && "$CASED_COMPONENT" != "SYDEFLASH" && "$CASED_COMPONENT" != "ALL" ]]; then
    echo "Invalid component: $COMPONENT" >&2
    exit 1
fi

# Build configurations
CONFIGS=(
    "GUI:../pjt/openSYDE:../temp_openSYDE:openSYDE:./result/tool"
    "CORE:../pjt/openSYDE:../temp_openSYDE:opensyde_core:"
    "CANMONITOR:../pjt/openSYDE_CAN_Monitor:../temp_openSYDE_CAN_Monitor:openSYDE_CAN_Monitor:./result/tool/CAN_Monitor"
    "SYDEFLASH:../pjt/SYDEflash:../temp_SYDEflash:SYDEflash:./result/utilities/SYDEflash"
)

# Helper functions
write_header() {
    echo "========================================"
    echo "$1"
    echo "========================================"
}

write_step() {
    echo "[BUILD] $1"
}

write_error() {
    echo "[ERROR] $1" >&2
}

# Detect Qt installation
find_qt6() {
    # Try pkg-config first
    if command -v pkg-config &> /dev/null && pkg-config --exists Qt6Core; then
        QT_PREFIX="$(pkg-config --variable=prefix Qt6Core)"
        
        # Handle Ubuntu/Debian system Qt6 installation
        # Detect architecture
        ARCH="$(uname -m)"
        case "$ARCH" in
            x86_64)
                LIB_DIR="x86_64-linux-gnu"
                ;;
            aarch64|arm64)
                LIB_DIR="aarch64-linux-gnu"
                ;;
            *)
                echo "Unknown architecture: $ARCH" >&2
                exit 1
                ;;
        esac
        
        # Try architecture-specific path first
        QT6_CONFIG_PATH="$QT_PREFIX/lib/$LIB_DIR/cmake/Qt6/Qt6Config.cmake"
        if [[ -f "$QT6_CONFIG_PATH" ]]; then
            export Qt6_DIR="$(dirname "$QT6_CONFIG_PATH")"
            export QT_DIR="$QT_PREFIX"
            export PATH="$QT_PREFIX/bin:$PATH"
            echo "Qt6 found via pkg-config at: $QT_PREFIX (Config: $QT6_CONFIG_PATH)"
            return 0
        fi
        
        # Look for Qt6Config.cmake in standard locations
        for config_dir in "lib/cmake/Qt6" "share/cmake/Qt6"; do
            QT6_CONFIG_PATH="$QT_PREFIX/$config_dir/Qt6Config.cmake"
            if [[ -f "$QT6_CONFIG_PATH" ]]; then
                export Qt6_DIR="$(dirname "$QT6_CONFIG_PATH")"
                export QT_DIR="$QT_PREFIX"
                export PATH="$QT_PREFIX/bin:$PATH"
                echo "Qt6 found via pkg-config at: $QT_PREFIX (Config: $QT6_CONFIG_PATH)"
                return 0
            fi
        done
        
        # If not found in standard paths, try the lib directory directly
        if [[ -d "$QT_PREFIX/lib/cmake/Qt6" ]]; then
            export Qt6_DIR="$QT_PREFIX/lib/cmake/Qt6"
            export QT_DIR="$QT_PREFIX"
            export PATH="$QT_PREFIX/bin:$PATH"
            echo "Qt6 found via pkg-config at: $QT_PREFIX (Config: $QT_PREFIX/lib/cmake/Qt6/Qt6Config.cmake)"
            return 0
        fi
        
        echo "Qt6 found via pkg-config but Qt6Config.cmake not found in $QT_PREFIX. Check installation." >&2
        exit 1
    fi
    
    # Check common Qt6 installation paths
    for qt_root in /opt/Qt/6.10.1/gcc_64 /usr/lib/x86_64-linux-gnu/qt6 /usr/local/Qt6; do
        if [[ -f "$qt_root/bin/qmake" ]] || [[ -f "$qt_root/bin/cmake" ]]; then
            # Look for Qt6Config.cmake
            for config_dir in "lib/cmake/Qt6" "share/cmake/Qt6"; do
                QT6_CONFIG_PATH="$qt_root/$config_dir/Qt6Config.cmake"
                if [[ -f "$QT6_CONFIG_PATH" ]]; then
                    export Qt6_DIR="$(dirname "$QT6_CONFIG_PATH")"
                    export QT_DIR="$qt_root"
                    export PATH="$qt_root/bin:$PATH"
                    echo "Qt6 found at: $qt_root (Config: $QT6_CONFIG_PATH)"
                    return 0
                fi
            done
            
            # Fall back to just the root if no config found (less reliable)
            export QT_DIR="$qt_root"
            export PATH="$qt_root/bin:$PATH"
            echo "Qt6 found at: $qt_root (using fallback)"
            return 0
        fi
    done
    
    # Try to find cmake in PATH
    if command -v cmake &> /dev/null; then
        # Use cmake to find Qt6 path
        if cmake --find-package Qt6 --name=Qt6Core --mode=compile 2>/dev/null; then
            # Extract the path from CMake's output
            CMAKE_QT_PATH=$(cmake --find-package Qt6 --name=Qt6Core --mode=compile 2>/dev/null | grep "Found Qt6Core" | sed -n 's/.*found at \(.*\)/\1/p')
            if [[ -n "$CMAKE_QT_PATH" ]] && [[ -d "$CMAKE_QT_PATH" ]]; then
                # Look for Qt6Config.cmake in the found path
                for config_dir in "lib/cmake/Qt6" "share/cmake/Qt6"; do
                    QT6_CONFIG_PATH="$CMAKE_QT_PATH/$config_dir/Qt6Config.cmake"
                    if [[ -f "$QT6_CONFIG_PATH" ]]; then
                        export Qt6_DIR="$(dirname "$QT6_CONFIG_PATH")"
                        export QT_DIR="$CMAKE_QT_PATH"
                        export PATH="$CMAKE_QT_PATH/bin:$PATH"
                        echo "Qt6 found via CMake find_package at: $CMAKE_QT_PATH (Config: $QT6_CONFIG_PATH)"
                        return 0
                    fi
                done
                
                # Fall back
                export Qt6_DIR="$CMAKE_QT_PATH"
                export QT_DIR="$CMAKE_QT_PATH"
                export PATH="$CMAKE_QT_PATH/bin:$PATH"
                echo "Qt6 found via CMake find_package at: $CMAKE_QT_PATH (using fallback)"
                return 0
            fi
        fi
    fi
    
    # If we get here, Qt6 was not found
    echo "Qt6 not found. Please install Qt 6.10.1 for Linux (e.g., from qt.io)" >&2
    exit 1
}

# Verify toolchain
verify_toolchain() {
    write_step "Checking prerequisites..."
    
    # Check CMake
    if ! command -v cmake &> /dev/null; then
        write_error "CMake not found. Install with: sudo apt install cmake"
        exit 1
    fi
    
    # Check Ninja
    if ! command -v ninja &> /dev/null; then
        write_error "Ninja not found. Install with: sudo apt install ninja-build"
        exit 1
    fi
    
    # Check GCC
    if ! command -v g++ &> /dev/null; then
        write_error "g++ not found. Install with: sudo apt install build-essential"
        exit 1
    fi
    
    # Find Qt6
    find_qt6
    
    # Check deployment tool if needed
    if [[ "$SKIP_DEPLOY" == "false" ]] && [[ "$COMPONENT" != "CORE" ]]; then
        if ! command -v linuxdeployqt &> /dev/null; then
            echo "linuxdeployqt not found. Deploying Qt libraries will be skipped. Install with:"
            echo "wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
            echo "chmod +x linuxdeployqt-continuous-x86_64.AppImage"
            echo "./linuxdeployqt-continuous-x86_64.AppImage --appimage-extract"
            echo "export PATH=\$PATH:\$HOME/squashfs-root/usr/bin"
        fi
    fi
    
    echo "  CMake      : $(command -v cmake)"
    echo "  Ninja      : $(command -v ninja)"
    echo "  GCC        : $(command -v g++)"
    echo "  Qt6        : $(which qmake 2>/dev/null || echo \"detected via CMake\")"
}

# Deploy Qt libraries
deploy_qt_libs() {
    local exe_path="$1"
    local component_name="$2"
    
    write_step "Deploying Qt libraries for $component_name..."
    
    if [[ ! -f "$exe_path" ]]; then
        write_error "Executable not found: $exe_path"
        exit 1
    fi
    
    local deploy_dir="$(dirname "$exe_path")"
    
    # Check if linuxdeployqt is available
    if command -v linuxdeployqt &> /dev/null; then
        write_step "Using linuxdeployqt to deploy Qt libraries..."
        linuxdeployqt "$exe_path" -appimage -always-overwrite -verbose=2
    else
        write_step "linuxdeployqt not available. Manually copying Qt libraries may be required."
        write_step "Run: ldd \"$exe_path\" | grep Qt | cut -d'>' -f2 | xargs -I {} cp {} \"$deploy_dir\""
        echo "  Note: Libraries must be copied manually to make the executable portable."
    fi
}

# Build component
build_component() {
    local project_folder="$1"
    local temp_folder="$2"
    local component_name="$3"
    local target="$4"
    local skip_install="$5"
    local skip_deploy="$6"
    local exe_name="$7"
    local install_dir="$8"
    
    write_header "Building $component_name ($BUILD_TYPE)"
    
    # Setup environment
    export PATH="$QT_DIR/bin:$PATH"
    
    # Create temp folder if needed
    if [[ ! -d "$temp_folder" ]]; then
        write_step "Creating build directory: $temp_folder"
        mkdir -p "$temp_folder"
    fi
    
    # Clean if requested
    if [[ "$CLEAN" == "true" ]] && [[ -f "$temp_folder/build.ninja" ]]; then
        write_step "Cleaning previous build..."
        rm -rf "$temp_folder"/*
    fi
    
    pushd "$temp_folder" > /dev/null
    
    # Step 1: CMake configure
    if [[ ! -f "build.ninja" ]]; then
        write_step "Step 1/3 - CMake configure..."
        cmake -S "$project_folder" -B . -G Ninja "-DCMAKE_BUILD_TYPE=$BUILD_TYPE" -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_DIR/toolchain_linux.cmake" -DQt6_DIR="$Qt6_DIR"
        if [[ $? -ne 0 ]]; then
            write_error "CMake configure failed"
            popd > /dev/null
            exit 1
        fi
    else
        write_step "Step 1/3 - Using existing CMake configuration (use -Clean to reconfigure)"
    fi
    
    # Step 2: Build
    write_step "Step 2/3 - Building target '$target' (parallel jobs: 8)..."
    cmake --build . --target "$target" -- -j8
    if [[ $? -ne 0 ]]; then
        write_error "Build failed"
        popd > /dev/null
        exit 1
    fi
    
    # Step 3: Install
    if [[ "$skip_install" != "true" ]]; then
        write_step "Step 3/3 - Installing to result folder..."
        cmake --build . --target install
        if [[ $? -ne 0 ]]; then
            write_error "Install failed"
            popd > /dev/null
            exit 1
        fi
    else
        write_step "Step 3/3 - Install skipped (library-only build)"
    fi
    
    echo "\n  $component_name build SUCCESS"
    
    # Deploy Qt libraries if needed
    if [[ "$skip_deploy" != "true" ]] && [[ "$SKIP_DEPLOY" != "true" ]] && [[ -n "$exe_name" ]] && [[ -n "$install_dir" ]]; then
        local exe_full_path="$SCRIPT_DIR/../$install_dir/$exe_name"
        deploy_qt_libs "$exe_full_path" "$component_name"
    fi
    
    popd > /dev/null
}

# Main
write_header "openSYDE Build System"
echo "  Component : $COMPONENT"
echo "  Build Type: $BUILD_TYPE"
echo "  Clean     : $CLEAN"
echo "  Deploy Qt : $([[ "$SKIP_DEPLOY" == "true" ]] && echo "No" || echo "Yes")"

verify_toolchain

if [[ "$COMPONENT" == "All" ]]; then
    write_header "Building All Components"
    for config in "${CONFIGS[@]}"; do
        IFS=':' read -r comp proj temp name install <<< "$config"
        if [[ "$comp" != "CORE" ]]; then
            build_component "$proj" "$temp" "$name" "all" "false" "false" "$name" "$install"
        fi
    done
    write_header "All Components Built Successfully"
else
    for config in "${CONFIGS[@]}"; do
        IFS=':' read -r comp proj temp name install <<< "$config"
        if [[ "$(echo "$comp" | tr '[:upper:]' '[:lower:]')" == "$(echo "$COMPONENT" | tr '[:upper:]' '[:lower:]')" ]]; then
            if [[ "$comp" == "CORE" ]]; then
                build_component "$proj" "$temp" "$name" "$name" "true" "true" "" ""
            else
                build_component "$proj" "$temp" "$name" "all" "false" "$SKIP_DEPLOY" "$name" "$install"
            fi
            break
        fi
    done
fi

write_header "Build artifacts: opensyde_tool/result/"
