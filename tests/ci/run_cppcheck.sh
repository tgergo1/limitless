#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

CPPCHECK_BIN="${CPPCHECK_BIN:-cppcheck}"

if ! command -v "$CPPCHECK_BIN" >/dev/null 2>&1; then
  echo "cppcheck not found: $CPPCHECK_BIN" >&2
  exit 1
fi

"$CPPCHECK_BIN" \
  --std=c99 \
  --language=c \
  --enable=warning,style,performance,portability \
  --error-exitcode=1 \
  --suppress=missingIncludeSystem \
  --suppress=assertWithSideEffect \
  --quiet \
  tests/test_limitless.c \
  tests/test_limitless_api.c \
  tests/test_limitless_parse_edges.c \
  tests/test_limitless_conversion_edges.c \
  tests/test_limitless_invariants.c \
  tests/test_limitless_allocator_contract.c \
  tests/ci/limitless_cli.c

"$CPPCHECK_BIN" \
  --std=c++11 \
  --language=c++ \
  --enable=warning,style,performance,portability \
  --error-exitcode=1 \
  --suppress=missingIncludeSystem \
  --suppress=assertWithSideEffect \
  --quiet \
  tests/test_limitless_cpp.cpp \
  tests/test_cpp_namespace_strict.cpp \
  tests/test_cpp_legacy_bridge.cpp \
  tests/test_limitless_threads.cpp \
  tests/test_limitless_threads_c_api.cpp \
  tests/test_limitless_cpp_cross_thread_status.cpp

echo "cppcheck checks passed"
