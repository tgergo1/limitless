/* SPDX-License-Identifier: GPL-3.0-only */
#include <cassert>
#include "../limitless.hpp"

limitless_ctx* tests_cpp_get_default_ctx_b(void);
limitless_status tests_cpp_get_last_status_b(void);
void tests_cpp_set_error_status_b(void);
void tests_cpp_set_ok_status_b(void);

int main() {
  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);

  limitless_cpp_set_default_ctx(&ctx);
  assert(limitless_cpp_get_default_ctx() == &ctx);
  assert(tests_cpp_get_default_ctx_b() == &ctx);

  (void)limitless_number::parse("not_a_number", 10);
  assert(limitless_cpp_last_status() == LIMITLESS_EPARSE);
  assert(tests_cpp_get_last_status_b() == LIMITLESS_EPARSE);

  (void)limitless_number::parse("7/3", 10);
  assert(limitless_cpp_last_status() == LIMITLESS_OK);
  assert(tests_cpp_get_last_status_b() == LIMITLESS_OK);

  tests_cpp_set_error_status_b();
  assert(limitless_cpp_last_status() == LIMITLESS_EPARSE);
  assert(tests_cpp_get_last_status_b() == LIMITLESS_EPARSE);

  tests_cpp_set_ok_status_b();
  assert(limitless_cpp_last_status() == LIMITLESS_OK);
  assert(tests_cpp_get_last_status_b() == LIMITLESS_OK);

  assert(tests_cpp_get_default_ctx_b() == &ctx);
  return 0;
}
