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

static void check_str(limitless_ctx* ctx, const limitless_number* n, int base, const char* expect) {
  char buf[2048];
  limitless_size written = 0;
  limitless_status st = limitless_number_to_cstr(ctx, n, base, buf, (limitless_size)sizeof(buf), &written);
  assert(st == LIMITLESS_OK);
  assert(strcmp(buf, expect) == 0);
  assert(written == (limitless_size)strlen(expect));
}

static void test_min_basic(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number a, b, out;
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);

  assert(limitless_number_from_i64(&ctx, &a, 3) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 7) == LIMITLESS_OK);
  assert(limitless_number_min(&ctx, &out, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &out, 10, "3");

  assert(limitless_number_min(&ctx, &out, &b, &a) == LIMITLESS_OK);
  check_str(&ctx, &out, 10, "3");

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
}

static void test_max_basic(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number a, b, out;
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);

  assert(limitless_number_from_i64(&ctx, &a, 3) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 7) == LIMITLESS_OK);
  assert(limitless_number_max(&ctx, &out, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &out, 10, "7");

  assert(limitless_number_max(&ctx, &out, &b, &a) == LIMITLESS_OK);
  check_str(&ctx, &out, 10, "7");

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
}

static void test_min_max_equal(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number a, b, out;
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);

  assert(limitless_number_from_i64(&ctx, &a, 42) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 42) == LIMITLESS_OK);
  assert(limitless_number_min(&ctx, &out, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &out, 10, "42");
  assert(limitless_number_max(&ctx, &out, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &out, 10, "42");

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
}

static void test_min_max_negative(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number a, b, out;
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);

  assert(limitless_number_from_i64(&ctx, &a, -10) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 5) == LIMITLESS_OK);
  assert(limitless_number_min(&ctx, &out, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &out, 10, "-10");
  assert(limitless_number_max(&ctx, &out, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &out, 10, "5");

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
}

static void test_min_max_rational(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number a, b, out;
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);

  assert(limitless_number_from_cstr(&ctx, &a, "1/3", 10) == LIMITLESS_OK);
  assert(limitless_number_from_cstr(&ctx, &b, "1/2", 10) == LIMITLESS_OK);
  assert(limitless_number_min(&ctx, &out, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &out, 10, "1/3");
  assert(limitless_number_max(&ctx, &out, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &out, 10, "1/2");

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
}

static void test_min_max_null(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number a, out;
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &a, 1) == LIMITLESS_OK);

  assert(limitless_number_min(NULL, &out, &a, &a) == LIMITLESS_EINVAL);
  assert(limitless_number_min(&ctx, NULL, &a, &a) == LIMITLESS_EINVAL);
  assert(limitless_number_min(&ctx, &out, NULL, &a) == LIMITLESS_EINVAL);
  assert(limitless_number_min(&ctx, &out, &a, NULL) == LIMITLESS_EINVAL);

  assert(limitless_number_max(NULL, &out, &a, &a) == LIMITLESS_EINVAL);
  assert(limitless_number_max(&ctx, NULL, &a, &a) == LIMITLESS_EINVAL);
  assert(limitless_number_max(&ctx, &out, NULL, &a) == LIMITLESS_EINVAL);
  assert(limitless_number_max(&ctx, &out, &a, NULL) == LIMITLESS_EINVAL);

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &out);
}

static void test_is_negative_positive(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number n;
  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);

  assert(limitless_number_from_i64(&ctx, &n, 5) == LIMITLESS_OK);
  assert(limitless_number_is_positive(&n) == 1);
  assert(limitless_number_is_negative(&n) == 0);

  assert(limitless_number_from_i64(&ctx, &n, -5) == LIMITLESS_OK);
  assert(limitless_number_is_positive(&n) == 0);
  assert(limitless_number_is_negative(&n) == 1);

  assert(limitless_number_from_i64(&ctx, &n, 0) == LIMITLESS_OK);
  assert(limitless_number_is_positive(&n) == 0);
  assert(limitless_number_is_negative(&n) == 0);

  limitless_number_clear(&ctx, &n);
}

static void test_is_negative_positive_rational(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number n;
  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);

  assert(limitless_number_from_cstr(&ctx, &n, "3/7", 10) == LIMITLESS_OK);
  assert(limitless_number_is_positive(&n) == 1);
  assert(limitless_number_is_negative(&n) == 0);

  assert(limitless_number_from_cstr(&ctx, &n, "-3/7", 10) == LIMITLESS_OK);
  assert(limitless_number_is_positive(&n) == 0);
  assert(limitless_number_is_negative(&n) == 1);

  limitless_number_clear(&ctx, &n);
}

static void test_is_negative_positive_null(void) {
  assert(limitless_number_is_negative(NULL) == 0);
  assert(limitless_number_is_positive(NULL) == 0);
}

static void test_min_max_inplace(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number a, b;
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);

  assert(limitless_number_from_i64(&ctx, &a, 10) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 20) == LIMITLESS_OK);
  assert(limitless_number_min(&ctx, &a, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &a, 10, "10");

  assert(limitless_number_from_i64(&ctx, &a, 10) == LIMITLESS_OK);
  assert(limitless_number_max(&ctx, &a, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &a, 10, "20");

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
}

int main(void) {
  test_min_basic();
  test_max_basic();
  test_min_max_equal();
  test_min_max_negative();
  test_min_max_rational();
  test_min_max_null();
  test_is_negative_positive();
  test_is_negative_positive_rational();
  test_is_negative_positive_null();
  test_min_max_inplace();
  printf("ok\n");
  return 0;
}
