/* SPDX-License-Identifier: GPL-3.0-only */
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>
#include <utility>

#define LIMITLESS_CPP_NO_LEGACY_DEPRECATION 1
#define LIMITLESS_IMPLEMENTATION
#include "../limitless.hpp"

struct fail_alloc_state {
  int fail_after;
  int calls;
};

static void* fail_alloc(void* user, limitless_size size) {
  fail_alloc_state* st = static_cast<fail_alloc_state*>(user);
  if (st->calls++ >= st->fail_after) return NULL;
  return std::malloc(static_cast<size_t>(size));
}

static void* fail_realloc(void* user, void* ptr, limitless_size old_size, limitless_size new_size) {
  fail_alloc_state* st = static_cast<fail_alloc_state*>(user);
  (void)old_size;
  if (st->calls++ >= st->fail_after) return NULL;
  return std::realloc(ptr, static_cast<size_t>(new_size));
}

static void fail_free(void* user, void* ptr, limitless_size size) {
  (void)user;
  (void)size;
  std::free(ptr);
}

static void test_all_constructor_types() {
  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  limitless::limitless_cpp_set_default_ctx(&ctx);

  /* i32 constructor (line 105) */
  {
    limitless::number a(static_cast<limitless_i32>(42));
    assert(a.str() == "42");
  }

  /* u32 constructor (line 106) */
  {
    limitless::number b(static_cast<limitless_u32>(99));
    assert(b.str() == "99");
  }

  /* i64 constructor (line 107) */
  {
    limitless::number c(static_cast<limitless_i64>(-12345));
    assert(c.str() == "-12345");
  }

  /* u64 constructor (line 108) */
  {
    limitless::number d(static_cast<limitless_u64>(18446744073709551615ULL));
    assert(d.str() == "18446744073709551615");
  }

  /* float constructor (line 109) */
  {
    limitless::number e(0.5f);
    assert(e.str() == "1/2");
  }

  /* double constructor (line 110) */
  {
    limitless::number f(0.25);
    assert(f.str() == "1/4");
  }

  limitless::limitless_cpp_set_default_ctx(NULL);
}

static void test_self_move_assign() {
  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  limitless::limitless_cpp_set_default_ctx(&ctx);

  /* Self-move-assign (line 156-158) */
  {
    limitless::number n = 42;
    limitless::number& ref = n;
    n = std::move(ref);
    assert(limitless::limitless_cpp_last_status() == LIMITLESS_OK);
    assert(n.str() == "42");
  }

  limitless::limitless_cpp_set_default_ctx(NULL);
}

static void test_moved_from_destructor() {
  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  limitless::limitless_cpp_set_default_ctx(&ctx);

  /* Destructor of moved-from number (line 133 false branch) */
  {
    limitless::number a = 100;
    limitless::number b = std::move(a);
    assert(b.str() == "100");
    /* a is now moved-from: owner_ctx_ == NULL, destructor skips clear */
  }

  limitless::limitless_cpp_set_default_ctx(NULL);
}

static void test_double_assign_operator() {
  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  limitless::limitless_cpp_set_default_ctx(&ctx);

  /* operator=(double) normal path (line 200) */
  {
    limitless::number n;
    n = 3.14;
    assert(limitless::limitless_cpp_last_status() == LIMITLESS_OK);
  }

  limitless::limitless_cpp_set_default_ctx(NULL);
}

static void test_rebind_owner_branches() {
  limitless_ctx ctx1, ctx2;
  assert(limitless_ctx_init_default(&ctx1) == LIMITLESS_OK);
  assert(limitless_ctx_init_default(&ctx2) == LIMITLESS_OK);

  /* rebind_owner with existing owner to different ctx (lines 323-325) */
  limitless::limitless_cpp_set_default_ctx(&ctx1);
  {
    limitless::number n = 5;
    /* n now has owner_ctx_ == &ctx1 */
    /* Switch default ctx and assign — forces rebind through ensure_owner path */
    limitless::limitless_cpp_set_default_ctx(&ctx2);
    limitless::number m = 10;
    /* Copy-assign m to n: ctx is n's owner (ctx1), so rebind happens if needed */
    n = m;
    assert(n.str() == "10");
  }

  limitless::limitless_cpp_set_default_ctx(NULL);
}

