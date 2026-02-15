/* SPDX-License-Identifier: GPL-3.0-only */
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>

#define LIMITLESS_CPP_NO_LEGACY_DEPRECATION 1
#define LIMITLESS_IMPLEMENTATION
#include "../limitless.hpp"

#include "generated_cpp_vectors.inc"

#ifndef LIMITLESS_CPP_PROP_ITERS
#define LIMITLESS_CPP_PROP_ITERS 4000
#endif

#ifndef LIMITLESS_CPP_PROP_ITERS_EXTENDED
#define LIMITLESS_CPP_PROP_ITERS_EXTENDED 40000
#endif

#if defined(LIMITLESS_EXTENDED_STRESS)
#define LIMITLESS_CPP_PROP_RUNS LIMITLESS_CPP_PROP_ITERS_EXTENDED
#else
#define LIMITLESS_CPP_PROP_RUNS LIMITLESS_CPP_PROP_ITERS
#endif

static std::string raw_to_string(limitless_ctx* ctx, const limitless_c_number* n, int base) {
  limitless_size need = 0;
  limitless_status st = limitless_number_to_cstr(ctx, n, base, NULL, 0, &need);
  assert(st == LIMITLESS_EBUF || st == LIMITLESS_OK);

  std::string out;
  out.resize(static_cast<size_t>(need + 1));
  st = limitless_number_to_cstr(ctx, n, base, &out[0], need + 1, NULL);
  assert(st == LIMITLESS_OK);
  out.resize(static_cast<size_t>(need));
  return out;
}

static void expect_str(const limitless_number& n, const char* expected) {
  std::string s = n.str(10);
  assert(limitless_cpp_last_status() == LIMITLESS_OK);
  assert(s == expected);
}

static void test_beginner_expression(void) {
  int a = 3;
  limitless_number x = 33424234;
  limitless_number y = (x + a) / 2.3f;
  assert(limitless_cpp_last_status() == LIMITLESS_OK);
  expect_str(y, "140191410946048/9646899");
}

static void test_parse_and_string_helpers(void) {
  limitless_size i;
  for (i = 0; i < LIMITLESS_GENERATED_CPP_PARSE_CASE_COUNT; ++i) {
    const limitless_generated_cpp_parse_case* tc = &LIMITLESS_GENERATED_CPP_PARSE_CASES[i];
    limitless_number v = limitless_number::parse(tc->value, tc->base);
    assert(limitless_cpp_last_status() == LIMITLESS_OK);
    expect_str(v, tc->expected);
  }

  {
    limitless_number bad = limitless_number::parse("not_a_number", 10);
    (void)bad;
    assert(limitless_cpp_last_status() == LIMITLESS_EPARSE);
  }
}

static void test_generated_int_left_matrix(void) {
  limitless_size i;
  for (i = 0; i < LIMITLESS_GENERATED_CPP_INT_LEFT_CASE_COUNT; ++i) {
    const limitless_generated_cpp_int_left_case* tc = &LIMITLESS_GENERATED_CPP_INT_LEFT_CASES[i];
    limitless_number rhs = limitless_number::parse(tc->rhs, 10);
    assert(limitless_cpp_last_status() == LIMITLESS_OK);

    {
      limitless_number out = tc->lhs + rhs;
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      expect_str(out, tc->add_s);
    }

    {
      limitless_number out = tc->lhs - rhs;
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      expect_str(out, tc->sub_s);
    }

    {
      limitless_number out = tc->lhs * rhs;
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      expect_str(out, tc->mul_s);
    }

    {
      limitless_number out = tc->lhs / rhs;
      limitless_status st = limitless_cpp_last_status();
      assert(st == static_cast<limitless_status>(tc->div_status));
      if (st == LIMITLESS_OK) {
        expect_str(out, tc->div_s);
      }
    }

    {
      int cmp = (tc->lhs < rhs) ? -1 : ((tc->lhs > rhs) ? 1 : 0);
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      assert(cmp == tc->cmp);
      assert((tc->lhs == rhs) == (tc->cmp == 0));
      assert((tc->lhs != rhs) == (tc->cmp != 0));
      assert((tc->lhs <= rhs) == (tc->cmp <= 0));
      assert((tc->lhs >= rhs) == (tc->cmp >= 0));
    }
  }
}

static void test_generated_double_left_matrix(void) {
  limitless_size i;
  for (i = 0; i < LIMITLESS_GENERATED_CPP_DOUBLE_LEFT_CASE_COUNT; ++i) {
    const limitless_generated_cpp_double_left_case* tc = &LIMITLESS_GENERATED_CPP_DOUBLE_LEFT_CASES[i];
    limitless_number lhs_exact = tc->lhs;
    limitless_number lhs_expect = limitless_number::parse(tc->lhs_exact, 10);
    limitless_number rhs = limitless_number::parse(tc->rhs, 10);
    assert(limitless_cpp_last_status() == LIMITLESS_OK);

    assert(lhs_exact == lhs_expect);
    assert(limitless_cpp_last_status() == LIMITLESS_OK);

    {
      limitless_number out = tc->lhs + rhs;
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      expect_str(out, tc->add_s);
    }

    {
      limitless_number out = tc->lhs - rhs;
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      expect_str(out, tc->sub_s);
    }

    {
      limitless_number out = tc->lhs * rhs;
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      expect_str(out, tc->mul_s);
    }

    {
      limitless_number out = tc->lhs / rhs;
      limitless_status st = limitless_cpp_last_status();
      assert(st == static_cast<limitless_status>(tc->div_status));
      if (st == LIMITLESS_OK) {
        expect_str(out, tc->div_s);
      }
    }

    {
      int cmp = (tc->lhs < rhs) ? -1 : ((tc->lhs > rhs) ? 1 : 0);
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      assert(cmp == tc->cmp);
    }
  }
}

