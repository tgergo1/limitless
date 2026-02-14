#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

DIST_DIR="${1:-dist}"
VERSION_STR="${2:-$(tr -d '[:space:]' < VERSION)}"

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

tar -C "$VERIFY_ROOT/install-root" -xzf "$INSTALL_ARCHIVE"

cat > "$VERIFY_ROOT/CMakeLists.txt" <<'CMAKE_EOF'
cmake_minimum_required(VERSION 3.20)
project(limitless_release_archive_consumer C)
find_package(Limitless CONFIG REQUIRED)
add_executable(consumer main.c limitless_impl.c)
target_link_libraries(consumer PRIVATE limitless::limitless)
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

cmake -S "$VERIFY_ROOT" -B "$VERIFY_ROOT/build" -DCMAKE_PREFIX_PATH="$VERIFY_ROOT/install-root"
cmake --build "$VERIFY_ROOT/build"
"$VERIFY_ROOT/build/consumer"

echo "release artifact verification passed"
