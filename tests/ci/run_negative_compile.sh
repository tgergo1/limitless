#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

CXX_BIN="${CXX_BIN:-c++}"
BUILD_DIR="$ROOT_DIR/build/ci/negative"
mkdir -p "$BUILD_DIR"

LOG="$BUILD_DIR/strict_legacy_symbol_fail.log"
OBJ="$BUILD_DIR/strict_legacy_symbol_fail.o"

if "$CXX_BIN" -std=c++11 -Wall -Wextra -Werror -pedantic -c tests/negative/strict_legacy_symbol_fail.cpp -o "$OBJ" >"$LOG" 2>&1; then
  echo "negative compile unexpectedly succeeded" >&2
  cat "$LOG" >&2
  exit 1
fi

if ! grep -Eq "(limitless_number|limitless_cpp_set_default_ctx|not declared|undeclared|identifier)" "$LOG"; then
  echo "negative compile failed, but expected symbol diagnostic not found" >&2
  cat "$LOG" >&2
  exit 1
fi

echo "negative compile checks passed"
