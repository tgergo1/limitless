/* SPDX-License-Identifier: GPL-3.0-only */
#include "bench_util.h"

#define LIMITLESS_IMPLEMENTATION
#include "../../limitless.h"

int main(void) {
  const uint64_t iterations = 250U;
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

  if (limitless_number_from_str(&ctx, &a, "99999999999999999999999999999999999999999999999999") != LIMITLESS_OK) return 1;
  if (limitless_number_from_str(&ctx, &b, "97") != LIMITLESS_OK) return 1;

  if (!limitless_bench_begin(&report, "bench_div", "limitless_number_div", iterations, 1U)) return 1;
  for (i = 0; i < (int)iterations; ++i) {
    if (limitless_number_div(&ctx, &out, &a, &b) != LIMITLESS_OK) return 1;
  }
  if (!limitless_bench_end(&report)) return 1;

  limitless_bench_print_json(&report);
  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
  return 0;
}
