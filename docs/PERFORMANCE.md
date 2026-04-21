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
- bigint division uses a dedicated single-limb absolute divider when the divisor fits in `u32`
- exact division reuses the same single-limb fast path
- bigint `gcd` now uses a small-integer Euclidean shortcut when either side fits in `u32`
- rational normalization returns immediately when the denominator is already `1`

## How it was measured

Three methods were used locally:

1. existing repo benchmark gate: `bash tests/ci/run_bench_regression.sh`
2. full default validation matrix: `bash tests/ci/run_unix_matrix.sh default`
3. allocation-counting and large-input regression tests in `tests/test_limitless.c`

In addition, a local scaling probe was used during development to compare 32/128/512/2048-digit parse, format, and `div by 97` behavior before and after the change.

## Local benchmark results

`tests/ci/run_bench_regression.sh` on the task environment:

| benchmark | before | after | improvement |
| --- | ---: | ---: | ---: |
| `bench_bigint_mul` | 11 µs | 11 µs | ~1.0x |
| `bench_div` | 1003 µs | 83 µs | ~12.1x |
| `bench_parse_format` | 1418 µs | 495 µs | ~2.9x |
| `bench_pow_modexp` | 493 µs | 212 µs | ~2.3x |

Scaling probe highlights for 2048-digit decimal inputs:

| operation | before | after | improvement |
| --- | ---: | ---: | ---: |
| parse | 16042 µs | 2209 µs | ~7.3x |
| format | 89952 µs | 11497 µs | ~7.8x |
| divide by `97` | 192152 µs | 434 µs | ~442x |

## Regression coverage

New coverage added in `tests/test_limitless.c`:

- large base-36 roundtrip
- allocation/regression contract for 2048-digit parse/format
- large integer division by small integers for both exact and rational outcomes

## Remaining gap

The generic multi-limb divider is still a simple shift/subtract implementation. Similar mature bigint libraries usually move to Knuth-style long division or Burnikel-Ziegler for large operands. That is the next major performance target after these fast paths.
