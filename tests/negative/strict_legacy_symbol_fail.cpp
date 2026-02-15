/* SPDX-License-Identifier: GPL-3.0-only */
#define LIMITLESS_CPP_LEGACY_API 0
#include "../../limitless.hpp"

int main() {
  limitless_number x = 1;
  limitless_cpp_set_default_ctx(0);
  return (int)(x.str().size());
}
