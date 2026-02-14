/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIMITLESS_IMPLEMENTATION
#include "../limitless.h"

#ifndef LIMITLESS_PARSER_FUZZ_ITERS
#define LIMITLESS_PARSER_FUZZ_ITERS 12000
#endif

#ifndef LIMITLESS_PARSER_VALID_ITERS
#define LIMITLESS_PARSER_VALID_ITERS 4000
#endif

static limitless_u64 rng_next(limitless_u64* state) {
  limitless_u64 x = *state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  *state = x;
  return x;
}

static char* number_to_alloc_str(limitless_ctx* ctx, const limitless_number* n, int base) {
  limitless_size need = 0;
  char* out;
  limitless_status st = limitless_number_to_cstr(ctx, n, base, NULL, 0, &need);
  assert(st == LIMITLESS_EBUF || st == LIMITLESS_OK);
  out = (char*)malloc((size_t)need + 1u);
  assert(out != NULL);
  st = limitless_number_to_cstr(ctx, n, base, out, need + 1u, NULL);
  assert(st == LIMITLESS_OK);
  return out;
}

static void set_marker(limitless_ctx* ctx, limitless_number* n) {
  assert(limitless_number_from_str(ctx, n, "11/7") == LIMITLESS_OK);
}

static void expect_marker(limitless_ctx* ctx, const limitless_number* n) {
  char buf[32];
  assert(limitless_number_to_str(ctx, n, buf, (limitless_size)sizeof(buf), NULL) == LIMITLESS_OK);
  assert(strcmp(buf, "11/7") == 0);
}

static void make_noise_string(limitless_u64* seed, char* out, size_t cap) {
  static const char chars[] =
      "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-/xXbB_ .";
  size_t len;
  size_t i;
  assert(cap > 1);
  len = (size_t)(rng_next(seed) % (limitless_u64)(cap - 1));
  if ((rng_next(seed) & 15ULL) == 0ULL) {
    out[0] = '\0';
    return;
  }
  for (i = 0; i < len; ++i) {
    out[i] = chars[rng_next(seed) % (sizeof(chars) - 1u)];
  }
  out[len] = '\0';
}

static void make_valid_number_string(limitless_u64* seed, char* out, size_t cap) {
  size_t pos = 0;
  int make_rat;
  int num_digits;
  int den_digits;
  int i;

  assert(cap > 4);

  if ((rng_next(seed) & 1ULL) != 0ULL) {
    out[pos++] = '-';
  }

  num_digits = 1 + (int)(rng_next(seed) % 80ULL);
  if ((rng_next(seed) & 7ULL) == 0ULL) {
    out[pos++] = '0';
  } else {
    out[pos++] = (char)('1' + (rng_next(seed) % 9ULL));
    for (i = 1; i < num_digits && pos + 1 < cap; ++i) {
      out[pos++] = (char)('0' + (rng_next(seed) % 10ULL));
    }
  }

  make_rat = ((rng_next(seed) % 3ULL) == 0ULL);
  if (!make_rat || pos + 3 >= cap) {
    out[pos] = '\0';
    return;
  }

  out[pos++] = '/';
  den_digits = 1 + (int)(rng_next(seed) % 40ULL);
  out[pos++] = (char)('1' + (rng_next(seed) % 9ULL));
  for (i = 1; i < den_digits && pos + 1 < cap; ++i) {
    out[pos++] = (char)('0' + (rng_next(seed) % 10ULL));
  }
  out[pos] = '\0';
}

static void expect_roundtrip_base10(limitless_ctx* ctx, const limitless_number* n) {
  limitless_number parsed;
  char* s;
  limitless_status st;
  limitless_status cmp_st = LIMITLESS_OK;
  assert(limitless_number_init(ctx, &parsed) == LIMITLESS_OK);

  s = number_to_alloc_str(ctx, n, 10);
  st = limitless_number_from_cstr(ctx, &parsed, s, 10);
  assert(st == LIMITLESS_OK);
  assert(limitless_number_cmp(ctx, n, &parsed, &cmp_st) == 0);
  assert(cmp_st == LIMITLESS_OK);

  free(s);
  limitless_number_clear(ctx, &parsed);
}

