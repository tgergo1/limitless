// SPDX-License-Identifier: GPL-3.0-only
#include "limitless.hpp"

int main() {
  limitless::number a = limitless::number::parse("7/3", 10);
  if (limitless::limitless_cpp_last_status() != LIMITLESS_OK) return 1;
  limitless::number b = 2;
  limitless::number c = a + b;
  if (limitless::limitless_cpp_last_status() != LIMITLESS_OK) return 1;
  auto s = c.str();
  return limitless::limitless_cpp_last_status() == LIMITLESS_OK && s == "13/3" ? 0 : 1;
}
