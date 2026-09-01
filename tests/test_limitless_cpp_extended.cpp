/* SPDX-License-Identifier: GPL-3.0-only */
#include <cassert>
#include <functional>
#include <limits>
#include <sstream>
#include <string>

#define LIMITLESS_CPP_LEGACY_API 0
#define LIMITLESS_IMPLEMENTATION
#include "../limitless.hpp"

int main() {
  limitless_ctx ctx;
  limitless::number value;
  limitless::number equivalent;
  std::ostringstream output;
  std::istringstream input("1.25");
  std::istringstream invalid("not-a-number");
  std::hash<limitless::number> hash;

  assert(limitless_ctx_init_default(&ctx) == LIMITLESS_OK);
  limitless::limitless_cpp_set_default_ctx(&ctx);

  value = limitless::number::parse("7/3");
  assert(limitless::limitless_cpp_last_status() == LIMITLESS_OK);
  output << value;
  assert(output.str() == "7/3");

  input >> value;
  assert(!input.fail());
  assert(value.str() == "5/4");
  invalid >> value;
  assert(invalid.fail());
  assert(value.str() == "5/4");

  value = limitless::number::parse("1/2");
  equivalent = limitless::number::parse("2/4");
  assert(hash(value) == hash(equivalent));

  static_assert(std::numeric_limits<limitless::number>::is_specialized, "limitless numbers have numeric limits metadata");
  static_assert(!std::numeric_limits<limitless::number>::is_bounded, "limitless numbers are unbounded");
  static_assert(std::numeric_limits<limitless::number>::is_exact, "limitless numbers are exact");
  assert(std::numeric_limits<limitless::number>::min().str() == "0");
  assert(std::numeric_limits<limitless::number>::max().str() == "0");
  return 0;
}