static void test_curated_invalid_cases(void) {
  limitless_ctx ctx;
  limitless_number n;
  const char* bad[] = {
      "", "+", "-", "/", "1/", "/2", "1//2", "0x", "0b", "0xg", "0b102",
      "1/2/3", "1 2", "--1", "++1", "-/3", "nan", "inf", "1/_", "_1",
  };
  size_t i;

  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &n) == LIMITLESS_OK);

  for (i = 0; i < sizeof(bad) / sizeof(bad[0]); ++i) {
    limitless_status st;
    set_marker(&ctx, &n);
    st = limitless_number_from_cstr(&ctx, &n, bad[i], 0);
    assert(st == LIMITLESS_EPARSE || st == LIMITLESS_EINVAL || st == LIMITLESS_EDIVZERO);
    expect_marker(&ctx, &n);
  }

  set_marker(&ctx, &n);
  assert(limitless_number_from_cstr(&ctx, &n, "12/0", 10) == LIMITLESS_EDIVZERO);
  expect_marker(&ctx, &n);

  limitless_number_clear(&ctx, &n);
}

static void test_noise_fuzz(void) {
  limitless_ctx ctx;
  limitless_number out;
  limitless_number parsed;
  limitless_u64 seed = 0x123456789abcdef0ULL;
  int iter;

  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &parsed) == LIMITLESS_OK);

  for (iter = 0; iter < LIMITLESS_PARSER_FUZZ_ITERS; ++iter) {
    char token[128];
    int base_choices[] = {0, 2, 8, 10, 16, 36};
    int base = base_choices[rng_next(&seed) % 6ULL];
    limitless_status st;

    make_noise_string(&seed, token, sizeof(token));
    set_marker(&ctx, &out);
    st = limitless_number_from_cstr(&ctx, &out, token, base);
    if (st == LIMITLESS_OK) {
      char* repr;
      limitless_status cmp_st = LIMITLESS_OK;
      repr = number_to_alloc_str(&ctx, &out, 10);
      assert(limitless_number_from_cstr(&ctx, &parsed, repr, 10) == LIMITLESS_OK);
      assert(limitless_number_cmp(&ctx, &out, &parsed, &cmp_st) == 0);
      assert(cmp_st == LIMITLESS_OK);
      free(repr);
    } else {
      assert(st == LIMITLESS_EPARSE || st == LIMITLESS_EINVAL || st == LIMITLESS_EDIVZERO);
      expect_marker(&ctx, &out);
    }
  }

  limitless_number_clear(&ctx, &out);
  limitless_number_clear(&ctx, &parsed);
}

static void test_valid_roundtrips(void) {
  limitless_ctx ctx;
  limitless_number out;
  limitless_number rt;
  limitless_u64 seed = 0x0f1e2d3c4b5a6978ULL;
  int iter;

  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &rt) == LIMITLESS_OK);

  for (iter = 0; iter < LIMITLESS_PARSER_VALID_ITERS; ++iter) {
    char token[256];
    int bases[] = {2, 8, 10, 16, 36};
    int base = bases[rng_next(&seed) % 5ULL];
    char* base_repr;
    limitless_status cmp_st = LIMITLESS_OK;

    make_valid_number_string(&seed, token, sizeof(token));
    assert(limitless_number_from_str(&ctx, &out, token) == LIMITLESS_OK);

    expect_roundtrip_base10(&ctx, &out);

    if (limitless_number_is_integer(&out)) {
      base_repr = number_to_alloc_str(&ctx, &out, base);
      assert(limitless_number_from_cstr(&ctx, &rt, base_repr, base) == LIMITLESS_OK);
      assert(limitless_number_cmp(&ctx, &out, &rt, &cmp_st) == 0);
      assert(cmp_st == LIMITLESS_OK);
      free(base_repr);
    }
  }

  limitless_number_clear(&ctx, &out);
  limitless_number_clear(&ctx, &rt);
}

int main(void) {
  test_curated_invalid_cases();
  test_noise_fuzz();
  test_valid_roundtrips();
  printf("parser fuzz tests ok\n");
  return 0;
}
