/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LIMITLESS_IMPLEMENTATION
#include "../../limitless.h"

static void print_err(limitless_status st) {
  printf("ERR:%d\n", (int)st);
}

static int print_number(limitless_ctx* ctx, const limitless_number* n) {
  limitless_size need = 0;
  char* out;
  limitless_status st = limitless_number_to_str(ctx, n, NULL, 0, &need);
  if (st != LIMITLESS_EBUF && st != LIMITLESS_OK) {
    print_err(st);
    return 1;
  }
  out = (char*)malloc((size_t)need + 1u);
  if (!out) {
    print_err(LIMITLESS_EOOM);
    return 1;
  }
  st = limitless_number_to_str(ctx, n, out, need + 1u, NULL);
  if (st != LIMITLESS_OK) {
    free(out);
    print_err(st);
    return 1;
  }
  printf("OK:%s\n", out);
  free(out);
  return 0;
}

int main(int argc, char** argv) {
  const char* op;
  limitless_ctx ctx;
  limitless_number a;
  limitless_number b;
  limitless_number out;
  limitless_status st;

  if (argc < 3) {
    fprintf(stderr, "usage: %s <op> <a> [b]\n", argv[0]);
    return 2;
  }

  op = argv[1];
  st = limitless_ctx_init_default(&ctx);
  if (st != LIMITLESS_OK) {
    print_err(st);
    return 1;
  }
  if (limitless_number_init(&ctx, &a) != LIMITLESS_OK ||
      limitless_number_init(&ctx, &b) != LIMITLESS_OK ||
      limitless_number_init(&ctx, &out) != LIMITLESS_OK) {
    print_err(LIMITLESS_EOOM);
    return 1;
  }

  if (strcmp(op, "neg") == 0 || strcmp(op, "abs") == 0 || strcmp(op, "parse") == 0) {
    st = limitless_number_from_str(&ctx, &a, argv[2]);
    if (st != LIMITLESS_OK) {
      print_err(st);
      goto done;
    }

    if (strcmp(op, "neg") == 0) st = limitless_number_neg(&ctx, &out, &a);
    else if (strcmp(op, "abs") == 0) st = limitless_number_abs(&ctx, &out, &a);
    else st = limitless_number_copy(&ctx, &out, &a);

    if (st != LIMITLESS_OK) {
      print_err(st);
      goto done;
    }
    (void)print_number(&ctx, &out);
    goto done;
  }

  if (argc < 4) {
    fprintf(stderr, "binary op requires two operands\n");
    return 2;
  }

  st = limitless_number_from_str(&ctx, &a, argv[2]);
  if (st != LIMITLESS_OK) {
    print_err(st);
    goto done;
  }
  st = limitless_number_from_str(&ctx, &b, argv[3]);
  if (st != LIMITLESS_OK) {
    print_err(st);
    goto done;
  }

  if (strcmp(op, "add") == 0) st = limitless_number_add(&ctx, &out, &a, &b);
  else if (strcmp(op, "sub") == 0) st = limitless_number_sub(&ctx, &out, &a, &b);
  else if (strcmp(op, "mul") == 0) st = limitless_number_mul(&ctx, &out, &a, &b);
  else if (strcmp(op, "div") == 0) st = limitless_number_div(&ctx, &out, &a, &b);
  else if (strcmp(op, "gcd") == 0) st = limitless_number_gcd(&ctx, &out, &a, &b);
  else if (strcmp(op, "cmp") == 0) {
    limitless_status cst = LIMITLESS_OK;
    int cmp = limitless_number_cmp(&ctx, &a, &b, &cst);
    if (cst != LIMITLESS_OK) {
      print_err(cst);
      goto done;
    }
    printf("OK:%d\n", cmp);
    goto done;
  } else {
    fprintf(stderr, "unknown op: %s\n", op);
    return 2;
  }

  if (st != LIMITLESS_OK) {
    print_err(st);
    goto done;
  }
  (void)print_number(&ctx, &out);

done:
  limitless_number_clear(&ctx, &a);
  limitless_number_clear(&ctx, &b);
  limitless_number_clear(&ctx, &out);
  return 0;
}
