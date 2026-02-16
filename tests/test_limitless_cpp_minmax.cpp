/* SPDX-License-Identifier: GPL-3.0-only */
#include <cassert>
#include <iostream>
#include <string>

#define LIMITLESS_IMPLEMENTATION
#include "../limitless.hpp"

int main() {
  limitless_ctx ctx;
  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  limitless::limitless_cpp_set_default_ctx(&ctx);

  /* --- min / max free functions --- */
  {
    limitless::number a = 3;
    limitless::number b = 7;
    limitless::number lo = limitless::min(a, b);
    limitless::number hi = limitless::max(a, b);
    assert(lo.str() == "3");
    assert(hi.str() == "7");
  }

  /* min / max with negative numbers */
  {
    limitless::number a = -10;
    limitless::number b = 5;
    assert(limitless::min(a, b).str() == "-10");
    assert(limitless::max(a, b).str() == "5");
  }

  /* min / max with equal values */
  {
    limitless::number a = 42;
    limitless::number b = 42;
    assert(limitless::min(a, b).str() == "42");
    assert(limitless::max(a, b).str() == "42");
  }

  /* min / max with rationals */
  {
    limitless::number a = limitless::number::parse("1/3", 10);
    limitless::number b = limitless::number::parse("1/2", 10);
    assert(limitless::min(a, b).str() == "1/3");
    assert(limitless::max(a, b).str() == "1/2");
  }

  /* --- abs method --- */
  {
    limitless::number pos = 42;
    limitless::number neg = -42;
    limitless::number z = 0;
    assert(pos.abs().str() == "42");
    assert(neg.abs().str() == "42");
    assert(z.abs().str() == "0");
  }

  /* abs with rational */
  {
    limitless::number r = limitless::number::parse("-3/7", 10);
    assert(r.abs().str() == "3/7");
  }

  /* --- is_zero / is_integer / is_negative / is_positive / sign --- */
  {
    limitless::number pos = 5;
    limitless::number neg = -5;
    limitless::number z = 0;
    limitless::number rat = limitless::number::parse("3/7", 10);

    assert(!pos.is_zero());
    assert(!neg.is_zero());
    assert(z.is_zero());

    assert(pos.is_integer());
    assert(neg.is_integer());
    assert(z.is_integer());
    assert(!rat.is_integer());

    assert(pos.is_positive());
    assert(!pos.is_negative());

    assert(!neg.is_positive());
    assert(neg.is_negative());

    assert(!z.is_positive());
    assert(!z.is_negative());

    assert(pos.sign() > 0);
    assert(neg.sign() < 0);
    assert(z.sign() == 0);
  }

  /* is_negative / is_positive with rationals */
  {
    limitless::number r1 = limitless::number::parse("3/7", 10);
    limitless::number r2 = limitless::number::parse("-3/7", 10);
    assert(r1.is_positive());
    assert(!r1.is_negative());
    assert(!r2.is_positive());
    assert(r2.is_negative());
  }

  std::cout << "ok" << std::endl;
  return 0;
}
