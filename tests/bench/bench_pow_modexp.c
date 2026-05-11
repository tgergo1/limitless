/* SPDX-License-Identifier: GPL-3.0-only */
#include "bench_util.h"

#define LIMITLESS_IMPLEMENTATION
#include "../../limitless.h"

int main(void) {
  const uint64_t iterations = 120U;
  limitless_ctx ctx;
  limitless_number base;
  limitless_number mod;
  limitless_number out;
  int i;
  limitless_bench_report report;

  if (limitless_ctx_init_default(&ctx) != LIMITLESS_OK) return 1;
  if (limitless_number_init(&ctx, &base) != LIMITLESS_OK) return 1;
  if (limitless_number_init(&ctx, &mod) != LIMITLESS_OK) return 1;
  if (limitless_number_init(&ctx, &out) != LIMITLESS_OK) return 1;

  if (limitless_number_from_str(&ctx, &base, "123456789") != LIMITLESS_OK) return 1;
  if (limitless_number_from_str(&ctx, &mod, "1000000007") != LIMITLESS_OK) return 1;

  if (!limitless_bench_begin(&report,
                             "bench_pow_modexp",
                             "limitless_number_pow_u64+limitless_number_modexp_u64",
                             iterations,
                             2U)) {
    return 1;
  }
  for (i = 0; i < (int)iterations; ++i) {
    if (limitless_number_pow_u64(&ctx, &out, &base, 17) != LIMITLESS_OK) return 1;
    if (limitless_number_modexp_u64(&ctx, &out, &base, 29, &mod) != LIMITLESS_OK) return 1;
  }
  if (!limitless_bench_end(&report)) return 1;

  limitless_bench_print_json(&report);
  limitless_number_clear(&ctx, &base);
  limitless_number_clear(&ctx, &mod);
  limitless_number_clear(&ctx, &out);
  return 0;
}
