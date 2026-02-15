#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-3.0-only
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
cd "$ROOT_DIR"

CC_BIN="${CC_BIN:-gcc}"
BUILD_DIR="$ROOT_DIR/build/bench"
BASELINE_FILE="${1:-tests/bench/baseline/linux-gcc13.json}"
mkdir -p "$BUILD_DIR"

compile_bench() {
  local out="$1"
  local src="$2"
  "$CC_BIN" -std=c99 -O2 -Wall -Wextra -Werror -pedantic "$src" -o "$out"
}

median_of_three() {
  local exe="$1"
  local a b c
  a="$($exe)"
  b="$($exe)"
  c="$($exe)"
  python3 - "$a" "$b" "$c" <<'PY'
import statistics
import sys
vals = [float(sys.argv[1]), float(sys.argv[2]), float(sys.argv[3])]
print(f"{statistics.median(vals):.3f}")
PY
}

compile_bench "$BUILD_DIR/bench_bigint_mul" tests/bench/bench_bigint_mul.c
compile_bench "$BUILD_DIR/bench_div" tests/bench/bench_div.c
compile_bench "$BUILD_DIR/bench_parse_format" tests/bench/bench_parse_format.c
compile_bench "$BUILD_DIR/bench_pow_modexp" tests/bench/bench_pow_modexp.c

current_json="$BUILD_DIR/current.json"
cat > "$current_json" <<EOF_JSON
{
  "bench_bigint_mul": $(median_of_three "$BUILD_DIR/bench_bigint_mul"),
  "bench_div": $(median_of_three "$BUILD_DIR/bench_div"),
  "bench_parse_format": $(median_of_three "$BUILD_DIR/bench_parse_format"),
  "bench_pow_modexp": $(median_of_three "$BUILD_DIR/bench_pow_modexp")
}
EOF_JSON

python3 - "$BASELINE_FILE" "$current_json" <<'PY'
import json
import sys

baseline_path, current_path = sys.argv[1], sys.argv[2]
with open(baseline_path, 'r', encoding='utf-8') as f:
    baseline = json.load(f)
with open(current_path, 'r', encoding='utf-8') as f:
    current = json.load(f)

violations = []
for key, base in baseline.items():
    cur = current.get(key)
    if cur is None:
        violations.append(f"missing benchmark key: {key}")
        continue
    limit = float(base) * 1.15
    if float(cur) > limit:
        violations.append(f"{key}: current={cur:.3f} baseline={base:.3f} limit={limit:.3f}")

if violations:
    print("benchmark regression detected:", file=sys.stderr)
    for line in violations:
        print(f"  - {line}", file=sys.stderr)
    raise SystemExit(1)

print("benchmark regression check passed")
PY

cat "$current_json"
