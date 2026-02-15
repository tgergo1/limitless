/* SPDX-License-Identifier: GPL-3.0-only */
#include <cassert>

#define LIMITLESS_IMPLEMENTATION
#include "../limitless.hpp"

int main() {
  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);

  limitless_cpp_set_default_ctx(&ctx);
  assert(limitless_cpp_get_default_ctx() == &ctx);

  limitless_number a = limitless_number::parse("7/3", 10);
  assert(limitless_cpp_last_status() == LIMITLESS_OK);
  limitless_number b = 2;
  limitless_number c = a + b;
  assert(limitless_cpp_last_status() == LIMITLESS_OK);
  assert(c.str() == "13/3");
  limitless_cpp_set_default_ctx(NULL);
  return 0;
}
