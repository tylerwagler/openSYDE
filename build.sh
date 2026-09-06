#!/bin/bash
# openSYDE Unified Build Script (Linux)
#
# Usage: ./build.sh [options] [tool ...]
#
# Tools:
#   opensyde        Main openSYDE GUI application
#   canmonitor      CAN Monitor application
#   sydeflash       SYDEflash application
#   sydesup         SYDEsup system updater
#   syde_x_gen      X-config generator
#   syde_coder_c    C code generator
#   flash_tool      Command-line flash tool
#   tsp_convert     V2-to-V3 Target Support Package converter
#   all             Build all tools (default)
#
# Options:
#   -b, --build-type <Release|Debug>   Build type (default: Release)
#   -c, --clean                        Clean build directory before building
#   -j, --jobs <N>                     Parallel jobs (default: nproc)
#   -d, --deploy                       Also deploy binaries to $INSTALL_DIR
#                                      (default: ~/.local/opt/openSYDE; override
#                                      via INSTALL_DIR env var)
#   -h, --help                         Show this help
#
# Examples:
#   ./build.sh                         # Build all tools (Release)
#   ./build.sh opensyde canmonitor     # Build only openSYDE and CAN Monitor
#   ./build.sh -b Debug sydesup        # Debug build of SYDEsup
#   ./build.sh -c all                  # Clean rebuild of everything
#   ./build.sh -d opensyde             # Build openSYDE and deploy it

set -euo pipefail

# --- Configuration ---
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$SCRIPT_DIR"

BUILD_TYPE="Release"
CLEAN=false
JOBS="$(nproc)"
DEPLOY=false
INSTALL_DIR="${INSTALL_DIR:-$HOME/.local/opt/openSYDE}"
TOOLS=()

# --- Helper functions ---
write_header() {
    echo ""
    echo "========================================"
    echo "  $1"
    echo "========================================"
}

write_step() {
    echo "[BUILD] $1"
}

write_error() {
    echo "[ERROR] $1" >&2
}

usage() {
    sed -n '2,/^$/{ s/^# \?//; p }' "$0"
    exit 0
}

# --- Parse arguments ---
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -b|--build-type) BUILD_TYPE="$2"; shift 2 ;;
        -c|--clean)      CLEAN=true; shift ;;
        -j|--jobs)       JOBS="$2"; shift 2 ;;
        -d|--deploy)     DEPLOY=true; shift ;;
        -h|--help)       usage ;;
        -*)              write_error "Unknown option: $1"; usage ;;
        *)               TOOLS+=("$1"); shift ;;
    esac
done

