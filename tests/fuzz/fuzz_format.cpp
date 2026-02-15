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
  char buf[2048];
  int base;

  if (limitless_ctx_init_default(&ctx) != LIMITLESS_OK) return 0;
  if (limitless_number_init(&ctx, &n) != LIMITLESS_OK) return 0;

  s.assign(reinterpret_cast<const char*>(data), size);
  if (limitless_number_from_str(&ctx, &n, s.c_str()) == LIMITLESS_OK) {
    base = (size == 0) ? 10 : static_cast<int>((data[0] % 35) + 2);
    (void)limitless_number_to_cstr(&ctx, &n, base, buf, (limitless_size)sizeof(buf), NULL);
    (void)limitless_number_to_cstr(&ctx, &n, 10, NULL, 0, NULL);
  }

  limitless_number_clear(&ctx, &n);
  return 0;
}
