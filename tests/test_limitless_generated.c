/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIMITLESS_IMPLEMENTATION
#include "../limitless.h"

#include "generated_core_vectors.inc"

#ifndef LIMITLESS_PROP_ITERS
#define LIMITLESS_PROP_ITERS 5000
#endif

#ifndef LIMITLESS_PROP_ITERS_EXTENDED
#define LIMITLESS_PROP_ITERS_EXTENDED 50000
#endif

#if defined(LIMITLESS_EXTENDED_STRESS)
#define LIMITLESS_PROP_RUNS LIMITLESS_PROP_ITERS_EXTENDED
#else
#define LIMITLESS_PROP_RUNS LIMITLESS_PROP_ITERS
#endif

static char* limitless_test_to_string(limitless_ctx* ctx, const limitless_number* n, int base) {
  limitless_status st;
  limitless_size need = 0;
  char* out;
  st = limitless_number_to_cstr(ctx, n, base, NULL, 0, &need);
  assert(st == LIMITLESS_EBUF || st == LIMITLESS_OK);
  out = (char*)malloc((size_t)need + 1u);
  assert(out != NULL);
  st = limitless_number_to_cstr(ctx, n, base, out, need + 1u, NULL);
  assert(st == LIMITLESS_OK);
  return out;
}

static void limitless_test_expect_str(limitless_ctx* ctx, const limitless_number* n, const char* expected) {
  char* s = limitless_test_to_string(ctx, n, 10);
  if (strcmp(s, expected) != 0) {
    fprintf(stderr, "string mismatch: got=%s expected=%s\n", s, expected);
    assert(0);
  }
  free(s);
}

static void limitless_test_expect_cmp(limitless_ctx* ctx, const limitless_number* a, const limitless_number* b, int expected) {
  limitless_status st = LIMITLESS_OK;
  int got = limitless_number_cmp(ctx, a, b, &st);
  assert(st == LIMITLESS_OK);
  if (got != expected) {
    char* sa = limitless_test_to_string(ctx, a, 10);
    char* sb = limitless_test_to_string(ctx, b, 10);
    fprintf(stderr, "cmp mismatch: a=%s b=%s got=%d expected=%d\n", sa, sb, got, expected);
    free(sa);
    free(sb);
    assert(0);
  }
}

static void limitless_test_assert_normalized(const limitless_number* n) {
  if (n->kind == LIMITLESS_KIND_INT) {
    if (n->v.i.used == 0) {
      assert(n->v.i.sign == 0);
    } else {
      assert(n->v.i.sign == 1 || n->v.i.sign == -1);
    }
    return;
  }

  assert(n->kind == LIMITLESS_KIND_RAT);
  assert(n->v.r.den.sign == 1);
  assert(n->v.r.den.used > 0);
  assert(!(n->v.r.den.used == 1 && n->v.r.den.limbs[0] == (limitless_limb)1));
  assert(n->v.r.num.used > 0);
}

static float limitless_test_float_from_bits(limitless_u32 bits) {
  union {
    limitless_u32 u;
    float f;
  } u;
  u.u = bits;
  return u.f;
}

static double limitless_test_double_from_bits(limitless_u64 bits) {
  union {
    limitless_u64 u;
    double d;
  } u;
  u.u = bits;
  return u.d;
}

static void test_default_ctx_and_aliases(void) {
  limitless_ctx ctx;
  limitless_number n;
  limitless_size needed = 0;
  char buf[64];

  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);

  assert(limitless_number_from_str(&ctx, &n, "0xff") == LIMITLESS_OK);
  limitless_test_expect_str(&ctx, &n, "255");

  assert(limitless_number_from_str(&ctx, &n, "010") == LIMITLESS_OK);
  limitless_test_expect_str(&ctx, &n, "8");

  assert(limitless_number_from_str(&ctx, &n, "-12/18") == LIMITLESS_OK);
  limitless_test_expect_str(&ctx, &n, "-2/3");

  assert(limitless_number_to_str(&ctx, &n, NULL, 0, &needed) == LIMITLESS_EBUF);
  assert(needed == 4);
  assert(limitless_number_to_str(&ctx, &n, buf, (limitless_size)sizeof(buf), NULL) == LIMITLESS_OK);
  assert(strcmp(buf, "-2/3") == 0);

  limitless_number_clear(&ctx, &n);
}

