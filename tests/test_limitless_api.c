/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <limits.h>
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
  char buf[4096];
  limitless_size written = 0;
  assert(limitless_number_to_cstr(ctx, n, base, buf, (limitless_size)sizeof(buf), &written) == LIMITLESS_OK);
  assert(strcmp(buf, expect) == 0);
  assert(written == (limitless_size)strlen(expect));
}

static void check_cmp_eq(limitless_ctx* ctx, const limitless_number* a, const limitless_number* b) {
  limitless_status st = LIMITLESS_OK;
  assert(limitless_number_cmp(ctx, a, b, &st) == 0);
  assert(st == LIMITLESS_OK);
}

static void set_marker(limitless_ctx* ctx, limitless_number* n) {
  assert(limitless_number_from_str(ctx, n, "777/13") == LIMITLESS_OK);
}

static void expect_marker(limitless_ctx* ctx, const limitless_number* n) {
  check_str(ctx, n, 10, "777/13");
}

static void format_ulong_dec(unsigned long v, char* out, size_t cap) {
  char rev[64];
  size_t i = 0;
  size_t j;
  assert(cap > 0);
  do {
    rev[i++] = (char)('0' + (v % 10UL));
    v /= 10UL;
  } while (v != 0UL);
  assert(i + 1 <= cap);
  for (j = 0; j < i; ++j) out[j] = rev[i - 1 - j];
  out[i] = '\0';
}

static void format_long_dec(long v, char* out, size_t cap) {
  if (v < 0) {
    unsigned long mag;
    assert(cap > 1);
    out[0] = '-';
    mag = (unsigned long)(-(v + 1L)) + 1UL;
    format_ulong_dec(mag, out + 1, cap - 1);
    return;
  }
  format_ulong_dec((unsigned long)v, out, cap);
}

static void format_ull_dec(unsigned long long v, char* out, size_t cap) {
  char rev[64];
  size_t i = 0;
  size_t j;
  assert(cap > 0);
  do {
    rev[i++] = (char)('0' + (v % 10ULL));
    v /= 10ULL;
  } while (v != 0ULL);
  assert(i + 1 <= cap);
  for (j = 0; j < i; ++j) out[j] = rev[i - 1 - j];
  out[i] = '\0';
}

static void format_ll_dec(long long v, char* out, size_t cap) {
  if (v < 0) {
    unsigned long long mag;
    assert(cap > 1);
    out[0] = '-';
    mag = (unsigned long long)(-(v + 1LL)) + 1ULL;
    format_ull_dec(mag, out + 1, cap - 1);
    return;
  }
  format_ull_dec((unsigned long long)v, out, cap);
}

static void set_invalid_kind(limitless_number* n) {
  memset(&n->kind, 0x7f, sizeof(n->kind));
}

static void test_constructor_variants_and_copy(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number n;
  limitless_number copy;
  long sv[] = {0L, 1L, -1L, LONG_MIN, LONG_MAX};
  unsigned long uv[] = {0UL, 1UL, 17UL, ULONG_MAX};
  long long llv[] = {0LL, 1LL, -1LL, LLONG_MIN, LLONG_MAX};
  unsigned long long ullv[] = {0ULL, 1ULL, 19ULL, ULLONG_MAX};
  size_t i;

  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &copy) == LIMITLESS_OK);

  for (i = 0; i < sizeof(sv) / sizeof(sv[0]); ++i) {
    char expect[128];
    assert(limitless_number_from_long(&ctx, &n, sv[i]) == LIMITLESS_OK);
    format_long_dec(sv[i], expect, sizeof(expect));
    check_str(&ctx, &n, 10, expect);
  }

  for (i = 0; i < sizeof(uv) / sizeof(uv[0]); ++i) {
    char expect[128];
    assert(limitless_number_from_ulong(&ctx, &n, uv[i]) == LIMITLESS_OK);
    format_ulong_dec(uv[i], expect, sizeof(expect));
    check_str(&ctx, &n, 10, expect);
  }

  for (i = 0; i < sizeof(llv) / sizeof(llv[0]); ++i) {
    char expect[128];
    assert(limitless_number_from_ll(&ctx, &n, llv[i]) == LIMITLESS_OK);
    format_ll_dec(llv[i], expect, sizeof(expect));
    check_str(&ctx, &n, 10, expect);
  }

  for (i = 0; i < sizeof(ullv) / sizeof(ullv[0]); ++i) {
    char expect[128];
    assert(limitless_number_from_ull(&ctx, &n, ullv[i]) == LIMITLESS_OK);
    format_ull_dec(ullv[i], expect, sizeof(expect));
    check_str(&ctx, &n, 10, expect);
  }

  assert(limitless_number_from_str(&ctx, &n, "-9876543210123456789/1000000000000000003") == LIMITLESS_OK);
  assert(limitless_number_copy(&ctx, &copy, &n) == LIMITLESS_OK);
  check_cmp_eq(&ctx, &copy, &n);

  assert(limitless_number_from_str(&ctx, &n, "123456789/37") == LIMITLESS_OK);
  assert(limitless_number_copy(&ctx, &n, &n) == LIMITLESS_OK);
  check_str(&ctx, &n, 10, "123456789/37");

  limitless_number_clear(&ctx, &n);
  limitless_number_clear(&ctx, &copy);
}

