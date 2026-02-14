#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

if ! command -v apt-get >/dev/null 2>&1; then
  echo "apt-get not found; skipping apt source preparation"
  exit 0
fi

sanitize_file() {
  local file="$1"
  if [[ ! -f "$file" ]]; then
    return 0
  fi

  if sudo grep -Eqs 'packages\.microsoft\.com' "$file"; then
    echo "disabling apt source: $file"
    sudo mv "$file" "${file}.disabled"
  fi
}

for file in /etc/apt/sources.list.d/*.list /etc/apt/sources.list.d/*.sources; do
  [[ -e "$file" ]] || continue
  sanitize_file "$file"
done

sudo apt-get update \
  -o Acquire::Retries=3 \
  -o Acquire::http::Timeout=30 \
  -o Acquire::https::Timeout=30