static void test_generated_binary_matrix(void) {
  limitless_ctx ctx;
  limitless_number a;
  limitless_number b;
  limitless_number out;
  limitless_size i;

  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);

  for (i = 0; i < LIMITLESS_GENERATED_BINARY_CASE_COUNT; ++i) {
    const limitless_generated_binary_case* tc = &LIMITLESS_GENERATED_BINARY_CASES[i];
    assert(limitless_number_from_str(&ctx, &a, tc->a) == LIMITLESS_OK);
    assert(limitless_number_from_str(&ctx, &b, tc->b) == LIMITLESS_OK);

    assert(limitless_number_add(&ctx, &out, &a, &b) == LIMITLESS_OK);
    limitless_test_assert_normalized(&out);
    limitless_test_expect_str(&ctx, &out, tc->add_s);

    assert(limitless_number_sub(&ctx, &out, &a, &b) == LIMITLESS_OK);
    limitless_test_assert_normalized(&out);
    limitless_test_expect_str(&ctx, &out, tc->sub_s);

    assert(limitless_number_mul(&ctx, &out, &a, &b) == LIMITLESS_OK);
    limitless_test_assert_normalized(&out);
    limitless_test_expect_str(&ctx, &out, tc->mul_s);

    assert(limitless_number_from_i64(&ctx, &out, 777) == LIMITLESS_OK);
    {
      limitless_status st = limitless_number_div(&ctx, &out, &a, &b);
      assert(st == (limitless_status)tc->div_status);
      if (st == LIMITLESS_OK) {
        limitless_test_assert_normalized(&out);
        limitless_test_expect_str(&ctx, &out, tc->div_s);
      } else {
        limitless_test_expect_str(&ctx, &out, "777");
      }
    }

    limitless_test_expect_cmp(&ctx, &a, &b, tc->cmp_ab);
  }

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
}

static void test_generated_unary_matrix(void) {
  limitless_ctx ctx;
  limitless_number value;
  limitless_number out;
  limitless_number rt;
  limitless_i64 i64v;
  limitless_u64 u64v;
  limitless_size i;

  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &value) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &rt) == LIMITLESS_OK);

  for (i = 0; i < LIMITLESS_GENERATED_UNARY_CASE_COUNT; ++i) {
    const limitless_generated_unary_case* tc = &LIMITLESS_GENERATED_UNARY_CASES[i];
    assert(limitless_number_from_str(&ctx, &value, tc->value) == LIMITLESS_OK);

    assert(limitless_number_neg(&ctx, &out, &value) == LIMITLESS_OK);
    limitless_test_assert_normalized(&out);
    limitless_test_expect_str(&ctx, &out, tc->neg_s);

    assert(limitless_number_abs(&ctx, &out, &value) == LIMITLESS_OK);
    limitless_test_assert_normalized(&out);
    limitless_test_expect_str(&ctx, &out, tc->abs_s);

    assert(limitless_number_sign(&value) == tc->sign);
    assert(limitless_number_is_zero(&value) == tc->is_zero);
    assert(limitless_number_is_integer(&value) == tc->is_integer);

    assert(limitless_number_to_i64(&ctx, &value, &i64v) == (limitless_status)tc->to_i64_status);
    if (tc->to_i64_status == LIMITLESS_OK) {
      assert(limitless_number_from_i64(&ctx, &rt, i64v) == LIMITLESS_OK);
      limitless_test_expect_cmp(&ctx, &value, &rt, 0);
    }

    assert(limitless_number_to_u64(&ctx, &value, &u64v) == (limitless_status)tc->to_u64_status);
    if (tc->to_u64_status == LIMITLESS_OK) {
      assert(limitless_number_from_u64(&ctx, &rt, u64v) == LIMITLESS_OK);
      limitless_test_expect_cmp(&ctx, &value, &rt, 0);
    }
  }

  limitless_number_clear(&ctx, &value);
  limitless_number_clear(&ctx, &out);
  limitless_number_clear(&ctx, &rt);
}

