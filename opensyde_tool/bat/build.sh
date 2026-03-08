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

# Resolve directories relative to this script
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
TOOLCHAIN_DIR="$PROJECT_DIR/pjt"
RESULT_DIR="$PROJECT_DIR/result"

# Default values
COMPONENT="GUI"
BUILD_TYPE="Release"
CLEAN=false
SKIP_DEPLOY=false

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

usage() {
    echo "Usage: $0 [-Component <GUI|Core|CANMonitor|SYDEflash|All>] [-BuildType <Release|Debug>] [-Clean] [-SkipDeploy]"
    exit 0
}

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
        -h|--help)
            usage
            ;;
        *)
            write_error "Unknown parameter: $1"
            usage
            ;;
    esac
done

# Normalize component name to uppercase for matching
COMPONENT_UPPER="$(echo "$COMPONENT" | tr '[:lower:]' '[:upper:]')"

# Validate component
case "$COMPONENT_UPPER" in
    GUI|CORE|CANMONITOR|SYDEFLASH|ALL) ;;
    *)
        write_error "Invalid component: $COMPONENT"
        write_error "Valid components: GUI, Core, CANMonitor, SYDEflash, All"
        exit 1
        ;;
esac

# Build configurations: COMPONENT:PROJECT_SUBDIR:TEMP_FOLDER:TARGET_NAME:INSTALL_SUBDIR
CONFIGS=(
    "GUI:pjt/openSYDE:temp_openSYDE:openSYDE:result/tool"
    "CORE:pjt/openSYDE:temp_openSYDE:opensyde_core:"
    "CANMONITOR:pjt/openSYDE_CAN_Monitor:temp_openSYDE_CAN_Monitor:openSYDE_CAN_Monitor:result/tool/CAN_Monitor"
    "SYDEFLASH:pjt/SYDEflash:temp_SYDEflash:SYDEflash:result/utilities/SYDEflash"
)

# Detect Qt6 installation and set Qt6_DIR
find_qt6() {
    local arch
    arch="$(uname -m)"

    # Architecture-specific library directory (Debian/Ubuntu convention)
    local lib_dir
    case "$arch" in
        x86_64)  lib_dir="x86_64-linux-gnu" ;;
        aarch64) lib_dir="aarch64-linux-gnu" ;;
        *)       lib_dir="" ;;
    esac

    # Search candidate directories for Qt6Config.cmake
    local candidates=()
    [[ -n "$lib_dir" ]] && candidates+=("/usr/lib/$lib_dir/cmake/Qt6")
    candidates+=(
        "/usr/lib/cmake/Qt6"
        "/usr/local/lib/cmake/Qt6"
        "/opt/Qt/6.10.1/gcc_64/lib/cmake/Qt6"
    )

    for dir in "${candidates[@]}"; do
        if [[ -f "$dir/Qt6Config.cmake" ]]; then
            export Qt6_DIR="$dir"
            write_step "Qt6 found: $dir"
            return 0
        fi
    done

    # Fall back: let CMake try to find it on its own
    write_step "Qt6Config.cmake not found in known paths; relying on CMake to locate Qt6"
    return 0
}

# Verify build prerequisites
verify_toolchain() {
    write_step "Checking prerequisites..."

    local missing=false

    if ! command -v cmake &> /dev/null; then
        write_error "CMake not found. Install with: sudo apt install cmake"
        missing=true
    fi

    if ! command -v ninja &> /dev/null; then
        write_error "Ninja not found. Install with: sudo apt install ninja-build"
        missing=true
    fi

    if ! command -v g++ &> /dev/null; then
        write_error "g++ not found. Install with: sudo apt install build-essential"
        missing=true
    fi

    if [[ "$missing" == "true" ]]; then
        exit 1
    fi

    find_qt6

    echo "  CMake : $(cmake --version | head -1)"
    echo "  Ninja : $(ninja --version)"
    echo "  GCC   : $(g++ --version | head -1)"
}

