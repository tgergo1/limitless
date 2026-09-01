/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define LIMITLESS_IMPLEMENTATION
#include "../limitless.h"

static limitless_ctx make_ctx(void) {
  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  return ctx;
}

static void expect_str(limitless_ctx* ctx, const limitless_number* value, const char* expected) {
  char buffer[512];
  assert(limitless_number_to_str(ctx, value, buffer, (limitless_size)sizeof(buffer), NULL) == LIMITLESS_OK);
  assert(strcmp(buffer, expected) == 0);
}

static void test_integer_and_bit_apis(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number a, b, q, r, out;
  limitless_size bits;
  int bit;
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &q) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &r) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);

  assert(limitless_number_from_i64(&ctx, &a, -17) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 5) == LIMITLESS_OK);
  assert(limitless_number_divmod(&ctx, &q, &r, &a, &b) == LIMITLESS_OK);
  expect_str(&ctx, &q, "-3");
  expect_str(&ctx, &r, "-2");
  assert(limitless_number_mod(&ctx, &out, &a, &b) == LIMITLESS_OK);
  expect_str(&ctx, &out, "-2");

  assert(limitless_number_from_str(&ctx, &a, "240") == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &b, "15") == LIMITLESS_OK);
  assert(limitless_number_bit_and(&ctx, &out, &a, &b) == LIMITLESS_OK);
  expect_str(&ctx, &out, "0");
  assert(limitless_number_bit_or(&ctx, &out, &a, &b) == LIMITLESS_OK);
  expect_str(&ctx, &out, "255");
  assert(limitless_number_bit_xor(&ctx, &out, &a, &b) == LIMITLESS_OK);
  expect_str(&ctx, &out, "255");
  assert(limitless_number_from_i64(&ctx, &a, 5) == LIMITLESS_OK);
  assert(limitless_number_bit_not(&ctx, &out, &a) == LIMITLESS_OK);
  expect_str(&ctx, &out, "-6");
  assert(limitless_number_from_i64(&ctx, &a, -5) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 3) == LIMITLESS_OK);
  assert(limitless_number_bit_and(&ctx, &out, &a, &b) == LIMITLESS_OK);
  expect_str(&ctx, &out, "3");

  assert(limitless_number_from_i64(&ctx, &a, -9) == LIMITLESS_OK);
  assert(limitless_number_shr(&ctx, &out, &a, 2) == LIMITLESS_OK);
  expect_str(&ctx, &out, "-2");
  assert(limitless_number_shift_left(&ctx, &out, &a, 3) == LIMITLESS_OK);
  expect_str(&ctx, &out, "-72");

  assert(limitless_number_from_str(&ctx, &a, "256") == LIMITLESS_OK);
  assert(limitless_number_bit_length(&ctx, &a, &bits) == LIMITLESS_OK);
  assert(bits == 9);
  assert(limitless_number_get_bit(&ctx, &a, 8, &bit) == LIMITLESS_OK);
  assert(bit == 1);
  assert(limitless_number_get_bit(&ctx, &a, 7, &bit) == LIMITLESS_OK);
  assert(bit == 0);
  assert(limitless_number_set_bit(&ctx, &out, &a, 0) == LIMITLESS_OK);
  expect_str(&ctx, &out, "257");
  assert(limitless_number_clear_bit(&ctx, &out, &out, 8) == LIMITLESS_OK);
  expect_str(&ctx, &out, "1");

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &q);
  limitless_number_clear(&ctx, &r);
  limitless_number_clear(&ctx, &out);
}

static void test_number_theory_apis(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number a, b, g, x, y, left, right, out;
  int prime;
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &g) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &x) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &y) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &left) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &right) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);

  assert(limitless_number_from_i64(&ctx, &a, 21) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, -6) == LIMITLESS_OK);
  assert(limitless_number_lcm(&ctx, &out, &a, &b) == LIMITLESS_OK);
  expect_str(&ctx, &out, "42");

  assert(limitless_number_from_i64(&ctx, &a, 240) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 46) == LIMITLESS_OK);
  assert(limitless_number_extended_gcd(&ctx, &g, &x, &y, &a, &b) == LIMITLESS_OK);
  expect_str(&ctx, &g, "2");
  assert(limitless_number_mul(&ctx, &left, &a, &x) == LIMITLESS_OK);
  assert(limitless_number_mul(&ctx, &right, &b, &y) == LIMITLESS_OK);
  assert(limitless_number_add(&ctx, &out, &left, &right) == LIMITLESS_OK);
  expect_str(&ctx, &out, "2");

  assert(limitless_number_from_i64(&ctx, &a, -3) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 11) == LIMITLESS_OK);
  assert(limitless_number_mod_inverse(&ctx, &out, &a, &b) == LIMITLESS_OK);
  expect_str(&ctx, &out, "7");
  assert(limitless_number_from_i64(&ctx, &a, 6) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 9) == LIMITLESS_OK);
  assert(limitless_number_mod_inverse(&ctx, &out, &a, &b) == LIMITLESS_EINVAL);

  assert(limitless_number_from_str(&ctx, &a, "2305843009213693951") == LIMITLESS_OK);
  assert(limitless_number_is_prime(&ctx, &a, &prime) == LIMITLESS_OK);
  assert(prime == 1);
  assert(limitless_number_from_str(&ctx, &a, "341550071728321") == LIMITLESS_OK);
  assert(limitless_number_is_prime(&ctx, &a, &prime) == LIMITLESS_OK);
  assert(prime == 0);
  assert(limitless_number_from_i64(&ctx, &a, 2) == LIMITLESS_OK);
  assert(limitless_number_next_prime(&ctx, &out, &a) == LIMITLESS_OK);
  expect_str(&ctx, &out, "3");
  assert(limitless_number_from_i64(&ctx, &a, 14) == LIMITLESS_OK);
  assert(limitless_number_next_prime(&ctx, &out, &a) == LIMITLESS_OK);
  expect_str(&ctx, &out, "17");

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &g);
  limitless_number_clear(&ctx, &x);
  limitless_number_clear(&ctx, &y);
  limitless_number_clear(&ctx, &left);
  limitless_number_clear(&ctx, &right);
  limitless_number_clear(&ctx, &out);
}

