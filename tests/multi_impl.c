/* SPDX-License-Identifier: GPL-3.0-only */
#define LIMITLESS_IMPLEMENTATION
#include "../limitless.h"

limitless_status tests_make_ctx(limitless_ctx* ctx) {
  return limitless_ctx_init_default(ctx);
}
