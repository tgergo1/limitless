/* SPDX-License-Identifier: GPL-3.0-only */
#include "bench_util.h"

#define LIMITLESS_IMPLEMENTATION
#include "../../limitless.h"

int main(void) {
  const uint64_t iterations = 120U;
  limitless_ctx ctx;
  limitless_number a;
  limitless_number b;
  limitless_number out;
  int i;
  limitless_bench_report report;

  if (limitless_ctx_init_default(&ctx) != LIMITLESS_OK) return 1;
  if (limitless_number_init(&ctx, &a) != LIMITLESS_OK) return 1;
  if (limitless_number_init(&ctx, &b) != LIMITLESS_OK) return 1;
  if (limitless_number_init(&ctx, &out) != LIMITLESS_OK) return 1;

  if (limitless_number_from_str(&ctx, &a, "123456789012345678901234567890123456789012345678901234567890") != LIMITLESS_OK) return 1;
  if (limitless_number_from_str(&ctx, &b, "998877665544332211009988776655443322110099887766554433221100") != LIMITLESS_OK) return 1;

  if (!limitless_bench_begin(&report, "bench_bigint_mul", "limitless_number_mul", iterations, 1U)) return 1;
  for (i = 0; i < (int)iterations; ++i) {
    if (limitless_number_mul(&ctx, &out, &a, &b) != LIMITLESS_OK) return 1;
  }
  if (!limitless_bench_end(&report)) return 1;

  limitless_bench_print_json(&report);
  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
  return 0;
}
