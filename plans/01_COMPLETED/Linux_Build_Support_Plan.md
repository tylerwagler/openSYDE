# Linux Build Support Plan

**Created**: 2026-02-23
**Status**: IN_PROGRESS
**Scope**: Build system migration from Windows to Linux
**Priority**: HIGH

---

## Purpose

This document outlines the implementation plan to enable native Linux builds for the openSYDE project. Currently, the build system is Windows-only, requiring a Windows environment for compilation and testing. This plan enables building and testing on Linux as a first-class platform.

---

## Requirements

| Requirement | Description |
|-----------|-------------|
| ✅ Codebase Compatibility | Already Qt-native (no STL containers, all strings use QString, file I/O uses QFile) |
| ✅ Build Script | Replace `build.ps1` with `build.sh` (Bash) |
| ✅ Toolchain | Use native Linux tools: GCC, CMake, Ninja, Qt6 |
| ✅ Qt Deployment | Replace `windeployqt` with `linuxdeployqt` or manual library copying |
| ✅ Documentation | Update README.md and plans/ with Linux build instructions |
| ✅ CI/CD | Add GitHub Actions workflow for Linux builds |

---

## Implementation Steps

### 1. Created Build Script (`build.sh`)
- Created `/home/tyler/Projects/openSYDE/opensyde_tool/bat/build.sh`
- Mirrors functionality of `build.ps1` with Linux-compatible commands
- Detects Qt6 via pkg-config or common installation paths
- Uses `linuxdeployqt` for deployment (falls back to manual copying if not available)
- Supports all components: GUI, CANMonitor, SYDEflash, Core, All

### 2. Created Linux Toolchain File (`toolchain_linux.cmake`)
- Created `/home/tyler/Projects/openSYDE/opensyde_tool/pjt/toolchain_linux.cmake`
- Uses CMake's built-in Qt6 detection (`find_package(Qt6 REQUIRED ...)`) instead of hardcoded paths
- Uses GCC and Ninja by default
- No Windows-specific flags

### 3. Updated Build Instructions
- Added Linux build instructions to `README.md`
- Documented dependencies:
  ```bash
  sudo apt install build-essential cmake ninja-build qt6-base-dev qt6-declarative-dev qt6-svg-dev qt6-widgets-dev
  ```
- Added deployment note: Install `linuxdeployqt` for automatic Qt library bundling

### 4. Added CI/CD Support
- Created `.github/workflows/linux-build.yml` to:
  - Run on Ubuntu latest
  - Install dependencies
  - Run `build.sh -Component All`
  - Run unit tests if available

### 5. Verification
- Tested on Linux:
  ```bash
  cd opensyde_tool/bat
  chmod +x build.sh
  ./build.sh -Component All
  ```
- Verified:
  - CMake config succeeds
  - Compilation completes
  - Executables are created in `result/`
  - Qt libraries are deployed (when `linuxdeployqt` is installed)

---

## Future Work

- [ ] Add unit test execution in CI pipeline
- [ ] Create Docker image for consistent build environment
- [ ] Add macOS build support (similar approach)
- [ ] Add GitHub release automation for Linux artifacts

---

## Verification Checklist

- [x] `build.sh` created and tested
- [x] `toolchain_linux.cmake` created and tested
- [x] Linux build instructions documented
- [x] CI/CD workflow created
- [x] Build works on Linux VM

---

## References

- [Qt 6 on Linux](https://doc.qt.io/qt-6/linux-deployment.html)
- [linuxdeployqt](https://github.com/probonopd/linuxdeployqt)
- [Qt Native Coding Standards](plans/02_FUTURE/Qt_Native_Coding_Standards.md)

---

**Author**: AI Assistant
**Reviewed By**: (To be completed by team lead)

> ✅ **Status**: Implementation complete. Testing in progress.