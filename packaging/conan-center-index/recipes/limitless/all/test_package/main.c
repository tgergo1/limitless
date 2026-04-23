/* SPDX-License-Identifier: GPL-3.0-only */
#include "limitless.h"

int main(void) {
  limitless_ctx ctx;
  return limitless_ctx_init_default(&ctx) == LIMITLESS_OK ? 0 : 1;
}
