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
  limitless_number a;
  limitless_number b;
  limitless_number out;
  int i;
  clock_t t0;
  clock_t t1;

  if (limitless_ctx_init_default(&ctx) != LIMITLESS_OK) return 1;
  if (limitless_number_init(&ctx, &a) != LIMITLESS_OK) return 1;
  if (limitless_number_init(&ctx, &b) != LIMITLESS_OK) return 1;
  if (limitless_number_init(&ctx, &out) != LIMITLESS_OK) return 1;

  if (limitless_number_from_str(&ctx, &a, "99999999999999999999999999999999999999999999999999") != LIMITLESS_OK) return 1;
  if (limitless_number_from_str(&ctx, &b, "97") != LIMITLESS_OK) return 1;

  t0 = clock();
  for (i = 0; i < 250; ++i) {
    if (limitless_number_div(&ctx, &out, &a, &b) != LIMITLESS_OK) return 1;
  }
  t1 = clock();

  printf("%.3f\n", elapsed_us(t0, t1));
  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
  return 0;
}
