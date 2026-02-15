/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdio.h>
#include <time.h>

#define LIMITLESS_IMPLEMENTATION
#include "../../limitless.h"

static double elapsed_us(clock_t start, clock_t end) {
  return ((double)(end - start) * 1000000.0) / (double)CLOCKS_PER_SEC;
}

int main(void) {
  limitless_ctx ctx;
  limitless_number n;
  char out[1024];
  int i;
  clock_t t0;
  clock_t t1;

  if (limitless_ctx_init_default(&ctx) != LIMITLESS_OK) return 1;
  if (limitless_number_init(&ctx, &n) != LIMITLESS_OK) return 1;

  t0 = clock();
  for (i = 0; i < 800; ++i) {
    if (limitless_number_from_str(&ctx, &n, "123456789012345678901234567890/97") != LIMITLESS_OK) return 1;
    if (limitless_number_to_str(&ctx, &n, out, (limitless_size)sizeof(out), NULL) != LIMITLESS_OK) return 1;
  }
  t1 = clock();

  printf("%.3f\n", elapsed_us(t0, t1));
  limitless_number_clear(&ctx, &n);
  return 0;
}
