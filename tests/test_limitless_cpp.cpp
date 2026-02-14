/* SPDX-License-Identifier: GPL-3.0-only */
#include <cassert>
#include <cstring>

#define LIMITLESS_IMPLEMENTATION
#include "../limitless.hpp"

int main() {
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
