/* SPDX-License-Identifier: GPL-3.0-only */
#include <cstddef>
#include <cstdint>
#include <string>

#define LIMITLESS_IMPLEMENTATION
#include "../../limitless.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  limitless_ctx ctx;
  limitless_number a;
  limitless_number b;
  limitless_number out;
  std::string sa;
  std::string sb;
  size_t mid = size / 2;

  if (limitless_ctx_init_default(&ctx) != LIMITLESS_OK) return 0;
  if (limitless_number_init(&ctx, &a) != LIMITLESS_OK) return 0;
  if (limitless_number_init(&ctx, &b) != LIMITLESS_OK) return 0;
  if (limitless_number_init(&ctx, &out) != LIMITLESS_OK) return 0;

  sa.assign(reinterpret_cast<const char*>(data), mid);
  sb.assign(reinterpret_cast<const char*>(data + mid), size - mid);

  if (limitless_number_from_str(&ctx, &a, sa.c_str()) == LIMITLESS_OK &&
      limitless_number_from_str(&ctx, &b, sb.c_str()) == LIMITLESS_OK) {
    (void)limitless_number_add(&ctx, &out, &a, &b);
    (void)limitless_number_sub(&ctx, &out, &a, &b);
    (void)limitless_number_mul(&ctx, &out, &a, &b);
    (void)limitless_number_div(&ctx, &out, &a, &b);
    (void)limitless_number_cmp(&ctx, &a, &b, NULL);
  }

  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
  return 0;
}
