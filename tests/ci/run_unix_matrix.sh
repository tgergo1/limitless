#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"
python3 tools/check_reserved_filenames.py

MODE="${1:-default}"
CC_BIN="${CC_BIN:-cc}"
CXX_BIN="${CXX_BIN:-c++}"
BUILD_DIR="$ROOT_DIR/build/ci/${MODE}"
mkdir -p "$BUILD_DIR"

CFLAGS_BASE="-std=c99 -Wall -Wextra -Werror -pedantic"
CXXFLAGS_BASE="-std=c++11 -Wall -Wextra -Werror -pedantic"
LDFLAGS_EXTRA=""
CPP_EXTRA=""
DEF_EXTRA=""
DIFF_ITERS="${LIMITLESS_DIFF_ITERS:-5000}"

case "$MODE" in
  default)
    ;;
  limb64)
    DEF_EXTRA="-DLIMITLESS_LIMB_BITS=64"
    ;;
  noexceptions)
    CPP_EXTRA="-fno-exceptions"
    ;;
  m32)
    CFLAGS_BASE="$CFLAGS_BASE -m32"
    CXXFLAGS_BASE="$CXXFLAGS_BASE -m32"
    LDFLAGS_EXTRA="-m32"
    DIFF_ITERS="${LIMITLESS_DIFF_ITERS:-1500}"
    ;;
  extended-stress)
    DEF_EXTRA="-DLIMITLESS_EXTENDED_STRESS"
    ;;
  asan-ubsan)
    CFLAGS_BASE="$CFLAGS_BASE -fsanitize=address,undefined -fno-omit-frame-pointer"
    CXXFLAGS_BASE="$CXXFLAGS_BASE -fsanitize=address,undefined -fno-omit-frame-pointer"
    LDFLAGS_EXTRA="-fsanitize=address,undefined"
    DIFF_ITERS="${LIMITLESS_DIFF_ITERS:-500}"
    ;;
  lsan)
    CFLAGS_BASE="$CFLAGS_BASE -fsanitize=leak -fno-omit-frame-pointer"
    CXXFLAGS_BASE="$CXXFLAGS_BASE -fsanitize=leak -fno-omit-frame-pointer"
    LDFLAGS_EXTRA="-fsanitize=leak"
    DIFF_ITERS="${LIMITLESS_DIFF_ITERS:-500}"
    ;;
  tsan)
    CFLAGS_BASE="$CFLAGS_BASE -fsanitize=thread -fno-omit-frame-pointer"
    CXXFLAGS_BASE="$CXXFLAGS_BASE -fsanitize=thread -fno-omit-frame-pointer"
    LDFLAGS_EXTRA="-fsanitize=thread"
    DIFF_ITERS="${LIMITLESS_DIFF_ITERS:-500}"
    ;;
  *)
    echo "unknown mode: $MODE" >&2
    exit 2
    ;;
esac

compile_c() {
  local out="$1"
  shift
  "$CC_BIN" $CFLAGS_BASE $DEF_EXTRA "$@" $LDFLAGS_EXTRA -o "$out"
}

compile_cpp() {
  local out="$1"
  shift
  "$CXX_BIN" $CXXFLAGS_BASE $CPP_EXTRA $DEF_EXTRA "$@" $LDFLAGS_EXTRA -o "$out"
}

compile_cpp_allow_deprecated() {
  local out="$1"
  shift
  "$CXX_BIN" $CXXFLAGS_BASE $CPP_EXTRA $DEF_EXTRA -Wno-error=deprecated-declarations "$@" $LDFLAGS_EXTRA -o "$out"
}

if [[ "$MODE" == "extended-stress" ]]; then
  compile_c "$BUILD_DIR/test_limitless_generated_stress" tests/test_limitless_generated.c
  "$BUILD_DIR/test_limitless_generated_stress"

  compile_cpp "$BUILD_DIR/test_limitless_cpp_generated_stress" tests/test_limitless_cpp_generated.cpp
  "$BUILD_DIR/test_limitless_cpp_generated_stress"
  exit 0
fi

compile_c "$BUILD_DIR/test_limitless_c_basic" tests/test_limitless.c
"$BUILD_DIR/test_limitless_c_basic"

compile_c "$BUILD_DIR/test_limitless_api" tests/test_limitless_api.c
"$BUILD_DIR/test_limitless_api"

compile_c "$BUILD_DIR/test_limitless_parse_edges" tests/test_limitless_parse_edges.c
"$BUILD_DIR/test_limitless_parse_edges"

compile_c "$BUILD_DIR/test_limitless_conversion_edges" tests/test_limitless_conversion_edges.c
"$BUILD_DIR/test_limitless_conversion_edges"

compile_c "$BUILD_DIR/test_limitless_invariants" tests/test_limitless_invariants.c
"$BUILD_DIR/test_limitless_invariants"

compile_c "$BUILD_DIR/test_limitless_allocator_contract" tests/test_limitless_allocator_contract.c
"$BUILD_DIR/test_limitless_allocator_contract"

compile_c "$BUILD_DIR/test_limitless_parser_fuzz" tests/test_limitless_parser_fuzz.c
"$BUILD_DIR/test_limitless_parser_fuzz"

compile_c "$BUILD_DIR/test_limitless_c_generated" tests/test_limitless_generated.c
"$BUILD_DIR/test_limitless_c_generated"

compile_c "$BUILD_DIR/test_default_allocator_override" tests/test_default_allocator_override.c
"$BUILD_DIR/test_default_allocator_override"

compile_c "$BUILD_DIR/test_multi" tests/multi_impl.c tests/multi_a.c tests/multi_b.c
"$BUILD_DIR/test_multi"

compile_cpp "$BUILD_DIR/test_multi_cpp" tests/multi_cpp_impl.cpp tests/multi_cpp_a.cpp tests/multi_cpp_b.cpp
"$BUILD_DIR/test_multi_cpp"

compile_cpp "$BUILD_DIR/test_limitless_cpp_basic" tests/test_limitless_cpp.cpp
"$BUILD_DIR/test_limitless_cpp_basic"

compile_cpp "$BUILD_DIR/test_limitless_cpp_generated" tests/test_limitless_cpp_generated.cpp
"$BUILD_DIR/test_limitless_cpp_generated"

compile_cpp "$BUILD_DIR/test_cpp_namespace_strict" tests/test_cpp_namespace_strict.cpp
"$BUILD_DIR/test_cpp_namespace_strict"

compile_cpp_allow_deprecated "$BUILD_DIR/test_cpp_legacy_bridge" tests/test_cpp_legacy_bridge.cpp
"$BUILD_DIR/test_cpp_legacy_bridge"

compile_cpp "$BUILD_DIR/test_limitless_threads" tests/test_limitless_threads.cpp
"$BUILD_DIR/test_limitless_threads"

compile_cpp "$BUILD_DIR/test_limitless_threads_c_api" tests/test_limitless_threads_c_api.cpp
"$BUILD_DIR/test_limitless_threads_c_api"

compile_cpp "$BUILD_DIR/test_limitless_cpp_cross_thread_status" tests/test_limitless_cpp_cross_thread_status.cpp
"$BUILD_DIR/test_limitless_cpp_cross_thread_status"

compile_cpp "$BUILD_DIR/test_include_repo_root" -I"$ROOT_DIR" tests/test_include_repo_root.cpp
"$BUILD_DIR/test_include_repo_root"

bash tests/ci/run_negative_compile.sh

compile_c "$BUILD_DIR/limitless_cli" tests/ci/limitless_cli.c
bash tests/ci/run_differential.sh "$BUILD_DIR/limitless_cli" "$DIFF_ITERS"
