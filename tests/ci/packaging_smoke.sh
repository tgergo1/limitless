#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"
python3 tools/check_reserved_filenames.py

read_version_file() {
  if [[ -f "LIMITLESS_VERSION.txt" ]]; then
    tr -d '[:space:]' < "LIMITLESS_VERSION.txt"
  elif [[ -f "VERSION" ]]; then
    tr -d '[:space:]' < "VERSION"
  else
    echo "missing LIMITLESS_VERSION.txt (legacy fallback: VERSION)" >&2
    exit 1
  fi
}

VERSION_STR="$(read_version_file)"
CC_BIN="${CC_BIN:-cc}"
CXX_BIN="${CXX_BIN:-c++}"
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
project(limitless_consumer LANGUAGES C CXX)
find_package(Limitless CONFIG REQUIRED)

add_executable(consumer_c main.c limitless_impl.c)
target_link_libraries(consumer_c PRIVATE limitless::limitless)

add_executable(consumer_cpp main.cpp limitless_impl.cpp)
set_target_properties(consumer_cpp PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
target_link_libraries(consumer_cpp PRIVATE limitless::limitless)
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
cat > "$BUILD_ROOT/consumer-cmake/main.cpp" <<'CPP_EOF'
#include <string>
#include "limitless.hpp"

int main() {
  limitless::number a = limitless::number::parse("7/3", 10);
  limitless::number b = 2;
  limitless::number c = a + b;
  return c.str() == "13/3" ? 0 : 1;
}
CPP_EOF
cat > "$BUILD_ROOT/consumer-cmake/limitless_impl.cpp" <<'CPP_EOF'
#define LIMITLESS_IMPLEMENTATION
#include "limitless.hpp"
CPP_EOF
cmake -S "$BUILD_ROOT/consumer-cmake" -B "$BUILD_ROOT/consumer-cmake/build" -DCMAKE_PREFIX_PATH="$INSTALL_ROOT"
cmake --build "$BUILD_ROOT/consumer-cmake/build"
"$BUILD_ROOT/consumer-cmake/build/consumer_c"
"$BUILD_ROOT/consumer-cmake/build/consumer_cpp"

PKG_PATHS=()
[[ -d "$INSTALL_ROOT/lib/pkgconfig" ]] && PKG_PATHS+=("$INSTALL_ROOT/lib/pkgconfig")
[[ -d "$INSTALL_ROOT/lib64/pkgconfig" ]] && PKG_PATHS+=("$INSTALL_ROOT/lib64/pkgconfig")
PKG_CONFIG_PATH_VALUE="$(IFS=:; echo "${PKG_PATHS[*]:-}")"
export PKG_CONFIG_PATH="$PKG_CONFIG_PATH_VALUE"
pkg-config --cflags --libs limitless >/dev/null
PKG_CFLAGS=()
read -r -a PKG_CFLAGS <<< "$(pkg-config --cflags limitless)"

"$CC_BIN" -std=c99 -Wall -Wextra -Werror -pedantic \
  "${PKG_CFLAGS[@]}" \
  "$BUILD_ROOT/consumer-cmake/main.c" \
  "$BUILD_ROOT/consumer-cmake/limitless_impl.c" \
  -o "$BUILD_ROOT/pkg-config-consumer-c"
"$BUILD_ROOT/pkg-config-consumer-c"

"$CXX_BIN" -std=c++11 -Wall -Wextra -Werror -pedantic \
  "${PKG_CFLAGS[@]}" \
  "$BUILD_ROOT/consumer-cmake/main.cpp" \
  "$BUILD_ROOT/consumer-cmake/limitless_impl.cpp" \
  -o "$BUILD_ROOT/pkg-config-consumer-cpp"
"$BUILD_ROOT/pkg-config-consumer-cpp"

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
project(limitless_conan_consumer LANGUAGES C CXX)
find_package(Limitless CONFIG REQUIRED)

add_executable(consumer_c main.c limitless_impl.c)
target_link_libraries(consumer_c PRIVATE limitless::limitless)

add_executable(consumer_cpp main.cpp limitless_impl.cpp)
set_target_properties(consumer_cpp PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
target_link_libraries(consumer_cpp PRIVATE limitless::limitless)
EOF_CONAN_CMAKE
  cp "$BUILD_ROOT/consumer-cmake/main.c" "$BUILD_ROOT/consumer-conan/main.c"
  cp "$BUILD_ROOT/consumer-cmake/limitless_impl.c" "$BUILD_ROOT/consumer-conan/limitless_impl.c"
  cp "$BUILD_ROOT/consumer-cmake/main.cpp" "$BUILD_ROOT/consumer-conan/main.cpp"
  cp "$BUILD_ROOT/consumer-cmake/limitless_impl.cpp" "$BUILD_ROOT/consumer-conan/limitless_impl.cpp"

  conan install "$BUILD_ROOT/consumer-conan" --output-folder="$BUILD_ROOT/consumer-conan/build" --build=missing
  cmake -S "$BUILD_ROOT/consumer-conan" -B "$BUILD_ROOT/consumer-conan/build" -DCMAKE_TOOLCHAIN_FILE="$BUILD_ROOT/consumer-conan/build/conan_toolchain.cmake" -DCMAKE_BUILD_TYPE=Release
  cmake --build "$BUILD_ROOT/consumer-conan/build"
  "$BUILD_ROOT/consumer-conan/build/consumer_c"
  "$BUILD_ROOT/consumer-conan/build/consumer_cpp"
fi

if [[ -n "${VCPKG_ROOT:-}" && -x "${VCPKG_ROOT}/vcpkg" ]]; then
  if [[ -n "${VCPKG_DEFAULT_BINARY_CACHE:-}" ]]; then
    mkdir -p "${VCPKG_DEFAULT_BINARY_CACHE}"
  fi

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
  cat > "$BUILD_ROOT/consumer-vcpkg/CMakeLists.txt" <<'EOF_VCPKG_CMAKE'
cmake_minimum_required(VERSION 3.20)
project(limitless_vcpkg_consumer LANGUAGES C CXX)
find_package(Limitless CONFIG REQUIRED)

add_executable(consumer_c main.c limitless_impl.c)
target_link_libraries(consumer_c PRIVATE limitless::limitless)

add_executable(consumer_cpp main.cpp limitless_impl.cpp)
set_target_properties(consumer_cpp PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
target_link_libraries(consumer_cpp PRIVATE limitless::limitless)
EOF_VCPKG_CMAKE
  cp "$BUILD_ROOT/consumer-cmake/main.c" "$BUILD_ROOT/consumer-vcpkg/main.c"
  cp "$BUILD_ROOT/consumer-cmake/limitless_impl.c" "$BUILD_ROOT/consumer-vcpkg/limitless_impl.c"
  cp "$BUILD_ROOT/consumer-cmake/main.cpp" "$BUILD_ROOT/consumer-vcpkg/main.cpp"
  cp "$BUILD_ROOT/consumer-cmake/limitless_impl.cpp" "$BUILD_ROOT/consumer-vcpkg/limitless_impl.cpp"

  cmake -S "$BUILD_ROOT/consumer-vcpkg" -B "$BUILD_ROOT/consumer-vcpkg/build" \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DVCPKG_OVERLAY_PORTS="$ROOT_DIR/packaging/vcpkg/ports" \
    -DVCPKG_TARGET_TRIPLET="$VCPKG_TRIPLET"
  cmake --build "$BUILD_ROOT/consumer-vcpkg/build"
  "$BUILD_ROOT/consumer-vcpkg/build/consumer_c"
  "$BUILD_ROOT/consumer-vcpkg/build/consumer_cpp"
fi

echo "packaging smoke checks passed"