static void test_oom_in_cpp_wrapper() {
  fail_alloc_state state;
  limitless_alloc alloc;
  limitless_ctx fail_ctx;

  state.fail_after = 1000000;
  state.calls = 0;
  alloc.alloc = fail_alloc;
  alloc.realloc = fail_realloc;
  alloc.free = fail_free;
  alloc.user = &state;
  assert(limitless_ctx_init(&fail_ctx, &alloc) == LIMITLESS_OK);
  limitless::limitless_cpp_set_default_ctx(&fail_ctx);

  /* Copy constructor OOM (line 118 false branch) */
  {
    limitless::number big = limitless::number::parse(
        "12345678901234567890123456789012345678901234567890");
    assert(limitless::limitless_cpp_last_status() == LIMITLESS_OK);

    state.calls = 0;
    state.fail_after = 0;
    limitless::number copy(big);
    assert(limitless::limitless_cpp_last_status() == LIMITLESS_EOOM);
    state.fail_after = 1000000;
  }

  /* parse OOM (line 283 false branch) */
  {
    state.calls = 0;
    state.fail_after = 0;
    limitless::number p = limitless::number::parse(
        "99887766554433221100998877665544332211");
    assert(limitless::limitless_cpp_last_status() != LIMITLESS_OK);
    state.fail_after = 1000000;
  }

  /* binary_op OOM (lines 362, 369 error paths) */
  {
    limitless::number a = 123456789;
    limitless::number b = 987654321;
    assert(limitless::limitless_cpp_last_status() == LIMITLESS_OK);

    state.calls = 0;
    state.fail_after = 0;
    limitless::number c = a + b;
    assert(limitless::limitless_cpp_last_status() != LIMITLESS_OK);
    state.fail_after = 1000000;
  }

  /* binary_op_inplace OOM (line 374 error path) */
  {
    limitless::number a = 123456789;
    limitless::number b = 987654321;
    assert(limitless::limitless_cpp_last_status() == LIMITLESS_OK);

    state.calls = 0;
    state.fail_after = 0;
    a += b;
    assert(limitless::limitless_cpp_last_status() != LIMITLESS_OK);
    state.fail_after = 1000000;
  }

  /* cmp OOM (line 391 error path) */
  {
    limitless::number a = limitless::number::parse("12345678901234567890/97");
    limitless::number b = limitless::number::parse("-998877665544332211/89");
    assert(limitless::limitless_cpp_last_status() == LIMITLESS_OK);

    state.calls = 0;
    state.fail_after = 0;
    bool result = (a == b);
    (void)result;
    assert(limitless::limitless_cpp_last_status() != LIMITLESS_OK);
    state.fail_after = 1000000;
  }

  /* operator=(double) OOM (line 200 error path) */
  {
    limitless::number n = 1;
    assert(limitless::limitless_cpp_last_status() == LIMITLESS_OK);

    state.calls = 0;
    state.fail_after = 0;
    n = 3.14;
    assert(limitless::limitless_cpp_last_status() != LIMITLESS_OK);
    state.fail_after = 1000000;
  }

  limitless::limitless_cpp_set_default_ctx(NULL);
}

static void test_all_free_template_operators() {
  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  limitless::limitless_cpp_set_default_ctx(&ctx);

  limitless::number rhs = 4;

  /* Test with different arithmetic types to exercise template instantiations */

  /* int (i32) versions */
  assert((6 + rhs).str() == "10");
  assert((6 - rhs).str() == "2");
  assert((6 * rhs).str() == "24");
  assert((6 / rhs).str() == "3/2");
  assert(!(6 == rhs));
  assert(6 != rhs);
  assert(3 < rhs);
  assert(4 <= rhs);
  assert(5 > rhs);
  assert(4 >= rhs);

  /* i64 versions */
  assert((static_cast<limitless_i64>(6) + rhs).str() == "10");
  assert((static_cast<limitless_i64>(6) - rhs).str() == "2");
  assert((static_cast<limitless_i64>(6) * rhs).str() == "24");
  assert((static_cast<limitless_i64>(6) / rhs).str() == "3/2");
  assert(!(static_cast<limitless_i64>(6) == rhs));
  assert(static_cast<limitless_i64>(6) != rhs);
  assert(static_cast<limitless_i64>(3) < rhs);
  assert(static_cast<limitless_i64>(4) <= rhs);
  assert(static_cast<limitless_i64>(5) > rhs);
  assert(static_cast<limitless_i64>(4) >= rhs);

  /* u64 versions */
  assert((static_cast<limitless_u64>(6) + rhs).str() == "10");

  /* double versions */
  assert((0.5 + rhs).str() == "9/2");

  limitless::limitless_cpp_set_default_ctx(NULL);
}

int main() {
  test_all_constructor_types();
  test_self_move_assign();
  test_moved_from_destructor();
  test_double_assign_operator();
  test_rebind_owner_branches();
  test_oom_in_cpp_wrapper();
  test_all_free_template_operators();
  std::printf("cpp branch coverage tests ok\n");
  return 0;
}
