#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

CC_BIN="${CC_BIN:-gcc}"
CXX_BIN="${CXX_BIN:-g++}"
BUILD_DIR="$ROOT_DIR/build/coverage"
DIFF_ITERS="${LIMITLESS_DIFF_ITERS:-1000}"
GCOVR_PARSE_POLICY="${GCOVR_PARSE_POLICY:-negative_hits.warn_once_per_file}"
mkdir -p "$BUILD_DIR"

CFLAGS_BASE="-std=c99 -Wall -Wextra -Werror -pedantic -O0 --coverage"
CXXFLAGS_BASE="-std=c++11 -Wall -Wextra -Werror -pedantic -O0 --coverage"
LDFLAGS_EXTRA="--coverage"

compile_c() {
  local out="$1"
  shift
  "$CC_BIN" $CFLAGS_BASE "$@" $LDFLAGS_EXTRA -o "$out"
}

compile_cpp() {
  local out="$1"
  shift
  "$CXX_BIN" $CXXFLAGS_BASE "$@" $LDFLAGS_EXTRA -o "$out"
}

compile_c "$BUILD_DIR/test_limitless_c_basic" tests/test_limitless.c
"$BUILD_DIR/test_limitless_c_basic"

compile_c "$BUILD_DIR/test_limitless_api" tests/test_limitless_api.c
"$BUILD_DIR/test_limitless_api"

compile_c "$BUILD_DIR/test_limitless_parser_fuzz" tests/test_limitless_parser_fuzz.c
"$BUILD_DIR/test_limitless_parser_fuzz"

compile_c "$BUILD_DIR/test_limitless_c_generated" tests/test_limitless_generated.c
"$BUILD_DIR/test_limitless_c_generated"

compile_c "$BUILD_DIR/test_default_allocator_override" tests/test_default_allocator_override.c
"$BUILD_DIR/test_default_allocator_override"

compile_c "$BUILD_DIR/test_multi" tests/multi_impl.c tests/multi_a.c tests/multi_b.c
"$BUILD_DIR/test_multi"

compile_cpp "$BUILD_DIR/test_limitless_cpp_basic" tests/test_limitless_cpp.cpp
"$BUILD_DIR/test_limitless_cpp_basic"

compile_cpp "$BUILD_DIR/test_limitless_cpp_generated" tests/test_limitless_cpp_generated.cpp
"$BUILD_DIR/test_limitless_cpp_generated"

compile_cpp "$BUILD_DIR/test_limitless_threads" tests/test_limitless_threads.cpp
"$BUILD_DIR/test_limitless_threads"

compile_c "$BUILD_DIR/limitless_cli" tests/ci/limitless_cli.c
bash tests/ci/run_differential.sh "$BUILD_DIR/limitless_cli" "$DIFF_ITERS"

gcovr \
  --root "$ROOT_DIR" \
  --filter "$ROOT_DIR/limitless.h" \
  --gcov-ignore-parse-errors "$GCOVR_PARSE_POLICY" \
  --xml-pretty \
  --output "$BUILD_DIR/coverage.xml"

gcovr \
  --root "$ROOT_DIR" \
  --filter "$ROOT_DIR/limitless.h" \
  --gcov-ignore-parse-errors "$GCOVR_PARSE_POLICY" \
  --txt \
  --output "$BUILD_DIR/coverage.txt"

cat "$BUILD_DIR/coverage.txt"
echo "coverage artifacts: $BUILD_DIR/coverage.xml and $BUILD_DIR/coverage.txt"
