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

  if (limitless_number_from_str(&ctx, &a, "123456789012345678901234567890123456789012345678901234567890") != LIMITLESS_OK) return 1;
  if (limitless_number_from_str(&ctx, &b, "998877665544332211009988776655443322110099887766554433221100") != LIMITLESS_OK) return 1;

  t0 = clock();
  for (i = 0; i < 120; ++i) {
    if (limitless_number_mul(&ctx, &out, &a, &b) != LIMITLESS_OK) return 1;
  }
  t1 = clock();

  printf("%.3f\n", elapsed_us(t0, t1));
  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
  return 0;
}