static void test_generated_base_roundtrips(void) {
  limitless_ctx ctx;
  limitless_number value;
  limitless_number parsed;
  limitless_size i;

  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &value) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &parsed) == LIMITLESS_OK);

  for (i = 0; i < LIMITLESS_GENERATED_BASE_CASE_COUNT; ++i) {
    const limitless_generated_base_case* tc = &LIMITLESS_GENERATED_BASE_CASES[i];
    char* base_repr;

    assert(limitless_number_from_str(&ctx, &value, tc->value) == LIMITLESS_OK);
    base_repr = limitless_test_to_string(&ctx, &value, tc->base);

    assert(limitless_number_from_cstr(&ctx, &parsed, base_repr, tc->base) == LIMITLESS_OK);
    limitless_test_expect_cmp(&ctx, &value, &parsed, 0);

    free(base_repr);
  }

  limitless_number_clear(&ctx, &value);
  limitless_number_clear(&ctx, &parsed);
}

static void test_generated_float_double_exact(void) {
  limitless_ctx ctx;
  limitless_number out;
  limitless_size i;

  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);

  for (i = 0; i < LIMITLESS_GENERATED_FLOAT_CASE_COUNT; ++i) {
    const limitless_generated_float_case* tc = &LIMITLESS_GENERATED_FLOAT_CASES[i];
    float fv = limitless_test_float_from_bits(tc->bits);

    assert(limitless_number_from_i64(&ctx, &out, 333) == LIMITLESS_OK);
    {
      limitless_status st = limitless_number_from_float_exact(&ctx, &out, fv);
      assert(st == (limitless_status)tc->status);
      if (st == LIMITLESS_OK) {
        limitless_test_assert_normalized(&out);
        limitless_test_expect_str(&ctx, &out, tc->expected);
      } else {
        limitless_test_expect_str(&ctx, &out, "333");
      }
    }
  }

  for (i = 0; i < LIMITLESS_GENERATED_DOUBLE_CASE_COUNT; ++i) {
    const limitless_generated_double_case* tc = &LIMITLESS_GENERATED_DOUBLE_CASES[i];
    double dv = limitless_test_double_from_bits(tc->bits);

    assert(limitless_number_from_i64(&ctx, &out, 444) == LIMITLESS_OK);
    {
      limitless_status st = limitless_number_from_double_exact(&ctx, &out, dv);
      assert(st == (limitless_status)tc->status);
      if (st == LIMITLESS_OK) {
        limitless_test_assert_normalized(&out);
        limitless_test_expect_str(&ctx, &out, tc->expected);
      } else {
        limitless_test_expect_str(&ctx, &out, "444");
      }
    }
  }

  limitless_number_clear(&ctx, &out);
}

