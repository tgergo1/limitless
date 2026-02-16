/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
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

static limitless_ctx make_fail_ctx(fail_alloc_state* state) {
  limitless_alloc alloc;
  limitless_ctx ctx;
  alloc.alloc = fail_alloc;
  alloc.realloc = fail_realloc;
  alloc.free = fail_free;
  alloc.user = state;
  assert(limitless_ctx_init(&ctx, &alloc) == LIMITLESS_OK);
  return ctx;
}

static void test_null_guards(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number n, a, b, out;
  limitless_alloc good_alloc;
  limitless_status st;
  char buf[64];

  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &a, 10) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 3) == LIMITLESS_OK);

  good_alloc.alloc = NULL;
  good_alloc.realloc = NULL;
  good_alloc.free = NULL;
  good_alloc.user = NULL;

  /* limitless_ctx_init: NULL ctx, invalid alloc */
  assert(limitless_ctx_init(NULL, &good_alloc) == LIMITLESS_EINVAL);
  assert(limitless_ctx_init(&ctx, &good_alloc) == LIMITLESS_EINVAL);

  /* limitless_ctx_init_default: NULL ctx */
  assert(limitless_ctx_init_default(NULL) == LIMITLESS_EINVAL);

  /* Restore ctx after the invalid init test above */
  ctx = make_ctx();

  /* limitless_ctx_set_karatsuba_threshold: NULL ctx */
  limitless_ctx_set_karatsuba_threshold(NULL, 10);

  /* limitless_number_init: NULL n */
  assert(limitless_number_init(&ctx, NULL) == LIMITLESS_EINVAL);

  /* limitless_number_clear: NULL n */
  limitless_number_clear(&ctx, NULL);

  /* limitless_number_copy: NULL for each arg */
  assert(limitless_number_copy(NULL, &out, &a) == LIMITLESS_EINVAL);
  assert(limitless_number_copy(&ctx, NULL, &a) == LIMITLESS_EINVAL);
  assert(limitless_number_copy(&ctx, &out, NULL) == LIMITLESS_EINVAL);

  /* limitless_number_from_u64: NULL args */
  assert(limitless_number_from_u64(NULL, &out, 1u) == LIMITLESS_EINVAL);
  assert(limitless_number_from_u64(&ctx, NULL, 1u) == LIMITLESS_EINVAL);

  /* limitless_number_to_u64: NULL out */
  assert(limitless_number_to_u64(&ctx, &a, NULL) == LIMITLESS_EINVAL);

  /* limitless_number_to_i64: NULL out */
  assert(limitless_number_to_i64(&ctx, &a, NULL) == LIMITLESS_EINVAL);

  /* limitless_number_div: NULL for each arg position */
  assert(limitless_number_div(NULL, &out, &a, &b) == LIMITLESS_EINVAL);
  assert(limitless_number_div(&ctx, NULL, &a, &b) == LIMITLESS_EINVAL);
  assert(limitless_number_div(&ctx, &out, NULL, &b) == LIMITLESS_EINVAL);
  assert(limitless_number_div(&ctx, &out, &a, NULL) == LIMITLESS_EINVAL);

  /* limitless_number_neg: NULL for each arg */
  assert(limitless_number_neg(NULL, &out, &a) == LIMITLESS_EINVAL);
  assert(limitless_number_neg(&ctx, NULL, &a) == LIMITLESS_EINVAL);
  assert(limitless_number_neg(&ctx, &out, NULL) == LIMITLESS_EINVAL);

  /* limitless_number_abs: NULL for each arg */
  assert(limitless_number_abs(NULL, &out, &a) == LIMITLESS_EINVAL);
  assert(limitless_number_abs(&ctx, NULL, &a) == LIMITLESS_EINVAL);
  assert(limitless_number_abs(&ctx, &out, NULL) == LIMITLESS_EINVAL);

  /* limitless_number_cmp: NULL st (valid comparison) */
  assert(limitless_number_from_i64(&ctx, &a, 5) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &b, 3) == LIMITLESS_OK);
  assert(limitless_number_cmp(&ctx, &a, &b, NULL) > 0);

  /* limitless_number_cmp: NULL b with valid st */
  st = LIMITLESS_OK;
  assert(limitless_number_cmp(&ctx, &a, NULL, &st) == 0);
  assert(st == LIMITLESS_EINVAL);

  /* limitless_number_cmp: NULL b with NULL st */
  assert(limitless_number_cmp(&ctx, &a, NULL, NULL) == 0);

  /* limitless_number_gcd: NULL for each arg */
  assert(limitless_number_gcd(NULL, &out, &a, &b) == LIMITLESS_EINVAL);
  assert(limitless_number_gcd(&ctx, NULL, &a, &b) == LIMITLESS_EINVAL);
  assert(limitless_number_gcd(&ctx, &out, NULL, &b) == LIMITLESS_EINVAL);
  assert(limitless_number_gcd(&ctx, &out, &a, NULL) == LIMITLESS_EINVAL);

  /* limitless_number_pow_u64: NULL for each arg */
  assert(limitless_number_pow_u64(NULL, &out, &a, 2) == LIMITLESS_EINVAL);
  assert(limitless_number_pow_u64(&ctx, NULL, &a, 2) == LIMITLESS_EINVAL);
  assert(limitless_number_pow_u64(&ctx, &out, NULL, 2) == LIMITLESS_EINVAL);

  /* limitless_number_modexp_u64: NULL for each arg */
  assert(limitless_number_modexp_u64(NULL, &out, &a, 2, &b) == LIMITLESS_EINVAL);
  assert(limitless_number_modexp_u64(&ctx, NULL, &a, 2, &b) == LIMITLESS_EINVAL);
  assert(limitless_number_modexp_u64(&ctx, &out, NULL, 2, &b) == LIMITLESS_EINVAL);
  assert(limitless_number_modexp_u64(&ctx, &out, &a, 2, NULL) == LIMITLESS_EINVAL);

  /* limitless_number_is_zero/is_integer/sign: NULL n */
  assert(limitless_number_is_zero(NULL) == 1);
  assert(limitless_number_is_integer(NULL) == 0);
  assert(limitless_number_sign(NULL) == 0);

  /* limitless_number_to_cstr: NULL n (EINVAL guard) */
  assert(limitless_number_to_cstr(&ctx, NULL, 10, buf, (limitless_size)sizeof(buf), NULL) == LIMITLESS_EINVAL);

  limitless_number_clear(&ctx, &n);
  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
}

