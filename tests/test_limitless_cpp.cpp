/* SPDX-License-Identifier: GPL-3.0-only */
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <string>

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

int main() {
  {
    limitless_cpp_set_default_ctx(NULL);
    limitless_number cold = 7;
    assert(limitless_cpp_last_status() == LIMITLESS_OK);
    assert(cold.str() == "7");
    assert(limitless_cpp_last_status() == LIMITLESS_OK);
  }

  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  limitless_cpp_set_default_ctx(&ctx);

  {
    int a = 3;
    limitless_number x = 33424234;
    limitless_number y = (x + a) / 2.3f;

    assert(limitless_cpp_get_default_ctx() == &ctx);
    assert(limitless_cpp_last_status() == LIMITLESS_OK);
    assert(y.str() == "140191410946048/9646899");
    assert(limitless_cpp_last_status() == LIMITLESS_OK);

    limitless_number p = limitless_number::parse("7/3");
    assert(limitless_cpp_last_status() == LIMITLESS_OK);
    assert(p > 2);
    assert(!(3 < p));
    assert(p != 2);
    assert(p == limitless_number::parse("7/3"));
    assert(limitless_cpp_last_status() == LIMITLESS_OK);
  }

  {
    limitless_number rhs = 4;
    limitless_number add = 6 + rhs;
    limitless_number sub = 6 - rhs;
    limitless_number mul = 6 * rhs;
    limitless_number div = 6 / rhs;
    assert(add.str() == "10");
    assert(sub.str() == "2");
    assert(mul.str() == "24");
    assert(div.str() == "3/2");
    assert(!(6 == rhs));
    assert(6 != rhs);
    assert(3 < rhs);
    assert(4 <= rhs);
    assert(5 > rhs);
    assert(4 >= rhs);
    assert(limitless_cpp_last_status() == LIMITLESS_OK);
  }

  {
    fail_alloc_state st;
    limitless_alloc alloc;
    limitless_ctx fail_ctx;
    limitless_size need = 0;
    int probe_calls;
    bool saw_second_call_failure = false;
    int k;

    st.fail_after = 1000000;
    st.calls = 0;
    alloc.alloc = fail_alloc;
    alloc.realloc = fail_realloc;
    alloc.free = fail_free;
    alloc.user = &st;
    assert(limitless_ctx_init(&fail_ctx, &alloc) == LIMITLESS_OK);
    limitless_cpp_set_default_ctx(&fail_ctx);

    limitless_number v = -1234567;
    assert(limitless_cpp_last_status() == LIMITLESS_OK);

    assert(v.str(1).empty());
    assert(limitless_cpp_last_status() == LIMITLESS_EINVAL);

    st.calls = 0;
    st.fail_after = 1000000;
    assert(limitless_number_to_cstr(&fail_ctx, v.raw(), 10, NULL, 0, &need) == LIMITLESS_EBUF);
    probe_calls = st.calls;

    for (k = probe_calls; k < probe_calls + 8; ++k) {
      st.calls = 0;
      st.fail_after = k;
      if (v.str().empty() && limitless_cpp_last_status() != LIMITLESS_OK) {
        saw_second_call_failure = true;
        break;
      }
    }
    assert(saw_second_call_failure);
  }

  limitless_cpp_set_default_ctx(&ctx);

  limitless_c_number x, y, z;
  assert(limitless_number_init(&ctx, &x) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &y) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &z) == LIMITLESS_OK);

  assert(limitless_number_from_cstr(&ctx, &x, "101010", 2) == LIMITLESS_OK);
  assert(limitless_number_from_i64(&ctx, &y, 10) == LIMITLESS_OK);
  assert(limitless_number_add(&ctx, &z, &x, &y) == LIMITLESS_OK);

  char buf[64];
  assert(limitless_number_to_cstr(&ctx, &z, 10, buf, sizeof(buf), NULL) == LIMITLESS_OK);
  assert(std::strcmp(buf, "52") == 0);

  limitless_number_clear(&ctx, &x);
  limitless_number_clear(&ctx, &y);
  limitless_number_clear(&ctx, &z);
  return 0;
}
