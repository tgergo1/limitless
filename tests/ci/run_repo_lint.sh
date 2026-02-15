#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

if ! command -v actionlint >/dev/null 2>&1; then
  echo "actionlint not found" >&2
  exit 1
fi
if ! command -v shellcheck >/dev/null 2>&1; then
  echo "shellcheck not found" >&2
  exit 1
fi
if ! command -v yamllint >/dev/null 2>&1; then
  echo "yamllint not found" >&2
  exit 1
fi

actionlint
shellcheck tests/ci/*.sh

yamllint \
  .github/workflows/*.yml \
  .github/dependabot.yml

echo "repo lint checks passed"
