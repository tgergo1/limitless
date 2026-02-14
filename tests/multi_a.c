/* SPDX-License-Identifier: GPL-3.0-only */
#include "../limitless.h"

limitless_status tests_make_ctx(limitless_ctx* ctx);

limitless_status tests_add_11(limitless_ctx* ctx, limitless_number* out) {
  limitless_number a, b;
  limitless_status st;
  st = limitless_number_init(ctx, &a); if (st != LIMITLESS_OK) return st;
  st = limitless_number_init(ctx, &b); if (st != LIMITLESS_OK) { limitless_number_clear(ctx, &a); return st; }

  st = limitless_number_from_i64(ctx, &a, 5);
  if (st == LIMITLESS_OK) st = limitless_number_from_i64(ctx, &b, 6);
  if (st == LIMITLESS_OK) st = limitless_number_add(ctx, out, &a, &b);

  limitless_number_clear(ctx, &a);
  limitless_number_clear(ctx, &b);
  return st;
}
