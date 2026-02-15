/* SPDX-License-Identifier: GPL-3.0-only */
#include <cassert>

#define LIMITLESS_CPP_LEGACY_API 0
#define LIMITLESS_IMPLEMENTATION
#include "../limitless.hpp"

int main() {
  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);

  limitless::limitless_cpp_set_default_ctx(&ctx);
  assert(limitless::limitless_cpp_get_default_ctx() == &ctx);

  limitless::number a = limitless::number::parse("7/3", 10);
  assert(limitless::limitless_cpp_last_status() == LIMITLESS_OK);
  limitless::number b = 2;
  limitless::number c = a + b;
  assert(limitless::limitless_cpp_last_status() == LIMITLESS_OK);
  assert(c.str() == "13/3");

  limitless::number d = 4;
  limitless::number e = 1;
  d /= e;
  assert(limitless::limitless_cpp_last_status() == LIMITLESS_OK);
  assert(d.str() == "4");

  return 0;
}