static void test_karatsuba_threshold_edges(void) {
  limitless_ctx ctx = make_ctx();

  /* limbs < 2 should be clamped to 2 */
  limitless_ctx_set_karatsuba_threshold(&ctx, 0);
  limitless_ctx_set_karatsuba_threshold(&ctx, 1);
  /* Valid threshold */
  limitless_ctx_set_karatsuba_threshold(&ctx, 64);
}

static void test_modexp_zero_modulus(void) {
  limitless_ctx ctx = make_ctx();
  limitless_number base, mod, out;
  assert(limitless_number_init(&ctx, &base) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &mod) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);

  assert(limitless_number_from_i64(&ctx, &base, 5) == LIMITLESS_OK);
  /* Zero modulus should return EDIVZERO */
  assert(limitless_number_modexp_u64(&ctx, &out, &base, 3, &mod) == LIMITLESS_EDIVZERO);

  limitless_number_clear(&ctx, &base);
  limitless_number_clear(&ctx, &mod);
  limitless_number_clear(&ctx, &out);
}

static void test_oom_rational_div(void) {
  fail_alloc_state state;
  limitless_ctx ctx;
  limitless_number lhs, rhs, out;
  int step;
  int saw_oom;

  state.fail_after = 1000000;
  state.calls = 0;
  ctx = make_fail_ctx(&state);

  assert(limitless_number_init(&ctx, &lhs) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &rhs) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);

  /* Use rational operands to exercise the else branch in div */
  assert(limitless_number_from_str(&ctx, &lhs,
    "12345678901234567890123456789/97") == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &rhs,
    "-99887766554433221100998877665544332211/89") == LIMITLESS_OK);

  saw_oom = 0;
  for (step = 0; step < 512; ++step) {
    state.fail_after = 1000000;
    state.calls = 0;
    assert(limitless_number_from_str(&ctx, &out, "777/13") == LIMITLESS_OK);
    state.fail_after = step;
    state.calls = 0;
    if (limitless_number_div(&ctx, &out, &lhs, &rhs) == LIMITLESS_EOOM) {
      saw_oom = 1;
      state.fail_after = 1000000;
    }
  }
  assert(saw_oom);

  state.fail_after = 1000000;
  limitless_number_clear(&ctx, &lhs);
  limitless_number_clear(&ctx, &rhs);
  limitless_number_clear(&ctx, &out);
}

