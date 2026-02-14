#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

if [[ $# -lt 1 ]]; then
  echo "usage: $0 <cli-path> [iters]" >&2
  exit 2
fi

CLI_PATH="$1"
ITERS="${2:-${LIMITLESS_DIFF_ITERS:-3000}}"
PYTHON_BIN="${PYTHON_BIN:-}"

if [[ -z "$PYTHON_BIN" ]]; then
  if command -v python3 >/dev/null 2>&1; then
    PYTHON_BIN="python3"
  elif command -v python >/dev/null 2>&1; then
    PYTHON_BIN="python"
  else
    echo "python interpreter not found (need python3 or python)" >&2
    exit 127
  fi
fi

"$PYTHON_BIN" tests/ci/diff_reference.py --cli "$CLI_PATH" --iters "$ITERS"
