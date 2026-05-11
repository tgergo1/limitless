<!-- SPDX-License-Identifier: GPL-3.0-only -->
# Performance notes

## What was wrong

Before this change, the hottest paths had avoidable algorithmic costs:

- base conversion parsed one digit at a time (`value = value * base + digit`)
- formatting emitted one digit at a time via repeated division by `base`
- division of a large integer by a small integer still used the generic bit-by-bit divider
- rational normalization always paid the full bigint `gcd` cost, even when one side fit in a machine word

Those choices are correct, but they scale poorly. Similar libraries usually add specialized fast paths here:

- GMP / LibTomMath style chunked parse/format (`10^k`, `16^k`, `36^k`, etc.)
- specialized single-limb division and remainder routines
- specialized small-value `gcd` helpers before falling back to full bigint algorithms
- a fast exit for already-normalized denominator `1`

## What changed

- parsing now consumes base digits in the largest `u32` chunk that fits for the selected base
- formatting now divides by the largest `u32` base chunk and emits whole digit groups
- formatting now reuses its generated buffer instead of allocating and copying a second string
- bigint division uses a dedicated single-limb absolute divider when the divisor fits in `u32`
- exact division reuses the same single-limb fast path
- bigint `gcd` now uses a small-integer Euclidean shortcut when either side fits in `u32`
- rational normalization returns immediately when the denominator is already `1`
- modular exponentiation now uses a dedicated `u32` modulus fast path when the modulus fits in one limb
- internal copy-heavy paths use compiler memcpy builtins when available

## How it was measured

Three methods were used locally:

1. existing repo benchmark gate: `bash tests/ci/run_bench_regression.sh`
2. full default validation matrix: `bash tests/ci/run_unix_matrix.sh default`
3. allocation-counting and large-input regression tests in `tests/test_limitless.c`

The benchmark executables now emit structured JSON per run with:

- UTC start/end timestamps in ISO-8601 form
- monotonic nanosecond durations
- per-iteration and per-operation averages
- three-sample min/median/max/mean/stdev aggregation in `build/bench/current.json`

This keeps wall-clock timestamps useful for traceability while using a monotonic clock for the actual duration measurements.

In addition, a local scaling probe was used during development to compare 32/128/512/2048-digit parse, format, and `div by 97` behavior before and after the change.

## Local benchmark results

Representative local runs on the task environment:

| benchmark | before | after | improvement |
| --- | ---: | ---: | ---: |
| `bench_bigint_mul` | 11 µs | 13 µs | ~0.8x |
| `bench_div` | 1003 µs | 99 µs | ~10.1x |
| `bench_parse_format` | 1418 µs | 578 µs | ~2.5x |
| `bench_pow_modexp` | 493 µs | 58 µs | ~8.5x |

`bench_bigint_mul` was not a target of this change. The representative run above was slightly noisier/slower, while the multiplication implementation itself was left unchanged and the benchmark gate still passes comfortably.

The benchmark gate baselines are intentionally looser than these representative medians so CI can tolerate normal runner variance without masking real regressions.

## Similar-library comparison

`limitless` intentionally keeps a tiny dependency-free single-header footprint, so the repository CI does not vendor or build heavyweight external bigint stacks for apples-to-apples benchmark runs. The practical comparison point is therefore methodology and optimization coverage versus state-of-the-art peers:

| library | primary focus | timing/reporting style | mature performance techniques relevant here |
| --- | --- | --- | --- |
| `limitless` | exact integers + rationals in a dependency-free single header | repo benchmarks now record ISO-8601 UTC timestamps plus monotonic nanosecond durations and aggregated run statistics | single-limb fast paths, chunked parse/format, exact rational normalization shortcuts |
| GMP | peak bigint throughput with assembly and platform specialization | external tools such as `tune/speed` typically use monotonic wall-clock timing and many repeated samples | Karatsuba/Toom-Cook/FFT multiplication, tuned division, aggressive limb-specialized code |
| LibTomMath | portable pure-C multiple-precision integers | benchmark suites usually report repeated wall-clock durations around portable C kernels | Comba multiplication, tuned reduction methods, long-division style big integer routines |
| Boost.Multiprecision (`cpp_int`) | C++ integration and generic backend selection | benchmarking is usually delegated to external harnesses (`std::chrono`, Google Benchmark, etc.) | backend swapping, expression-template optimizations, interoperability with GMP/MPIR backends |

Compared with those libraries, `limitless` still trades raw peak throughput for exact-rational support and zero-dependency embeddability. The new benchmark format makes that trade-off measurable with enough detail to compare repeated runs, CI regressions, and future external-library probes using the same timestamp and duration fields.

## Current bottlenecks

With the small-divisor and small-modulus fast paths in place, the remaining hotspots are now concentrated in the generic multi-limb algorithms:

- general multi-limb division still uses shift/subtract instead of Knuth-style long division
- multiplication is portable schoolbook/Karatsuba only, without Comba or architecture-tuned kernels
- parse/format still pay repeated bigint division costs once values outgrow the single-limb fast paths

Scaling probe highlights for 2048-digit decimal inputs:

| operation | before | after | improvement |
| --- | ---: | ---: | ---: |
| parse | 16042 µs | 2151 µs | ~7.5x |
| format | 89952 µs | 11366 µs | ~7.9x |
| divide by `97` | 192152 µs | 383 µs | ~502x |

## Regression coverage

New coverage added in `tests/test_limitless.c`:

- large base-36 roundtrip
- allocation/regression contract for 2048-digit parse/format
- large integer division by small integers for both exact and rational outcomes

## Remaining gap

The generic multi-limb divider is still a simple shift/subtract implementation. Similar mature bigint libraries usually move to Knuth-style long division or Burnikel-Ziegler for large operands, and pair that with more specialized multiplication kernels. That is the next major performance target after these fast paths.
