/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define LIMITLESS_IMPLEMENTATION
#include "../limitless.h"

static limitless_ctx make_ctx(void) {
  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  return ctx;
}

static void expect_status(limitless_status got, limitless_status expected) {
  assert(got == expected);
}

static void expect_str(limitless_ctx* ctx, const limitless_number* n, const char* expected) {
  char buf[4096];
  assert(limitless_number_to_str(ctx, n, buf, (limitless_size)sizeof(buf), NULL) == LIMITLESS_OK);
  assert(strcmp(buf, expected) == 0);
}

static void test_integer_exports(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number n;
  limitless_i64 si = 0;
  limitless_u64 ui = 0;

  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);

  assert(limitless_number_from_str(&ctx, &n, "9223372036854775807") == LIMITLESS_OK);
  expect_status(limitless_number_to_i64(&ctx, &n, &si), LIMITLESS_OK);
  assert(si == 9223372036854775807LL);

  assert(limitless_number_from_str(&ctx, &n, "9223372036854775808") == LIMITLESS_OK);
  expect_status(limitless_number_to_i64(&ctx, &n, &si), LIMITLESS_ERANGE);

  assert(limitless_number_from_str(&ctx, &n, "-9223372036854775808") == LIMITLESS_OK);
  expect_status(limitless_number_to_i64(&ctx, &n, &si), LIMITLESS_OK);
  assert(si == (-9223372036854775807LL - 1LL));

  assert(limitless_number_from_str(&ctx, &n, "-9223372036854775809") == LIMITLESS_OK);
  expect_status(limitless_number_to_i64(&ctx, &n, &si), LIMITLESS_ERANGE);

  assert(limitless_number_from_str(&ctx, &n, "18446744073709551615") == LIMITLESS_OK);
  expect_status(limitless_number_to_u64(&ctx, &n, &ui), LIMITLESS_OK);
  assert(ui == 18446744073709551615ULL);

  assert(limitless_number_from_str(&ctx, &n, "18446744073709551616") == LIMITLESS_OK);
  expect_status(limitless_number_to_u64(&ctx, &n, &ui), LIMITLESS_ERANGE);

  assert(limitless_number_from_str(&ctx, &n, "-1") == LIMITLESS_OK);
  expect_status(limitless_number_to_u64(&ctx, &n, &ui), LIMITLESS_ERANGE);

  assert(limitless_number_from_str(&ctx, &n, "3/2") == LIMITLESS_OK);
  expect_status(limitless_number_to_i64(&ctx, &n, &si), LIMITLESS_ETYPE);
  expect_status(limitless_number_to_u64(&ctx, &n, &ui), LIMITLESS_ETYPE);

  limitless_number_clear(&ctx, &n);
}

static float float_from_bits(limitless_u32 bits) {
  union {
    limitless_u32 u;
    float f;
  } v;
  v.u = bits;
  return v.f;
}

static double double_from_bits(limitless_u64 bits) {
  union {
    limitless_u64 u;
    double d;
  } v;
  v.u = bits;
  return v.d;
}

static void test_float_double_edges(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number n;
  float f_sub = float_from_bits(0x00000001u);
  double d_sub = double_from_bits(0x0000000000000001ULL);

  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);

  assert(limitless_number_from_float_exact(&ctx, &n, -0.0f) == LIMITLESS_OK);
  expect_str(&ctx, &n, "0");

  assert(limitless_number_from_double_exact(&ctx, &n, -0.0) == LIMITLESS_OK);
  expect_str(&ctx, &n, "0");

  assert(limitless_number_from_float_exact(&ctx, &n, f_sub) == LIMITLESS_OK);
  expect_str(&ctx, &n, "1/713623846352979940529142984724747568191373312");

  assert(limitless_number_from_double_exact(&ctx, &n, d_sub) == LIMITLESS_OK);
  expect_str(&ctx, &n,
             "1/202402253307310618352495346718917307049556649764142118356901358027430339567995346891960383701437124495187077864316811911389808737385793476867013399940738509921517424276566361364466907742093216341239767678472745068562007483424692698618103355649159556340810056512358769552333414615230502532186327508646006263307707741093494784");

  assert(limitless_number_from_float_exact(&ctx, &n, INFINITY) == LIMITLESS_EINVAL);
  assert(limitless_number_from_double_exact(&ctx, &n, NAN) == LIMITLESS_EINVAL);

  limitless_number_clear(&ctx, &n);
}

int main(void) {
  test_integer_exports();
  test_float_double_edges();
  printf("conversion edge tests ok\n");
  return 0;
}