static void test_generated_advanced_ops(void) {
  limitless_ctx ctx;
  limitless_number a;
  limitless_number b;
  limitless_number out;
  limitless_size i;

  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);

  for (i = 0; i < LIMITLESS_GENERATED_GCD_CASE_COUNT; ++i) {
    const limitless_generated_gcd_case* tc = &LIMITLESS_GENERATED_GCD_CASES[i];
    assert(limitless_number_from_str(&ctx, &a, tc->a) == LIMITLESS_OK);
    assert(limitless_number_from_str(&ctx, &b, tc->b) == LIMITLESS_OK);
    assert(limitless_number_gcd(&ctx, &out, &a, &b) == LIMITLESS_OK);
    limitless_test_expect_str(&ctx, &out, tc->g);
  }

  for (i = 0; i < LIMITLESS_GENERATED_POW_CASE_COUNT; ++i) {
    const limitless_generated_pow_case* tc = &LIMITLESS_GENERATED_POW_CASES[i];
    assert(limitless_number_from_str(&ctx, &a, tc->base) == LIMITLESS_OK);
    assert(limitless_number_pow_u64(&ctx, &out, &a, tc->exp) == LIMITLESS_OK);
    limitless_test_expect_str(&ctx, &out, tc->out);
  }

  for (i = 0; i < LIMITLESS_GENERATED_MODEXP_CASE_COUNT; ++i) {
    const limitless_generated_modexp_case* tc = &LIMITLESS_GENERATED_MODEXP_CASES[i];
    assert(limitless_number_from_str(&ctx, &a, tc->base) == LIMITLESS_OK);
    assert(limitless_number_from_str(&ctx, &b, tc->mod) == LIMITLESS_OK);
    assert(limitless_number_from_i64(&ctx, &out, 999) == LIMITLESS_OK);
    {
      limitless_status st = limitless_number_modexp_u64(&ctx, &out, &a, tc->exp, &b);
      assert(st == (limitless_status)tc->status);
      if (st == LIMITLESS_OK) {
        limitless_test_expect_str(&ctx, &out, tc->out);
      } else {
        limitless_test_expect_str(&ctx, &out, "999");
      }
    }
  }

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
}

static limitless_u64 limitless_test_rng_next(limitless_u64* state) {
  limitless_u64 x = *state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  *state = x;
  return x;
}

static void limitless_test_rand_int_literal(limitless_u64* state, char* out, size_t cap, int max_digits, int positive_only) {
  size_t pos = 0;
  int make_zero = (int)(limitless_test_rng_next(state) % 7ULL) == 0;
  int digits;
  int i;

  if (make_zero) {
    assert(cap > 1);
    out[0] = '0';
    out[1] = '\0';
    return;
  }

  if (!positive_only && ((limitless_test_rng_next(state) & 1ULL) != 0ULL)) {
    assert(pos + 1 < cap);
    out[pos++] = '-';
  }

  digits = 1 + (int)(limitless_test_rng_next(state) % (limitless_u64)max_digits);
  assert(pos + (size_t)digits + 1 < cap);

  out[pos++] = (char)('1' + (limitless_test_rng_next(state) % 9ULL));
  for (i = 1; i < digits; ++i) {
    out[pos++] = (char)('0' + (limitless_test_rng_next(state) % 10ULL));
  }
  out[pos] = '\0';
}

static void limitless_test_rand_value_literal(limitless_u64* state, char* out, size_t cap) {
  char num[128];
  char den[128];

  limitless_test_rand_int_literal(state, num, sizeof(num), 70, 0);
  if ((limitless_test_rng_next(state) % 3ULL) != 0ULL) {
    size_t n = strlen(num);
    assert(n + 1 <= cap);
    memcpy(out, num, n + 1);
    return;
  }

  limitless_test_rand_int_literal(state, den, sizeof(den), 40, 1);
  if (den[0] == '0' && den[1] == '\0') {
    den[0] = '1';
  }

  assert(snprintf(out, cap, "%s/%s", num, den) > 0);
}

