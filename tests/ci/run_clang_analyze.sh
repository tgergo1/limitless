#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

CLANG_BIN="${CLANG_BIN:-clang}"
CLANGXX_BIN="${CLANGXX_BIN:-clang++}"

CFLAGS_BASE=(
  -std=c99
  -Wall
  -Wextra
  -Werror
  -pedantic
)

CXXFLAGS_BASE=(
  -std=c++11
  -Wall
  -Wextra
  -Werror
  -pedantic
)

ANALYZER_FLAGS=(
  -Xanalyzer -analyzer-output=text
  -Xanalyzer -analyzer-disable-checker -Xanalyzer core.NullDereference
  -Xanalyzer -analyzer-disable-checker -Xanalyzer unix.Malloc
)

run_and_enforce_clean() {
  local label="$1"
  shift
  local log
  log="$(mktemp)"

  if ! "$@" >"$log" 2>&1; then
    cat "$log"
    rm -f "$log"
    echo "clang-analyze command failed for $label" >&2
    return 1
  fi

  if grep -q "warning:" "$log"; then
    cat "$log"
    rm -f "$log"
    echo "clang-analyze reported warnings for $label" >&2
    return 1
  fi

  rm -f "$log"
}

analyze_c() {
  local src="$1"
  run_and_enforce_clean "$src" "$CLANG_BIN" --analyze "${CFLAGS_BASE[@]}" "${ANALYZER_FLAGS[@]}" "$src"
}

analyze_cpp() {
  local src="$1"
  run_and_enforce_clean "$src" "$CLANGXX_BIN" --analyze "${CXXFLAGS_BASE[@]}" "${ANALYZER_FLAGS[@]}" "$src"
}

analyze_cpp_allow_deprecated() {
  local src="$1"
  run_and_enforce_clean "$src" "$CLANGXX_BIN" --analyze "${CXXFLAGS_BASE[@]}" -Wno-error=deprecated-declarations -Wno-deprecated-declarations "${ANALYZER_FLAGS[@]}" "$src"
}

analyze_cpp_with_repo_include() {
  local src="$1"
  run_and_enforce_clean "$src" "$CLANGXX_BIN" --analyze "${CXXFLAGS_BASE[@]}" "${ANALYZER_FLAGS[@]}" -I"$ROOT_DIR" "$src"
}

for src in \
  tests/test_limitless.c \
  tests/test_limitless_api.c \
  tests/test_limitless_parse_edges.c \
  tests/test_limitless_conversion_edges.c \
  tests/test_limitless_invariants.c \
  tests/test_limitless_allocator_contract.c \
  tests/test_limitless_generated.c \
  tests/test_limitless_parser_fuzz.c \
  tests/test_default_allocator_override.c \
  tests/multi_impl.c \
  tests/multi_a.c \
  tests/multi_b.c

do
  analyze_c "$src"
done

for src in \
  tests/test_limitless_cpp.cpp \
  tests/test_limitless_cpp_generated.cpp \
  tests/test_cpp_namespace_strict.cpp \
  tests/test_limitless_threads.cpp \
  tests/test_limitless_threads_c_api.cpp \
  tests/test_limitless_cpp_cross_thread_status.cpp \
  tests/multi_cpp_impl.cpp \
  tests/multi_cpp_a.cpp \
  tests/multi_cpp_b.cpp

do
  analyze_cpp "$src"
done

analyze_cpp_allow_deprecated tests/test_cpp_legacy_bridge.cpp

analyze_cpp_with_repo_include tests/test_include_repo_root.cpp

echo "clang static analysis checks passed"
