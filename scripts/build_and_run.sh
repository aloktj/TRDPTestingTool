#!/usr/bin/env bash
# Build and run TRDPTestingTool on Linux in a single step.
# Optional environment variables:
#   BUILD_DIR   - Directory for CMake build files (default: <repo>/build)
#   BUILD_TYPE  - CMake build type (default: Release)
#   TARGET      - CMake target to build (default: trdp_simulator)
#   SKIP_RUN    - If set to any value, the application will not be executed after building.

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-${ROOT_DIR}/build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
TARGET="${TARGET:-trdp_simulator}"

require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "[ERROR] Required command '$1' not found in PATH." >&2
        exit 1
    fi
}

require_command git
require_command cmake

printf "\n[INFO] Updating submodules...\n"
git -C "$ROOT_DIR" submodule update --init --recursive

printf "\n[INFO] Configuring CMake project (type=%s, dir=%s)...\n" "$BUILD_TYPE" "$BUILD_DIR"
cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

printf "\n[INFO] Building target '%s'...\n" "$TARGET"
cmake --build "$BUILD_DIR" --target "$TARGET"

if [[ -n "${SKIP_RUN:-}" ]]; then
    printf "\n[INFO] Build completed. Skipping run because SKIP_RUN is set.\n"
    exit 0
fi

APP_PATH="$BUILD_DIR/$TARGET"
if [[ ! -x "$APP_PATH" ]]; then
    echo "[ERROR] Built target not found at '$APP_PATH'." >&2
    exit 1
fi

printf "\n[INFO] Launching application...\n"
exec "$APP_PATH"