# Default to all if no tools specified
if [[ ${#TOOLS[@]} -eq 0 ]]; then
    TOOLS=("all")
fi

# --- Tool definitions ---
# Each tool: name|pjt_dir|toolchain|needs_qt|deploy_src|deploy_dst
#   deploy_src: path under result/$BUILD_TYPE/ of the built binary
#   deploy_dst: path under $INSTALL_DIR where it should land
TOOL_DEFS=(
    "opensyde|opensyde_tool/pjt/openSYDE|pjt/toolchains/linux_x64.cmake|yes|openSYDE/openSYDE|tool/openSYDE"
    "canmonitor|opensyde_can_monitor/pjt|pjt/toolchains/linux_x64.cmake|yes|openSYDE_CAN_Monitor/openSYDE_CAN_Monitor|tool/CAN_Monitor/openSYDE_CAN_Monitor"
    "sydeflash|opensyde_syde_flash/pjt|pjt/toolchains/linux_x64.cmake|yes|SYDEflash/SYDEflash|utilities/SYDEflash/SYDEflash"
    "sydesup|opensyde_syde_sup/pjt|opensyde_syde_sup/pjt/toolchain_ubuntu.cmake|no|SYDEsup/SYDEsup|utilities/SYDEsup/SYDEsup"
    "syde_x_gen|opensyde_syde_x_gen/pjt||no|syde_x_gen/syde_x_gen|connectors/syde_x_gen/syde_x_gen"
    "syde_coder_c|opensyde_syde_coder_c/pjt||no|syde_coder_c/osy_syde_coder_c|connectors/syde_coder_c/osy_syde_coder_c"
    "flash_tool|opensyde_cmd_line_flash_tool/pjt||no|cmd_line_flash_tool/osy_cmd_line_flash_tool|utilities/cmd_line_flash_tool/osy_cmd_line_flash_tool"
    "tsp_convert|opensyde_tsp_convert/pjt||no|tsp_convert/osy_tsp_convert|utilities/tsp_convert/osy_tsp_convert"
)

ALL_TOOL_NAMES=()
for def in "${TOOL_DEFS[@]}"; do
    IFS='|' read -r name _ _ _ _ _ <<< "$def"
    ALL_TOOL_NAMES+=("$name")
done

# --- Desktop entries (only for GUI tools that ship a logo) ---
# Each: tool_name|desktop_basename|display_name|icon_src|comment
#   desktop_basename: filename of the .desktop file (without extension), and
#                     also the value baked into setDesktopFileName() in main.cpp
DESKTOP_ENTRIES=(
    "opensyde|openSYDE|openSYDE|opensyde_tool/src/images/LogoOpensyde_XXL.png|System development and configuration tool"
    "canmonitor|openSYDE_CAN_Monitor|openSYDE CAN Monitor|opensyde_can_monitor/src/can_monitor/images/CAN_Monitor_logo.png|CAN bus traffic analysis"
    "sydeflash|SYDEflash|SYDEflash|opensyde_syde_flash/src/syde_flash/images/SYDEflash_logo.png|Firmware flashing tool"
)

# Expand "all"
RESOLVED_TOOLS=()
for t in "${TOOLS[@]}"; do
    t_lower="$(echo "$t" | tr '[:upper:]' '[:lower:]')"
    if [[ "$t_lower" == "all" ]]; then
        RESOLVED_TOOLS=("${ALL_TOOL_NAMES[@]}")
        break
    else
        RESOLVED_TOOLS+=("$t_lower")
    fi
done

# --- Detect Qt6 ---
find_qt6() {
    local arch lib_dir
    arch="$(uname -m)"
    case "$arch" in
        x86_64)  lib_dir="x86_64-linux-gnu" ;;
        aarch64) lib_dir="aarch64-linux-gnu" ;;
        *)       lib_dir="" ;;
    esac

    local candidates=()
    [[ -n "$lib_dir" ]] && candidates+=("/usr/lib/$lib_dir/cmake/Qt6")
    candidates+=(
        "/usr/lib/cmake/Qt6"
        "/usr/lib64/cmake/Qt6"
        "/usr/local/lib/cmake/Qt6"
    )

    for dir in "${candidates[@]}"; do
        if [[ -f "$dir/Qt6Config.cmake" ]]; then
            export Qt6_DIR="$dir"
            write_step "Qt6 found: $dir"
            return 0
        fi
    done
    write_step "Qt6 not found in known paths; relying on CMake to locate it"
    return 0
}

# --- Check prerequisites ---
check_prerequisites() {
    write_step "Checking prerequisites..."
    local missing=false

    for cmd in cmake ninja g++; do
        if ! command -v "$cmd" &>/dev/null; then
            write_error "$cmd not found"
            missing=true
        fi
    done

    if [[ "$missing" == "true" ]]; then
        exit 1
    fi

    echo "  CMake: $(cmake --version | head -1)"
    echo "  Ninja: $(ninja --version)"
    echo "  GCC:   $(g++ --version | head -1)"
}

