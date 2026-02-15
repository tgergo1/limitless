#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

CLANG_TIDY_BIN="${CLANG_TIDY_BIN:-clang-tidy}"
# Keep clang-tidy focused on analyzer checks while filtering known noisy false positives
# that are already handled by dedicated analyzer policy in run_clang_analyze.sh.
CLANG_TIDY_CHECKS="${CLANG_TIDY_CHECKS:-clang-analyzer-*,-clang-analyzer-core.NullDereference,-clang-analyzer-unix.Malloc,-clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling,-clang-analyzer-optin.core.EnumCastOutOfRange}"

if ! command -v "$CLANG_TIDY_BIN" >/dev/null 2>&1; then
  echo "clang-tidy not found: $CLANG_TIDY_BIN" >&2
  exit 1
fi

run_c() {
  local src="$1"
  "$CLANG_TIDY_BIN" -quiet -checks="$CLANG_TIDY_CHECKS" -warnings-as-errors='*' "$src" -- -std=c11 -Wall -Wextra -Werror -pedantic
}

run_cpp() {
  local src="$1"
  "$CLANG_TIDY_BIN" -quiet -checks="$CLANG_TIDY_CHECKS" -warnings-as-errors='*' "$src" -- -std=c++11 -Wall -Wextra -Werror -pedantic
}

for src in \
  tests/test_limitless.c \
  tests/test_limitless_api.c \
  tests/test_limitless_parse_edges.c \
  tests/test_limitless_conversion_edges.c \
  tests/test_limitless_invariants.c \
  tests/test_limitless_allocator_contract.c \
  tests/ci/limitless_cli.c

do
  run_c "$src"
done

for src in \
  tests/test_limitless_cpp.cpp \
  tests/test_cpp_namespace_strict.cpp \
  tests/test_limitless_threads.cpp \
  tests/test_limitless_threads_c_api.cpp \
  tests/test_limitless_cpp_cross_thread_status.cpp

do
  run_cpp "$src"
done

echo "clang-tidy checks passed"
