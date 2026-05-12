/* SPDX-License-Identifier: GPL-3.0-only */
#include "bench_util.h"

#define LIMITLESS_IMPLEMENTATION
#include "../../limitless.h"

int main(void) {
  const uint64_t iterations = 800U;
  limitless_ctx ctx;
  limitless_number n;
  char out[1024];
  int i;
  limitless_bench_report report;

  if (limitless_ctx_init_default(&ctx) != LIMITLESS_OK) return 1;
  if (limitless_number_init(&ctx, &n) != LIMITLESS_OK) return 1;

  if (!limitless_bench_begin(&report,
                             "bench_parse_format",
                             "limitless_number_from_str+limitless_number_to_str",
                             iterations,
                             2U)) {
    return 1;
  }
  for (i = 0; i < (int)iterations; ++i) {
    if (limitless_number_from_str(&ctx, &n, "123456789012345678901234567890/97") != LIMITLESS_OK) return 1;
    if (limitless_number_to_str(&ctx, &n, out, (limitless_size)sizeof(out), NULL) != LIMITLESS_OK) return 1;
  }
  if (!limitless_bench_end(&report)) return 1;

  limitless_bench_print_json(&report);
  limitless_number_clear(&ctx, &n);
  return 0;
}
