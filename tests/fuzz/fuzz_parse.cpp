/* SPDX-License-Identifier: GPL-3.0-only */
#include <cstddef>
#include <cstdint>
#include <string>

#define LIMITLESS_IMPLEMENTATION
#include "../../limitless.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  limitless_ctx ctx;
  limitless_number n;
  std::string s;
  int base;

  if (limitless_ctx_init_default(&ctx) != LIMITLESS_OK) return 0;
  if (limitless_number_init(&ctx, &n) != LIMITLESS_OK) return 0;

  s.assign(reinterpret_cast<const char*>(data), size);
  base = (size == 0) ? 0 : static_cast<int>(data[0] % 37);
  if (base == 1) base = 10;

  (void)limitless_number_from_cstr(&ctx, &n, s.c_str(), base);

  limitless_number_clear(&ctx, &n);
  return 0;
}
