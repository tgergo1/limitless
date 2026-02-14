/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <string.h>
#include "../limitless.h"

limitless_status tests_make_ctx(limitless_ctx* ctx);
limitless_status tests_add_11(limitless_ctx* ctx, limitless_number* out);

int main(void) {
  limitless_ctx ctx;
  limitless_number out;
  char buf[32];

  assert(tests_make_ctx(&ctx) == LIMITLESS_OK);
  assert(limitless_number_init(&ctx, &out) == LIMITLESS_OK);
  assert(tests_add_11(&ctx, &out) == LIMITLESS_OK);
  assert(limitless_number_to_cstr(&ctx, &out, 10, buf, sizeof(buf), NULL) == LIMITLESS_OK);
  assert(strcmp(buf, "11") == 0);

  limitless_number_clear(&ctx, &out);
  return 0;
}