# Deploy Qt libraries alongside an executable
deploy_qt_libs() {
    local exe_path="$1"
    local component_name="$2"

    if [[ ! -f "$exe_path" ]]; then
        write_error "Executable not found for deployment: $exe_path"
        return 1
    fi

    write_step "Deploying Qt libraries for $component_name..."

    if command -v linuxdeployqt &> /dev/null; then
        linuxdeployqt "$exe_path" -always-overwrite -verbose=2
    else
        write_step "linuxdeployqt not available — skipping Qt deployment."
        write_step "To make the executable portable, install linuxdeployqt or copy Qt libraries manually."
    fi
}

# Build a single component
build_component() {
    local project_folder="$PROJECT_DIR/$1"
    local temp_folder="$PROJECT_DIR/$2_${BUILD_TYPE}"
    local component_name="$3"
    local target="$4"
    local skip_install="$5"
    local skip_deploy="$6"
    local install_dir="$7"

    write_header "Building $component_name ($BUILD_TYPE)"

    # Create or clean build directory
    if [[ "$CLEAN" == "true" ]] && [[ -d "$temp_folder" ]]; then
        write_step "Cleaning previous build..."
        rm -rf "$temp_folder"
    fi
    mkdir -p "$temp_folder"

    pushd "$temp_folder" > /dev/null

    # Step 1: CMake configure
    if [[ ! -f "build.ninja" ]]; then
        write_step "Step 1/3 - CMake configure..."
        local cmake_args=(
            -S "$project_folder" -B . -G Ninja
            "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
            "-DCMAKE_TOOLCHAIN_FILE=$TOOLCHAIN_DIR/toolchain_linux.cmake"
        )
        [[ -n "${Qt6_DIR:-}" ]] && cmake_args+=("-DQt6_DIR=$Qt6_DIR")
        cmake "${cmake_args[@]}"
    else
        write_step "Step 1/3 - Using existing CMake configuration (use -Clean to reconfigure)"
    fi

    # Step 2: Build
    write_step "Step 2/3 - Building target '$target'..."
    cmake --build . --target "$target" -- -j"$(nproc)"

    # Step 3: Install
    if [[ "$skip_install" != "true" ]]; then
        write_step "Step 3/3 - Installing to result folder..."
        cmake --build . --target install
    else
        write_step "Step 3/3 - Install skipped (library-only build)"
    fi

    echo ""
    echo "  $component_name build SUCCESS"

    popd > /dev/null

    # Deploy Qt libraries if needed
    if [[ "$skip_deploy" != "true" ]] && [[ "$SKIP_DEPLOY" != "true" ]] && [[ -n "$install_dir" ]]; then
        local exe_path="$PROJECT_DIR/$install_dir/$component_name"
        deploy_qt_libs "$exe_path" "$component_name"
    fi
}

# Main
write_header "openSYDE Build System"
echo "  Component : $COMPONENT"
echo "  Build Type: $BUILD_TYPE"
echo "  Clean     : $CLEAN"
echo "  Deploy Qt : $([[ "$SKIP_DEPLOY" == "true" ]] && echo "No" || echo "Yes")"

verify_toolchain

# Dispatch builds
run_build() {
    local comp proj temp name install
    IFS=':' read -r comp proj temp name install <<< "$1"

    if [[ "$comp" == "CORE" ]]; then
        build_component "$proj" "$temp" "$name" "$name" "true" "true" ""
    else
        build_component "$proj" "$temp" "$name" "all" "false" "$SKIP_DEPLOY" "$install"
    fi
}

if [[ "$COMPONENT_UPPER" == "ALL" ]]; then
    write_header "Building All Components"
    for config in "${CONFIGS[@]}"; do
        IFS=':' read -r comp _ _ _ _ <<< "$config"
        [[ "$comp" == "CORE" ]] && continue
        run_build "$config"
    done
    write_header "All Components Built Successfully"
else
    for config in "${CONFIGS[@]}"; do
        IFS=':' read -r comp _ _ _ _ <<< "$config"
        if [[ "$comp" == "$COMPONENT_UPPER" ]]; then
            run_build "$config"
            break
        fi
    done
fi

write_header "Build artifacts: $RESULT_DIR/"