typedef limitless_status (*binary_op_fn)(limitless_ctx*, limitless_number*, const limitless_number*, const limitless_number*);
typedef limitless_status (*unary_op_fn)(limitless_ctx*, limitless_number*, const limitless_number*);

static void check_binary_alias(limitless_ctx* ctx, binary_op_fn fn, const char* a_s, const char* b_s, const char* out_s) {
  limitless_number a;
  limitless_number b;
  limitless_number out;
  assert(limitless_number_init(ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(ctx, &out) == LIMITLESS_OK);

  assert(limitless_number_from_str(ctx, &a, a_s) == LIMITLESS_OK);
  assert(limitless_number_from_str(ctx, &b, b_s) == LIMITLESS_OK);
  assert(fn(ctx, &out, &a, &b) == LIMITLESS_OK);
  check_str(ctx, &out, 10, out_s);

  assert(limitless_number_from_str(ctx, &a, a_s) == LIMITLESS_OK);
  assert(limitless_number_from_str(ctx, &b, b_s) == LIMITLESS_OK);
  assert(fn(ctx, &a, &a, &b) == LIMITLESS_OK);
  check_str(ctx, &a, 10, out_s);

  assert(limitless_number_from_str(ctx, &a, a_s) == LIMITLESS_OK);
  assert(limitless_number_from_str(ctx, &b, b_s) == LIMITLESS_OK);
  assert(fn(ctx, &b, &a, &b) == LIMITLESS_OK);
  check_str(ctx, &b, 10, out_s);

  limitless_number_clear(ctx, &a);
  limitless_number_clear(ctx, &b);
  limitless_number_clear(ctx, &out);
}

static void check_unary_alias(limitless_ctx* ctx, unary_op_fn fn, const char* in_s, const char* out_s) {
  limitless_number in;
  limitless_number out;
  assert(limitless_number_init(ctx, &in) == LIMITLESS_OK);
  assert(limitless_number_init(ctx, &out) == LIMITLESS_OK);

  assert(limitless_number_from_str(ctx, &in, in_s) == LIMITLESS_OK);
  assert(fn(ctx, &out, &in) == LIMITLESS_OK);
  check_str(ctx, &out, 10, out_s);

  assert(limitless_number_from_str(ctx, &in, in_s) == LIMITLESS_OK);
  assert(fn(ctx, &in, &in) == LIMITLESS_OK);
  check_str(ctx, &in, 10, out_s);

  limitless_number_clear(ctx, &in);
  limitless_number_clear(ctx, &out);
}

static void test_aliasing_and_inplace(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number a;
  limitless_number b;

  check_binary_alias(&ctx, limitless_number_add, "7/3", "5/2", "29/6");
  check_binary_alias(&ctx, limitless_number_sub, "7/3", "5/2", "-1/6");
  check_binary_alias(&ctx, limitless_number_mul, "7/3", "5/2", "35/6");
  check_binary_alias(&ctx, limitless_number_div, "7/3", "5/2", "14/15");

  check_unary_alias(&ctx, limitless_number_neg, "-9/4", "9/4");
  check_unary_alias(&ctx, limitless_number_abs, "-9/4", "9/4");

  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);

  assert(limitless_number_from_i64(&ctx, &a, -48) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 18) == LIMITLESS_OK);
  assert(limitless_number_gcd(&ctx, &a, &a, &b) == LIMITLESS_OK);
  check_str(&ctx, &a, 10, "6");

  assert(limitless_number_from_i64(&ctx, &a, -2) == LIMITLESS_OK);
  assert(limitless_number_pow_u64(&ctx, &a, &a, 5) == LIMITLESS_OK);
  check_str(&ctx, &a, 10, "-32");

  assert(limitless_number_from_i64(&ctx, &a, 4) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 497) == LIMITLESS_OK);
  assert(limitless_number_modexp_u64(&ctx, &a, &a, 13, &b) == LIMITLESS_OK);
  check_str(&ctx, &a, 10, "445");

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
}

