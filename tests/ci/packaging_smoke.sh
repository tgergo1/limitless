#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

VERSION_STR="$(tr -d '[:space:]' < VERSION)"
BUILD_ROOT="$ROOT_DIR/build/packaging"
INSTALL_ROOT="$BUILD_ROOT/install"
rm -rf "$BUILD_ROOT"
mkdir -p "$BUILD_ROOT"

python3 tools/check_version.py

cmake -S . -B "$BUILD_ROOT/cmake-build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$INSTALL_ROOT"
cmake --install "$BUILD_ROOT/cmake-build"

mkdir -p "$BUILD_ROOT/consumer-cmake"
cat > "$BUILD_ROOT/consumer-cmake/CMakeLists.txt" <<'CMAKE_EOF'
cmake_minimum_required(VERSION 3.20)
project(limitless_consumer_c C)
find_package(Limitless CONFIG REQUIRED)
add_executable(consumer main.c limitless_impl.c)
target_link_libraries(consumer PRIVATE limitless::limitless)
CMAKE_EOF
cat > "$BUILD_ROOT/consumer-cmake/main.c" <<'C_EOF'
#include "limitless.h"
int main(void) {
  limitless_ctx ctx;
  return limitless_ctx_init_default(&ctx) == LIMITLESS_OK ? 0 : 1;
}
C_EOF
cat > "$BUILD_ROOT/consumer-cmake/limitless_impl.c" <<'C_EOF'
#define LIMITLESS_IMPLEMENTATION
#include "limitless.h"
C_EOF
cmake -S "$BUILD_ROOT/consumer-cmake" -B "$BUILD_ROOT/consumer-cmake/build" -DCMAKE_PREFIX_PATH="$INSTALL_ROOT"
cmake --build "$BUILD_ROOT/consumer-cmake/build"
"$BUILD_ROOT/consumer-cmake/build/consumer"

PKG_PATHS=()
[[ -d "$INSTALL_ROOT/lib/pkgconfig" ]] && PKG_PATHS+=("$INSTALL_ROOT/lib/pkgconfig")
[[ -d "$INSTALL_ROOT/lib64/pkgconfig" ]] && PKG_PATHS+=("$INSTALL_ROOT/lib64/pkgconfig")
export PKG_CONFIG_PATH="$(IFS=:; echo "${PKG_PATHS[*]:-}")"
pkg-config --cflags --libs limitless >/dev/null

gcc -std=c99 -Wall -Wextra -Werror -pedantic \
  $(pkg-config --cflags limitless) \
  "$BUILD_ROOT/consumer-cmake/main.c" \
  "$BUILD_ROOT/consumer-cmake/limitless_impl.c" \
  -o "$BUILD_ROOT/pkg-config-consumer"
"$BUILD_ROOT/pkg-config-consumer"

if command -v conan >/dev/null 2>&1; then
  conan profile detect --force
  conan create . --version "$VERSION_STR" --build=missing

  mkdir -p "$BUILD_ROOT/consumer-conan"
  cat > "$BUILD_ROOT/consumer-conan/conanfile.txt" <<EOF_CONAN
[requires]
limitless/$VERSION_STR

[generators]
CMakeDeps
CMakeToolchain
EOF_CONAN
  cat > "$BUILD_ROOT/consumer-conan/CMakeLists.txt" <<'EOF_CONAN_CMAKE'
cmake_minimum_required(VERSION 3.20)
project(limitless_conan_consumer C)
find_package(Limitless CONFIG REQUIRED)
add_executable(consumer main.c limitless_impl.c)
target_link_libraries(consumer PRIVATE limitless::limitless)
EOF_CONAN_CMAKE
  cp "$BUILD_ROOT/consumer-cmake/main.c" "$BUILD_ROOT/consumer-conan/main.c"
  cp "$BUILD_ROOT/consumer-cmake/limitless_impl.c" "$BUILD_ROOT/consumer-conan/limitless_impl.c"

  conan install "$BUILD_ROOT/consumer-conan" --output-folder="$BUILD_ROOT/consumer-conan/build" --build=missing
  cmake -S "$BUILD_ROOT/consumer-conan" -B "$BUILD_ROOT/consumer-conan/build" -DCMAKE_TOOLCHAIN_FILE="$BUILD_ROOT/consumer-conan/build/conan_toolchain.cmake" -DCMAKE_BUILD_TYPE=Release
  cmake --build "$BUILD_ROOT/consumer-conan/build"
  "$BUILD_ROOT/consumer-conan/build/consumer"
fi

if [[ -n "${VCPKG_ROOT:-}" && -x "${VCPKG_ROOT}/vcpkg" ]]; then
  case "$(uname -s)" in
    Linux)
      VCPKG_TRIPLET="${VCPKG_TRIPLET:-x64-linux}"
      ;;
    Darwin)
      if [[ "$(uname -m)" == "arm64" ]]; then
        VCPKG_TRIPLET="${VCPKG_TRIPLET:-arm64-osx}"
      else
        VCPKG_TRIPLET="${VCPKG_TRIPLET:-x64-osx}"
      fi
      ;;
    *)
      VCPKG_TRIPLET="${VCPKG_TRIPLET:-x64-linux}"
      ;;
  esac

  "${VCPKG_ROOT}/vcpkg" install limitless --overlay-ports="$ROOT_DIR/packaging/vcpkg/ports" --triplet="$VCPKG_TRIPLET"

  mkdir -p "$BUILD_ROOT/consumer-vcpkg"
  cat > "$BUILD_ROOT/consumer-vcpkg/vcpkg.json" <<'EOF_VCPKG'
{
  "name": "limitless-vcpkg-consumer",
  "version-string": "0.0.1",
  "dependencies": ["limitless"]
}
EOF_VCPKG
  cp "$BUILD_ROOT/consumer-cmake/CMakeLists.txt" "$BUILD_ROOT/consumer-vcpkg/CMakeLists.txt"
  cp "$BUILD_ROOT/consumer-cmake/main.c" "$BUILD_ROOT/consumer-vcpkg/main.c"
  cp "$BUILD_ROOT/consumer-cmake/limitless_impl.c" "$BUILD_ROOT/consumer-vcpkg/limitless_impl.c"

  cmake -S "$BUILD_ROOT/consumer-vcpkg" -B "$BUILD_ROOT/consumer-vcpkg/build" \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_OVERLAY_PORTS="$ROOT_DIR/packaging/vcpkg/ports" \
    -DVCPKG_TARGET_TRIPLET="$VCPKG_TRIPLET"
  cmake --build "$BUILD_ROOT/consumer-vcpkg/build"
  "$BUILD_ROOT/consumer-vcpkg/build/consumer"
fi

echo "packaging smoke checks passed"
