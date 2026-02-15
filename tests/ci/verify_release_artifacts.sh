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

DIST_DIR="${1:-dist}"
VERSION_STR="${2:-$(read_version_file)}"

SBOM_PATH="$DIST_DIR/limitless-${VERSION_STR}.spdx.json"
INSTALL_ARCHIVE="$DIST_DIR/limitless-${VERSION_STR}-install.tar.gz"

if [[ ! -f "$DIST_DIR/SHA256SUMS" ]]; then
  echo "missing checksum file: $DIST_DIR/SHA256SUMS" >&2
  exit 1
fi
if [[ ! -f "$SBOM_PATH" ]]; then
  echo "missing sbom: $SBOM_PATH" >&2
  exit 1
fi
if [[ ! -f "$INSTALL_ARCHIVE" ]]; then
  echo "missing install archive: $INSTALL_ARCHIVE" >&2
  exit 1
fi

(
  cd "$DIST_DIR"
  sha256sum -c SHA256SUMS
)

python3 tools/validate_spdx_sbom.py "$SBOM_PATH"

VERIFY_ROOT="$ROOT_DIR/build/release-artifact-verify"
rm -rf "$VERIFY_ROOT"
mkdir -p "$VERIFY_ROOT/install-root"
CXX_BIN="${CXX_BIN:-c++}"
CC_BIN="${CC_BIN:-cc}"

tar -C "$VERIFY_ROOT/install-root" -xzf "$INSTALL_ARCHIVE"

cat > "$VERIFY_ROOT/CMakeLists.txt" <<'CMAKE_EOF'
cmake_minimum_required(VERSION 3.20)
project(limitless_release_archive_consumer LANGUAGES C CXX)
find_package(Limitless CONFIG REQUIRED)
add_executable(consumer_c main.c limitless_impl.c)
target_link_libraries(consumer_c PRIVATE limitless::limitless)

add_executable(consumer_cpp main.cpp limitless_impl.cpp)
set_target_properties(consumer_cpp PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON)
target_link_libraries(consumer_cpp PRIVATE limitless::limitless)
CMAKE_EOF

cat > "$VERIFY_ROOT/main.c" <<'C_EOF'
#include "limitless.h"
#include <stddef.h>
int main(void) {
  limitless_ctx ctx;
  limitless_number n;
  char buf[16];
  if (limitless_ctx_init_default(&ctx) != LIMITLESS_OK) return 1;
  if (limitless_number_init(&ctx, &n) != LIMITLESS_OK) return 1;
  if (limitless_number_from_str(&ctx, &n, "9/4") != LIMITLESS_OK) return 1;
  if (limitless_number_to_str(&ctx, &n, buf, sizeof(buf), NULL) != LIMITLESS_OK) return 1;
  limitless_number_clear(&ctx, &n);
  return buf[0] == '9' ? 0 : 1;
}
C_EOF

cat > "$VERIFY_ROOT/limitless_impl.c" <<'C_EOF'
#define LIMITLESS_IMPLEMENTATION
#include "limitless.h"
C_EOF

cat > "$VERIFY_ROOT/main.cpp" <<'CPP_EOF'
#include <string>
#include "limitless.hpp"

int main() {
  limitless::number a = limitless::number::parse("7/3", 10);
  limitless::number b = 2;
  limitless::number c = a + b;
  return c.str() == "13/3" ? 0 : 1;
}
CPP_EOF

cat > "$VERIFY_ROOT/limitless_impl.cpp" <<'CPP_EOF'
#define LIMITLESS_IMPLEMENTATION
#include "limitless.hpp"
CPP_EOF

[[ -f "$VERIFY_ROOT/install-root/include/limitless.h" ]]
[[ -f "$VERIFY_ROOT/install-root/include/limitless.hpp" ]]
[[ -f "$VERIFY_ROOT/install-root/share/limitless/LICENSE" ]]
[[ -f "$VERIFY_ROOT/install-root/share/limitless/README.md" ]]
[[ -f "$VERIFY_ROOT/install-root/share/limitless/LIMITLESS_VERSION.txt" ]]

cmake -S "$VERIFY_ROOT" -B "$VERIFY_ROOT/build" -DCMAKE_PREFIX_PATH="$VERIFY_ROOT/install-root"
cmake --build "$VERIFY_ROOT/build"
"$VERIFY_ROOT/build/consumer_c"
"$VERIFY_ROOT/build/consumer_cpp"

PKG_PATHS=()
[[ -d "$VERIFY_ROOT/install-root/lib/pkgconfig" ]] && PKG_PATHS+=("$VERIFY_ROOT/install-root/lib/pkgconfig")
[[ -d "$VERIFY_ROOT/install-root/lib64/pkgconfig" ]] && PKG_PATHS+=("$VERIFY_ROOT/install-root/lib64/pkgconfig")
export PKG_CONFIG_PATH="$(IFS=:; echo "${PKG_PATHS[*]:-}")"

if [[ -n "$PKG_CONFIG_PATH" && -x "$(command -v pkg-config || true)" ]]; then
  "$CC_BIN" -std=c99 -Wall -Wextra -Werror -pedantic \
    $(pkg-config --cflags limitless) \
    "$VERIFY_ROOT/main.c" \
    "$VERIFY_ROOT/limitless_impl.c" \
    -o "$VERIFY_ROOT/pkg_consumer_c"
  "$VERIFY_ROOT/pkg_consumer_c"

  "$CXX_BIN" -std=c++11 -Wall -Wextra -Werror -pedantic \
    $(pkg-config --cflags limitless) \
    "$VERIFY_ROOT/main.cpp" \
    "$VERIFY_ROOT/limitless_impl.cpp" \
    -o "$VERIFY_ROOT/pkg_consumer_cpp"
  "$VERIFY_ROOT/pkg_consumer_cpp"
fi

echo "release artifact verification passed"