static void test_failure_atomicity_matrix(void) {
  fail_alloc_state state;
  limitless_alloc alloc;
  limitless_ctx ctx;
  limitless_number out;
  limitless_number lhs;
  limitless_number rhs;
  limitless_number i1;
  limitless_number i2;
  limitless_number mod;

  state.fail_after = 1000000;
  state.calls = 0;
  alloc.alloc = fail_alloc;
  alloc.realloc = fail_realloc;
  alloc.free = fail_free;
  alloc.user = &state;

  assert(limitless_ctx_init(&ctx, &alloc) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &lhs) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &rhs) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &i1) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &i2) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &mod) == LIMITLESS_OK);

  assert(limitless_number_from_str(&ctx, &lhs,
                                   "12345678901234567890123456789012345678901234567890/97") == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &rhs,
                                   "-99887766554433221100998877665544332211009988776655/89") == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &i1,
                                   "1122334455667788990011223344556677889900112233445566778899") == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &i2,
                                   "9988776655443322110099887766554433221100998877665544332211") == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &mod,
                                   "100000000000000000000000000000000000000000000000003") == LIMITLESS_OK);

  set_marker(&ctx, &out);
  state.fail_after = state.calls;
  assert(limitless_number_copy(&ctx, &out, &lhs) == LIMITLESS_EOOM);
  state.fail_after = 1000000;
  expect_marker(&ctx, &out);

  set_marker(&ctx, &out);
  state.fail_after = state.calls;
  assert(limitless_number_from_cstr(&ctx, &out,
                                    "123456789012345678901234567890123456789012345678901234567890/1234567890123456789",
                                    10) == LIMITLESS_EOOM);
  state.fail_after = 1000000;
  expect_marker(&ctx, &out);

  set_marker(&ctx, &out);
  state.fail_after = state.calls;
  assert(limitless_number_from_double_exact(&ctx, &out, 0.1) == LIMITLESS_EOOM);
  state.fail_after = 1000000;
  expect_marker(&ctx, &out);

  set_marker(&ctx, &out);
  state.fail_after = state.calls;
  assert(limitless_number_add(&ctx, &out, &lhs, &rhs) == LIMITLESS_EOOM);
  state.fail_after = 1000000;
  expect_marker(&ctx, &out);

  set_marker(&ctx, &out);
  state.fail_after = state.calls;
  assert(limitless_number_sub(&ctx, &out, &lhs, &rhs) == LIMITLESS_EOOM);
  state.fail_after = 1000000;
  expect_marker(&ctx, &out);

  set_marker(&ctx, &out);
  state.fail_after = state.calls;
  assert(limitless_number_mul(&ctx, &out, &lhs, &rhs) == LIMITLESS_EOOM);
  state.fail_after = 1000000;
  expect_marker(&ctx, &out);

  set_marker(&ctx, &out);
  state.fail_after = state.calls;
  assert(limitless_number_div(&ctx, &out, &lhs, &rhs) == LIMITLESS_EOOM);
  state.fail_after = 1000000;
  expect_marker(&ctx, &out);

  set_marker(&ctx, &out);
  state.fail_after = state.calls;
  assert(limitless_number_neg(&ctx, &out, &lhs) == LIMITLESS_EOOM);
  state.fail_after = 1000000;
  expect_marker(&ctx, &out);

  set_marker(&ctx, &out);
  state.fail_after = state.calls;
  assert(limitless_number_abs(&ctx, &out, &rhs) == LIMITLESS_EOOM);
  state.fail_after = 1000000;
  expect_marker(&ctx, &out);

  set_marker(&ctx, &out);
  state.fail_after = state.calls;
  assert(limitless_number_gcd(&ctx, &out, &i1, &i2) == LIMITLESS_EOOM);
  state.fail_after = 1000000;
  expect_marker(&ctx, &out);

  set_marker(&ctx, &out);
  state.fail_after = state.calls;
  assert(limitless_number_pow_u64(&ctx, &out, &i1, 17) == LIMITLESS_EOOM);
  state.fail_after = 1000000;
  expect_marker(&ctx, &out);

  set_marker(&ctx, &out);
  state.fail_after = state.calls;
  assert(limitless_number_modexp_u64(&ctx, &out, &i1, 12345, &mod) == LIMITLESS_EOOM);
  state.fail_after = 1000000;
  expect_marker(&ctx, &out);

  limitless_number_clear(&ctx, &out);
  limitless_number_clear(&ctx, &lhs);
  limitless_number_clear(&ctx, &rhs);
  limitless_number_clear(&ctx, &i1);
  limitless_number_clear(&ctx, &i2);
  limitless_number_clear(&ctx, &mod);
}