static void test_oom_cmp_null_st(void) {
  fail_alloc_state state;
  limitless_ctx ctx;
  limitless_number int_n, rat_n;
  int step;
  int saw_oom_path;

  state.fail_after = 1000000;
  state.calls = 0;
  ctx = make_fail_ctx(&state);

  assert(limitless_number_init(&ctx, &int_n) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &rat_n) == LIMITLESS_OK);

  /* Cross-type comparison (int vs rational) triggers the alloc path */
  assert(limitless_number_from_i64(&ctx, &int_n, 42) == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &rat_n, "99/2") == LIMITLESS_OK);

  saw_oom_path = 0;
  for (step = 0; step < 256; ++step) {
    limitless_status cmp_st = LIMITLESS_OK;
    state.calls = 0;
    state.fail_after = step;
    (void)limitless_number_cmp(&ctx, &int_n, &rat_n, &cmp_st);
    if (cmp_st == LIMITLESS_EOOM) saw_oom_path = 1;
  }
  assert(saw_oom_path);

  /* Same sweep but with NULL status pointer to cover line 2180 */
  for (step = 0; step < 256; ++step) {
    state.calls = 0;
    state.fail_after = step;
    (void)limitless_number_cmp(&ctx, &int_n, &rat_n, NULL);
  }

  state.fail_after = 1000000;
  limitless_number_clear(&ctx, &int_n);
  limitless_number_clear(&ctx, &rat_n);
}

static void test_oom_modexp_negative_base(void) {
  fail_alloc_state state;
  limitless_ctx ctx;
  limitless_number base, mod, out;
  int step;
  int saw_oom;

  state.fail_after = 1000000;
  state.calls = 0;
  ctx = make_fail_ctx(&state);

  assert(limitless_number_init(&ctx, &base) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &mod) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);

  /* Small negative base with large modulus: after mod, base has few limbs but
     the add_signed correction (base + mod) must grow the allocation to mod's
     limb count, which forces a reserve → alloc that can OOM */
  assert(limitless_number_from_i64(&ctx, &base, -3) == LIMITLESS_OK);
  assert(limitless_number_from_str(&ctx, &mod,
    "100000000000000000000000000000000000000000000000003") == LIMITLESS_OK);

  /* Verify the normal case works first */
  assert(limitless_number_modexp_u64(&ctx, &out, &base, 5, &mod) == LIMITLESS_OK);

  /* OOM sweep to hit all failure points including the negative base correction
     and the squaring step in the loop */
  saw_oom = 0;
  for (step = 0; step < 768; ++step) {
    state.fail_after = 1000000;
    state.calls = 0;
    assert(limitless_number_from_str(&ctx, &out, "777/13") == LIMITLESS_OK);
    state.fail_after = step;
    state.calls = 0;
    if (limitless_number_modexp_u64(&ctx, &out, &base, 33, &mod) == LIMITLESS_EOOM) {
      saw_oom = 1;
      state.fail_after = 1000000;
    }
  }
  assert(saw_oom);

  state.fail_after = 1000000;
  limitless_number_clear(&ctx, &base);
  limitless_number_clear(&ctx, &mod);
  limitless_number_clear(&ctx, &out);
}

int main(void) {
  test_null_guards();
  test_karatsuba_threshold_edges();
  test_modexp_zero_modulus();
  test_oom_rational_div();
  test_oom_cmp_null_st();
  test_oom_modexp_negative_base();
  printf("branch coverage tests ok\n");
  return 0;
}
