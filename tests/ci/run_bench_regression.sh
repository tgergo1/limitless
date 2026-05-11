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

compile_bench "$BUILD_DIR/bench_bigint_mul" tests/bench/bench_bigint_mul.c
compile_bench "$BUILD_DIR/bench_div" tests/bench/bench_div.c
compile_bench "$BUILD_DIR/bench_parse_format" tests/bench/bench_parse_format.c
compile_bench "$BUILD_DIR/bench_pow_modexp" tests/bench/bench_pow_modexp.c

current_json="$BUILD_DIR/current.json"
python3 - "$BASELINE_FILE" "$current_json" "$CC_BIN" "$BUILD_DIR" <<'PY'
import datetime
import json
import os
import statistics
import subprocess
import sys

baseline_path, current_path, compiler_name, build_dir = sys.argv[1:5]

with open(baseline_path, 'r', encoding='utf-8') as f:
    baseline = json.load(f)

if isinstance(baseline, dict) and "benchmarks" in baseline:
    baseline_values = baseline["benchmarks"]
    baseline_unit = baseline.get("unit", "ns")
else:
    baseline_values = baseline
    baseline_unit = "us"

if baseline_unit == "us":
    baseline_values = {key: int(float(value) * 1000.0) for key, value in baseline_values.items()}
    baseline_unit = "ns"
elif baseline_unit != "ns":
    raise SystemExit(f"unsupported baseline unit: {baseline_unit}")

benchmarks = [
    ("bench_bigint_mul", os.path.join(build_dir, "bench_bigint_mul")),
    ("bench_div", os.path.join(build_dir, "bench_div")),
    ("bench_parse_format", os.path.join(build_dir, "bench_parse_format")),
    ("bench_pow_modexp", os.path.join(build_dir, "bench_pow_modexp")),
]

run_started_at = datetime.datetime.now(datetime.timezone.utc)
run_data = {}
violations = []

for benchmark_name, executable in benchmarks:
    runs = []
    durations = []
    for _ in range(3):
        payload = subprocess.check_output([executable], text=True)
        run = json.loads(payload)
        duration_ns = int(run["duration_ns"])
        durations.append(duration_ns)
        runs.append(run)

    median_ns = int(statistics.median(durations))
    mean_ns = statistics.fmean(durations)
    stdev_ns = statistics.stdev(durations) if len(durations) > 1 else 0.0
    baseline_ns = int(baseline_values[benchmark_name])
    limit_ns = int(round(float(baseline_ns) * 1.15))
    status = "pass" if median_ns <= limit_ns else "fail"

    run_data[benchmark_name] = {
        "samples_ns": durations,
        "min_duration_ns": min(durations),
        "median_duration_ns": median_ns,
        "max_duration_ns": max(durations),
        "mean_duration_ns": round(mean_ns, 3),
        "stdev_duration_ns": round(stdev_ns, 3),
        "median_duration_us": round(median_ns / 1000.0, 3),
        "baseline_duration_ns": baseline_ns,
        "regression_limit_ns": limit_ns,
        "regression_limit_ratio": 1.15,
        "status": status,
        "runs": runs,
    }

    if status != "pass":
        violations.append(
            f"{benchmark_name}: median={median_ns}ns baseline={baseline_ns}ns limit={limit_ns}ns"
        )

generated_at = datetime.datetime.now(datetime.timezone.utc)
current = {
    "schema_version": 2,
    "unit": "ns",
    "compiler": compiler_name,
    "run_started_at_utc": run_started_at.isoformat(timespec="milliseconds").replace("+00:00", "Z"),
    "generated_at_utc": generated_at.isoformat(timespec="milliseconds").replace("+00:00", "Z"),
    "benchmarks": run_data,
}

with open(current_path, 'w', encoding='utf-8') as f:
    json.dump(current, f, indent=2)
    f.write('\n')

for key in baseline_values:
    if key not in run_data:
        violations.append(f"missing benchmark key: {key}")

if violations:
    print("benchmark regression detected:", file=sys.stderr)
    for line in violations:
        print(f"  - {line}", file=sys.stderr)
    raise SystemExit(1)

print("benchmark regression check passed")
PY

cat "$current_json"
