#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

CLANGXX_BIN="${CLANGXX_BIN:-clang++}"
BUILD_DIR="$ROOT_DIR/build/fuzz-smoke"
mkdir -p "$BUILD_DIR"

COMMON_FLAGS=(
  -std=c++11
  -Wall
  -Wextra
  -Werror
  -pedantic
  -O1
  -fno-omit-frame-pointer
  -fsanitize=fuzzer,address,undefined
)

build_and_run() {
  local name="$1"
  local src="$2"
  local corpus="$3"
  local exe="$BUILD_DIR/$name"

  "$CLANGXX_BIN" "${COMMON_FLAGS[@]}" "$src" -o "$exe"
  "$exe" -runs=0 -max_total_time=20 "$corpus"
}

build_and_run fuzz_parse tests/fuzz/fuzz_parse.cpp tests/fuzz/corpus/parse
build_and_run fuzz_arith tests/fuzz/fuzz_arith.cpp tests/fuzz/corpus/arith
build_and_run fuzz_format tests/fuzz/fuzz_format.cpp tests/fuzz/corpus/format

echo "fuzz smoke checks passed"
