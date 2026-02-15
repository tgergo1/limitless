#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

failures=0

while IFS= read -r path; do
  head_text="$(sed -n '1,4p' "$path")"
  if ! grep -q "SPDX-License-Identifier:" <<<"$head_text"; then
    echo "missing SPDX header: $path" >&2
    failures=1
  fi
done < <(
  rg --files \
    -g '*.c' \
    -g '*.cpp' \
    -g '*.h' \
    -g '*.hpp' \
    -g '*.py' \
    -g '*.sh' \
    -g '*.ps1' \
    -g '*.yml' \
    -g '*.yaml' \
    -g '*.md' \
    .github tests tools
)

if [[ "$failures" -ne 0 ]]; then
  exit 1
fi

echo "SPDX header checks passed"