# --- Build a single tool ---
build_tool() {
    local tool_name="$1"

    # Find tool definition
    local pjt_dir toolchain needs_qt deploy_src deploy_dst
    local found=false
    for def in "${TOOL_DEFS[@]}"; do
        IFS='|' read -r name pjt tc qt dsrc ddst <<< "$def"
        if [[ "$name" == "$tool_name" ]]; then
            pjt_dir="$pjt"
            toolchain="$tc"
            needs_qt="$qt"
            deploy_src="$dsrc"
            deploy_dst="$ddst"
            found=true
            break
        fi
    done

    if [[ "$found" != "true" ]]; then
        write_error "Unknown tool: $tool_name"
        write_error "Available: ${ALL_TOOL_NAMES[*]}"
        return 1
    fi

    local build_dir="$REPO_ROOT/build/$BUILD_TYPE/$tool_name"
    local source_dir="$REPO_ROOT/$pjt_dir"

    write_header "Building $tool_name ($BUILD_TYPE)"

    # Clean if requested
    if [[ "$CLEAN" == "true" ]] && [[ -d "$build_dir" ]]; then
        write_step "Cleaning $build_dir..."
        rm -rf "$build_dir"
    fi
    mkdir -p "$build_dir"

    # Configure
    if [[ ! -f "$build_dir/build.ninja" ]]; then
        write_step "Configuring..."
        local cmake_args=(
            -S "$source_dir" -B "$build_dir" -G Ninja
            "-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
        )
        if [[ -n "$toolchain" ]]; then
            cmake_args+=("-DCMAKE_TOOLCHAIN_FILE=$REPO_ROOT/$toolchain")
        fi
        if [[ "$needs_qt" == "yes" ]] && [[ -n "${Qt6_DIR:-}" ]]; then
            cmake_args+=("-DQt6_DIR=$Qt6_DIR")
        fi
        if ! cmake "${cmake_args[@]}"; then
            write_error "$tool_name: configuration failed"
            return 1
        fi
    else
        write_step "Using existing configuration (use -c to reconfigure)"
    fi

    # Build
    write_step "Building (jobs=$JOBS)..."
    if ! cmake --build "$build_dir" -j"$JOBS"; then
        write_error "$tool_name: build failed"
        return 1
    fi

    # Install
    write_step "Installing..."
    if ! cmake --install "$build_dir"; then
        write_error "$tool_name: install failed"
        return 1
    fi

    # Optional deploy to $INSTALL_DIR
    if [[ "$DEPLOY" == "true" ]]; then
        local src="$REPO_ROOT/result/$BUILD_TYPE/$deploy_src"
        local dst="$INSTALL_DIR/$deploy_dst"
        if [[ ! -f "$src" ]]; then
            write_error "Deploy source not found: $src"
            return 1
        fi
        write_step "Deploying to $dst"
        mkdir -p "$(dirname "$dst")"
        cp "$src" "$dst"
        deploy_desktop_entry "$tool_name" "$dst"
    fi

    write_step "$tool_name built successfully"
}

# --- Deploy a freedesktop .desktop file + icon for GUI tools ---
deploy_desktop_entry() {
    local tool_name="$1"
    local exec_path="$2"

    local entry desktop_basename display_name icon_src comment
    local found=false
    for entry in "${DESKTOP_ENTRIES[@]}"; do
        IFS='|' read -r ename dbase dname isrc cmt <<< "$entry"
        if [[ "$ename" == "$tool_name" ]]; then
            desktop_basename="$dbase"
            display_name="$dname"
            icon_src="$REPO_ROOT/$isrc"
            comment="$cmt"
            found=true
            break
        fi
    done
    [[ "$found" == "true" ]] || return 0  # Not a GUI tool with an icon

    if [[ ! -f "$icon_src" ]]; then
        write_error "Icon source not found: $icon_src"
        return 1
    fi

    local icon_dst="$(dirname "$exec_path")/$(basename "$icon_src")"
    local apps_dir="$HOME/.local/share/applications"
    local desktop_file="$apps_dir/$desktop_basename.desktop"

    write_step "Deploying icon to $icon_dst"
    cp "$icon_src" "$icon_dst"

    write_step "Writing desktop entry $desktop_file"
    mkdir -p "$apps_dir"
    cat > "$desktop_file" <<EOF
[Desktop Entry]
Type=Application
Version=1.0
Name=$display_name
Comment=$comment
Exec=$exec_path
Icon=$icon_dst
Terminal=false
Categories=Development;
EOF
}

# --- Main ---
write_header "openSYDE Build System"
echo "  Tools:      ${RESOLVED_TOOLS[*]}"
echo "  Build type: $BUILD_TYPE"
echo "  Clean:      $CLEAN"
echo "  Jobs:       $JOBS"
echo "  Deploy:     $DEPLOY"
if [[ "$DEPLOY" == "true" ]]; then
    echo "  Install:    $INSTALL_DIR"
fi

check_prerequisites

# Detect Qt6 if any GUI tool is being built
for t in "${RESOLVED_TOOLS[@]}"; do
    if [[ "$t" == "opensyde" || "$t" == "canmonitor" || "$t" == "sydeflash" ]]; then
        find_qt6
        break
    fi
done

# Build each tool
FAILED=()
for tool in "${RESOLVED_TOOLS[@]}"; do
    if ! build_tool "$tool"; then
        FAILED+=("$tool")
    fi
done

# Summary
write_header "Build Summary"
echo "  Build type: $BUILD_TYPE"
echo "  Results:    $REPO_ROOT/result/$BUILD_TYPE/"
if [[ "$DEPLOY" == "true" ]]; then
    echo "  Deployed:   $INSTALL_DIR"
fi
if [[ ${#FAILED[@]} -gt 0 ]]; then
    echo "  FAILED:     ${FAILED[*]}"
    exit 1
else
    echo "  Status:     All tools built successfully"
fi