static void test_parse_binary_conversion_and_rounding(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number value, out, denominator;
  limitless_u8 binary[16];
  limitless_size written = 0;
  float f;
  double d;
  static const limitless_u8 expected[] = {0x04u, 0x01u, 0x07u, 0x0cu};
  static const limitless_u8 noncanonical[] = {0x01u, 0x00u};
  assert(limitless_number_init(&ctx, &value) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &denominator) == LIMITLESS_OK);

  assert(limitless_number_from_str(&ctx, &value, "1.25") == LIMITLESS_OK);
  expect_str(&ctx, &value, "5/4");
  assert(limitless_number_from_decimal(&ctx, &value, "-3.5e-2") == LIMITLESS_OK);
  expect_str(&ctx, &value, "-7/200");
  assert(limitless_number_from_str(&ctx, &value, ".5e+3") == LIMITLESS_OK);
  expect_str(&ctx, &value, "500");
  assert(limitless_number_from_str(&ctx, &value, "1e-3") == LIMITLESS_OK);
  expect_str(&ctx, &value, "1/1000");
  assert(limitless_number_from_str(&ctx, &value, "bad") == LIMITLESS_EPARSE);

  assert(limitless_number_from_str(&ctx, &value, "-7/12") == LIMITLESS_OK);
  assert(limitless_number_to_binary(&ctx, &value, NULL, 0, &written) == LIMITLESS_EBUF);
  assert(written == (limitless_size)sizeof(expected));
  assert(limitless_number_to_bytes(&ctx, &value, binary, (limitless_size)sizeof(binary), &written) == LIMITLESS_OK);
  assert(memcmp(binary, expected, sizeof(expected)) == 0);
  assert(limitless_number_from_binary(&ctx, &out, binary, written) == LIMITLESS_OK);
  expect_str(&ctx, &out, "-7/12");
  assert(limitless_number_from_bytes(&ctx, &out, noncanonical, (limitless_size)sizeof(noncanonical)) == LIMITLESS_EPARSE);
  expect_str(&ctx, &out, "-7/12");

  assert(limitless_number_from_str(&ctx, &value, "1/2") == LIMITLESS_OK);
  assert(limitless_number_to_float(&ctx, &value, &f) == LIMITLESS_OK);
  assert(f == 0.5f);
  assert(limitless_number_to_double(&ctx, &value, &d) == LIMITLESS_OK);
  assert(d == 0.5);
  assert(limitless_number_from_i64(&ctx, &value, 1) == LIMITLESS_OK);
  assert(limitless_number_shl(&ctx, &value, &value, 2000) == LIMITLESS_OK);
  assert(limitless_number_to_double(&ctx, &value, &d) == LIMITLESS_ERANGE);
  assert(d > 1.0e300);
  assert(limitless_number_from_i64(&ctx, &value, 1) == LIMITLESS_OK);
  assert(limitless_number_shl(&ctx, &denominator, &value, 1075) == LIMITLESS_OK);
  assert(limitless_number_div(&ctx, &value, &value, &denominator) == LIMITLESS_OK);
  assert(limitless_number_to_double(&ctx, &value, &d) == LIMITLESS_OK);
  assert(d == 0.0);

  assert(limitless_number_from_str(&ctx, &value, "-7/3") == LIMITLESS_OK);
  assert(limitless_number_floor(&ctx, &out, &value) == LIMITLESS_OK);
  expect_str(&ctx, &out, "-3");
  assert(limitless_number_ceil(&ctx, &out, &value) == LIMITLESS_OK);
  expect_str(&ctx, &out, "-2");
  assert(limitless_number_trunc(&ctx, &out, &value) == LIMITLESS_OK);
  expect_str(&ctx, &out, "-2");

  limitless_number_clear(&ctx, &value);
  limitless_number_clear(&ctx, &out);
  limitless_number_clear(&ctx, &denominator);
}

int main(void) {
  test_integer_and_bit_apis();
  test_number_theory_apis();
  test_parse_binary_conversion_and_rounding();
  printf("extended api tests ok\n");
  return 0;
}
