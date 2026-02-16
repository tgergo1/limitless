/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define LIMITLESS_IMPLEMENTATION
#include "../limitless.h"

static limitless_u64 rng_next(limitless_u64* state) {
  limitless_u64 x = *state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  *state = x;
  return x;
}

static void check_invariants(limitless_ctx* ctx, const limitless_number* n) {
  char text[1024];
  const char* slash;
  if (n->kind == LIMITLESS_KIND_INT) {
    if (n->v.i.used == 0) {
      assert(n->v.i.sign == 0);
    } else {
      assert(n->v.i.sign == 1 || n->v.i.sign == -1);
    }
    return;
  }

  assert(n->kind == LIMITLESS_KIND_RAT);
  assert(n->v.r.den.sign > 0);
  assert(n->v.r.den.used > 0);

  assert(limitless_number_to_str(ctx, n, text, (limitless_size)sizeof(text), NULL) == LIMITLESS_OK);
  slash = strchr(text, '/');
  assert(slash != NULL);
  assert(strchr(slash + 1, '-') == NULL);

  {
    limitless_number num;
    limitless_number den;
    limitless_number g;
    limitless_status st;
    char num_s[512];
    char den_s[512];
    limitless_size num_len = (limitless_size)(slash - text);
    limitless_size den_len = (limitless_size)(strlen(slash + 1));
    assert(num_len < (limitless_size)sizeof(num_s));
    assert(den_len < (limitless_size)sizeof(den_s));
    memcpy(num_s, text, (size_t)num_len);
    num_s[num_len] = '\0';
    memcpy(den_s, slash + 1, (size_t)den_len + 1u);

    st = limitless_number_init(ctx, &num);
    assert(st == LIMITLESS_OK);
    st = limitless_number_init(ctx, &den);
    assert(st == LIMITLESS_OK);
    st = limitless_number_init(ctx, &g);
    assert(st == LIMITLESS_OK);

    assert(limitless_number_from_str(ctx, &num, num_s) == LIMITLESS_OK);
    assert(limitless_number_from_str(ctx, &den, den_s) == LIMITLESS_OK);
    assert(limitless_number_gcd(ctx, &g, &num, &den) == LIMITLESS_OK);

    {
      char gtxt[32];
      assert(limitless_number_to_str(ctx, &g, gtxt, (limitless_size)sizeof(gtxt), NULL) == LIMITLESS_OK);
      assert(strcmp(gtxt, "1") == 0);
    }

    limitless_number_clear(ctx, &num);
    limitless_number_clear(ctx, &den);
    limitless_number_clear(ctx, &g);
  }
}

static void test_invariant_stress(void) {
  limitless_status st;
  limitless_ctx ctx;
  limitless_number a;
  limitless_number b;
  limitless_number out;
  limitless_u64 seed = 0x9e3779b97f4a7c15ULL;
  int i;

  st = limitless_ctx_init_default(&ctx);
  assert(st == LIMITLESS_OK);
  st = limitless_number_init(&ctx, &a);
  assert(st == LIMITLESS_OK);
  st = limitless_number_init(&ctx, &b);
  assert(st == LIMITLESS_OK);
  st = limitless_number_init(&ctx, &out);
  assert(st == LIMITLESS_OK);

  for (i = 0; i < 3500; ++i) {
    char sa[96];
    char sb[96];
    int na = (int)(rng_next(&seed) % 1000000ULL) - 500000;
    int da = (int)(rng_next(&seed) % 2000ULL) + 1;
    int nb = (int)(rng_next(&seed) % 1000000ULL) - 500000;
    int db = (int)(rng_next(&seed) % 2000ULL) + 1;
    if ((rng_next(&seed) & 3ULL) == 0ULL) {
      snprintf(sa, sizeof(sa), "%d", na);
    } else {
      snprintf(sa, sizeof(sa), "%d/%d", na, da);
    }
    if ((rng_next(&seed) & 3ULL) == 0ULL) {
      snprintf(sb, sizeof(sb), "%d", nb);
    } else {
      snprintf(sb, sizeof(sb), "%d/%d", nb, db);
    }

    assert(limitless_number_from_str(&ctx, &a, sa) == LIMITLESS_OK);
    assert(limitless_number_from_str(&ctx, &b, sb) == LIMITLESS_OK);

    assert(limitless_number_add(&ctx, &out, &a, &b) == LIMITLESS_OK);
    check_invariants(&ctx, &out);

    assert(limitless_number_sub(&ctx, &out, &a, &b) == LIMITLESS_OK);
    check_invariants(&ctx, &out);

    assert(limitless_number_mul(&ctx, &out, &a, &b) == LIMITLESS_OK);
    check_invariants(&ctx, &out);

    if (!limitless_number_is_zero(&b)) {
      assert(limitless_number_div(&ctx, &out, &a, &b) == LIMITLESS_OK);
      check_invariants(&ctx, &out);
    }
  }

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
}

int main(void) {
  test_invariant_stress();
  printf("invariant tests ok\n");
  return 0;
}
