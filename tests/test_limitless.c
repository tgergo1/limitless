/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIMITLESS_IMPLEMENTATION
#include "../limitless.h"

typedef struct fail_alloc_state {
  int fail_after;
  int calls;
} fail_alloc_state;

static void* fail_alloc(void* user, limitless_size size) {
  fail_alloc_state* st = (fail_alloc_state*)user;
  if (st->calls++ >= st->fail_after) return NULL;
  return malloc((size_t)size);
}

static void* fail_realloc(void* user, void* ptr, limitless_size old_size, limitless_size new_size) {
  fail_alloc_state* st = (fail_alloc_state*)user;
  (void)old_size;
  if (st->calls++ >= st->fail_after) return NULL;
  return realloc(ptr, (size_t)new_size);
}

static void fail_free(void* user, void* ptr, limitless_size size) {
  (void)user;
  (void)size;
  free(ptr);
}

static limitless_ctx make_ctx(void) {
  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  return ctx;
}

static void check_str(limitless_ctx* ctx, const limitless_number* n, int base, const char* expect) {
  char buf[2048];
  limitless_size written = 0;
  limitless_status st = limitless_number_to_cstr(ctx, n, base, buf, (limitless_size)sizeof(buf), &written);
  assert(st == LIMITLESS_OK);
  assert(strcmp(buf, expect) == 0);
  assert(written == (limitless_size)strlen(expect));
}

static void test_basic_arithmetic(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number a, b, c;
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &c) == LIMITLESS_OK);

  assert(limitless_number_from_i64(&ctx, &a, 7) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 3) == LIMITLESS_OK);
  assert(limitless_number_add(&ctx, &c, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &c, 10, "10");
  assert(limitless_number_sub(&ctx, &c, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &c, 10, "4");
  assert(limitless_number_mul(&ctx, &c, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &c, 10, "21");
  assert(limitless_number_div(&ctx, &c, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &c, 10, "7/3");

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &c);
}

static void test_rational_normalization(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number n;
  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);
  assert(limitless_number_from_cstr(&ctx, &n, "-8/-12", 10) == LIMITLESS_OK);
  check_str(&ctx, &n, 10, "2/3");
  assert(limitless_number_from_cstr(&ctx, &n, "6/3", 10) == LIMITLESS_OK);
  assert(limitless_number_is_integer(&n));
  check_str(&ctx, &n, 10, "2");
  limitless_number_clear(&ctx, &n);
}

static void test_large_and_karatsuba_path(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number a, b, c;
  char big1[2200];
  char big2[2200];
  int i;
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &c) == LIMITLESS_OK);

  for (i = 0; i < 2000; ++i) {
    big1[i] = (char)('1' + (i % 9));
    big2[i] = (char)('9' - (i % 9));
  }
  big1[2000] = '\0';
  big2[2000] = '\0';

  assert(limitless_number_from_cstr(&ctx, &a, big1, 10) == LIMITLESS_OK);
  assert(limitless_number_from_cstr(&ctx, &b, big2, 10) == LIMITLESS_OK);
  limitless_ctx_set_karatsuba_threshold(&ctx, 8);
  assert(limitless_number_mul(&ctx, &c, &a, &b) == LIMITLESS_OK);
  assert(!limitless_number_is_zero(&c));

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &c);
}

static void test_gcd_pow_modexp(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number a, b, out, mod;
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &mod) == LIMITLESS_OK);

  assert(limitless_number_from_i64(&ctx, &a, -48) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 18) == LIMITLESS_OK);
  assert(limitless_number_gcd(&ctx, &out, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &out, 10, "6");

  assert(limitless_number_from_i64(&ctx, &a, -2) == LIMITLESS_OK);
  assert(limitless_number_pow_u64(&ctx, &out, &a, 5) == LIMITLESS_OK);
  check_str(&ctx, &out, 10, "-32");

  assert(limitless_number_from_i64(&ctx, &a, 4) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &mod, 497) == LIMITLESS_OK);
  assert(limitless_number_modexp_u64(&ctx, &out, &a, 13, &mod) == LIMITLESS_OK);
  check_str(&ctx, &out, 10, "445");

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
  limitless_number_clear(&ctx, &mod);
}

static void test_float_exact(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number n;
  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);

  assert(limitless_number_from_double_exact(&ctx, &n, 0.5) == LIMITLESS_OK);
  check_str(&ctx, &n, 10, "1/2");

  assert(limitless_number_from_double_exact(&ctx, &n, 0.1) == LIMITLESS_OK);
  check_str(&ctx, &n, 10, "3602879701896397/36028797018963968");

  assert(limitless_number_from_float_exact(&ctx, &n, -0.0f) == LIMITLESS_OK);
  check_str(&ctx, &n, 10, "0");

  assert(limitless_number_from_double_exact(&ctx, &n, INFINITY) == LIMITLESS_EINVAL);
  assert(limitless_number_from_double_exact(&ctx, &n, NAN) == LIMITLESS_EINVAL);

  limitless_number_clear(&ctx, &n);
}