static void test_random_properties(void) {
  limitless_ctx ctx;
  limitless_number a;
  limitless_number b;
  limitless_number c;
  limitless_number r1;
  limitless_number r2;
  limitless_number r3;
  limitless_number r4;
  limitless_number r5;
  limitless_number rt;
  limitless_u64 seed = 0x9e3779b97f4a7c15ULL;
  int iter;

  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &c) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &r1) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &r2) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &r3) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &r4) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &r5) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &rt) == LIMITLESS_OK);

  for (iter = 0; iter < LIMITLESS_PROP_RUNS; ++iter) {
    char sa[256];
    char sb[256];
    char sc[256];
    char* s;
    limitless_status st;
    int cmp_ab;
    int cmp_ba;

    limitless_test_rand_value_literal(&seed, sa, sizeof(sa));
    limitless_test_rand_value_literal(&seed, sb, sizeof(sb));
    limitless_test_rand_value_literal(&seed, sc, sizeof(sc));

    assert(limitless_number_from_str(&ctx, &a, sa) == LIMITLESS_OK);
    assert(limitless_number_from_str(&ctx, &b, sb) == LIMITLESS_OK);
    assert(limitless_number_from_str(&ctx, &c, sc) == LIMITLESS_OK);

    assert(limitless_number_add(&ctx, &r1, &a, &b) == LIMITLESS_OK);
    assert(limitless_number_add(&ctx, &r2, &b, &a) == LIMITLESS_OK);
    limitless_test_expect_cmp(&ctx, &r1, &r2, 0);

    assert(limitless_number_mul(&ctx, &r1, &a, &b) == LIMITLESS_OK);
    assert(limitless_number_mul(&ctx, &r2, &b, &a) == LIMITLESS_OK);
    limitless_test_expect_cmp(&ctx, &r1, &r2, 0);

    assert(limitless_number_add(&ctx, &r1, &a, &b) == LIMITLESS_OK);
    assert(limitless_number_sub(&ctx, &r2, &r1, &b) == LIMITLESS_OK);
    limitless_test_expect_cmp(&ctx, &r2, &a, 0);

    assert(limitless_number_add(&ctx, &r1, &a, &b) == LIMITLESS_OK);
    assert(limitless_number_mul(&ctx, &r2, &r1, &c) == LIMITLESS_OK);
    assert(limitless_number_mul(&ctx, &r3, &a, &c) == LIMITLESS_OK);
    assert(limitless_number_mul(&ctx, &r4, &b, &c) == LIMITLESS_OK);
    assert(limitless_number_add(&ctx, &r5, &r3, &r4) == LIMITLESS_OK);
    limitless_test_expect_cmp(&ctx, &r2, &r5, 0);

    st = LIMITLESS_OK;
    cmp_ab = limitless_number_cmp(&ctx, &a, &b, &st);
    assert(st == LIMITLESS_OK);
    st = LIMITLESS_OK;
    cmp_ba = limitless_number_cmp(&ctx, &b, &a, &st);
    assert(st == LIMITLESS_OK);
    assert(cmp_ab == -cmp_ba);

    if (!limitless_number_is_zero(&b)) {
      assert(limitless_number_div(&ctx, &r1, &a, &b) == LIMITLESS_OK);
      assert(limitless_number_mul(&ctx, &r2, &r1, &b) == LIMITLESS_OK);
      limitless_test_expect_cmp(&ctx, &r2, &a, 0);
    }

    s = limitless_test_to_string(&ctx, &r1, 10);
    assert(limitless_number_from_str(&ctx, &rt, s) == LIMITLESS_OK);
    limitless_test_expect_cmp(&ctx, &r1, &rt, 0);
    free(s);

    limitless_test_assert_normalized(&r1);
    limitless_test_assert_normalized(&r2);
    limitless_test_assert_normalized(&r3);
    limitless_test_assert_normalized(&r4);
    limitless_test_assert_normalized(&r5);
  }

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &c);
  limitless_number_clear(&ctx, &r1);
  limitless_number_clear(&ctx, &r2);
  limitless_number_clear(&ctx, &r3);
  limitless_number_clear(&ctx, &r4);
  limitless_number_clear(&ctx, &r5);
  limitless_number_clear(&ctx, &rt);
}

int main(void) {
  test_default_ctx_and_aliases();
  test_generated_binary_matrix();
  test_generated_unary_matrix();
  test_generated_base_roundtrips();
  test_generated_float_double_exact();
  test_generated_advanced_ops();
  test_random_properties();
  printf("generated matrix + property tests ok\n");
  return 0;
}
