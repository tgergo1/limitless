// SPDX-License-Identifier: GPL-3.0-only
#include "limitless.hpp"

int main() {
  limitless::number a = limitless::number::parse("7/3", 10);
  limitless::number b = 2;
  limitless::number c = a + b;
  return c.str() == "13/3" ? 0 : 1;
}