static void test_parse_format_bases(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number n;
  char buf[1024];
  limitless_size written;
  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);

  assert(limitless_number_from_cstr(&ctx, &n, "0xff", 0) == LIMITLESS_OK);
  check_str(&ctx, &n, 10, "255");
  assert(limitless_number_to_cstr(&ctx, &n, 16, buf, sizeof(buf), &written) == LIMITLESS_OK);
  assert(strcmp(buf, "ff") == 0);

  assert(limitless_number_from_cstr(&ctx, &n, "0b101101", 0) == LIMITLESS_OK);
  check_str(&ctx, &n, 10, "45");

  assert(limitless_number_from_cstr(&ctx, &n, "z", 36) == LIMITLESS_OK);
  check_str(&ctx, &n, 10, "35");

  assert(limitless_number_from_cstr(&ctx, &n, "7/3", 10) == LIMITLESS_OK);
  assert(limitless_number_to_cstr(&ctx, &n, 10, NULL, 0, &written) == LIMITLESS_EBUF);
  assert(written == 3);

  limitless_number_clear(&ctx, &n);
}

static void test_default_aliases(void) {
  limitless_ctx ctx;
  limitless_number n;
  char buf[32];
  limitless_size written = 0;

  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &n, "0xff") == LIMITLESS_OK);
  assert(limitless_number_to_str(&ctx, &n, NULL, 0, &written) == LIMITLESS_EBUF);
  assert(written == 3);
  assert(limitless_number_to_str(&ctx, &n, buf, sizeof(buf), NULL) == LIMITLESS_OK);
  assert(strcmp(buf, "255") == 0);

  limitless_number_clear(&ctx, &n);
}

static void test_unchanged_on_failure(void) {
  fail_alloc_state state;
  limitless_alloc a;
  limitless_ctx ctx;
  limitless_number out, lhs, rhs;
  char buf[64];

  state.fail_after = 0;
  state.calls = 0;
  a.alloc = fail_alloc;
  a.realloc = fail_realloc;
  a.free = fail_free;
  a.user = &state;

  assert(limitless_ctx_init(&ctx, &a) == LIMITLESS_OK);

  state.fail_after = 1000;
  state.calls = 0;
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &lhs) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &rhs) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &out, 123) == LIMITLESS_OK);
  assert(limitless_number_from_cstr(&ctx, &lhs, "123456789012345678901234567890", 10) == LIMITLESS_OK);
  assert(limitless_number_from_cstr(&ctx, &rhs, "987654321098765432109876543210", 10) == LIMITLESS_OK);

  state.fail_after = state.calls; /* next alloc fails */
  assert(limitless_number_mul(&ctx, &out, &lhs, &rhs) == LIMITLESS_EOOM);
  state.fail_after = 1000;
  assert(limitless_number_to_cstr(&ctx, &out, 10, buf, sizeof(buf), NULL) == LIMITLESS_OK);
  assert(strcmp(buf, "123") == 0);

  limitless_number_clear(&ctx, &out);
  limitless_number_clear(&ctx, &lhs);
  limitless_number_clear(&ctx, &rhs);
}

static void test_integer_exports(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number n;
  limitless_i64 s;
  limitless_u64 u;

  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);
  assert(limitless_number_from_cstr(&ctx, &n, "9223372036854775807", 10) == LIMITLESS_OK);
  assert(limitless_number_to_i64(&ctx, &n, &s) == LIMITLESS_OK);
  assert(s == 9223372036854775807LL);

  assert(limitless_number_from_cstr(&ctx, &n, "18446744073709551615", 10) == LIMITLESS_OK);
  assert(limitless_number_to_u64(&ctx, &n, &u) == LIMITLESS_OK);
  assert(u == 18446744073709551615ULL);
  assert(limitless_number_to_i64(&ctx, &n, &s) == LIMITLESS_ERANGE);

  assert(limitless_number_from_cstr(&ctx, &n, "3/2", 10) == LIMITLESS_OK);
  assert(limitless_number_to_u64(&ctx, &n, &u) == LIMITLESS_ETYPE);

  limitless_number_clear(&ctx, &n);
}

int main(void) {
  test_basic_arithmetic();
  test_rational_normalization();
  test_large_and_karatsuba_path();
  test_gcd_pow_modexp();
  test_float_exact();
  test_parse_format_bases();
  test_default_aliases();
  test_unchanged_on_failure();
  test_integer_exports();
  printf("ok\n");
  return 0;
}
