/* SPDX-License-Identifier: GPL-3.0-only */
#define LIMITLESS_CPP_NO_LEGACY_DEPRECATION 1
#include "../limitless.hpp"

limitless_ctx* tests_cpp_get_default_ctx_b(void) {
  return limitless_cpp_get_default_ctx();
}

limitless_status tests_cpp_get_last_status_b(void) {
  return limitless_cpp_last_status();
}

void tests_cpp_set_error_status_b(void) {
  (void)limitless_number::parse("not_a_number", 10);
}

void tests_cpp_set_ok_status_b(void) {
  (void)limitless_number::parse("7/3", 10);
}