static void test_invalid_kind_and_guard_paths(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number good;
  limitless_number weird;
  limitless_status st = LIMITLESS_OK;
  char buf[64];

  assert(limitless_number_init(&ctx, &good) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &weird) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &good, 11) == LIMITLESS_OK);

  set_invalid_kind(&weird);
  assert(limitless_number_copy(&ctx, &good, &weird) == LIMITLESS_EINVAL);
  assert(limitless_number_to_cstr(&ctx, &weird, 10, buf, (limitless_size)sizeof(buf), NULL) == LIMITLESS_EINVAL);
  assert(limitless_number_is_zero(&weird) == 1);
  assert(limitless_number_is_integer(&weird) == 0);
  assert(limitless_number_sign(&weird) == 0);

  assert(limitless_number_cmp(NULL, &good, &good, &st) == 0);
  assert(st == LIMITLESS_EINVAL);
  assert(limitless_number_cmp(&ctx, NULL, &good, &st) == 0);
  assert(st == LIMITLESS_EINVAL);

  weird.kind = LIMITLESS_KIND_INT;
  limitless_number_clear(&ctx, &good);
  limitless_number_clear(&ctx, &weird);
}

static void test_deep_oom_sweeps(void) {
  fail_alloc_state state;
  limitless_alloc alloc;
  limitless_ctx ctx;
  limitless_number out;
  limitless_number neg_int;
  limitless_number neg_rat;
  limitless_number div_a;
  limitless_number div_b;
  limitless_number cmp_a;
  limitless_number cmp_b;
  limitless_number pow_base;
  limitless_number mod;
  int step;
  int saw_oom;
  char buf[1024];
  limitless_status cmp_st;
  limitless_size written = 0;

  state.fail_after = 1000000;
  state.calls = 0;
  alloc.alloc = fail_alloc;
  alloc.realloc = fail_realloc;
  alloc.free = fail_free;
  alloc.user = &state;
  assert(limitless_ctx_init(&ctx, &alloc) == LIMITLESS_OK);

  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &neg_int) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &neg_rat) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &div_a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &div_b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &cmp_a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &cmp_b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &pow_base) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &mod) == LIMITLESS_OK);

  assert(limitless_number_from_i64(&ctx, &neg_int, -123456789) == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &neg_rat, "-9876543210123456789/97") == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &div_a, "123456789012345678901234567890123456789") == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &div_b, "97") == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &cmp_a, "12345678901234567890123456789/97") == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &cmp_b, "-998877665544332211009988776655/89") == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &pow_base, "11223344556677889900112233445566778899") == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &mod, "100000000000000000000000000000000000003") == LIMITLESS_OK);

  set_marker(&ctx, &out);
  state.calls = 0;
  state.fail_after = 0;
  assert(limitless_number_from_i64(&ctx, &out, 1) == LIMITLESS_EOOM);
  state.fail_after = 1000000;
  expect_marker(&ctx, &out);

  set_marker(&ctx, &out);
  state.calls = 0;
  state.fail_after = 0;
  assert(limitless_number_from_u64(&ctx, &out, 1u) == LIMITLESS_EOOM);
  state.fail_after = 1000000;
  expect_marker(&ctx, &out);

  saw_oom = 0;
  for (step = 0; step < 256; ++step) {
    state.fail_after = 1000000;
    state.calls = 0;
    set_marker(&ctx, &out);
    state.fail_after = step;
    state.calls = 0;
    if (limitless_number_from_cstr(&ctx, &out,
                                   "12345678901234567890123456789012345678901234567890/1",
                                   10) == LIMITLESS_EOOM) {
      saw_oom = 1;
      state.fail_after = 1000000;
      expect_marker(&ctx, &out);
    }
  }
  assert(saw_oom);

  saw_oom = 0;
  for (step = 0; step < 128; ++step) {
    state.calls = 0;
    state.fail_after = step;
    if (limitless_number_to_cstr(&ctx, &neg_int, 10, buf, (limitless_size)sizeof(buf), &written) == LIMITLESS_EOOM) {
      saw_oom = 1;
    }
  }
  assert(saw_oom);

  saw_oom = 0;
  for (step = 0; step < 128; ++step) {
    state.calls = 0;
    state.fail_after = step;
    if (limitless_number_to_cstr(&ctx, &neg_rat, 10, buf, (limitless_size)sizeof(buf), &written) == LIMITLESS_EOOM) {
      saw_oom = 1;
    }
  }
  assert(saw_oom);

  saw_oom = 0;
  for (step = 0; step < 256; ++step) {
    state.fail_after = 1000000;
    state.calls = 0;
    set_marker(&ctx, &out);
    state.fail_after = step;
    state.calls = 0;
    if (limitless_number_div(&ctx, &out, &div_a, &div_b) == LIMITLESS_EOOM) {
      saw_oom = 1;
      state.fail_after = 1000000;
      expect_marker(&ctx, &out);
    }
  }
  assert(saw_oom);

  saw_oom = 0;
  for (step = 0; step < 256; ++step) {
    state.calls = 0;
    state.fail_after = step;
    (void)limitless_number_cmp(&ctx, &cmp_a, &cmp_b, &cmp_st);
    if (cmp_st == LIMITLESS_EOOM) {
      saw_oom = 1;
    }
  }
  assert(saw_oom);

  saw_oom = 0;
  for (step = 0; step < 256; ++step) {
    state.fail_after = 1000000;
    state.calls = 0;
    set_marker(&ctx, &out);
    state.fail_after = step;
    state.calls = 0;
    if (limitless_number_pow_u64(&ctx, &out, &pow_base, 29) == LIMITLESS_EOOM) {
      saw_oom = 1;
      state.fail_after = 1000000;
      expect_marker(&ctx, &out);
    }
  }
  assert(saw_oom);

  saw_oom = 0;
  for (step = 0; step < 384; ++step) {
    state.fail_after = 1000000;
    state.calls = 0;
    set_marker(&ctx, &out);
    state.fail_after = step;
    state.calls = 0;
    if (limitless_number_modexp_u64(&ctx, &out, &pow_base, 33, &mod) == LIMITLESS_EOOM) {
      saw_oom = 1;
      state.fail_after = 1000000;
      expect_marker(&ctx, &out);
    }
  }
  assert(saw_oom);

  state.fail_after = 1000000;
  limitless_number_clear(&ctx, &out);
  limitless_number_clear(&ctx, &neg_int);
  limitless_number_clear(&ctx, &neg_rat);
  limitless_number_clear(&ctx, &div_a);
  limitless_number_clear(&ctx, &div_b);
  limitless_number_clear(&ctx, &cmp_a);
  limitless_number_clear(&ctx, &cmp_b);
  limitless_number_clear(&ctx, &pow_base);
  limitless_number_clear(&ctx, &mod);
}

int main(void) {
  test_constructor_variants_and_copy();
  test_aliasing_and_inplace();
  test_failure_atomicity_matrix();
  test_invalid_kind_and_guard_paths();
  test_deep_oom_sweeps();
  printf("api/alias/failure-atomic tests ok\n");
  return 0;
}
