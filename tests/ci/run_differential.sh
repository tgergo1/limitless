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

python3 tests/ci/diff_reference.py --cli "$CLI_PATH" --iters "$ITERS"