static limitless_u64 rng_next(limitless_u64* state) {
  limitless_u64 x = *state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  *state = x;
  return x;
}

static std::string rand_int_literal(limitless_u64* state, int max_digits, bool positive_only) {
  bool zero = (rng_next(state) % 7ULL) == 0ULL;
  std::string out;
  int i;

  if (zero) return "0";
  if (!positive_only && ((rng_next(state) & 1ULL) != 0ULL)) out.push_back('-');

  {
    int digits = 1 + static_cast<int>(rng_next(state) % static_cast<limitless_u64>(max_digits));
    out.push_back(static_cast<char>('1' + (rng_next(state) % 9ULL)));
    for (i = 1; i < digits; ++i) {
      out.push_back(static_cast<char>('0' + (rng_next(state) % 10ULL)));
    }
  }
  return out;
}

static std::string rand_value_literal(limitless_u64* state) {
  std::string num = rand_int_literal(state, 60, false);
  if ((rng_next(state) % 3ULL) != 0ULL) return num;
  {
    std::string den = rand_int_literal(state, 35, true);
    if (den == "0") den = "1";
    return num + "/" + den;
  }
}

static void test_wrapper_matches_c_api(limitless_ctx* ctx) {
  limitless_c_number a;
  limitless_c_number b;
  limitless_c_number out;
  limitless_u64 seed = 0x243f6a8885a308d3ULL;
  int iter;

  assert(limitless_number_init(ctx, &a) == LIMITLESS_OK);
  assert(limitless_number_init(ctx, &b) == LIMITLESS_OK);
  assert(limitless_number_init(ctx, &out) == LIMITLESS_OK);

  for (iter = 0; iter < LIMITLESS_CPP_PROP_RUNS; ++iter) {
    std::string sa = rand_value_literal(&seed);
    std::string sb = rand_value_literal(&seed);

    limitless_number wa = limitless_number::parse(sa.c_str(), 10);
    limitless_number wb = limitless_number::parse(sb.c_str(), 10);
    assert(limitless_cpp_last_status() == LIMITLESS_OK);

    assert(limitless_number_from_str(ctx, &a, sa.c_str()) == LIMITLESS_OK);
    assert(limitless_number_from_str(ctx, &b, sb.c_str()) == LIMITLESS_OK);

    {
      limitless_number w = wa + wb;
      std::string ws;
      std::string cs;
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      assert(limitless_number_add(ctx, &out, &a, &b) == LIMITLESS_OK);
      ws = w.str();
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      cs = raw_to_string(ctx, &out, 10);
      assert(ws == cs);
    }

    {
      limitless_number w = wa - wb;
      std::string ws;
      std::string cs;
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      assert(limitless_number_sub(ctx, &out, &a, &b) == LIMITLESS_OK);
      ws = w.str();
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      cs = raw_to_string(ctx, &out, 10);
      assert(ws == cs);
    }

    {
      limitless_number w = wa * wb;
      std::string ws;
      std::string cs;
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      assert(limitless_number_mul(ctx, &out, &a, &b) == LIMITLESS_OK);
      ws = w.str();
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      cs = raw_to_string(ctx, &out, 10);
      assert(ws == cs);
    }

    {
      limitless_number w = wa / wb;
      limitless_status wst = limitless_cpp_last_status();
      limitless_status cst = limitless_number_div(ctx, &out, &a, &b);
      assert(wst == cst);
      if (cst == LIMITLESS_OK) {
        std::string ws = w.str();
        std::string cs = raw_to_string(ctx, &out, 10);
        assert(limitless_cpp_last_status() == LIMITLESS_OK);
        assert(ws == cs);
      }
    }

    {
      limitless_status st = LIMITLESS_OK;
      int ccmp = limitless_number_cmp(ctx, &a, &b, &st);
      int wcmp = (wa < wb) ? -1 : ((wa > wb) ? 1 : 0);
      assert(st == LIMITLESS_OK);
      assert(limitless_cpp_last_status() == LIMITLESS_OK);
      assert(wcmp == ccmp);
    }
  }

  limitless_number_clear(ctx, &a);
  limitless_number_clear(ctx, &b);
  limitless_number_clear(ctx, &out);
}

int main() {
  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  limitless_cpp_set_default_ctx(&ctx);

  test_beginner_expression();
  test_parse_and_string_helpers();
  test_generated_int_left_matrix();
  test_generated_double_left_matrix();
  test_wrapper_matches_c_api(&ctx);

  std::puts("c++ generated matrix + property tests ok");
  return 0;
}
